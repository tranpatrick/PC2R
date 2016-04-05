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

#define PORT 2016

int main(){
  int sock_connexion, sock_com;
  pthread_t tid;
    
  struct sockaddr_in sin; /* Nom de la socket de connexion */
  struct sockaddr_in exp; /* Nom de la socket de communication pour un client */

  int fromlen = sizeof(exp);

  /* initialisation de la variable plateaux */
  plateaux = (char**) malloc(3*sizeof(char*));
  plateaux[0] = strdup("(0,3,D)(0,11,D)(1,13,G)(1,13,H)(2,5,D)(2,5,B)(2,9,D)(2,9,B)(4,0,B)(4,2,D)(4,2,H)(4,15,H)(5,7,G)(5,7,B)(5,14,G)(5,14,B)(6,1,G)(6,1,H)(6,7,B)(6,8,B)(6,11,H)(6,11,D)(7,6,D)(7,9,G)(8,5,H)(8,5,D)(8,6,D)(8,9,G)(9,1,B)(9,1,D)(9,12,H)(9,12,D)(10,4,G)(10,4,B)(10,15,H)(12,0,H)(12,9,G)(12,9,H)(13,5,H)(13,5,D)(13,14,G)(13,14,B)(14,3,G)(14,3,H)(14,11,D)(14,11,B)(15,6,D)(15,13,D)(8,7,B)(8,8,B)");
  plateaux[1] = strdup("(0,4,D)(0,9,D)(1,6,D)(1,6,B)(1,14,H)(1,14,G)(2,1,H)(2,1,G)(2,11,G)(2,11,B)(3,15,B)(5,0,B)(5,6,H)(5,6,D)(6,3,G)(6,3,B)(6,7,B)(6,8,B)(6,13,B)(6,13,D)(7,6,D)(7,9,G)(7,10,H)(7,10,D)(8,5,B)(8,5,D)(8,6,D)(8,9,G)(8,12,G)(8,12,H)(9,7,H)(9,8,H)(9,10,G)(9,10,B)(10,2,H)(10,2,D)(10,15,B)(11,0,B)(11,9,B)(11,9,D)(12,14,H)(12,14,D)(13,4,G)(13,4,H)(14,6,G)(14,6,B)(14,13,G)(14,13,H)(15,4,D)(15,10,D)");
  plateaux[2] = strdup("(0,1,D)(0,9,D)(1,4,G)(1,4,H)(1,14,G)(1,14,H)(2,1,H)(2,1,D)(2,11,G)(2,11,B)(3,6,B)(3,6,D)(3,15,B)(5,0,B)(6,3,G)(6,3,B)(6,7,B)(6,8,B)(6,13,B)(6,13,D)(7,6,D)(7,9,G)(7,10,H)(7,10,D)(8,5,H)(8,5,D)(8,6,D)(8,9,G)(9,1,B)(9,1,D)(9,7,H)(9,8,H)(9,14,G)(9,14,H)(10,4,G)(10,4,B)(10,11,G)(10,11,B)(10,15,B)(12,0,H)(12,9,H)(12,9,D)(13,5,H)(13,5,D)(14,3,G)(14,3,H)(14,13,B)(14,13,D)(15,7,G)(15,11,D)");

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
  listen(sock_connexion, SOMAXCONN);

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
