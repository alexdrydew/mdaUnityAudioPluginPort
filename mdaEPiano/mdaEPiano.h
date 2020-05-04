//See associated .cpp file for copyright and other info

#ifndef __mdaEPiano__
#define __mdaEPiano__


#include <string>
#include <vector>

#include "..//AudioPluginUtil.h"

#define NPARAMS 12       //number of parameters
#define NPROGS   8       //number of programs
#define NOUTS    2       //number of outputs
#define NVOICES 32       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define WAVELEN 422414   //wave data bytes

template <typename T>
T reduced_diff(T first, T second) {
    return first >= second ? first - second : 0;
}

struct mdaEPianoProgram {
    float param[NPARAMS];
    std::string name;
};

struct MidiEvent {
    UINT64 sample;
    union {
        UINT32 msg;
        char msgBytes[4];
    };
};

struct VOICE  //voice state
{
    INT32  delta;  //sample playback
    INT32  frac;
    INT32  pos;
    INT32  end;
    INT32  loop;

    float env;  //envelope
    float dec;

    float f0;   //first-order LPF
    float f1;
    float ff;

    float outl;
    float outr;
    INT32  note; //remember what note triggered this
};


struct KGRP  //keygroup
{
    INT32  root;  //MIDI root note
    INT32  high;  //highest note
    INT32  pos;
    INT32  end;
    INT32  loop;
};

class mdaEPiano {
public:
    mdaEPiano(UINT32 samplerate, UINT32 buffersize);
    ~mdaEPiano();

    virtual void processReplacing(float* inbuffer, float* outbuffer, int inputChannels, int outputChannels, INT32 sampleframes);
    virtual UINT32 processEvents(std::vector<MidiEvent>& events);

    virtual void setProgram(INT32 program);
    virtual void setProgramName(char* name);
    virtual std::string getProgramName();
    virtual void setParameter(INT32 index, float value);
    virtual float getParameter(INT32 index);
    virtual std::string getParameterLabel(INT32 index);
    virtual std::string getParameterDisplay(INT32 index);
    virtual std::string getParameterName(INT32 index);
    virtual void setBlockSize(INT32 blockSize);
    virtual void resume();

    //virtual bool getOutputProperties (UINT32 index, VstPinProperties* properties);
    virtual std::string getProgramNameIndexed(INT32 category, INT32 index);
    virtual bool copyProgram(INT32 destination);
    virtual std::string getEffectName();
    virtual std::string getVendorString();
    virtual std::string getProductString();
    virtual INT32 getVendorVersion() { return 1; }
    virtual INT32 canDo(char* text);

    virtual INT32 getNumMidiInputChannels() { return 1; }

    INT32 guiUpdate;

    mdaEPianoProgram* programs;
    INT32 curProgram;

private:
    void noteOn(INT32 note, INT32 velocity);

    void update();  //my parameter update
    void fillpatch(INT32 p, char* name, float p0, float p1, float p2, float p3, float p4,
                   float p5, float p6, float p7, float p8, float p9, float p10, float p11);
    float Fs, iFs;
    INT32 buffersize;

#define EVENTBUFFER 120
#define EVENTS_DONE 99999999
    INT32 notes[EVENTBUFFER + 8];  //list of delta|note|velocity for current block

    ///global internal variables
    KGRP  kgrp[34];
    VOICE voice[NVOICES];
    INT32 activevoices, poly;
    short* waves;
    float width;
    INT32 size, sustain;
    float lfo0, lfo1, dlfo, lmod, rmod;
    float treb, tfrq, tl, tr;
    float tune, fine, random, stretch, overdrive;
    float muff, muffvel, sizevel, velsens, volume, modwhl;
};

#endif
