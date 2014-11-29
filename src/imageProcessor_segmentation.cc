#include "imageProcessor.hh"
#include <cstdio>
#include <cstdlib>
#include <opencv/highgui.h>
#include <stdlib.h>

#include "util.hh"
#include "3rdparty.h"

namespace ya_imagekit {

  extern int new_window_position_x;
  extern int new_window_position_y;

  int Image::createSegmentsByKmeans(int k, bool quiet) {
    using namespace cv;
    // allocate segmentation map
    const float luminance_reduction_ratio = .2; // [0,1]
    const float spatial_correlation = 30.; //[0,100]
    
    // convert to data array
    Mat pixels, vectorByPixels, pixels_array, segments_array;
    int numOfPixels = lab.rows * lab.cols;

    lab.convertTo(pixels, CV_32FC1);
    segments_array = Mat(numOfPixels, 1, CV_32SC1);
    vectorByPixels = Mat(numOfPixels, 5, CV_32FC1);

    pixels_array = vectorByPixels.colRange(1,4);
    pixels.reshape(1, numOfPixels).copyTo(pixels_array);
    pixels_array.col(0) = luminance_reduction_ratio * pixels_array.col(0);//scale luminance 

    float smallerBound = std::min(lab.rows,lab.cols) / std::log(k);
    for (int i=0;i<numOfPixels;++i) {
      vectorByPixels.at<float>(i,3) = spatial_correlation * 
	(float) (i/lab.cols)/ (float) (smallerBound);
      vectorByPixels.at<float>(i,4) = spatial_correlation * 
	(float) (i%lab.cols)/ (float) (smallerBound);
    }    

    /** OpenCV implementation of Kmeans **/
    kmeans(vectorByPixels, k, segments_array,
	   cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 50, 1.0),
	   3, KMEANS_PP_CENTERS);

    segments_map = segments_array.reshape(0, lab.rows);// using int    

    if (!quiet) printf("Lab kmeans segmentation finished!\n");
    return k;
  }


  int Image::createSegmentsByFelzenszwalbP04(int k, bool quiet) {
    using namespace cv;

    segments_map = Mat(rgb->height, rgb->width, CV_32SC1);
    segmentationByFelzenszwalbP04(rgb, segments_map, k);
    return k;
  }

  int Image::createSegmentsByComanicuM02MeanShift(int k, bool quiet) {
    using namespace cv;

    segments_map = Mat(rgb->height, rgb->width, CV_32SC1);
    segmentationByComanicuM02MeanShift(rgb, segments_map, k);
    return k;
  }
  


  /*********************************************************************/
  // Post Processing

  const int window_size = 5;
  const int threshold = 5;

  inline void buildConnectHistogram(int row, int col, const cv::Mat &maps, 
				    std::vector<int> &histogram) {
    int k = std::sqrt(histogram.size());
    std::vector<int> v(k, 0);

    int halfwindow_size = (window_size -1)/2;
    
    for (int i=row-halfwindow_size; i<=row + halfwindow_size; ++i) 
      for (int j=col -halfwindow_size; j <=col + halfwindow_size; ++j) 
	if (i<0 || j<0 || i >= maps.rows || j >= maps.cols) {
	} else {
	  v[maps.at<int>(i,j)] ++;
	}

    int max1=0, max2=1;
    for (int i=2; i<k; ++i) // find the max two segments in window
      if (v[i]>v[max1]) max1=i;
      else if (v[i]>v[max2]) max2=i;
    
    if (v[max1]>=2 && v[max2]>=2) {
      histogram[max1 *k +max2] ++;
      histogram[max1 + max2*k] ++;
    }

    return ;
  }

  inline bool checkBoundary(int row, int col, const cv::Mat &maps) {

    int halfwindow_size = (window_size -1)/2;
    int count = 0;
    const int label = maps.at<int>(row, col);
    for (int i=row-halfwindow_size; i<=row + halfwindow_size; ++i) 
      for (int j=col -halfwindow_size; j <=col + halfwindow_size; ++j) 
	if (i<0 || j<0 || i >= maps.rows || j >= maps.cols) {
	  ++count;
	} else if (maps.at<int>(i,j) != label) {
	  ++count;
	}

    return count >= threshold;
  }

  void Image::restoreSpatialAttributes(const std::set<int> &setOfIndex, BinarySeg & bseg) {
    using namespace cv;
    int rows = segments_map.rows, cols = segments_map.cols;
    int numOfPixels = rows * cols;

    Mat markedPixels = Mat(rows, cols, CV_32SC1, Scalar(0));
    


    for (std::set<int>::iterator i=setOfIndex.begin(); i!=setOfIndex.end(); ++i) {
      for (int j=0; j<numOfPixels; ++j) {
	if (segments_map.at<int>(j) ==  *i) {
	  markedPixels.at<int>(j) = 1;
	}
      }
    }    

    std::vector<UnarySeg>  tworegions;
    restoreSpatialAttributes(markedPixels, tworegions);


    bseg.size = tworegions[1].size; 
    bseg.mean[0] = tworegions[1].mean[0];
    bseg.mean[1] = tworegions[1].mean[1];
    bseg.dev[0]= tworegions[1].dev[0];
    bseg.dev[1]= tworegions[1].dev[1];
    bseg.dev[2]= tworegions[1].dev[2];
    bseg.dev[3]= tworegions[1].dev[3];

    bseg.compactness = tworegions[1].compactness;
    bseg.elongation = tworegions[1].elongation;
    bseg.centrality = tworegions[1].centrality;

    //bseg.print(stdout);

  }

  void Image::restoreSpatialAttributes(const cv::Mat &segments_map, std::vector<UnarySeg> & usegs) {
    using namespace cv;
    double minVal, maxVal, numOfSegments;
    minMaxLoc(segments_map, &minVal, &maxVal);
    numOfSegments = maxVal +1;

    // initialize segments
    for (int i=0; i < numOfSegments; ++i) {
      
      //      UnarySeg u;
      usegs.push_back(UnarySeg()); 
      usegs.back().label = i;
      usegs.back().bbx_x1 = 100000; 
      usegs.back().bbx_y1 = 100000;
      usegs.back().bbx_x2 = 0;
      usegs.back().bbx_y2 = 0;
    }


    
    // compute average Lab, mean
    int rows = segments_map.rows, cols = segments_map.cols;
    int numOfPixels = rows * cols;

    boundary_mask = Mat(rows, cols, CV_8U, Scalar(0));


    std::vector<std::vector<int> > setOfRows, setOfCols;
    setOfRows.resize(numOfSegments); setOfCols.resize(numOfSegments);


    for (int i=0; i<numOfPixels; ++i) {
      int idx = segments_map.at<int> (i);
      Vec3b &pixel = lab.at<Vec3b>(i);

      int x = i % cols, y = i / cols; //opencv mat is row major
      if (x < usegs[idx].bbx_x1)   usegs[idx].bbx_x1 = x;
      if (x > usegs[idx].bbx_x2-1) usegs[idx].bbx_x2 = x + 1;
      if (y < usegs[idx].bbx_y1)   usegs[idx].bbx_y1 = y;
      if (y > usegs[idx].bbx_y2-1) usegs[idx].bbx_y2 = y + 1;

      usegs[idx].avgLab[0] += pixel[0];
      usegs[idx].avgLab[1] += pixel[1];
      usegs[idx].avgLab[2] += pixel[2];

      float a = pixel[1] - 128, b = pixel[2] -128, L = pixel[0]*100./255.;
      if (L != 0 ) 
	usegs[idx].saturation += std::sqrt(a*a + b*b)/std::sqrt(a*a + b*b + L*L);
      else usegs[idx].saturation += 1;

      usegs[idx].center.y += i/cols; setOfRows[idx].push_back(i/cols);
      usegs[idx].center.x += i%cols; setOfCols[idx].push_back(i%cols);

      usegs[idx].area ++;
      if (checkBoundary(i/cols, i%cols, segments_map)) {
	usegs[idx].boundary ++; 
	boundary_mask.at<uchar>(i) = 1;
      }
    }

    for (int i=0; i<usegs.size(); ++i) {

      usegs[i].avgLab[0] /= usegs[i].area;
      usegs[i].avgLab[1] /= usegs[i].area;
      usegs[i].avgLab[2] /= usegs[i].area;

      usegs[i].saturation /=usegs[i].area;

      usegs[i].compactness = 1. - (float) usegs[i].boundary / (float) usegs[i].area;
      usegs[i].center.y /= usegs[i].area;
      usegs[i].center.x /= usegs[i].area;

      int min, max, box_height, box_width;
      softBoundingBox(&setOfRows[i][0], setOfRows[i].size(), min, max); box_height = max-min;
      softBoundingBox(&setOfCols[i][0], setOfRows[i].size(), min, max); box_width = max-min;
      
      usegs[i].elongation =  1- (float) (std::min(box_height, box_width))/ (float) (std::max(box_height, box_width));
    }

    for (int i=0; i<numOfPixels; ++i) {
      int idx = segments_map.at<int> (i);
      usegs[idx].dev[0] += (i/cols - usegs[idx].center.y) * (i/cols - usegs[idx].center.y);
      usegs[idx].dev[2] = usegs[idx].dev[1] += (i/cols - usegs[idx].center.y) * (i%cols - usegs[idx].center.x);
      usegs[idx].dev[3] += (i%cols - usegs[idx].center.x) * (i%cols - usegs[idx].center.x);
    }


    int largerBound = std::max(rows, cols);


    for (int i=0; i<usegs.size(); ++i) {
      usegs[i].size = (float) usegs[i].area / (float) numOfPixels;

      usegs[i].mean[0] =.5 + (float) (usegs[i].center.y - rows/2) / (float) largerBound;
      usegs[i].mean[1] =.5 + (float) (usegs[i].center.x - cols/2) / (float) largerBound;

      float normalize = usegs[i].area * usegs[i].area;
      usegs[i].dev[0] = std::tanh(usegs[i].dev[0] / normalize);
      usegs[i].dev[1] = usegs[i].dev[2] = std::tanh(usegs[i].dev[2] / normalize); 
      usegs[i].dev[3] = std::tanh (usegs[i].dev[3] / normalize);



      usegs[i].centrality = std::sqrt((usegs[i].mean[0] - .5)*(usegs[i].mean[0] - .5) + (usegs[i].mean[1] - .5)*(usegs[i].mean[1] - .5))/std::sqrt(.5);
    }
  }

  int Image::prepareSegments() {
    using namespace cv;
    if (segments_map.empty()) {printf("Empty Segmentation Map"); return -1;}


    restoreSpatialAttributes(segments_map, usegs);


    int rows = segments_map.rows, cols = segments_map.cols;
    int numOfSegments = usegs.size(), numOfPixels = rows * cols;

    std::vector<int> hist(numOfSegments * numOfSegments);

    for (int i=0; i<numOfPixels; ++i) {
      buildConnectHistogram(i/cols, i%cols, segments_map, hist);
    }

    for (int i=0; i<numOfSegments; ++i) {
      for (int j=i+1; j<numOfSegments; ++j) {
	//printf("%5d", hist[i*numOfSegments + j]);	
	const float threshold = .1;
	if (hist[i*numOfSegments + j] > 
	    threshold * std::sqrt(usegs[i].boundary * usegs[j].boundary)) 
	  {
	    BinarySeg b;
	    b.labels[0] = i; b.labels[1] = j; // set labels/indices
	    b.neighborCount = hist[i*numOfSegments + j];	  	  
	    bsegs.push_back(b); 
	    bsegs.back().bbx_x1 = std::min(usegs[i].bbx_x1, usegs[j].bbx_x1);
	    bsegs.back().bbx_x2 = std::max(usegs[i].bbx_x2, usegs[j].bbx_x2);
	    bsegs.back().bbx_y1 = std::min(usegs[i].bbx_y1, usegs[j].bbx_y1);
	    bsegs.back().bbx_y2 = std::max(usegs[i].bbx_y2, usegs[j].bbx_y2);
	    
	  }
      }
      //std::cout << std::endl;
    }    

    // compute context region for two segments
    /*
    std::vector<std::set<int> > setOfNeighbors (numOfSegments);
    for (int i=0; i<bsegs.size(); ++i) {
      setOfNeighbors[bsegs[i].labels[0]].insert(bsegs[i].labels[1]);
      setOfNeighbors[bsegs[i].labels[1]].insert(bsegs[i].labels[0]);
    }

    for (int i=0; i<bsegs.size(); ++i) {
      std::set<int> set;
      set.insert(setOfNeighbors[bsegs[i].labels[0]].begin(), 
		 setOfNeighbors[bsegs[i].labels[0]].end());
      set.insert(setOfNeighbors[bsegs[i].labels[1]].begin(), 
		 setOfNeighbors[bsegs[i].labels[1]].end());
      restoreSpatialAttributes(set, bsegs[i]);
    }
    */
    return 0;
  }

  Mat Image::displaySegments(bool *isShown, bool isDrawGraph) {
    using namespace cv;
    assert (usegs.size() > 0 );
    int rows = lab.rows;
    int cols = lab.cols;


    Mat lab_array = lab.clone();
    Mat rgb_array = Mat(rgb, true);

    for (int i=0; i<cols*rows; ++i) {
      int idx = segments_map.at<int> (i);
      Vec3b &pixel = lab_array.at<Vec3b>(i);
      if ((isShown == NULL || isShown[idx])) {
	pixel[0] = round (usegs[idx].avgLab[0]);
	pixel[1] = round (usegs[idx].avgLab[1]);
	pixel[2] = round (usegs[idx].avgLab[2]);            
      }
      else {
	pixel[0] = 255; pixel[1]=pixel[2]=127;
      }
    }

    cvtColor(lab_array, rgb_array, CV_Lab2BGR);

    // draw graph
    if (isDrawGraph) {
    for (int i=0; i<bsegs.size(); ++i) 
    if (false){
      // draw lines
      line(rgb_array,
	   usegs[bsegs[i].labels[0]].center,
	   usegs[bsegs[i].labels[1]].center,
	   Scalar(255,255,255));	   
    }

    Mat colorbar = Mat(1, usegs.size(), CV_8UC3);
    Mat rgb_colorbar = colorbar.clone();
    for (int i=0; i<usegs.size(); ++i) {
      Vec3b &color = colorbar.at<Vec3b>(i);
      color[0] = usegs[i].avgLab[0];
      color[1] = usegs[i].avgLab[1];
      color[2] = usegs[i].avgLab[2];
    }
    cvtColor(colorbar, rgb_colorbar, CV_Lab2BGR);

    for (int i=0; i<usegs.size(); ++i) 
      if (false){
      circle(rgb_array, 
	     usegs[i].center,
	     std::log(1+sqrt(usegs[i].size)*100) + std::min(rgb_array.rows, rgb_array.cols)/48,
	     Scalar(rgb_colorbar.at<Vec3b>(i)[0],
		    rgb_colorbar.at<Vec3b>(i)[1],
		    rgb_colorbar.at<Vec3b>(i)[2]),
	     -1, 16
	     );

      //      std::cout << rgb_colorbar.at<Vec3b>(i) << std::endl;

      circle(rgb_array, 
	     usegs[i].center,
	     std::log(1+sqrt(usegs[i].size)*100) + std::min(rgb_array.rows, rgb_array.cols)/48,
	     Scalar(255,255,255),
	     1, 16
	     );

      }else {
	stringstream ss; ss<< i;
	string text = ss.str();
	int fontFace = FONT_HERSHEY_COMPLEX_SMALL;
	double fontScale = 0.5;
	int thickness = 1;
	int baseline=0;

	Size textSize = getTextSize(text, fontFace,
				    fontScale, thickness, &baseline);

	putText(rgb_array, text, 
		Point(usegs[i].center.x - textSize.width/2, 
		      usegs[i].center.y + textSize.height/2),
		fontFace, fontScale, Scalar::all(255), thickness, CV_AA);
      }
    }

    /*
    imshow("segmentation map", rgb_array);
    moveWindow("segmentation map", new_window_position_x, new_window_position_y);
    new_window_position_x += rgb_array.cols;

    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");
    */
    return rgb_array;
  }

}
