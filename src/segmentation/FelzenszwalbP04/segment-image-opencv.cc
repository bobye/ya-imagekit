#include <cstdio>
#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "segment-graph.h"
//#include "segment-image.h"
#include "segment-image-opencv.h"

#include <pnmfile.h>
// random color
rgb random_rgb(){ 
  rgb c;
  double r;
  
  c.r = (uchar)random();
  c.g = (uchar)random();
  c.b = (uchar)random();

  return c;
}

// dissimilarity measure between pixels
static inline float diff(image<float> *r, image<float> *g, image<float> *b,
			 int x1, int y1, int x2, int y2) {
  return sqrt(square(imRef(r, x1, y1)-imRef(r, x2, y2)) +
	      square(imRef(g, x1, y1)-imRef(g, x2, y2)) +
	      square(imRef(b, x1, y1)-imRef(b, x2, y2)));
}


/*
 * Segment an image
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
void segment_image(image<rgb> *im, float sigma, float c, int min_size,
		   int *num_ccs, int *classes) {
  int width = im->width();
  int height = im->height();

  image<float> *r = new image<float>(width, height);
  image<float> *g = new image<float>(width, height);
  image<float> *b = new image<float>(width, height);

  // smooth each color channel  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      imRef(r, x, y) = imRef(im, x, y).r;
      imRef(g, x, y) = imRef(im, x, y).g;
      imRef(b, x, y) = imRef(im, x, y).b;
    }
  }
  image<float> *smooth_r = smooth(r, sigma);
  image<float> *smooth_g = smooth(g, sigma);
  image<float> *smooth_b = smooth(b, sigma);
  delete r;
  delete g;
  delete b;
 
  // build graph
  edge *edges = new edge[width*height*4];
  int num = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x < width-1) {
	edges[num].a = y * width + x;
	edges[num].b = y * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
	num++;
      }

      if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
	num++;
      }

      if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
	num++;
      }

      if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
	num++;
      }
    }
  }
  delete smooth_r;
  delete smooth_g;
  delete smooth_b;

  // segment
  universe *u = segment_graph(width*height, num, edges, c);


  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;

  *num_ccs = u->num_sets();
  //std::cout << *num_ccs << std::endl;

  for (int i = 0; i<width*height; ++i) classes[i] = u->find(i);

  /*
  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  rgb *colors = new rgb[width*height];
  for (int i = 0; i < width*height; i++)
    colors[i] = random_rgb();
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
      imRef(output, x, y) = colors[comp];
    }
  }  

  delete [] colors;  
  */
  delete u;

  //  return output;
}
void segmentationByFelzenszwalbP04(const IplImage *cvimage, cv::Mat &segments_map, int &k) {

  int width = cvimage->width;
  int height = cvimage->height;

  cv::Mat cvimmat(cvimage, false);
  image<rgb> *im;
  im = new image<rgb>(width, height);


  for (int i=0; i<width*height; ++i) {
    cv::Vec3b &color = cvimmat.at<cv::Vec3b>(i);
    im->data[i].r = color[2];
    im->data[i].g = color[1];
    im->data[i].b = color[0];
  }
  
  /* test image conversion */
  //savePPM(im,"converted.ppm");

  int num_ccs;
  int *classes = new int [width * height];

  segment_image(im, .5, 500, width*height*.001, &num_ccs, classes);



  typedef std::map<int, int> mapping_type;
  mapping_type mappings;
  mapping_type::iterator mapping_iter;
 
  int index = 0;
  for (int i=0; i<width*height; ++i) {
    mapping_iter = mappings.find(classes[i]);

    if (mapping_iter != mappings.end()){
      segments_map.at<int>(i) = mappings[classes[i]];
    }else {
      mappings[classes[i]] = index; 
      segments_map.at<int>(i) = index;
      index ++;
    }
  }
  k = index;
  //std::cout << index << std::endl;

  delete [] classes;

}

