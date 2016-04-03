#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../include/clientlist.h"
#include "../include/funcserver.h"

pthread_t get_tid(client_list *l, char *name){
  client_list *aux = l;
  while(aux != NULL && strcmp(aux->name, name) != 0){
    aux = aux->next;
  }
  if(aux == NULL){
    return NULL;
  }
  return aux->tid;
}

client_list* get_client(client_list* l, char *name){
  client_list *aux = l;
  while(aux != NULL){
    if(strcmp(aux->name, name) == 0){
      return aux;
    }
    aux = aux->next;
  }
  return NULL;
}

client_list* create_client_list(pthread_t tid, int sock, client_list* next){
  client_list *res = (client_list*) malloc(sizeof(client_list));
  res->tid = tid;
  res->socket = sock;
  res->score = 0;
  res->proposition = -1;
  res->next = next;
  return res;
}

client_list* add_client(client_list *l, client_list *client){
  client->next = l;
  return client;
}

client_list* add_new_client(client_list *l, pthread_t tid, int sock){
  client_list *e = create_client_list(tid, sock, l);
  l = e;
  return l;
}

client_list* suppr_client(client_list *l, char* name){
  client_list *aux;
  /* si la liste est vide */
  if(l == NULL){
    /*fprintf(stderr, "suppr_client : tentative de suppression sur une liste vide\n");*/
    return NULL;
  }
  /* si le premier élément correspond au tid recherché */
  if(strcmp(l->name, name) == 0){
    l = l->next;
    return l;
  }
  /* recherche de l'élément à supprimer */
  aux = l;
  while(aux->next != NULL && strcmp(aux->next->name, name) != 0){
    aux = aux->next;
  }

  /* s'il n'y a pas de next (l'élément recherché n'est pas dans la liste) */
  if(aux->next == NULL){
    fprintf(stderr, "suppr_client : element pas dans la liste\n");
    return l;
  }

  /* l'élément suivant correspond a l'element recherché */
  aux->next = aux->next->next;
  printf("suppr client : ");
  print_client_list(l);
  return l;
}

client_list* add_name_client(client_list *l, int socket, char *name){
  client_list *aux = l;
  if(aux == NULL){
    fprintf(stderr, "Client list empty.\n");
    return l;
  }
  while(aux->socket != socket){
    aux = aux->next;
  }
  if(aux == NULL){
    fprintf(stderr, "add_name_client: l is NULL\n");
    return l;
  }
  aux->name = (char*) malloc(strlen(name)*sizeof(char));
  strcpy(aux->name, name);
  return l;
}



int client_exists(client_list *l, char *name){
  client_list *aux = l;
  while(aux != NULL){
    if(aux->name != NULL){
      if(strcmp(aux->name, name) == 0)
	return 1;
    }
    aux = aux->next;
  }
  return 0;
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
  client_list *aux;
  aux = l;
  while(aux != NULL){
    printf("[%Lu, %s, %d] -> ",(unsigned long) aux->tid, aux->name, aux->score);
    aux = aux->next;
  }
  printf("NULL\n");
}

void destroy_client_list(client_list *l){
  client_list *aux = l;
  if(aux->next == NULL){
    free(aux->name);
    free(aux);
  }
  else{
    destroy_client_list(aux->next);
    free(aux->name);
    free(aux);
  }
}
