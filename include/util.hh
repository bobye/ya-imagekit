#include <algorithm>    // std::sort

namespace ya_imagekit {
  
  template <class T>
  inline void softBoundingBox(T* array, int size, int &min, int &max) {
    const float threshold = .05;
    std::sort<T* > (array, array + size);
    min = *(array + (int) std::floor(threshold * size));
    max = *(array + (int) std::floor((1-threshold)*size));
    return;
  }

  /** NOT USED **/
  template <class T>
  T mean(const T* array, int size) {
    T meanVal = 0;
    for (int i = 0; i < size; ++i)  mean += array[i];
    return (T) meanVal/size;
  }

}
