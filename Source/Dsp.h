#pragma once
#include "Modules.h"
#include "Controls.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>

namespace pong {
constexpr int N=1024,H=256,R=2048,B=5;
constexpr float pi=3.14159265358979323846f;
inline float db(float x) { return 20*std::log10(std::max(x,1.e-9f)); }
inline float gain(float x) { return std::pow(10.f,x*.05f); }
enum Param { Lo,Hi,Drive,Output,Threshold,Ratio,Knee,Attack,Release,Range,Mix,Character,Transient,Sustain,Colour,Bypass,Solo,Edge, Slope, Count };
using Values=std::array<float,Count>;
inline Values defaults(int b) {
 constexpr float lo[]{20,130,700,4000,9000},hi[]{180,900,4500,12000,20000};
 return {lo[b],hi[b],0,0,0,1,6,15,160,18,1,0,0,0,0,0,0,0,24};
}
// Fixed-size radix-2 transform: no allocation, shared state or backend locks.
struct FFT {
 std::array<std::complex<float>,N/2> tw{};
 FFT() { for(int i=0;i<N/2;++i) tw[i]=std::polar(1.f,-2*pi*i/N); }
 void run(std::array<std::complex<float>,N>& a,bool inverse) const {
  for(int i=1,j=0;i<N;++i){int k=N>>1;for(;j&k;k>>=1)j^=k;j^=k;if(i<j)std::swap(a[i],a[j]);}
  for(int len=2;len<=N;len*=2) for(int i=0;i<N;i+=len) for(int j=0;j<len/2;++j){auto w=tw[j*N/len];if(inverse)w=std::conj(w);auto u=a[i+j],v=a[i+j+len/2]*w;a[i+j]=u+v;a[i+j+len/2]=u-v;}
  if(inverse)for(auto& v:a)v/=float(N);
 }
};
struct Detector { float env=0,fast=0,slow=0,gr=0; };
#include "Engine.h"
}
