#include "colorfactor.hpp"
using namespace ya_imagekit;
int main(int argc, char ** argv) {
  Image img;
  int err;

  if (argc == 1) {
    if (err = img.read("test0.jpg") != 0) exit(err);
    //    img.createSegmentsByKmeans(20);
    std::cout << img.createSegmentsByFelzenszwalbP04(20) << std::endl;
    //std::cout << img.createSegmentsByComanicuM02MeanShift(20) << std::endl;

    img.prepareSegments();
    img.displaySegments();
  
    img.writeSegmentSchema();
    img.writeUnarySegments();
    img.writePairSegments();
  }
  else if (argc == 2) {
    if (err = img.read(argv[1], true) != 0) exit(err);
    img.createSegmentsByKmeans(20, true);
    img.prepareSegments();
    img.writePairSegments();
  }

  return 0;
}
