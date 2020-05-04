#ifndef __mdaDelay_H
#define __mdaDelay_H

#include "..//AudioPluginUtil.h"

class mdaDelay {
public:
    mdaDelay(float samplerate, INT32 buffersize);
    ~mdaDelay();

    virtual void processReplacing(float* inputs, float* outputs, INT32 sampleFrames);
    virtual void setProgramName(char* name);
    virtual void getProgramName(char* name);
    virtual bool getProgramNameIndexed(INT32 category, INT32 index, char* name);
    virtual void setParameter(INT32 index, float value);
    virtual float getParameter(INT32 index);
    virtual void getParameterLabel(INT32 index, char* label);
    virtual void getParameterDisplay(INT32 index, char* text);
    virtual void getParameterName(INT32 index, char* text);
    virtual void suspend();

    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);
    virtual INT32 getVendorVersion() { return 1000; }

protected:
    float fParam0;
    float fParam1;
    float fParam2;
    float fParam3;
    float fParam4;
    float fParam5;
    float fParam6;

    INT32 buffersize;
    float samplerate;

    float* buffer;               //delay
    INT32 size, ipos, ldel, rdel; //delay max time, pointer, left time, right time
    float wet, dry, fbk;         //wet & dry mix
    float lmix, hmix, fil, fil0; //low & high mix, crossover filter coeff & buffer

    char programName[32];
};

#endif
