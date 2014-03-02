#include "msImageProcessor.h"
#include "3rdparty.h"
#include <opencv/highgui.h>

void segmentationByComanicuM02MeanShift(const IplImage *cvimage, cv::Mat &segments_map, int &k) {
  int width = cvimage->width;
  int height = cvimage->height;

  cv::Mat cvimmat(cvimage, false);
  
  byte *rgb_array = new byte [width*height*3];
  for (int i=0; i<width*height; ++i) {
    cv::Vec3b &color = cvimmat.at<cv::Vec3b>(i);
    rgb_array[3*i]   = color[2];
    rgb_array[3*i+1] = color[1];
    rgb_array[3*i+2] = color[0];
  }

  msImageProcessor ip;
  ip.DefineImage(rgb_array, COLOR, height, width);
  ip.Segment((width+height)/30,15,width*height/1000, MED_SPEEDUP);

  int regionCount; int *labels; float *modes; int *modePointCounts;


  regionCount = ip.GetRegions(&labels, &modes, &modePointCounts);

  
  //  std::cout << regionCount << std::endl;


  typedef std::map<int, int> mapping_type;
  mapping_type mappings;
  mapping_type::iterator mapping_iter;
 
  int index = 0;
  for (int i=0; i<width*height; ++i) {
    mapping_iter = mappings.find(labels[i]);

    if (mapping_iter != mappings.end()){
      segments_map.at<int>(i) = mappings[labels[i]];
    }else {
      mappings[labels[i]] = index; 
      segments_map.at<int>(i) = index;
      index ++;
    }
  }
  k = index;
  //std::cout << index << std::endl;

  delete [] labels;
  delete [] modes;
  delete [] modePointCounts;

  /*
  ip.GetResults(rgb_array);

  for (int i=0; i<width*height; ++i) {
    cv::Vec3b &color = cvimmat.at<cv::Vec3b>(i);
    color[2] = rgb_array[3*i];
    color[1] = rgb_array[3*i+1];
    color[0] = rgb_array[3*i+2];
  }
    
  imshow("image", cvimmat);
  */
}
