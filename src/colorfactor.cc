#include "colorfactor.hpp"
#include <stdio.h>
#include <opencv/highgui.h>

#include "util.hpp"

#include "segment-image-opencv.h"
#include "msImageProcessor_interface.h"

namespace ya_imagekit {
  int Image::read(const char * filename, bool quiet) {
    using namespace cv;

    rgb = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);    
    if (rgb == NULL) {fprintf(stderr, "Image NOT loaded!\n"); return -1;}

    if (std::string(rgb->colorModel).compare("RGB") != 0) 
      {fprintf(stderr, "Image NOT in RGB!"); return -1;};


    // (to implement) check if it is a color image 
    // ...

    // display THE image 
    if (!quiet) {
      imshow("original image", Mat(rgb));
      printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
      printf("[done]\n");
    }

    // convert to RGB to Lab space
    
    lab = Mat(rgb->height, rgb->width, CV_8UC3);
    cvtColor(Mat(rgb), lab, CV_BGR2Lab);// data array actually is bgr,bgr,

    return 0;
  }

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

    int smallerBound = std::min(lab.rows,lab.cols);
    for (int i=0;i<numOfPixels;++i) {
      vectorByPixels.at<float>(i,3) = spatial_correlation * 
	(float) (i/lab.cols)/ (float) (smallerBound);
      vectorByPixels.at<float>(i,4) = spatial_correlation * 
	(float) (i%lab.cols)/ (float) (smallerBound);
    }    
    
    kmeans(vectorByPixels, k, segments_array,
	   cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 50, 1.0),
	   3, KMEANS_RANDOM_CENTERS);

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


  int Image::prepareSegments() {
    using namespace cv;
    if (segments_map.empty()) {printf("Empty Segmentation Map"); return -1;}

    double minVal, maxVal, numOfSegments;
    minMaxLoc(segments_map, &minVal, &maxVal);
    numOfSegments = maxVal +1;

    // initialize segments
    for (int i=0; i < numOfSegments; ++i) {
      UnarySeg s; 
      s.label=i;s.area=0;
      s.avgLab[0] = s.avgLab[1] = s.avgLab[2] =0;
      s.saturation = 0;
      s.center = cv::Point(0,0);//s.mean[0]= s.mean[1] =0;
      s.dev[0]=s.dev[1]=s.dev[2]=s.dev[3]=0;
      s.boundary = 0;
      usegs.push_back(s);
    }


    
    // compute average Lab, mean
    int rows = segments_map.rows, cols = segments_map.cols;
    int numOfPixels = rows * cols;

    std::vector<std::vector<int> > setOfRows, setOfCols;
    setOfRows.resize(numOfSegments); setOfCols.resize(numOfSegments);


    for (int i=0; i<numOfPixels; ++i) {
      int idx = segments_map.at<int> (i);
      Vec3b &pixel = lab.at<Vec3b>(i);

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
      if (checkBoundary(i/cols, i%cols, segments_map)) usegs[idx].boundary ++;
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

      usegs[i].dev[0] /= (float) largerBound * largerBound * (float) usegs[i].area;
      usegs[i].dev[1] = usegs[i].dev[2] /= (float) largerBound * largerBound * (float) usegs[i].area;
      usegs[i].dev[3] /= (float) largerBound * largerBound * (float) usegs[i].area;



      usegs[i].centrality = std::sqrt((usegs[i].mean[0] - .5)*(usegs[i].mean[0] - .5) + (usegs[i].mean[1] - .5)*(usegs[i].mean[1] - .5))/std::sqrt(.5);
    }


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
	    b.labels[0] = i; b.labels[1] = j;
	    b.neighborCount = hist[i*numOfSegments + j];	  
	  
	    bsegs.push_back(b);
	  }
      }
      //std::cout << std::endl;
    }    
    
    for (int i=0; i<bsegs.size(); ++i) {
      //std::cout << bsegs[i].labels[0] << " " << bsegs[i].labels[1] << " " << bsegs[i].neighborCount << std::endl;
    }

    return 0;
  }

  int Image::displaySegments(bool *isShown) {
    using namespace cv;
    if (usegs.size() == 0 ) {printf("No Segments!\n"); return -1;}

    Mat lab_array = lab.clone();
    Mat rgb_array = Mat(rgb, true);

    for (int i=0; i<segments_map.cols*segments_map.rows; ++i) {
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
    for (int i=0; i<bsegs.size(); ++i) {
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
      if (true){
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

    }


    imshow("segmentation map", rgb_array);
    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");

    return 0;
  }

}
