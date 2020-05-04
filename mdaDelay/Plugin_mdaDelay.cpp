#include "mdaDelay.h"

#include <vector>

namespace MDADelay {
    enum Param {
        P_LDEL,
        P_RRAT,
        P_FB,
        P_TONE,
        P_WM,
        P_OUT,
        P_NUM
    };

    struct EffectData {

        mdaDelay* plugin;
        float p[P_NUM];

    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition) {
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "delay", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_LDEL);
        RegisterParameter(definition, "ratio", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_RRAT);
        RegisterParameter(definition, "feedback", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_FB);
        RegisterParameter(definition, "tone", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_TONE);
        RegisterParameter(definition, "wm", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_WM);
        RegisterParameter(definition, "out level", "%", 0.0f, 1.0f, 0.5f, 100, 1, P_OUT);
        return numparams;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state) {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));

        effectdata->plugin = new mdaDelay(state->samplerate, state->dspbuffersize);

        state->effectdata = effectdata;
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        for (int i = 0; i < 6; ++i) {
            effectdata->plugin->setParameter(i, 0.5f);
        }
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state) {
        delete state->GetEffectData<EffectData>()->plugin;
        delete state->effectdata;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value) {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        data->plugin->setParameter(index, value);
        data->p[index] = value;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char* valuestr) {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
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

        memset(outbuffer, 0, outchannels * length * sizeof(float));

        data->plugin->processReplacing(inbuffer, outbuffer, length);

        return UNITY_AUDIODSP_OK;
    }
}
