#include "colorfactor.hpp"
#include "stdio.h"

#include "opencv/highgui.h"

namespace ya_imagekit {
  int Image::read(char * filename) {
    rgb = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);    
    if (rgb == NULL) {printf("Image NOT loaded!\n"); return -1;}
  
    // display THE image 
    cvNamedWindow("original image", CV_WINDOW_AUTOSIZE);
    cvShowImage("image", rgb);
    printf("Press a button to continue ..."); fflush(stdout); cvWaitKey(0);
    printf("[done]\n");
    cvDestroyWindow("original image");
    return 0;
  }

}
