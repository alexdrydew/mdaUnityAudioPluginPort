//See associated .cpp file for copyright and other info

#ifndef __mdaPiano__
#define __mdaPiano__

#include <string.h>
#include <vector>

#include "..//AudioPluginUtil.h"

#define NPARAMS 12       //number of parameters
#define NPROGS   8       //number of programs
#define NOUTS    2       //number of outputs
#define NVOICES 32       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define WAVELEN 586348   //wave data bytes

class mdaPianoProgram {
    friend class mdaPiano;
public:
    mdaPianoProgram();
    ~mdaPianoProgram() {}

private:
    float param[NPARAMS];
    char  name[24];
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

class mdaPiano {
public:
    mdaPiano(float samplerate, INT32 buffersize);
    ~mdaPiano();

    virtual void processReplacing(float* inputbuf, float* outputbuf, int inchannels, int outchannels, INT32 sampleframes);
    virtual INT32 processEvents(std::vector<MidiEvent>& ev);

    virtual void setProgram(INT32 program);
    virtual void setProgramName(char* name);
    virtual void getProgramName(char* name);
    virtual void setParameter(INT32 index, float value);
    virtual float getParameter(INT32 index);
    virtual void getParameterLabel(INT32 index, char* label);
    virtual void getParameterDisplay(INT32 index, char* text);
    virtual void getParameterName(INT32 index, char* text);
    virtual void setBlockSize(INT32 blockSize);
    virtual void resume();

    virtual bool getProgramNameIndexed(INT32 category, INT32 index, char* text);
    virtual bool copyProgram(INT32 destination);
    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);
    virtual INT32 getVendorVersion() { return 1; }
    virtual INT32 canDo(char* text);

    virtual INT32 getNumMidiInputChannels() { return 1; }

    INT32 guiUpdate;
    void guiGetDisplay(INT32 index, char* label);

private:
    void update();  //my parameter update
    void noteOn(INT32 note, INT32 velocity);
    void fillpatch(INT32 p, char* name, float p0, float p1, float p2, float p3, float p4,
                   float p5, float p6, float p7, float p8, float p9, float p10, float p11);

    float param[NPARAMS];
    mdaPianoProgram* programs;
    float Fs, iFs;

#define EVENTBUFFER 120
#define EVENTS_DONE 99999999
    INT32 notes[EVENTBUFFER + 8];  //list of delta|note|velocity for current block

    ///global internal variables
    KGRP  kgrp[16];
    VOICE voice[NVOICES];
    INT32  activevoices, poly, cpos;
    short* waves;
    INT32  cmax;
    float* comb, cdep, width, trim;
    INT32  size, sustain;
    float tune, fine, random, stretch;
    float muff, muffvel, sizevel, velsens, volume;

    INT32 buffersize;
    float samplerate;

    INT32 curProgram = 0;
};

#endif
