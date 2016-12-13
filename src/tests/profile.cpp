#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Ptexture.h"
using namespace Ptex;

// std
#include <time.h>
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h> // for GetSystemTime
#else
#  include <sys/time.h>
#  include <sys/times.h>
#endif

namespace util {
  /*! return system time in seconds - taken from the ospray project,
      www.ospray.org, under Apache 2.0 license*/
  double getSysTime() {
#ifdef _WIN32
    SYSTEMTIME tp; GetSystemTime(&tp);
    return double(tp.wSecond) + double(tp.wMilliseconds) / 1E3;
#else
    struct timeval tp; gettimeofday(&tp,NULL); 
    return double(tp.tv_sec) + double(tp.tv_usec)/1E6; 
#endif
  }
}

int main(int argc, char** argv)
{
  int maxmem = argc >= 2 ? atoi(argv[1]) : 1024*1024;
  PtexPtr<PtexCache> c(PtexCache::create(0, maxmem));
  
  /*! target profiling time in seconds */
  double targetProfileTime = 5.f;
  /*! the same rate used for the profile look. default from ftest.cpp
      was .125; using lower value means more samples */
  float  samplingRate = .125f;
  
  Ptex::String error;
  PtexPtr<PtexTexture> r ( c->get("test.ptx", error) );

  if (!r) {
    std::cerr << error.c_str() << std::endl;
    return 1;
  }

  PtexFilter::Options opts(PtexFilter::f_bicubic, 0, 1.0);
  PtexPtr<PtexFilter> f ( PtexFilter::getFilter(r, opts) );
  float result[4];
  int faceid = 0;

  
  const double profileStartTime = util::getSysTime();
  size_t numSamples = 0.f;
  while (1) {
    /* run one complete resampling frame */
    float u=0, v=0, uw=samplingRate, vw=samplingRate;
    for (v = 0; v <= 1; v += samplingRate) {
      for (u = 0; u <= 1; u += samplingRate) {
        f->eval(result, 0, 1, faceid, u, v, uw, 0, 0, vw);
        ++ numSamples;
      }
    }
    const double profileCurrentTime = util::getSysTime() - profileStartTime;
    if (profileCurrentTime >= targetProfileTime) {
      double sps = numSamples / profileCurrentTime;
      if      (sps >= 1e9f)
        printf("num samples taken per second: %.2fG\n",sps*1e-9f);
      else if (sps >= 1e6f)
        printf("num samples taken per second: %.2fM\n",sps*1e-6f);
      else if (sps >= 1e3f)
        printf("num samples taken per second: %.2fK\n",sps*1e-3f);
      else
        printf("num samples taken per second: %.2f\n",sps*1e-3f);
      break;
    }
  }

  return 0;
}
