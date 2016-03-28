#ifndef _CLIENT_LIST_H_
#define _CLIENT_LIST_H_

#include <pthread.h>

typedef struct client_list{
  pthread_t tid;
  int socket;
  char *name;
  struct client_list *next;
} client_list;

client_list* create_client_list(pthread_t tid, int sock, client_list* next);
client_list* add_client(client_list *l, pthread_t tid, int sock);
client_list* suppr_client(client_list *l, char *name);
void add_name_client(client_list *l, int socket, char *name);
void transfert(client_list *l1, client_list *l2, char *name);
int client_exists(client_list *l, char *name);
int client_list_length(client_list *l);
void print_client_list(client_list *l);
void destroy_client_list(client_list *l);

#endif
