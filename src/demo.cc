#include "colorfactor.hpp"
using namespace ya_imagekit;
int main(int argc, char ** argv) {
  Image img;
  img.read("test0.jpg");
  img.createSegmentsByKmeans(20);
  img.prepareSegments();
  img.displaySegments();
  return 0;
}
