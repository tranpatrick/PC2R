#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../include/clientlist.h"

client_list* create_client_list(pthread_t tid, int sock, client_list* next){
  client_list *res = malloc(sizeof(client_list));
  res->tid = tid;
  res->socket = sock;
  res->next = next;
  return res;
}

client_list* add_client(client_list *l, pthread_t tid, int sock){
  client_list *e = create_client_list(tid, sock, NULL);
  client_list *aux;
  if(l == NULL){
    return e;
  }
  aux = l;
  while(aux->next != NULL){
    aux = aux->next;
  }
  aux->next = e;
  return l;
}

client_list* suppr_client(client_list *l, char* name){
  client_list *res, *aux, *tmp;
  /* si la liste est vide */
  if(l == NULL){
    /*sprintf(stderr, "Attention, tentative de suppression sur une liste vide\n");*/
    return NULL;
  }
  /* si le premier élément correspond au tid recherché */
  if(strcmp(l->name, name) == 0){
    res = l->next;
    free(l);
    return res;
  }
  /* recherche de l'élément à supprimer */
  aux = l;
  while(aux->next != NULL && strcmp(aux->next->name, name) != 0){
    aux = aux->next;
  }
  /* s'il n'y a pas de next (l'élément recherché n'est pas dans la liste) */
  if(aux->next == NULL)
    return l;
  /* l'élément suivant correspond au tid recherché */
  tmp = aux->next;
  aux->next = aux->next->next;
  free(tmp->name);
  free(tmp);
  return l;
}

void add_name_client(client_list *l, int socket, char *name){
  if(l == NULL){
    sprintf(stderr, "Client list empty.\n");
    exit(1);
  }
  while(l->socket == socket){
    l = l->next;
  }
  if(l == NULL){
    sprintf(stderr, "add_name_client: l is NULL\n");
    exit(1);
  }
  l->name = (char*) malloc(strlen(name)*sizeof(char));
  strcpy(l->name, name);
}

int client_exists(client_list *l, char *name){
  while(l != NULL){
    if(l->name != NULL){
      if(strcmp(l->name, name) == 0)
	return 1;
    }
    l = l->next;
  }
  return 0;
}

void transfert(client_list *l1, client_list *l2, char *name){
  if(client_list *l1 == NULL){
    sprintf(stderr, "transfert: liste source vide\n");
    return;
  }
  clinet_list *l1
}

int client_list_length(client_list *l){
  int res = 0;
  client_list *aux = l;
  while(aux != NULL){
    res++;
    aux = aux->next;
  }
  return res;
}
  
void print_client_list(client_list *l){
  while(l != NULL){
    printf("[%Lu, %s] -> ",(unsigned long) l->tid, l->name);
    l = l->next;
  }
  printf("NULL\n");
}

void destroy_client_list(client_list *l){
  if(l->next == NULL){
    free(l->name);
    free(l);
  }
  else{
    destroy_client_list(l->next);
    free(l->name);
    free(l);
  }
}
