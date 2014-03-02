#include <opencv/cv.h>
#include <vector>
#include <cmath>

namespace ya_imagekit {
  struct UnarySeg {
    int label;
    int area, boundary;    
    cv::Point center;

    // color properties
    float avgLab[3], saturation;
    
    // spatial attributes:
    //float p[10];
    float size, 
      mean[2],
      dev[4], //2x2 matrix
      compactness,
      elongation,
      centrality;
    /*
    UnarySeg(): size(p[0]),
		     mean(p+1),
		     dev(p+3),
		     compactness(p[7]),
		     elongation(p[8]),
		     centrality(p[9]) {};
    UnarySeg &operator = (const UnarySeg &that) {return *this;}
    */
  };

  
  struct BinarySeg {
    int labels[2];
    int neighborCount; // the number of neighboring pixels between two segments

    //float p[10];//spatial attribute of surrounding
    float size, 
      mean[2],
      dev[4], //2x2 matrix
      compactness,
      elongation,
      centrality;

    /*
    BinarySeg(): size(p[0]),
		     mean(p+1),
		     dev(p+3),
		     compactness(p[7]),
		     elongation(p[8]),
		     centrality(p[9]) {};

    BinarySeg &operator = (const BinarySeg &that) {return *this;}
    */
  };


  //const UnarySeg EmptyUnarySeg;
  //const BinarySeg EmptyBinarySeg;


  class Image {
    IplImage *rgb;
    cv::Mat lab, segments_map;
    cv::Mat context_aware_saliency_map;

    std::vector<UnarySeg> usegs;
    std::vector<BinarySeg> bsegs;

  public:
    int read(const char * filename, bool quiet = false);

    // write segments_map
    int createSegmentsByKmeans(int k, bool quiet = false); // k: the number of segments

    int createSegmentsByFelzenszwalbP04(int k, bool quiet = false); // k: auto computed

    int createSegmentsByComanicuM02MeanShift(int k, bool quiet = false); // k: auto computed

    // write context_aware_saliency_map
    int createSaliencyByContextAware(const char* filename) { //read from files
      //compute CA Saliency is slow ...
      return 0;
    }

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
	  fprintf(fp,"%3d ", i);
	  fprintf(fp,"%6.3f %6.3f %6.3f ", usegs[i].avgLab[0] / 255., 
		  (usegs[i].avgLab[1] - 128.)/128., (usegs[i].avgLab[2] - 128.)/128.);
	  fprintf(fp,"%6.3f ", usegs[i].saturation);
	  fprintf(fp,"%6.3f ", -usegs[i].size*log2(usegs[i].size));
	  fprintf(fp,"%6.3f %6.3f ", usegs[i].mean[0], usegs[i].mean[1]);
	  fprintf(fp,"%6.3f %6.3f %6.3f %6.3f ", usegs[i].dev[0], usegs[i].dev[1], usegs[i].dev[2], usegs[i].dev[3]);
	  fprintf(fp,"%6.3f ", usegs[i].elongation);
	  fprintf(fp,"%6.3f ", usegs[i].compactness);
	  fprintf(fp,"%6.3f ", usegs[i].centrality);
	  fprintf(fp,"\n");
	}

      return 0;
    };

    int writePairSegments(FILE *fp = NULL) {
      if (fp == NULL) fp = stdout;

      for (int i=0; i<bsegs.size(); ++i)
	if (true) {
	  fprintf(fp, "%d %d\n", bsegs[i].labels[0], bsegs[i].labels[1]);
	  /*
	  fprintf(fp,"%3.0f %3.0f %3.0f\t", 
		  usegs[bsegs[i].labels[0]].avgLab[0] * 100 / 255, 
		  usegs[bsegs[i].labels[0]].avgLab[1] - 128, 
		  usegs[bsegs[i].labels[0]].avgLab[2] - 128);
	  fprintf(fp,"%3.0f %3.0f %3.0f\t", 
		  usegs[bsegs[i].labels[1]].avgLab[0] * 100 / 255, 
		  usegs[bsegs[i].labels[1]].avgLab[1] - 128, 
		  usegs[bsegs[i].labels[1]].avgLab[2] - 128);	  
	  fprintf(fp,"\n");
	  */
	}
      return 0;
    }
        
    int displaySegments(bool *isShown = NULL);

    ~Image() {
      cvReleaseImage(&rgb); 
    };

  private:
    void restoreSpatialAttributes(const std::set<int> &setOfIndex, float *p);
    void restoreSpatialAttributes(const cv::Mat &, std::vector<UnarySeg> & );
    void restoreSpatialAttributes();

  };
}



