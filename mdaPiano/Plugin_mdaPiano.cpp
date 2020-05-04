#include "mdaPiano.h"

#include <string>

namespace MDAPiano {


    enum Param {
        P_DECAY,
        P_RELEASE,
        P_HARDNESS,
        P_VTOH,
        P_MUFF,
        P_VTOM,
        P_VELSENSE,
        P_SWIDTH,
        P_POL,
        P_FINE,
        P_RND,
        P_STR,
        P_NUM
    };

    struct EffectData {
        mdaPiano* plugin;
        float p[P_NUM];
    };

    static RingBuffer<8192, MidiEvent> scheduledata;

    static std::string paramNames[NPARAMS];
    static std::string paramLabels[NPARAMS];

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition) {
        int numparams = NPARAMS;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "Env. Decay", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_DECAY);
        RegisterParameter(definition, "Env. Release", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_RELEASE);
        RegisterParameter(definition, "Hardness", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_HARDNESS);
        RegisterParameter(definition, "Vel. to Hard.", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_VTOH);
        RegisterParameter(definition, "Mufflinf filt.", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_MUFF);
        RegisterParameter(definition, "Vel. to Muff.", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_VTOM);
        RegisterParameter(definition, "Vel. Sense", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_VELSENSE);
        RegisterParameter(definition, "Stereo Width", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_SWIDTH);
        RegisterParameter(definition, "Polyphony", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_POL);
        RegisterParameter(definition, "Fine Tuning", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_FINE);
        RegisterParameter(definition, "Rnd Tuning", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_RND);
        RegisterParameter(definition, "Str. Tuning", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_STR);
        return numparams;
    }


    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state) {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));

        effectdata->plugin = new mdaPiano(state->samplerate, state->dspbuffersize);

        state->effectdata = effectdata;
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        for (int i = 0; i < NPARAMS; ++i) {
            effectdata->plugin->setParameter(i, 0.5f);
        }
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state) {
        EffectData* data = state->GetEffectData<EffectData>();
        delete data->plugin;
        delete data;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value) {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= NPARAMS)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        data->plugin->setParameter(index, value);
        data->p[index] = value;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char* valuestr) {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= NPARAMS)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        if (value != NULL)
            * value = data->plugin->getParameter(index);
        if (valuestr != NULL)
            valuestr[0] = 0;
        return UNITY_AUDIODSP_OK;
    }

    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples) {
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int outchannels) {
        EffectData* data = state->GetEffectData<EffectData>();

        MidiEvent ev;
        std::vector<MidiEvent> eventsToProcess;
        std::vector<MidiEvent> pendingEvents;

        while (scheduledata.Peek(ev)) {
            if (!scheduledata.Read(ev)) {
                break;
            }
            if (ev.sample > state->currdsptick + length) {
                pendingEvents.push_back(ev);
            } else {
                ev.sample = ev.sample >= state->currdsptick ? ev.sample - state->currdsptick : 0;
                eventsToProcess.push_back(ev);
            }
        }
        for (auto& ev : pendingEvents) {
            scheduledata.Feed(ev);
        }

        data->plugin->processEvents(eventsToProcess);
        data->plugin->processReplacing(nullptr, outbuffer, inchannels, outchannels, length);

        return UNITY_AUDIODSP_OK;
    }

    void AddEvent(UINT64 sample, UINT32 msg) {
        scheduledata.Feed(MidiEvent{ sample, msg });
    }

    void PlayNote(UINT64 sample, UINT32 note, UINT32 velocity, UINT32 sample_length) {
        scheduledata.Feed(MidiEvent{ sample, 0x90 + note * 0x100 + (velocity & 0x7F) * 0x10000 });
        scheduledata.Feed(MidiEvent{ sample + sample_length, 0x80 + note * 0x100 });
    }

    void MuteAll() {
        scheduledata.writepos = 0;
        scheduledata.readpos = 0;
        for (UINT32 note = 21; note <= 108; ++note) {
            scheduledata.Feed(MidiEvent{ 0, 0x80 + note * 0x100 });
        }
    }
}

extern "C" UNITY_AUDIODSP_EXPORT_API void MDAPiano_MuteAll() {
    MDAPiano::MuteAll();
}

extern "C" UNITY_AUDIODSP_EXPORT_API void MDAPiano_PlayNote(UINT64 sample, UINT32 note, UINT32 velocity, UINT32 sample_length) {
    MDAPiano::PlayNote(sample, note, velocity, sample_length);
}

extern "C" UNITY_AUDIODSP_EXPORT_API void MDAPiano_AddEvent(UINT64 sample, UINT32 msg) {
    MDAPiano::AddEvent(sample, msg);
}
