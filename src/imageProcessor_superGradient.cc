#include "imageProcessor.hh"
#include <cstdio>
#include <cstdlib>
#include <opencv/highgui.h>
#include "util.hh"
#include "d2_solver.h"
#include "blas_like.h"
#include <Accelerate/Accelerate.h>
#include <iostream>
#include <fstream>
#include <algorithm>    // std::min_element, std::max_element

namespace ya_imagekit {


  const int sizeT = 19;
  const double h = 3.0;

  void superGradient(int d, int n, int m, 
		     std::vector<double> &feat, 
		     std::vector<double> &patch,
		     std::vector<double> &tcurve) {
    assert((n+m)*3 == feat.size());
    double *feat0 = &feat[0];
    double *feat1 = &feat[3*n];

    std::vector<double> xmatch(n*m + n + m);
    double fval = d2_match_by_sample(d, n, feat0, m, feat1, &xmatch[0], &xmatch[n*m]);
    printf("%e %d %d\n", fval, n, m); 

    
    // count how many nonzero edges
    int count = 0;
    for (int i = 0; i < n*m; ++i) if (xmatch[i] > 1E-10) { count ++; }

    // create interpolated weighted samples
    std::vector<double> interpol(count*d);
    std::vector<double> interpol_w(count);
    
    for (int t = 0; t <= sizeT; ++t) {

      double realt = (double) t / (double) sizeT;

      int idx = 0;
      for (int i = 0; i < n; ++i)
	for (int j = 0; j < m; ++j)
	  if (xmatch[i+j*n] > 1E-10) { // nonzero
	    for (int k = 0; k < d; ++k) 
	      interpol[idx*d+k] = (1-realt)*feat0[i*d+k] + realt*feat1[j*d+k];
	    interpol_w[idx] = xmatch[i+j*n];
	    idx ++;	    
	  }      

      // compute KDE
      int size = patch.size() / d;
      std::vector<double> D(count * size);
      _dpdist2(3, count, size, &interpol[0], &patch[0], &D[0]);
      cblas_dscal(count * size, -1.0/(2*h*h), &D[0], 1);
      _dexp(count * size, &D[0]);
      _dgcms(count, size, &D[0], &interpol_w[0]);
      _dcsum(count, size, &D[0], &tcurve[t*size]);
	
    }

    /* ad-hoc display solved tcurve */
    FILE *fp = fopen("tcurve.txt", "w");
    for (int i = 0; i < n+m; ++i) {
      for (int t = 0; t <= sizeT; ++t) {
	fprintf(fp, "%f ", tcurve[t*(n+m) + i]);
      }
      fprintf(fp, "\n");      
    }
    fclose(fp);
    return;
  }

  int Image::exportSubSegmentMaps(string path) {
    assert(bsegs.size() > 0 && usegs.size() > 0);

    for (int k = 0; k<bsegs.size(); ++k) {
      int i0 = bsegs[k].labels[0]; 
      int i1 = bsegs[k].labels[1];
      //printf("%d %d\n", i0, i1);
      Mat subseg_map = segments_map(Range(bsegs[k].bbx_y1, bsegs[k].bbx_y2), Range(bsegs[k].bbx_x1, bsegs[k].bbx_x2));

      /* write out subregion of segmentation map */
      char filename[255]; sprintf(filename, "/bseg_map-%d.dat", k); 
      string filepath(path); 
      ofstream fout(filepath.append(filename));
      /* output by row major */
      for (int i=0; i<subseg_map.rows; i++) {
	for (int j=0; j<subseg_map.cols; j++) 
	  if (subseg_map.at<int> (i,j) == i0)
	    fout << 1 << "\t";
	  else if (subseg_map.at<int> (i,j) == i1)
	    fout << -1<< "\t";
	  else
	    fout << 0 << "\t";
	fout << std::endl;
      }
      fout.close();
    }    
    return 0;
  }


  extern int new_window_position_x;
  extern int new_window_position_y;

  /** super gradient is computed all pairs of neighboring superpixels */
  int Image::computeSuperGradient(string path) {
    assert(bsegs.size() > 0 && usegs.size() > 0);
    int rows = lab.rows;
    int cols = lab.cols;
    int numOfPixels = lab.rows * lab.cols;

    /* visualization copy of original image and segmentation map */
    Mat rgb2 = Mat(rgb).clone();
    Mat seg_map = displaySegments(NULL, true);


    // retrieve the indices of pixels by segments
    std::vector< std::vector<int> > indices(usegs.size());
    for (int i=0; i<usegs.size(); ++i) {
      map2arr((int*) segments_map.data, numOfPixels, i, indices[i]);
    }

    
    d2_solver_setup(); // setup LP solvers
    for (int k = 0; k<bsegs.size(); ++k) {

      if (k != 320 && k != 148) continue; // select superpixel pairs

      const int i0 = bsegs[k].labels[0]; 
      const int i1 = bsegs[k].labels[1];

      // indices of pixels from the two superpixels.
      std::vector<int> bnd(indices[i0]); 
      bnd.insert( bnd.end(), indices[i1].begin(), indices[i1].end() );

      // output bounding box
      printf("%d %d\n", i0, i1);
      printf("%d %d %d %d\n", bsegs[k].bbx_x1, bsegs[k].bbx_x2, bsegs[k].bbx_y1, bsegs[k].bbx_y2);


      const int n = indices[i0].size();
      const int m = indices[i1].size();      

      /* load distane map */
      std::vector<double> distance((n+m));
      char filename[255]; sprintf(filename, "/bseg_map-%d-d.dat", k); 
      string filepath(path); 
      ifstream fin(filepath.append(filename)); 
      
      const int sub_base_x= bsegs[k].bbx_x1;
      const int sub_base_y= bsegs[k].bbx_y1;
      const int sub_width = bsegs[k].bbx_x2 - bsegs[k].bbx_x1;
      const int sub_height= bsegs[k].bbx_y2 - bsegs[k].bbx_y1;
      const int sub_num_pixels = sub_width * sub_height;
      
      std::vector<double> path_distance(sub_num_pixels);
      for (int i = 0; i < sub_num_pixels; ++i) {
	fin >> path_distance[i];
      }
      fin.close();
      std::cout << "load relative distances from " << filepath << std::endl;

      for (int i = 0; i < n+m; ++i) {
	int local_x = bnd[i] % cols - sub_base_x;
	int local_y = bnd[i] / cols - sub_base_y;
	distance[i] = path_distance[local_x + local_y * sub_width];
      }

      /* quantize feat[] */
      std::vector<double> feat(n * 3 + m * 3);      
      for (int j = 0; j < n+m; ++j) {
	Vec3b f = lab.at<Vec3b>(bnd[j]);
	feat[3*j]   = f[0]; feat[3*j+1] = f[1]; feat[3*j+2] = f[2];
      }

      /* quantize local patch */
      std::vector<double> patch( sub_width*sub_height*3 );
      for (int i=0; i<sub_height; ++i)
	for (int j=0; j<sub_width; ++j) {
	  Vec3b f = lab.at<Vec3b>(i + sub_base_y, j + sub_base_x);
	  int m = i*sub_width + j;
	  patch[3*m] = f[0]; patch[3*m+1] = f[1]; patch[3*m+2] = f[2];
	}

      /* compute tcurve for patch */
      std::vector<double> tcurve((sub_num_pixels)*(sizeT+1));
      superGradient(3, n, m, feat, patch, tcurve);

      /* prepare visualized tcurve patch x*/
      Mat T = Mat(sub_height, (sizeT+1+1)*sub_width, CV_8U, Scalar::all(255));

      double tmax = *std::max_element(tcurve.begin(), tcurve.end());
      for (int i=0; i<= sizeT; ++i) {
	for (int j=0; j< sub_num_pixels; ++j) {
	  int local_x = j % sub_width;
	  int local_y = j / sub_width;
	  T.at<uchar>(local_y, local_x + i*sub_width) = 
	    255 - 255 * (tcurve[j + i*sub_num_pixels] / tmax);
	}
      }

      /* append the relative distance in the last */
      double dmax = *std::max_element(path_distance.begin(), path_distance.end());
      double dmin = *std::min_element(path_distance.begin(), path_distance.end());
      for (int j=0; j< sub_num_pixels; ++j) {
	  int local_x = j % sub_width;
	  int local_y = j / sub_width;
	  T.at<uchar>(local_y, local_x + (sizeT+1)*sub_width) = 
	    255 - 255 * (path_distance[j] - dmin) / (dmax - dmin);	
      }


      /* graph relative distance and tcurve */
      dmax = *std::max_element(distance.begin(), distance.end()); // reset max distance
      dmin = *std::min_element(distance.begin(), distance.end()); // reset min distance

      const int intervals = 20;
      std::vector<double> signature((sizeT+1) * intervals, 0);
      std::vector<int> interval_counts(intervals, 0);
      for (int j=0; j< sub_num_pixels; ++j)  {
	int idx = (intervals * (path_distance[j] - dmin)) / (dmax - dmin + 0.01);
	interval_counts[idx] ++;
      }
      for (int i=0; i<=sizeT; ++i) {
	for (int j=0; j< sub_num_pixels; ++j) {
	  int idx = (intervals * (path_distance[j] - dmin)) / (dmax - dmin + 0.01);	  
	  double &val = tcurve[j + i*sub_num_pixels];
	  signature[idx + i*intervals] += tcurve[j + i*sub_num_pixels];
	}
	for (int j=0; j< intervals; ++j)
	  signature[j + i*intervals] /= (double) interval_counts[j];
      }
      char filename2[255]; sprintf(filename2, "/bseg_sig-%d-d.dat", k); 
      string filepath2(path); 
      ofstream fout(filepath2.append(filename2)); 
      for (int i=0; i<=sizeT; ++i) {
	for (int j=0; j<intervals; ++j)
	  fout << signature[j + i*intervals] << " ";
	fout << std::endl;
      }
      fout.close();
      std::cout << "save signature to " << filepath2 << std::endl;


      
      /* ad-hoc draw boundingbox*/
      rectangle(rgb2, 
		Point2i(bsegs[k].bbx_x1, bsegs[k].bbx_y1), 
		Point2i(bsegs[k].bbx_x2, bsegs[k].bbx_y2),
		Scalar(255, 255, 255) );
      rectangle(seg_map, 
		Point2i(bsegs[k].bbx_x1, bsegs[k].bbx_y1), 
		Point2i(bsegs[k].bbx_x2, bsegs[k].bbx_y2),
		Scalar(255, 255, 255) );




      /* draw visualized tcurve patch*/
      char tcurve_name[255];  sprintf(tcurve_name, "tcurve-%d", k);
      imshow(tcurve_name, T);
      moveWindow(tcurve_name, new_window_position_x, new_window_position_y);
      //      new_window_position_x += T.cols;
      
    }

    d2_solver_release();

    imshow("pair of segments", rgb2);
    moveWindow("pair of segments", new_window_position_x, new_window_position_y);
    new_window_position_x += rgb2.cols;
    imshow("pair of segments 2", seg_map);
    moveWindow("pair of segments 2", new_window_position_x, new_window_position_y);
    new_window_position_x += seg_map.cols;

    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");

    
    return 0;
  }

}
