#include "XPLMUtilities.h"
#include "speech.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"



#ifndef __in
 #define __in
#endif
#ifndef __out
 #define __out
#endif
#ifndef __inout
 #define __inout
#endif
#ifndef __in_opt
 #define __in_opt
#endif
#ifndef __out_opt
 #define __out_opt
#endif
#ifndef __inout_opt
 #define __inout_opt
#endif
#ifndef __in_ecount
 #define __in_ecount(x)
#endif
#ifndef __out_ecount
 #define __out_ecount(x)
#endif
#ifndef __inout_ecount
 #define __inout_ecount(x)
#endif
#ifndef __in_bcount
 #define __in_bcount(x)
#endif
#ifndef __out_bcount
 #define __out_bcount(x)
#endif
#ifndef __inout_bcount
 #define __inout_bcount(x)
#endif
#ifndef __out_xcount
 #define __out_xcount(x)
#endif
#ifndef __deref_opt_out
 #define __deref_opt_out
#endif
#ifndef __deref_out
 #define __deref_out
#endif
#ifndef __out_ecount_opt
 #define __out_ecount_opt(x)
#endif
#ifndef __in_bcount_opt
 #define __in_bcount_opt(x)
#endif
#ifndef __out_bcount_opt
 #define __out_bcount_opt(x)
#endif
#ifndef __deref_out_opt
 #define __deref_out_opt
#endif
#ifndef __in_z
 #define __in_z
#endif
#ifndef __out_ecount_part
 #define __out_ecount_part(x,y)
#endif
#ifndef __in_ecount_opt
 #define __in_ecount_opt(x)
#endif

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
