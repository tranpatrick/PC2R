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

/* Gère les timers pour les phases */
void* thread_timer(void *arg){
  int sec = (int) arg;
  int phase = get_phase();
  sleep(sec);
  switch(phase){
  case PHASE_REFLEXION:  
    pthread_mutex_lock(&mutex_cond_reflexion);
    pthread_cond_signal(&cond_reflexion);
    pthread_mutex_unlock(&mutex_cond_reflexion);
    break;
  case PHASE_ENCHERE:
    pthread_mutex_lock(&mutex_cond_enchere);
    pthread_cond_signal(&cond_enchere);
    pthread_mutex_unlock(&mutex_cond_enchere);
    break;
  case PHASE_RESOLUTION:
    pthread_mutex_lock(&mutex_cond_resolution);
    pthread_cond_signal(&cond_resolution);
    pthread_mutex_unlock(&mutex_cond_resolution);
    break;
  default:
    break;
  }
  pthread_exit(NULL);
}

/* Gère la phase de résolution */
void *thread_resolution(void *arg){
  printf("PHASE DE REOSLUTION\n\n");

  set_phase("resolution");

  
  
}

/* Gère la phase d'enchere */
void* thread_enchere(void *arg){
  printf("PHASE D'ENCHERE\n\n");
  
  set_phase("enchere");

  if(pthread_create(&tid_timer, NULL, thread_timer, (void *) 30) != 0){
    perror("pthread_create thread_timer in thread_enchere");
    return EXIT_FAILURE;
  }
  pthread_mutex_lock(&mutex_cond_enchere);
  pthread_cond_wait(&cond_enchere, &mutex_cond_enchere);
  pthread_mutex_unlock(&mutex_cond_enchere);

  /* Fin de la phase d'enchere */
  finenchere();
}

/* Gère la phase de réflexion */
void* thread_reflexion(void *arg){

  printf("PHASE DE REFLEXION\n\n");
  
  /* Nouveau tour */
  set_phase("reflexion");
  tour(enigme);
  
  if(pthread_create(&tid_timer, NULL, thread_timer, (void *) 300) != 0){
    perror("pthread_create thread_timer in thread_reflexion");
    return EXIT_FAILURE;
  }
  pthread_mutex_lock(&mutex_cond_reflexion);
  pthread_cond_wait(&cond_reflexion, &mutex_cond_reflexion);
  pthread_mutex_unlock(&mutex_cond_reflexion);

  /* Fin de la phase de reflexion */
  pthread_mutex_lock(&mutex_joueur_solution);
  if(joueur_solution == 0)
    finreflexion();
  pthread_mutex_unlock(&mutex_joueur_solution);
  
  /* Lancement de la phase d'enchere */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_enchere, (void *) NULL) != 0){
    perror("pthread_create thread_enchere in thread_reflexion");
    return EXIT_FAILURE;
  }
  pthread_mutex_unlock(&mutex_phase);
  
  pthread_exit(NULL);
}

/* gère la reception des messages d'un client */
void* thread_reception(void *arg){
  int sock_com = (int) arg;
  int nb_lus, nb_clients;
  char cmd[20], user[MAX_SIZE], coups[200];
  int coups_int;
  char buffer[MAX_SIZE];
  while((nb_lus = read(sock_com, &buffer, MAX_SIZE)) > 0){
    sscanf(buffer, "%[^/]/%[^/]/%[^/]/", cmd, user, coups);
    printf("\ncmd: %s\n", cmd);
    printf("user: %s\n", user);
    printf("coups: %s\n", coups);

    coups_int = atoi(coups);
    
    memset(buffer, '\0', MAX_SIZE);

    /* CONNEXION */
    if(strcmp(cmd, "CONNEXION") == 0){
      bienvenue(user, sock_com);

      /* Si le nombre de joueurs est au moins 2, lancer session et timer */
      pthread_mutex_lock(&mutex_clients);
      nb_clients = client_list_length(clients);
      pthread_mutex_unlock(&mutex_clients);
      if(nb_clients == 2){
	session(plateau);
	/* Création d'une thread qui va gérer la phase de reflexion */
	pthread_mutex_lock(&mutex_phase);
	if(pthread_create(&tid_phase, NULL, thread_reflexion, NULL) != 0){
	  perror("pthread_create thread_reflexion");
	  return EXIT_FAILURE;
	}
	pthread_mutex_unlock(&mutex_phase);
      }
    }

    /* DECONNEXION */
    else if(strcmp(cmd, "SORT") == 0){
      sorti(user);
      /* Si le nombre de joueur descend en dessous de 2, fin de la session */
      pthread_mutex_lock(&mutex_clients);
      nb_clients = client_list_length(clients);
      pthread_mutex_unlock(&mutex_clients);
      if(nb_clients == 1){
        
      }
      pthread_exit(NULL);
    }

    /* SOLUTION */
    else if(strcmp(cmd, "SOLUTION") == 0){
      if(get_phase() == PHASE_REFLEXION)
	tuastrouve(user, coups_int);
      else
	sasolution(user, coups);
    }

    /* ENCHERE */
    else if(strcmp(cmd, "ENCHERE") == 0){
      traitement_enchere(user, coups_int);
    }

 

    /*printf("Listes : \n");
    pthread_mutex_lock(&mutex_attente);
    print_client_list(file_attente);
    pthread_mutex_unlock(&mutex_attente);
    pthread_mutex_lock(&mutex_clients);
    print_client_list(clients);
    pthread_mutex_unlock(&mutex_clients);
    printf("\n"); */
    
    memset(cmd, '\0', 20);
    memset(user, '\0', MAX_SIZE);
    memset(coups, '\0', 20);
    memset(buffer, '\0', MAX_SIZE);
  }
  pthread_exit((void*) 0);
 
}

/* traitement à effectuer pour chaque client */
void* client_thread(void *arg){
  int sock_com = (int) arg;
  int nb_lus;
  pthread_t tid;
  char cmd[20], user[MAX_SIZE], coups[20];
  char buffer[MAX_SIZE];

  pthread_mutex_lock(&mutex_attente);
  file_attente = add_new_client(file_attente, pthread_self(), sock_com);
  pthread_mutex_unlock(&mutex_attente);

  /* Création d'une thread qui va gérer la réception de message */  
  if(pthread_create(&tid, NULL, thread_reception, (void *) sock_com) != 0){
    perror("pthread_create");
    return EXIT_FAILURE;
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
