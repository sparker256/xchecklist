#include "XPLMUtilities.h"
#include <speech.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include <sapi.h>

static const float speech_speed = 12.0f; //characters per second

static bool active = false;

ISpVoice * pVoice = NULL;
wchar_t* pwszString = NULL;
int iRequiredSize = 0;

HRESULT hr;

static size_t speech_length = 0;

bool init_speech()
{
    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    active = true;
    return true;
}

void say(const char *text)
{
    if(!active){
        return;
    }
    speech_length = strlen(text);
    iRequiredSize = MultiByteToWideChar (CP_ACP, 0, text, -1, NULL, 0);
    pwszString = new wchar_t[iRequiredSize];
    MultiByteToWideChar (CP_ACP, 0, text, -1, pwszString, iRequiredSize);

    if (FAILED(::CoInitialize(NULL)))
        return;

    if( SUCCEEDED( hr )) {
        if (pwszString) {
            hr = pVoice->Speak(pwszString, SPF_ASYNC, NULL); //Specifies that the Speak call should be asynchronous
            delete [] pwszString;
            pwszString = NULL;
        }

    }
    return;
}

void close_speech()
{
    printf("Someone closed the speech!!!\n");
    active = false;
    pVoice->Release();
    pVoice = NULL;
    ::CoUninitialize();
}

void cleanup_speech()
{
}

bool speech_active()
{
    //printf("Speech: %s\n", active?"Active\n":"Inactive");
    return active;
}

bool spoken(float elapsed)
{
    if(!active){
        return true;

    }
    if(elapsed > speech_length / speech_speed){
        return true;
    }else{
        return false;
    }
}
