#include <opencv/cv.h>
#include <vector>

namespace ya_imagekit {
  struct UnarySeg {
    int label;
    int area, boundary;    
    cv::Point center;

    // color properties
    float avgLab[3], saturation;
    
    // attributes:
    float size, 
      mean[2],
      dev[4], //2x2 matrix
      compactness,
      elongation,
      centrality;
      
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
    int read(const char * filename, bool quiet = false);

    // write segments_map
    int createSegmentsByKmeans(int k, bool quiet = false); // k: the number of segments

    int createSegmentsByFelzenszwalbP04(int k, bool quiet = false);

    int createSegmentsByMSA3C(int k, bool = false);

    // segment quantization
    int prepareSegments();

    // 
    static int writeSegmentSchema(FILE * fp = NULL){
      if (fp == NULL) fp = stdout;
      fprintf(fp, "idx\tavgLab{3}\tsaturation\tsize(%%)\tmean{2}\tdev{2x2}\telongation\tcompactness\tcentrality\n");
      return 0;
    };

    int writeUnarySegments(FILE *fp = NULL){
      if (fp == NULL) fp = stdout;

      for (int i=0; i<usegs.size(); ++i) 
	if (true || usegs[i].compactness > .5) {
	  fprintf(fp,"%d\t", i);
	  fprintf(fp,"%3.0f %3.0f %3.0f\t", usegs[i].avgLab[0] * 100 / 255, 
		  usegs[i].avgLab[1] - 128, usegs[i].avgLab[2] - 128);
	  fprintf(fp,"%.3f\t", usegs[i].saturation);
	  fprintf(fp,"%.2f\t", 100*usegs[i].size);
	  fprintf(fp,"%.2f %.2f\t", usegs[i].mean[0], usegs[i].mean[1]);
	  fprintf(fp,"%.2f %.2f %.2f %.2f\t", 100*usegs[i].dev[0], 100*usegs[i].dev[1], 100*usegs[i].dev[2], 100*usegs[i].dev[3]);
	  fprintf(fp,"%.3f\t", usegs[i].elongation);
	  fprintf(fp,"%.3f\t", usegs[i].compactness);
	  fprintf(fp,"%.3f\t", usegs[i].centrality);
	  fprintf(fp,"\n");
	}

      return 0;
    };

    int writePairSegments(FILE *fp = NULL) {
      if (fp == NULL) fp = stdout;

      for (int i=0; i<bsegs.size(); ++i)
	if (true) {
	  fprintf(fp,"%3.0f %3.0f %3.0f\t", 
		  usegs[bsegs[i].labels[0]].avgLab[0] * 100 / 255, 
		  usegs[bsegs[i].labels[0]].avgLab[1] - 128, 
		  usegs[bsegs[i].labels[0]].avgLab[2] - 128);
	  fprintf(fp,"%3.0f %3.0f %3.0f\t", 
		  usegs[bsegs[i].labels[1]].avgLab[0] * 100 / 255, 
		  usegs[bsegs[i].labels[1]].avgLab[1] - 128, 
		  usegs[bsegs[i].labels[1]].avgLab[2] - 128);	  
	  fprintf(fp,"\n");
	}
      return 0;
    }
        
    int displaySegments(bool *isShown = NULL);

    ~Image() {
      cvReleaseImage(&rgb); 
    };
  };
}



