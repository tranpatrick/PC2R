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
#include "../include/server.h"
#include "../include/clientlist.h"
#include "../include/funcserver.h"

#define MAX_CO 100
#define PORT 2016

int main(){
  int sock_connexion, sock_com;
  pthread_t tid;
  enigme = (char*) malloc(40*sizeof(char));
  
  struct sockaddr_in sin; /* Nom de la socket de connexion */
  struct sockaddr_in exp; /* Nom de la socket de communication pour un client */

  int fromlen = sizeof(exp);

  /* Liste de clients vide */
  /*  clients = NULL;
  file_attente = NULL;*/

  /* Phase est nulle */
  set_phase("nulle");
  
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
