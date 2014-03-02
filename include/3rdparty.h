#include <opencv/cv.h>

// segmentation
extern void segmentationByComanicuM02MeanShift(const IplImage *, cv::Mat &, int &);
extern void segmentationByFelzenszwalbP04(const IplImage *, cv::Mat &, int &k);

// saliency
