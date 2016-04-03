#ifndef _CLIENT_LIST_H_
#define _CLIENT_LIST_H_

#include <pthread.h>

typedef struct client_list{
  pthread_t tid;
  int socket;
  char *name;
  int score; /* à remettre à 0 pour chaque nouvelle session */
  int proposition; /* à remettre à -1 pour chaque nouveau tour */
  struct client_list *next;
} client_list;

pthread_t get_tid(client_list *l, char *name);
client_list* get_client(client_list *l, char *name);
client_list* create_client_list(pthread_t tid, int sock, client_list* next);
client_list* add_client(client_list *l, client_list *client);
client_list* add_new_client(client_list *l, pthread_t tid, int sock);
client_list* suppr_client(client_list *l, char *name);
client_list* add_name_client(client_list *l, int socket, char *name);
int client_exists(client_list *l, char *name);
int client_list_length(client_list *l);
void print_client_list(client_list *l);
void destroy_client_list(client_list *l);

#endif
