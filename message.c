#include <stdio.h>

struct message {
  char message[1024];
  char is_last;
  struct message *next;
};

int main() {
  int index = 0;
  for (; index < 10; index++) {
    struct message *message_ = NULL;
    message_ = malloc(sizeof(message));
    sprintf(message_.message, "This is a test, %i", index);
  }
}
