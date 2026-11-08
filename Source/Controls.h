#pragma once
#include <array>
namespace pong {
enum Global {Play,Speed,Sync,Depth,MinHz,MaxHz,Seed,Adapt,Interaction,Recovery,Hold,PreDelay,Decay,Damping,Send,Return,HallPos,GlueColour,GlueComp,GlueTrim,Ceiling,MidiCC,OutputTrim,GCount};
struct Spec {const char* id;const char* label;float min,max,value;};
inline constexpr std::array<Spec,GCount> specs{{
 {"play","EDIT / PLAY",0,1,0},{"speed","Speed",.1f,4,1},{"sync","Tempo sync",0,1,1},{"depth","Collision / Drive depth",0,1,.5f},{"minHz","Left wall limit Hz",20,10000,80},{"maxHz","Right wall limit Hz",100,20000,16000},{"seed","Motion seed",1,65535,22},
 {"adapt","ADAPT",0,1,0},{"interaction","INTERACTION",0,1,1},{"recovery","RECOVERY seconds",.2f,20,3},{"hold","HOLD",0,1,0},
 {"predelay","Plate pre-delay ms",0,200,20},{"decay","Plate decay seconds",.2f,12,2.8f},{"damping","Plate damping",.05f,.98f,.5f},{"send","Plate send",0,1,0},{"return","Plate return",0,1,.35f},{"hallPos","Plate routing",0,2,2},
 {"glueColour","GLUE saturation",0,1,0},{"glueComp","GLUE compression",0,1,0},{"glueTrim","GLUE trim dB",-48,48,0},{"ceiling","Limiter ceiling dBFS",-12,-.1f,-1},{"midiCC","MIDI CC number",0,127,1},{"outTrim","Master output dB",-48,48,0}
}};
struct Route {int source=0,band=0,destination=0;float depth=0,smoothMs=50;};
inline constexpr int Routes=8,Sources=18,Destinations=5;
inline constexpr const char* sourceNames[]{"LOW energy","BODY energy","PRESENCE energy","AIR energy","HIGH energy","LOW transient","BODY transient","PRESENCE transient","AIR transient","HIGH transient","Input energy","External sidechain","MIDI note","MIDI velocity","MIDI gate","MIDI CC","Plate energy","Collision"};
inline constexpr const char* destinationNames[]{"Drive (+/-24 dB)","Threshold (+/-24 dB)","Frequency (+/-2 oct)","Character","Plate send"};
inline constexpr const char* detectorNames[]{"Own band","Broadband input","External sidechain","LOW","BODY","PRESENCE","AIR","HIGH","Plate return","MIDI velocity x gate"};
}
