#include <opencv/cv.h>
#include <vector>
#include <cmath>

namespace ya_imagekit {

  using namespace cv;
  using namespace std;

  class Image {
    
    /* data structure of segments (unary and binary)
     */
    struct UnarySeg {
      int label;
      int area, boundary;    
      Point center;

      // color properties
      float avgLab[3], saturation;
    
      // spatial attributes:
      float size, 
	mean[2],
	dev[4], //2x2 matrix
	compactness,
	elongation,
	centrality;


      void print(FILE* fp) {
	fprintf(fp,"%3d -- ", label);
	fprintf(fp,"%6.3f %6.3f %6.3f ", avgLab[0] / 255., 
		(avgLab[1] - 128.)/128., (avgLab[2] - 128.)/128.);
	fprintf(fp,"%6.3f -- ", saturation);
	fprintf(fp,"%6.3f ", -size*log2(size));
	fprintf(fp,"%6.3f %6.3f ", mean[0], mean[1]);
	fprintf(fp,"%6.3f %6.3f %6.3f %6.3f ", dev[0], dev[1], dev[2], dev[3]);
	fprintf(fp,"%6.3f ", elongation);
	fprintf(fp,"%6.3f ", compactness);
	fprintf(fp,"%6.3f ", centrality);
	fprintf(fp,"\n");
      }


    };  
  
    struct BinarySeg {
      int labels[2];
      int neighborCount; // the number of neighboring pixels between two segments

      //spatial attribute of surrounding
      float size, 
	mean[2],
	dev[4], //2x2 matrix
	compactness,
	elongation,
	centrality;


      void print(FILE* fp) {
	fprintf(fp, "%3d %3d -- ", labels[0], labels[1]);
	fprintf(fp,"%6.3f ", -size*log2(size));
	fprintf(fp,"%6.3f %6.3f ", mean[0], mean[1]);
	fprintf(fp,"%6.3f %6.3f %6.3f %6.3f ", dev[0], dev[1], dev[2], dev[3]);
	fprintf(fp,"%6.3f ", elongation);
	fprintf(fp,"%6.3f ", compactness);
	fprintf(fp,"%6.3f ", centrality);
	fprintf(fp,"\n");
      }    

    };

    IplImage *rgb; // image read
    Mat lab; // image in lab space
    Mat segments_map; // segmentation: pixel => (int) index of segments
    Mat edge_mask, boundary_mask;
    Mat context_aware_saliency_map; 

    vector<UnarySeg> usegs; // unary segments
    vector<BinarySeg> bsegs; // segment pairs

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
    int prepareEdges();

    // 
    static int writeUnarySegmentSchema(FILE * fp = NULL){
      if (fp == NULL) fp = stdout;
      fprintf(fp, "idx -- avgLab{3} saturation -- size(ent) mean{2} dev{2x2} elongation compactness centrality\n");
      return 0;
    };

    static int writeBinarySegmentSchema(FILE * fp = NULL){
      if (fp == NULL) fp = stdout;
      fprintf(fp, "idx1 idx2 -- size(ent) mean{2} dev{2x2} elongation compactness centrality\n");
      return 0;
    };


    int writeUnarySegments(FILE *fp = NULL){
      if (fp == NULL) fp = stdout;

      for (int i=0; i<usegs.size(); ++i) 
	if (true || usegs[i].compactness > .5) {
	  usegs[i].print(fp);
	}

      return 0;
    };

    int writeBinarySegments(FILE *fp = NULL) {
      if (fp == NULL) fp = stdout;

      for (int i=0; i<bsegs.size(); ++i)
	if (true) {
	  bsegs[i].print(fp);
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
        
    int displaySegments(bool *isShown = NULL, bool isDrawGraph = false);

    int reSamplingPixelsFromSeg();

    ~Image();

  private:
    void restoreSpatialAttributes(const set<int> &setOfIndex, BinarySeg &);
    void restoreSpatialAttributes(const Mat &, vector<UnarySeg> & );


  };
}



