#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct message_linked_list {
  char data[BUFFER_SIZE];
  unsigned char is_last;
  struct message_linked_list *next;
};

struct message_linked_list* create_message_linked_list() {
    struct message_linked_list *mll = (struct message_linked_list*)\
      malloc(sizeof(struct message_linked_list));
    if (mll == NULL) {
      return NULL;
    }
    memset(mll->data, 0, BUFFER_SIZE);
    mll->is_last = 0;
    mll->next = NULL;
    return mll;
}

int main() {
  int index = 0;
  struct message_linked_list *head = NULL;
  struct message_linked_list *message = NULL;
  struct message_linked_list *previous = NULL;
  for (; index < 10; index++) {
    message = create_message_linked_list();
    if (!head) {
      head = message;
    }
    if (previous) {
      previous->next = message;
    }
    sprintf(message->data, "This is a test, %i", index);
    previous = message;
  }
  struct message_linked_list *working = head;
  while (working) {
    printf("Data: %s\n", working->data);
    working = working->next;
  }
  struct message_linked_list *working_free = head;
  struct message_linked_list *working_free_old = NULL;
  while (working_free) {
    printf("Free: %s\n", working_free->data);
    working_free_old = working_free;
    working_free = working_free->next;
    free(working_free_old);
  }
}
