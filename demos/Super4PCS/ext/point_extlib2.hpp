#ifndef __POINT_EXTLIB2_H__
#define __POINT_EXTLIB2_H__

namespace extlib2
{
struct PointType2 {
  float* posBuffer;   // position buffer
  float* nBuffer;     // normal buffer
  float* colorBuffer; // color buffer
  int id;             // id (or index)
};
}

#endif