#include "XPLMUtilities.h"
#include <speech.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include <sapi.h>

static bool active = false;

ISpVoice * pVoice = NULL;
wchar_t* pwszString = NULL;
int iRequiredSize = 0;

HRESULT hr,hrinit;
SPVOICESTATUS status;

static size_t speech_length = 0;

bool init_speech()
{
    hrinit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // hr == S_OK
    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if (SUCCEEDED(hrinit) && SUCCEEDED(hr)) {
        active = true;
        return true;
    } else {
        active = false;
        return false;
    }
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
    if (voice_state){
        pVoice = NULL;
        ::CoUninitialize();
    }

    // ::CoUninitialize();
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
    (void)elapsed;
    hr = pVoice->GetStatus(& status, NULL);
    if(status.dwRunningState == SPRS_DONE) {
        return true;
    } else {
        return false;
    }
}
