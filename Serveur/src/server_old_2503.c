#define _POSIX_SOURCE 1
#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "../include/clientlist.h"
#include "../include/funcserver.h"

#define MAX_SIZE 256
#define MAX_CO 100
#define PORT 2016

#define PHASE_NULL -1
#define PHASE_REFLEXION 1
#define PHASE_ENCHERE 2
#define PHASE_RESOLUTION 3

client_list *clients = NULL; /* Liste des clients connectés */
client_list *file_attente = NULL;

/*
  if(strcmp(cmd, "SORT") == 0){
  sorti(clients, user);
  pthread_mutex_lock(&mutex_clients);
  clients = suppr_client(clients, pthread_self());
  pthread_mutex_unlock(&mutex_clients);
  close(sock_com);
  pthread_exit((void*) 0);
  }
*/


void* thread_att_connexion(){
 
  set_phase("connexion");
 
  msg_broadcast(clients, "WAITCO/\n");
   
  sleep(10);

  pthread_mutex_lock(&mutex_cond);
  pthread_cond_broadcast(&cond_fin_connexion);
  pthread_mutex_unlock(&mutex_cond);
  
  set_phase("reflexion");
   
  msg_broadcast(clients, "ENDWAITCO/\n");
    
  pthread_exit((void*) 0);
}

/* traitement à effectuer pour chaque client */
void* client_thread(void *arg){
  int sock_com = (int) arg;
  int nb_lus;
  pthread_t tid;
  char cmd[20], user[MAX_SIZE], coups[20];
  char buffer[MAX_SIZE];

  pthread_mutex_lock(&mutex_clients);
  clients = add_client(file_attente, pthread_self(), sock_com);
  pthread_mutex_unlock(&mutex_clients);

  while((nb_lus = read(sock_com, &buffer, MAX_SIZE)) > 0){
    sscanf(buffer, "%[^/]/%[^/]/%[^/]/", cmd, user, coups);
    printf("cmd: %s\n", cmd);
    printf("user: %s\n", user);
    printf("coups: %s\n", coups);

    memset(buffer, '\0', MAX_SIZE);
    
    /* CONNEX */
    if(strcmp(cmd, "CONNEX") == 0){
      bienvenue(clients, user, sock_com);
      /*printf("nb clients : %d\n", client_list_length(clients));*/
      
      if(get_phase() == PHASE_NULL){
	pthread_mutex_lock(&mutex_clients);
	/* ICI VOIR SI TOUS LES CLIENTS ONT DES NOMS */
	if(client_list_length(clients) == 2){
	  pthread_create(&tid, NULL, thread_att_connexion, NULL);
	}
	pthread_mutex_unlock(&mutex_clients);
      }
      pthread_mutex_unlock(&mutex_phase);
    
      if(get_phase() == PHASE_CONNEXION){
	pthread_mutex_lock(&mutex_cond);
	pthread_cond_wait(&cond_fin_connexion, &mutex_cond);
	pthread_mutex_unlock(&mutex_cond);
      }

      if(get_phase() > 0){
	pthread_mutex_lock(&mutex_cond);
	pthread_cond_wait(&cond_nouveau_tour, &mutex_cond);
	pthread_mutex_unlock(&mutex_cond);
      }
      
    }

    if(strcmp(cmd, "SORT") == 0){
      sorti(clients, user);
      pthread_mutex_lock(&mutex_clients);
      clients = suppr_client(clients, pthread_self());
      pthread_mutex_unlock(&mutex_clients);
      close(sock_com);
      pthread_exit((void*) 0);
    }    
    
    memset(cmd, '\0', 20);
    memset(user, '\0', MAX_SIZE);
    memset(coups, '\0', 20);
    memset(buffer, '\0', MAX_SIZE);
  }
  pthread_exit((void*) 0);
}

int main(){
  int sock_connexion, sock_com;
  pthread_t tid;
  
  struct sockaddr_in sin; /* Nom de la socket de connexion */
  struct sockaddr_in exp; /* Nom de la socket de communication pour un client */

  int fromlen = sizeof(exp);

  /* Liste de clients vide */
  clients = NULL;
  file_attente = NULL;

  /* Phase est nulle */
  set_phase("null");
  
  /* Création de la socket */
  if((sock_connexion = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket");
    return EXIT_FAILURE;
  }

  /* Initialisation de la variable sin */
  memset((char *) &sin, 0, sizeof(sin));
  sin.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY = machine locale */
  sin.sin_port = htons(PORT);
  sin.sin_family = AF_INET;

  /* Pour réutiliser le port et l'adresse */
  if(setsockopt(sock_connexion, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
    perror("setsockopt");
  
  /* Nommage */
  if(bind(sock_connexion, (struct sockaddr*) &sin, sizeof(sin)) < 0){
    perror("bind");
    return EXIT_FAILURE;
  }

  /* Le serveur est pret à attendre */
  listen(sock_connexion, MAX_CO);

  for(;;){
    printf("Waiting for players to connect ...\n");
    
    /* Connexion d'un client */
    if((sock_com = accept(sock_connexion, (struct sockaddr*) &exp, (socklen_t*) &fromlen)) == -1){
      perror("accept");
      return EXIT_FAILURE;
    }
    
    /* Création d'une thread qui va gérer le client */  
    if(pthread_create(&tid, NULL, client_thread, (void *) sock_com) != 0){
      perror("pthread_create");
      return EXIT_FAILURE;
    }
    

  }

  

  return 0;
}
