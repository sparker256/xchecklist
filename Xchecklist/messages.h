#ifndef MESSAGE__H
#define MESSAGE__H
#include <stdbool.h>

struct message_wrap_t;
typedef struct message_wrap_t *message_t;

bool new_message(message_t *msg);
void dispose_message(message_t *msg);

bool msg_from_str(const char* str, message_t msg);
const char *str_from_msg(message_t msg);
bool write_message(int fd, message_t msg);
bool read_message(int fd, message_t msg);

#endif
