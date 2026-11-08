#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace pong {
// Fixed 4x FIR interpolation / decimation. Combined group delay = 16 base samples.
struct Saturator4x {
 static constexpr int Latency=16,Length=65;
 std::array<float,Length> h{};std::array<float,Length*2> high{};std::array<float,34> history{};std::array<float,17> dry{};
 int pos=0,hpos=0,dpos=0,tail=0;
 Saturator4x(){double sum=0;for(int i=0;i<Length;++i){double x=i-32,cut=.1125;double sinc=x==0?2*cut:std::sin(2*3.141592653589793*cut*x)/(3.141592653589793*x);double w=.42-.5*std::cos(2*3.141592653589793*i/64)+.08*std::cos(4*3.141592653589793*i/64);h[i]=float(sinc*w);sum+=h[i];}for(auto&v:h)v/=float(sum);}
 void reset(){high={};history={};dry={};pos=hpos=dpos=tail=0;}
 static float curve(float x){x=std::clamp(x,-3.f,3.f);return x*(27+x*x)/(27+9*x*x);}
 float process(float x,float amount,float character){
  history[pos]=history[pos+17]=x;float delta=0;
  if(amount>1.e-7f)tail=20;else if(tail>0)--tail;
  if(tail==0)hpos=(hpos+4)%Length;
  else for(int phase=0;phase<4;++phase){float up=0;if(amount>1.e-7f)for(int k=phase;k<Length;k+=4)up+=4*h[k]*history[pos+17-k/4];
   float soft=curve(up*1.6f)/1.6f,hard=curve(up*4.f)/4.f;
   high[hpos]=high[hpos+Length]=amount*((soft+(hard-soft)*character)-up);
   if(phase==0)for(int k=0;k<Length;++k)delta+=h[k]*high[hpos+Length-k];
   hpos=(hpos+1)%Length;
  }
  dry[dpos]=x;float original=dry[(dpos+1)%17];dpos=(dpos+1)%17;pos=(pos+1)%17;return original+delta;
 }
};
template<int Size>struct Delay {std::array<float,Size+1> data{};int p=0;void reset(){data={};p=0;}float tick(float x){data[p]=x;float y=data[(p+1)%(Size+1)];p=(p+1)%(Size+1);return y;}};
struct Biquad {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void setup(double rate,double hz,double q,bool highpass){double w=2*3.141592653589793*std::clamp(hz,10.,rate*.45)/rate,c=std::cos(w),alpha=std::sin(w)/(2*q),den=1+alpha;b0=(highpass?1+c:1-c)*.5/den;b1=(highpass?-(1+c):1-c)/den;b2=b0;a1=-2*c/den;a2=(1-alpha)/den;}
 float tick(float x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return float(y);}
 void reset(){z1=z2=0;}
};
struct CutBank {
 std::array<std::array<Biquad,8>,4> filters{};std::array<float,4> weights{0,1,0,0};
 void reset(){for(auto& chain:filters)for(auto&f:chain)f.reset();}
 void setup(float sr,float hz,bool hp){for(int type=0;type<4;++type){int order=2<<type;for(int k=0;k<order/2;++k){double q=1/(2*std::cos(3.141592653589793*(2*k+1)/(2*order)));filters[type][k].setup(sr,hz,q,hp);}}}
 float tick(float x,float slope,float smooth){int index=slope<18?0:slope<36?1:slope<72?2:3;float out=0;for(int type=0;type<4;++type){weights[type]+=smooth*((type==index?1.f:0.f)-weights[type]);float y=x;for(int k=0;k<(1<<type);++k)y=filters[type][k].tick(y);out+=weights[type]*y;}return out;}
};
// Eight-line Householder FDN plus four allpass diffusers. No proprietary plate model.
struct Plate {
 std::array<std::array<float,8192>,8> lines{};std::array<int,8> at{},length{};std::array<float,8> damp{},feedback{};
 std::array<std::array<float,1024>,4> allpass{};std::array<int,4> apPos{},apLen{};
 std::array<float,24001> pre{};int prePos=0,preSamples=0;float damping=.5f;
 void reset(){lines={};at={};damp={};allpass={};apPos={};pre={};prePos=0;}
 void setup(float sr,float preMs,float decay,float tone){constexpr float seconds[]{.0297f,.0371f,.0411f,.0437f,.0531f,.0617f,.0671f,.0719f};for(int i=0;i<8;++i){length[i]=std::clamp(int(seconds[i]*sr),1,8191);feedback[i]=std::pow(10.f,-3.f*length[i]/(sr*std::max(.2f,decay)));at[i]%=length[i];}constexpr float ap[]{.0017f,.0031f,.0047f,.0071f};for(int i=0;i<4;++i){apLen[i]=std::clamp(int(ap[i]*sr),1,1023);apPos[i]%=apLen[i];}preSamples=std::clamp(int(preMs*.001f*sr),0,24000);damping=std::clamp(tone,.05f,.98f);}
 std::array<float,2> tick(std::array<float,2> x){pre[prePos]=.5f*(x[0]+x[1]);float input=pre[(prePos+24001-preSamples)%24001];prePos=(prePos+1)%24001;
  for(int j=0;j<4;++j){float delayed=allpass[j][apPos[j]],v=delayed-.65f*input;allpass[j][apPos[j]]=input+.65f*v;apPos[j]=(apPos[j]+1)%std::max(1,apLen[j]);input=v;}
  std::array<float,8> tap{};float sum=0;for(int j=0;j<8;++j){tap[j]=lines[j][at[j]];damp[j]+=(1-damping)*(tap[j]-damp[j]);sum+=damp[j];}
  for(int j=0;j<8;++j){lines[j][at[j]]=.28f*input+(damp[j]-.25f*sum)*feedback[j];at[j]=(at[j]+1)%std::max(1,length[j]);}
  return { .25f*(tap[0]+tap[2]-tap[4]+tap[6]),.25f*(tap[1]-tap[3]+tap[5]+tap[7])};
 }
};
struct Limiter {
 static constexpr int Look=128;
 std::array<std::array<float,Look+1>,2> delay{};std::array<float,Look+1> peaks{};int pos=0;float g=1,reduction=0;
 void reset(){delay={};peaks={};pos=0;g=1;}
 std::array<float,2> tick(std::array<float,2> x,float ceiling,float sr){peaks[pos]=std::max(std::abs(x[0]),std::abs(x[1]));float peak=0;for(float v:peaks)peak=std::max(peak,v);float wanted=std::min(1.f,ceiling/std::max(peak,1.e-12f));float release=1-std::exp(-1/(sr*.06f));g=std::min(wanted,g+release*(1-g));std::array<float,2> y{};for(int c=0;c<2;++c){delay[c][pos]=x[c];y[c]=std::clamp(delay[c][(pos+1)%(Look+1)]*g,-ceiling,ceiling);}pos=(pos+1)%(Look+1);reduction=-20*std::log10(std::max(1.e-9f,g));return y;}
};
struct Ball {float x=.5f,y=.5f,vx=.1f,vy=.2f,pulse=0;};
struct Motion {
 std::array<Ball,3> ball{};std::uint32_t rng=1;int left=0,right=0;std::array<bool,3> contact{};
 float random(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng&0xffffff)/16777216.f;}
 void reset(unsigned seed){rng=seed?seed:1;left=right=0;contact={};for(int i=0;i<3;++i)ball[i]={.2f+.3f*i,.25f+.25f*i,(i==1?-.12f:.12f)*(.8f+.4f*random()),(.15f+.12f*random())*(i==1?-1:1),0};}
 void step(float dt,float speed,float low,float high,float paddleL,float paddleR){
  for(auto& b:ball){b.pulse*=std::exp(-dt/.12f);b.x+=b.vx*dt*speed;b.y+=b.vy*dt*speed;if(b.y<.03f||b.y>.97f){b.y=std::clamp(b.y,.03f,.97f);b.vy=-b.vy;b.pulse=std::max(b.pulse,.4f);}if(b.x<low||b.x>high){bool l=b.x<low;float paddle=l?paddleL:paddleR;if(std::abs(b.y-paddle)<.18f){b.x=std::clamp(b.x,low,high);b.vx=-b.vx;b.pulse=1;}else{if(l)++right;else++left;b.x=(low+high)*.5f;b.y=.2f+.6f*random();b.vx=(l?1:-1)*(.1f+.05f*random());b.pulse=.25f;}}}
  int pair=0;for(int a=0;a<3;++a)for(int b=a+1;b<3;++b){float dx=(ball[a].x-ball[b].x)*3,dy=ball[a].y-ball[b].y;bool touching=dx*dx+dy*dy<.055f*.055f;if(touching&&!contact[pair]){std::swap(ball[a].vx,ball[b].vx);std::swap(ball[a].vy,ball[b].vy);ball[a].pulse=ball[b].pulse=1;}contact[pair++]=touching;}
 }
};
}
