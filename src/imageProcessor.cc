#include "imageProcessor.hh"
#include <cstdio>
#include <cstdlib>
#include <opencv/highgui.h>
#include <GL/gl.h>



namespace ya_imagekit {

  int new_window_position_x=0;
  int new_window_position_y=100;
  string image_filename;

  Image::~Image() {      
    // take screenshot of results and save to disk

    setOpenGlContext("take screenshot");	
    
    Mat img(new_window_position_x, rgb->height, CV_8UC3);
    glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3)?1:4);
    glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
    glReadPixels(0, new_window_position_y, 
		 img.cols, img.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
    printf("ok\n");
    Mat flipped(img);
    flip(img, flipped, 0);
    imwrite((image_filename + "-snapshot.png").c_str(), img);

    // release memory
    destroyAllWindows();
    cvReleaseImage(&rgb); 
  }


  int Image::read(const char * filename, bool quiet) {
    using namespace cv;

    rgb = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);    
    if (rgb == NULL) {fprintf(stderr, "Image NOT loaded!\n"); return -1;}

    image_filename = string(filename);

    if (std::string(rgb->colorModel).compare("RGB") != 0) 
      {fprintf(stderr, "Image NOT in RGB!"); return -1;};


    // (to implement) check if it is a color image 
    // ...

    // display THE image 
    if (!quiet) {
      imshow("original image", Mat(rgb));
      moveWindow("original image", new_window_position_x, new_window_position_y);
      new_window_position_x += rgb->width;
      printf("Press a button to continue ..."); fflush(stdout); //waitKey(0);
      printf("[done]\n");
    }

    // convert to RGB to Lab space
    
    lab = Mat(rgb->height, rgb->width, CV_8UC3);
    cvtColor(Mat(rgb), lab, CV_BGR2Lab);// data array actually is bgr,bgr,

    return 0;
  }





}
