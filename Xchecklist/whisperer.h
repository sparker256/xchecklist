#ifndef WHISPERER__H
#define WHISPERER__H

#include <stdbool.h>

bool whisperer_init(const char *prog);
void whisperer_close(void);
bool whisper(const char *str);
bool whispered(void);

#endif
