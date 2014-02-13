#include "colorfactor.hpp"
#include "stdio.h"

#include "opencv/highgui.h"

namespace ya_imagekit {
  int Image::read(char * filename) {
    rgb = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);    
    if (rgb == NULL) {printf("Image NOT loaded!\n"); return -1;}

    if (std::string(rgb->colorModel).compare("RGB") != 0) 
      {printf("Image NOT in RGB!"); return -1;};


    // (to implement) check if it is a color image 
    // ...

    // display THE image 
    cvNamedWindow("original image", CV_WINDOW_AUTOSIZE);
    cvShowImage("image", rgb);
    printf("Press a button to continue ..."); fflush(stdout); cvWaitKey(0);
    printf("[done]\n");
    cvDestroyWindow("original image");

    // convert to RGB to Lab space
    lab = cvCreateMat(rgb->height, rgb->width, CV_8UC3);
    cvCvtColor(rgb, lab, CV_RGB2Lab);

    return 0;
  }

}
