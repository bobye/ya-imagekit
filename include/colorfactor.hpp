#include <opencv/cv.h>

namespace ya_imagekit {
  class Image {
    IplImage *rgb;
    CvMat *lab;
  public:
    int read(char * filename);
  
    ~Image() {
      cvReleaseImage(&rgb); cvReleaseMat(&lab);
    };
  };
}
