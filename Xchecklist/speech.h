#ifndef SPEECH__H
#define SPEECH__H


#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

bool init_speech();
void say(const char *text);
void close_speech();
void cleanup_speech();
bool speech_active();
bool spoken(float elapsed);
extern char xcbuf[];

#ifdef __cplusplus
}
#endif
#endif
