#include "Dsp.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <new>
#include <memory>
#include <iomanip>
static size_t allocations=0;
void* operator new(std::size_t n){++allocations;if(auto p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
static int failed=0;
static void check(bool ok,const char* name,double v=0){std::cout<<(ok?"PASS ":"FAIL ")<<name<<" : "<<std::setprecision(9)<<v<<"\n";if(!ok)++failed;}
static float randomSample(unsigned& seed){seed=seed*1664525u+1013904223u;return (float(seed>>8)/8388608.f-1)*.25f;}
static void wav(const char* path,const std::vector<float>& v){std::ofstream o(path,std::ios::binary);auto u16=[&](unsigned n){char b[]{char(n),char(n>>8)};o.write(b,2);};auto u32=[&](unsigned n){char b[]{char(n),char(n>>8),char(n>>16),char(n>>24)};o.write(b,4);};o.write("RIFF",4);u32(36+unsigned(v.size()*2));o.write("WAVEfmt ",8);u32(16);u16(1);u16(1);u32(48000);u32(96000);u16(2);u16(16);o.write("data",4);u32(unsigned(v.size()*2));for(auto s:v)u16(unsigned(short(std::clamp(s,-1.f,1.f)*32767)));}
int main(){
 for(float sr:{44100.f,48000.f,88200.f,96000.f}){
  auto e=std::make_unique<pong::Engine>();e->prepare(sr);std::vector<float> input(14000);unsigned seed=7;float err=0;
  for(int i=0;i<int(input.size());++i){input[i]=(i==0?.7f:0)+.1f*std::sin(2*pong::pi*997*i/sr)+randomSample(seed);if(i%1700==0)for(int b=0;b<5;++b){e->target[b][0]=20+float((i+b*243)%10000);e->target[b][1]=20000-float((i+b*123)%12000);}auto y=e->tick({input[i],-input[i]},2);float ref=i>=pong::Engine::Latency?input[i-pong::Engine::Latency]:0;err=std::max(err,std::max(std::abs(y[0]-ref),std::abs(y[1]+ref)));}check(err<2.e-6f,"neutral moving windows stereo",err);
  for(int b=0;b<5;++b){e->target[b]=pong::defaults(b);e->target[b][0]=20;e->target[b][1]=20000;e->target[b][pong::Output]=6;}e->prepare(sr);err=0;seed=17;
  for(int i=0;i<int(input.size());++i){input[i]=randomSample(seed);auto y=e->tick({input[i],0},1);if(i>pong::N*2)err=std::max(err,std::abs(y[0]-input[i-pong::Engine::Latency]*pong::gain(6)));}check(err<3.e-6,"WOLA masks unity under full overlap +6dB",err);
  for(int b=0;b<5;++b){auto& t=e->target[b];t[pong::Drive]=24;t[pong::Threshold]=-60;t[pong::Ratio]=30;t[pong::Transient]=18;t[pong::Sustain]=-18;t[pong::Colour]=1;t[pong::Character]=1;}e->prepare(sr);bool finite=true;float peak=0;size_t before=allocations;for(int i=0;i<int(sr);++i){auto y=e->tick({(i%2000<1000?1.f:0.f),.3f},2);finite&=std::isfinite(y[0])&&std::isfinite(y[1]);peak=std::max(peak,std::abs(y[0]));}size_t after=allocations;check(finite,"extreme finite",peak);check(after==before,"DSP callback allocations",double(after-before));
 }
 auto run=[](int block){auto e=std::make_unique<pong::Engine>();std::vector<float> out(24000);for(int offset=0;offset<24000;offset+=block)for(int i=offset;i<std::min(offset+block,24000);++i){if(i%3000==0){e->target[1][pong::Drive]=float(i/3000);e->target[1][pong::Transient]=i%6000?12:-12;e->target[2][pong::Lo]=float(300+i/5);}out[i]=e->tick({.25f*std::sin(i*.03f),0},1)[0];}return out;};auto reference=run(32);for(int bs:{64,128,256,512,1024}){auto result=run(bs);float error=0;for(int i=0;i<24000;++i)error=std::max(error,std::abs(result[i]-reference[i]));check(error==0,"host chunk independence (sample-timed targets)",error);}
 auto render=[](int character,float trans){auto e=std::make_unique<pong::Engine>();for(auto& t:e->target){t[pong::Threshold]=-24;t[pong::Ratio]=8;t[pong::Drive]=9;t[pong::Colour]=.8f;t[pong::Character]=float(character);t[pong::Transient]=trans;}e->prepare(48000);std::vector<float> out(48000*4);unsigned seed=1;for(int i=0;i<int(out.size())+pong::Engine::Latency;++i){float time=float(i)/48000,beat=std::fmod(time,.25f);float x=.35f*std::exp(-beat*40)*std::sin(2*pong::pi*75*time)+.2f*std::sin(2*pong::pi*440*time)+randomSample(seed)*std::exp(-beat*70);auto y=e->tick({x,0},1);if(i>=pong::Engine::Latency)out[i-pong::Engine::Latency]=y[0];}return out;};
 auto v=render(0,0),i=render(1,0),plus=render(1,18),minus=render(1,-18);double diff=0,transdiff=0;for(size_t n=0;n<v.size();++n){diff+=std::abs(v[n]-i[n]);transdiff+=std::abs(plus[n]-minus[n]);}check(diff/v.size()>.0001,"Velvet vs Iron waveform difference",diff/v.size());check(transdiff/v.size()>.0001,"positive vs negative transient",transdiff/v.size());
 for(auto* audio:{&v,&i}){double power=0;for(auto s:*audio)power+=s*s;float normal=.12f/float(std::sqrt(power/audio->size()));for(auto& s:*audio)s*=normal;}wav("Velvet-RMS-matched.wav",v);wav("Iron-RMS-matched.wav",i);
 auto e=std::make_unique<pong::Engine>();e->target[2][pong::Colour]=1;e->prepare(48000);auto begin=std::chrono::steady_clock::now();for(int n=0;n<48000*60;++n)e->tick({.2f*std::sin(n*.03f),.15f},2);double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();std::cout<<"BENCH 60 seconds stereo / 48k: "<<seconds<<" seconds wall; realtime fraction "<<seconds/60<<"\n";
 check(std::isfinite(e->outputPeak),"60 second run",e->outputPeak);
 e->target[0][pong::Threshold]=-24;e->target[0][pong::Ratio]=4;e->target[0][pong::Attack]=20;e->prepare(48000);float early=0,late=0,tail=0;
 for(int n=0;n<192000;++n){e->tick({n<48000?.4f:0,0},1);if(n==1100)early=e->reduction[0];if(n==47000)late=e->reduction[0];if(n==191999)tail=e->reduction[0];}check(late>early&&tail<late*.1f,"attack rises and release recovers after 3s silence",tail);
 e->prepare(48000);e->tick({std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()},2);bool clean=true;for(int n=0;n<10000;++n){auto z=e->tick({0,0},2);clean&=std::isfinite(z[0])&&std::isfinite(z[1]);}check(clean,"NaN / Inf / silence recovery");
 std::cout<<"Failures: "<<failed<<"\n";return failed?1:0;
}

