#include "colorfactor.hpp"
using namespace ya_imagekit;
int main(int argc, char ** argv) {
  Image img;

  if (argc == 1) {
    img.read("test0.jpg");
    img.createSegmentsByKmeans(20);
    img.prepareSegments();
    img.displaySegments();
  
    img.writeSegmentSchema();
    img.writeUnarySegments();
  }
  else if (argc == 2) {
    img.read(argv[1], true);
    img.createSegmentsByKmeans(20, true);
    img.prepareSegments();
    img.writePairSegments();
  }

  return 0;
}
