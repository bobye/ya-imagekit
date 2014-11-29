#include <cstdlib>
#include "imageProcessor.hh"
using namespace ya_imagekit;
int main(int argc, char ** argv) {
  Image img;

  int err; if ((err = img.read(argv[1], false)) != 0) exit(err);
  int k = atoi(argv[2]);

  std::cout << img.createSegmentsByKmeans(k) << std::endl;
  // std::cout << img.createSegmentsByFelzenszwalbP04(20) << std::endl;
  //std::cout << img.createSegmentsByComanicuM02MeanShift(200) << std::endl;

  img.prepareSegments();

  FILE * binaryStatFile = fopen("bstatfile.txt", "w");
  img.writeBinarySegments(binaryStatFile);
  fclose(binaryStatFile);

  //  img.reSamplingPixelsFromSeg();


  string tmpPath("subseg_map");
  system((string("rm ") + tmpPath + "/*").c_str());
  img.exportSubSegmentMaps(tmpPath);
  system("/Applications/MATLAB_R2012a.app/bin/matlab -nojvm -r 'computeRelativeDistance;exit'");
  img.computeSuperGradient(tmpPath);


  /*
    img.writeUnarySegmentSchema();
    img.writeUnarySegments();
    img.writeBinarySegmentSchema();
    img.writeBinarySegments();
  */

  return 0;
}
