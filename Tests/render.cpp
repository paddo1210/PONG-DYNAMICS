#include "Dsp.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
void wav(const std::string& path,const std::vector<float>& v){std::ofstream o(path,std::ios::binary);auto u16=[&](unsigned n){char b[]{char(n),char(n>>8)};o.write(b,2);};auto u32=[&](unsigned n){char b[]{char(n),char(n>>8),char(n>>16),char(n>>24)};o.write(b,4);};o.write("RIFF",4);u32(36+unsigned(v.size()*2));o.write("WAVEfmt ",8);u32(16);u16(1);u16(2);u32(48000);u32(192000);u16(4);u16(16);o.write("data",4);u32(unsigned(v.size()*2));for(float x:v)u16(unsigned(short(x*32767)));}
int main(){constexpr int length=48000*6;std::array<std::vector<float>,4> clips;std::array<double,4> rms{},peaks{};std::filesystem::create_directory("Audio-v2");
 for(int mode=0;mode<4;++mode){auto e=std::make_unique<pong::Engine>();if(mode){for(auto&t:e->target){t[pong::Threshold]=-26;t[pong::Ratio]=5;t[pong::Drive]=6;t[pong::Colour]=.6f;t[pong::Mix]=.8f;}e->global[pong::Send]=.2f;e->global[pong::GlueColour]=.2f;}if(mode>=2){e->global[pong::Adapt]=.7f;e->routes[0]={0,1,1,-.6f,50};e->routes[1]={6,2,2,.3f,40};}if(mode==3){e->global[pong::Play]=1;e->global[pong::Depth]=.35f;}e->prepare(48000);clips[mode].resize(length*2);unsigned rng=1;
  for(int n=0;n<length+pong::Engine::Latency;++n){double time=n/48000.,beat=std::fmod(time,.5);rng=1664525*rng+1013904223;float noise=(float(rng>>8)/8388608-1);float kick=.5f*float(std::sin(2*3.141592653589793*65*time)*std::exp(-beat*35));float snare=.18f*noise*float(std::exp(-std::fmod(time+.25,.5)*60));float bass=.15f*float(std::sin(2*3.141592653589793*110*time));float pad=.08f*float(std::sin(2*3.141592653589793*330*time)+std::sin(2*3.141592653589793*440*time));auto y=e->tick({kick+snare+bass+pad,kick-snare+bass+pad*.8f},2);if(n>=pong::Engine::Latency){int at=2*(n-pong::Engine::Latency);for(int c=0;c<2;++c){clips[mode][at+c]=y[c];rms[mode]+=y[c]*y[c];peaks[mode]=std::max(peaks[mode],double(std::abs(y[c])));}}}rms[mode]=std::sqrt(rms[mode]/(length*2));
 }
 double target=.12;for(int m=0;m<4;++m)target=std::min(target,.85*rms[m]/peaks[m]);const char* names[]{"01-Neutral","02-Static","03-Adaptive-Matrix","04-Pong-Play"};for(int m=0;m<4;++m){double g=target/rms[m];for(float&x:clips[m])x*=float(g);wav(std::string("Audio-v2/")+names[m]+".wav",clips[m]);std::cout<<names[m]<<" RMS "<<20*std::log10(target)<<" dBFS; peak "<<20*std::log10(peaks[m]*g)<<" dBFS\n";}return 0;
}
