#include <opencv/cv.h>
#include <vector>

namespace ya_imagekit {
  struct UnarySeg {
    int label;
    float avgLab[3], avgRGB[3];
    int area, boundary;    
  };
  
  struct BinarySeg {
    int labels[2];
    int neighborCount; // the number of neighboring pixels between two segments
  };


  class Image {
    IplImage *rgb;
    cv::Mat lab, segments_map;

    std::vector<UnarySeg> usegs;
    std::vector<BinarySeg> bsegs;

  public:
    int read(const char * filename);
    int createSegmentsByKmeans(int k); // k: the number of segments
    int prepareSegments();
    int displaySegments();

    ~Image() {
      cvReleaseImage(&rgb); 
    };
  };
}

