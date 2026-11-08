// Included inside namespace pong by Dsp.h.
struct Engine {
 static constexpr int Latency=N+2*Saturator4x::Latency+Limiter::Look;
 FFT fft;
 std::array<Values,B> target{},p{},effective{};
 std::array<float,GCount> global{},gs{};
 std::array<Route,Routes> routes{};std::array<float,Routes> routeValue{};
 std::array<std::array<float,B*Destinations>,Routes> routeWeights{};
 std::array<std::array<float,Destinations>,B> modulation{};
 std::array<float,Sources> sources{};std::array<int,B> detectorSource{};
 std::array<float,B> adaptive{},transients{};std::array<bool,B> stress{};
 Motion motion;unsigned lastSeed=22;float tempo=120,wallLo=80,wallHi=16000;
 std::array<std::uint64_t,128> notes{};std::array<float,128> velocities{};std::uint64_t noteCounter=0;int note=-1;float velocity=0,cc=0;
 std::array<std::array<float,N>,2> input{};
 std::array<std::array<std::array<float,R>,B>,2> ola{};
 std::array<std::array<float,N>,B> mask{};
 std::array<float,N> window{};
 std::array<std::complex<float>,N> spectrum{},scratch{};
 std::array<Detector,B> det{};
 std::array<float,B> meter{},reduction{},a{},rel{},inGain{},outGain{},wet{};
 std::array<std::array<Saturator4x,2>,B> oversample{};
 std::array<Saturator4x,2> glueOS{};
 std::array<std::array<Delay<16>,2>,B> bandDelay{};
 std::array<Delay<16>,2> dryDelay{};
 std::array<Delay<N+32>,2> bypassDelay{};
 Delay<N> sideDelay;std::array<std::array<CutBank,2>,2> cuts{};
 Plate prePlate,postPlate;Limiter limiter;
 float sr=48000,smoothing=0,fastC=0,slowC=0,globalMix=1,globalTarget=1,soloMix=0;
 float outputPeak=0,sideEnv=0,broadEnv=0,hallEnv=0,glueEnv=0,glueReduction=0,plateSend=0;
 std::array<float,3> hallWeights{0,0,1};std::array<float,2> lastHall{};
 std::uint64_t clock=0;int controlCounter=0,filterCounter=0;
 Engine(){for(int b=0;b<B;++b)target[b]=p[b]=effective[b]=defaults(b);for(int i=0;i<GCount;++i)global[i]=gs[i]=specs[i].value;for(int i=0;i<N;++i)window[i]=std::sin(pi*(i+.5f)/N);prepare(48000);}
 void prepare(float rate){sr=rate;smoothing=1-std::exp(-1/(sr*.02f));fastC=std::exp(-1/(sr*.001f));slowC=std::exp(-1/(sr*.06f));input={};ola={};det={};meter={};reduction={};clock=0;p=target;effective=p;gs=global;globalMix=globalTarget;soloMix=0;sideEnv=broadEnv=hallEnv=glueEnv=outputPeak=0;lastHall={};adaptive={};stress={};routeValue={};modulation={};sources={};transients={};notes={};velocities={};note=-1;velocity=cc=0;noteCounter=0;
  for(auto& group:oversample)for(auto& s:group)s.reset();for(auto&s:glueOS)s.reset();for(auto&group:bandDelay)for(auto&d:group)d.reset();for(auto&d:dryDelay)d.reset();for(auto&d:bypassDelay)d.reset();sideDelay.reset();for(auto&group:cuts)for(auto&c:group)c.reset();prePlate.reset();postPlate.reset();limiter.reset();lastSeed=unsigned(global[Seed]);motion.reset(lastSeed);hallWeights={0,0,0};hallWeights[std::clamp(int(global[HallPos]),0,2)]=1;routeWeights={};for(int r=0;r<Routes;++r)routeWeights[r][std::clamp(routes[r].band,0,4)*Destinations+std::clamp(routes[r].destination,0,Destinations-1)]=1;controlCounter=filterCounter=0;control(false);
 }
 void midi(int type,int number,float value){number=std::clamp(number,0,127);if(type==1&&value>0){notes[number]=++noteCounter;velocities[number]=value;}else if(type==0||type==1){notes[number]=0;}else if(type==2){if(number==int(global[MidiCC]))cc=value;if(number==120||number==123)notes={};}note=-1;std::uint64_t latest=0;for(int n=0;n<128;++n)if(notes[n]>latest){latest=notes[n];note=n;}velocity=note>=0?velocities[note]:0;}
 static float energy(float value){return std::clamp((db(value)+60)/60,0.f,1.f);}
 void control(bool advance=true){
  float dt=32/sr;bool hold=!advance||global[Hold]>.5f;
  if(unsigned(global[Seed])!=lastSeed){lastSeed=unsigned(global[Seed]);motion.reset(lastSeed);}
  float widest=1;for(int b=1;b<4;++b)widest=std::max(widest,std::sqrt(std::max(1.f,p[b][Hi]/std::max(20.f,p[b][Lo]))));widest=std::min(widest,31.6f);wallLo=std::clamp(gs[MinHz],20*widest,20000/widest);wallHi=std::clamp(gs[MaxHz],wallLo,20000/widest);float wallL=std::log(wallLo/20)/std::log(1000.f),wallR=std::log(wallHi/20)/std::log(1000.f);
  if(global[Play]>.5f&&!hold)motion.step(dt,gs[Speed]*(global[Sync]>.5f?tempo/60:1),wallL,wallR,std::clamp((p[0][Drive]+48)/96,.05f,.95f),std::clamp((p[4][Drive]+48)/96,.05f,.95f));
  else if(!hold)for(auto&b:motion.ball)b.pulse*=std::exp(-dt/.12f);
  if(!hold){
   for(int b=0;b<B;++b){if(meter[b]>.8f||det[b].gr>12)stress[b]=true;else if(meter[b]<.5f&&det[b].gr<6)stress[b]=false;float targetReduction=stress[b]?1.f:0.f;float time=stress[b]?.08f:gs[Recovery];float delta=(targetReduction-adaptive[b])*(1-std::exp(-dt/time));adaptive[b]+=std::clamp(delta,-dt/std::max(.2f,gs[Recovery]),dt*8);}
   for(int r=0;r<Routes;++r){float s=sources[std::clamp(routes[r].source,0,Sources-1)];float alpha=1-std::exp(-dt/(std::max(5.f,routes[r].smoothMs)*.001f));routeValue[r]+=alpha*(s*routes[r].depth-routeValue[r]);int targetIndex=std::clamp(routes[r].band,0,4)*Destinations+std::clamp(routes[r].destination,0,Destinations-1);float routeFade=1-std::exp(-dt/.03f);for(int j=0;j<B*Destinations;++j)routeWeights[r][j]+=routeFade*((j==targetIndex?1.f:0.f)-routeWeights[r][j]);}
  }
  modulation={};for(int r=0;r<Routes;++r)for(int b=0;b<B;++b)for(int d=0;d<Destinations;++d)modulation[b][d]+=routeWeights[r][b*Destinations+d]*routeValue[r]*gs[Interaction]*(1-.75f*adaptive[b]*gs[Adapt]);
  plateSend=gs[Send];
  for(int b=0;b<B;++b){effective[b]=p[b];float shift=std::clamp(modulation[b][2]*2,-4.f,4.f);float playPulse=0;
   if(gs[Play]>1.e-6f&&b>0&&b<4){auto& ball=motion.ball[b-1];float base=std::sqrt(std::max(20.f,p[b][Lo])*std::max(p[b][Lo],p[b][Hi]));float centre=20*std::pow(1000.f,ball.x);shift+=gs[Play]*std::log2(centre/base);effective[b][Drive]+=gs[Play]*gs[Depth]*(ball.y-.5f)*96;playPulse=ball.pulse*gs[Depth]*gs[Play];effective[b][Transient]+=playPulse*9;}
   float width=std::sqrt(std::max(1.f,p[b][Hi]/std::max(20.f,p[b][Lo]))),centre=std::sqrt(std::max(20.f,p[b][Lo])*std::max(p[b][Lo],p[b][Hi]))*std::pow(2.f,shift);centre=std::clamp(centre,20*width,20000/width);effective[b][Lo]=centre/width;effective[b][Hi]=centre*width;
   effective[b][Drive]=std::clamp(effective[b][Drive]+24*modulation[b][0]-24*adaptive[b]*gs[Adapt],-48.f,48.f);
   effective[b][Threshold]=std::clamp(p[b][Threshold]+24*modulation[b][1],-60.f,0.f);effective[b][Character]=std::clamp(p[b][Character]+modulation[b][3]+playPulse*.4f,0.f,1.f);plateSend+=modulation[b][4]*.2f+playPulse*.1f;
   a[b]=std::exp(-1/(sr*std::max(.05f,p[b][Attack])*.001f));rel[b]=std::exp(-1/(sr*std::max(5.f,p[b][Release])*.001f));inGain[b]=gain(effective[b][Drive]);outGain[b]=gain(p[b][Output]);wet[b]=p[b][Mix]*(1-p[b][Bypass]);
  }
  plateSend=std::clamp(plateSend,0.f,1.f);
  // All route sources are sampled only after targets were calculated: one control-period delay.
  for(int b=0;b<B;++b){sources[b]=energy(det[b].slow);sources[5+b]=transients[b];}sources[10]=energy(broadEnv);sources[11]=energy(sideEnv);sources[12]=note>=0?std::clamp((note-60)/24.f,-1.f,1.f):0;sources[13]=velocity;sources[14]=note>=0?1.f:0;sources[15]=cc;sources[16]=energy(hallEnv);sources[17]=std::max({motion.ball[0].pulse,motion.ball[1].pulse,motion.ball[2].pulse});
  if(filterCounter++%8==0){for(int c=0;c<2;++c){cuts[0][c].setup(sr,p[0][Hi],true);cuts[1][c].setup(sr,p[4][Lo],false);}prePlate.setup(sr,gs[PreDelay],gs[Decay],gs[Damping]);postPlate.setup(sr,gs[PreDelay],gs[Decay],gs[Damping]);}
 }
 void analyse(int channels){
  for(int k=0;k<=N/2;++k){float f=std::max(1.f,k*sr/N),sum=0;for(int b=0;b<B;++b){float lo=std::clamp(effective[b][Lo],20.f,sr*.48f),hi=std::clamp(effective[b][Hi],lo,sr*.49f);float l=b==0?1.f:1/(1+std::pow(lo/f,4.f)),h=b==4?1.f:1/(1+std::pow(f/hi,4.f));mask[b][k]=l*h;sum+=l*h;}for(int b=0;b<B;++b){mask[b][k]/=std::max(1.f,sum);if(k>0&&k<N/2)mask[b][N-k]=mask[b][k];}}
  for(int c=0;c<channels;++c){for(int i=0;i<N;++i)spectrum[i]={input[c][(clock+1+i)%N]*window[i],0};fft.run(spectrum,false);for(int b=0;b<B;++b){for(int i=0;i<N;++i)scratch[i]=spectrum[i]*mask[b][i];fft.run(scratch,true);for(int i=0;i<N;++i)ola[c][b][(clock+1+i)%R]+=scratch[i].real()*window[i]*.5f;}}
 }
 std::array<float,2> tick(std::array<float,2> x,int channels,float side=0){
  for(int b=0;b<B;++b)for(int j=0;j<Count;++j)p[b][j]+=smoothing*(target[b][j]-p[b][j]);for(int j=0;j<GCount;++j)gs[j]+=smoothing*(global[j]-gs[j]);globalMix+=smoothing*(globalTarget-globalMix);
  bool solo=false;for(int b=0;b<B;++b)solo|=target[b][Solo]>.5f;soloMix+=smoothing*((solo?1.f:0.f)-soloMix);if(controlCounter==0)control();controlCounter=(controlCounter+1)%32;
  for(auto& v:x){if(!std::isfinite(v))v=0;v=std::clamp(v,-16.f,16.f);}if(channels==1)x[1]=x[0];if(!std::isfinite(side))side=0;side=sideDelay.tick(std::clamp(side,-16.f,16.f));sideEnv=slowC*sideEnv+(1-slowC)*std::abs(side);
  for(int i=0;i<3;++i)hallWeights[i]+=smoothing*((int(global[HallPos])==i?1.f:0.f)-hallWeights[i]);
  auto pre=prePlate.tick({x[0]*plateSend,x[1]*plateSend});std::array<float,2> y{},dry{},sumSolo{},original{};float broad=0;
  for(int c=0;c<channels;++c){original[c]=bypassDelay[c].tick(x[c]);float delayed=input[c][clock%N];broad=std::max(broad,std::abs(delayed));dry[c]=dryDelay[c].tick(delayed);input[c][clock%N]=x[c]+hallWeights[0]*gs[Return]*pre[c];y[c]=dry[c];}
  broadEnv=slowC*broadEnv+(1-slowC)*broad;
  for(int b=0;b<B;++b){std::array<float,2> band{};float peak=0;for(int c=0;c<channels;++c){band[c]=ola[c][b][clock%R];ola[c][b][clock%R]=0;peak=std::max(peak,std::abs(band[c]*inGain[b]));}
   float detector=peak;int ds=detectorSource[b];if(ds==1)detector=broad;if(ds==2)detector=std::abs(side);if(ds>=3&&ds<=7)detector=std::pow(10.f,(sources[ds-3]*60-60)/20);if(ds==8)detector=hallEnv;if(ds==9)detector=note>=0?velocity:0;
   auto& d=det[b];float ch=effective[b][Character],envC=detector>d.env?a[b]:(rel[b]+(1-rel[b])*(1-ch)*std::clamp(d.gr/30,0.f,.75f));d.env=envC*d.env+(1-envC)*detector;d.fast=fastC*d.fast+(1-fastC)*peak;d.slow=slowC*d.slow+(1-slowC)*peak;
   float over=db(d.env)-effective[b][Threshold],k=std::max(.001f,p[b][Knee]),slope=1-1/std::max(1.f,p[b][Ratio]);float knee=over<=-k/2?0:(over>=k/2?over:(over+k/2)*(over+k/2)/(2*k));float velvet=1-.3f*std::exp(-std::max(0.f,over)/12);d.gr=std::clamp(knee*slope*(velvet+(1-velvet)*ch),0.f,p[b][Range]);
   float t=std::clamp((d.fast-d.slow)/(d.slow+.01f),-1.f,1.f);transients[b]=t;float transient=effective[b][Transient]*std::max(0.f,t)+p[b][Sustain]*std::max(0.f,-t);float g=gain(std::clamp(-d.gr+transient,-p[b][Range],p[b][Range]));
   for(int c=0;c<channels;++c){float dryBand=bandDelay[b][c].tick(band[c]);float v=oversample[b][c].process(band[c]*inGain[b]*g,p[b][Colour],ch)*outGain[b];float processed=dryBand+wet[b]*(v-dryBand);y[c]+=processed-dryBand;sumSolo[c]+=p[b][Solo]*processed;}
   meter[b]=std::max(peak,meter[b]*.9995f);reduction[b]=d.gr;
  }
  for(int c=0;c<channels;++c){y[c]+=soloMix*(sumSolo[c]-y[c]);float hp=cuts[0][c].tick(y[c],p[0][Slope],smoothing);y[c]+=p[0][Edge]*wet[0]*(hp-y[c]);float lp=cuts[1][c].tick(y[c],p[4][Slope],smoothing);y[c]+=p[4][Edge]*wet[4]*(lp-y[c]);}
  if(channels==1)y[1]=y[0];auto hall=postPlate.tick({y[0]*plateSend,y[1]*plateSend});hallEnv=slowC*hallEnv+(1-slowC)*std::max(std::abs(hall[0]),std::abs(hall[1]));lastHall=hall;
  for(int c=0;c<2;++c)y[c]+=gs[Return]*(hallWeights[1]*(hall[c]-y[c])+hallWeights[2]*hall[c]);
  float gluePeak=std::max(std::abs(y[0]),std::abs(y[1]));glueEnv=std::max(gluePeak,glueEnv*std::exp(-1/(sr*.15f)));glueReduction=std::clamp((db(glueEnv)+18)*.25f,0.f,6.f)*gs[GlueComp];float glueGain=gain(-glueReduction),trim=gain(gs[GlueTrim]+gs[OutputTrim]);
  for(int c=0;c<channels;++c){float processed=glueOS[c].process(y[c]*glueGain,gs[GlueColour],.15f)*trim;y[c]=original[c]+globalMix*(processed-original[c]);if(!std::isfinite(y[c]))y[c]=0;}
  if(channels==1)y[1]=y[0];y=limiter.tick(y,gain(gs[Ceiling]),sr);outputPeak=std::max({outputPeak*.9995f,std::abs(y[0]),std::abs(y[1])});
  if((clock+1)%H==0)analyse(channels);++clock;return y;
 }
};
