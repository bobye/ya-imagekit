#include "imageProcessor.hh"
#include <cstdio>
#include <cstdlib>
#include <opencv/highgui.h>

#include "util.hh"
#include "3rdparty.h"

namespace ya_imagekit {

  extern int new_window_position_x;
  extern int new_window_position_y;

  Mat src;
  Mat src_gray, dst, detected_edges, de_filtered;
  Mat boundary_edges;

  int edgeThresh = 1;
  int lowThreshold;
  int const max_lowThreshold = 100;
  int ratio = 3;
  int kernel_size = 3;
  char* window_name = "Edge Map";

  /**
   * @function CannyThreshold
   * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
   */
  void CannyThreshold(int, void*)
  {
    /// Reduce noise with a kernel 3x3
    blur( src_gray, detected_edges, Size(3,3) );

    /// Canny detector
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display our result
    dst = Scalar::all(0);
    de_filtered = Scalar::all(0);

    detected_edges.copyTo(de_filtered, boundary_edges);
    src.copyTo( dst, de_filtered);
    imshow( window_name, dst );
  }


  int Image::reSamplingPixelsFromSeg() {
    using namespace cv;
    src= Mat(rgb, false);
    Mat rgb_clone = Mat(rgb, true);
    int n = segments_map.rows * segments_map.cols;

    assert(usegs.size() > 0);

    /* Compute edge map
     */
    boundary_edges = boundary_mask; // load boundary mask of segments
    /// Create a matrix of the same type and size as src (for dst)
    dst.create( src.size(), src.type() );    
    /// Convert the image to grayscale
    cvtColor( src, src_gray, CV_BGR2GRAY );
    /// Create a window
    namedWindow( window_name, CV_WINDOW_AUTOSIZE );
    moveWindow(window_name, new_window_position_x, new_window_position_y);
    new_window_position_x += src.cols;

    /// Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);
    /// Show the image
    CannyThreshold(lowThreshold, 0);
    /// Wait until user exit program by pressing a key
    waitKey(0);


    Mat segments_map_without_edges = segments_map.clone();

    /* Join with segments boundaries
     */
    for (int i=0; i<n; ++i)
      if (boundary_mask.at<uchar>(i) !=0 && detected_edges.at<uchar>(i) != 0) {
	segments_map_without_edges.at<int>(i) = -1;
      }

    /* Re-sample
     */
    std::vector< std::vector<int> > indices(usegs.size());
    for (int i=0; i<usegs.size(); ++i) {
      map2arr((int*) segments_map_without_edges.data, n, i, indices[i]);
    }

    for (int i=0; i<n; ++i) 
      if (segments_map_without_edges.at<int>(i) >= 0) {
	int j = segments_map_without_edges.at<int>(i);
	int idx = ((double) rand() / (double) RAND_MAX) * indices[j].size();
	rgb_clone.at<Vec3b>(i) = src.at<Vec3b>(indices[j][idx]);
      }

    imshow("resampled image", rgb_clone);
    moveWindow("resampled image", new_window_position_x, new_window_position_y);
    new_window_position_x += rgb_clone.cols;
    printf("Press a button to continue ..."); fflush(stdout); waitKey(0);
    printf("[done]\n");    

    return 0;   
  }

}
