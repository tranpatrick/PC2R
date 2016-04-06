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

#define MAX_SIZE 256

#define PHASE_NULL -1
#define PHASE_REFLEXION 1
#define PHASE_ENCHERE 2
#define PHASE_RESOLUTION 3
#define TEMPS_REFLEXION 300
#define TEMPS_ENCHERE 30
#define TEMPS_RESOLUTION 60

/* Permet d'unlock un mutex lorsqu'une thread qui attendait dessus a été cancel (avec pthread_cleanup_push) */
void unlock_mutex(void *arg){
  int num = (int) arg;
  switch(num){
  case 1:
    pthread_mutex_unlock(&mutex_cond_reflexion);
    break;
  case 2:
    pthread_mutex_unlock(&mutex_cond_enchere);
    break;
  case 3:
    pthread_mutex_unlock(&mutex_cond_resolution);
    pthread_mutex_unlock(&mutex_trop_long);
    break;
  }
}

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
    pthread_mutex_lock(&mutex_trop_long);
    trop_long = 1;
    pthread_mutex_unlock(&mutex_trop_long);  
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
  client_list *prop;
  
  printf("PHASE DE RESOLUTION\n\n");

  /* Permet de relacher le mutex quand la thread est canceled */
  pthread_cleanup_push(unlock_mutex, 3);
  
  /* Si aucun joueur actif, FINRESO directement */
  pthread_mutex_lock(&mutex_joueur_actif);
  prop = joueur_actif;
  pthread_mutex_unlock(&mutex_joueur_actif);
  if(prop == NULL){
    finreso();
  }

  /* Mise à 0 du "booleen" permettant de savoir si le timer a été dépassé ou non */
  pthread_mutex_lock(&mutex_trop_long);
  trop_long = 0;
  pthread_mutex_unlock(&mutex_trop_long);  
  
  /* Lancement du timer */
  if(pthread_create(&tid_timer, NULL, thread_timer, (void *) TEMPS_RESOLUTION) != 0){
    perror("pthread_create thread_timer in thread_resolution");
    exit(1);
  }

  /* Attente du signal en provenance du timer ou du client (après avoir entré la solution) */
  pthread_mutex_lock(&mutex_cond_resolution);
  pthread_cond_wait(&cond_resolution, &mutex_cond_resolution);
  pthread_mutex_unlock(&mutex_cond_resolution);

  /* Annulation de la thread timer dans tous les cas */
  pthread_cancel(tid_timer);

  /* Si aucune solution n'a été entré dans le temps imparti, TROPLONG */
  pthread_mutex_lock(&mutex_trop_long);  
  if(trop_long == 1)
    troplong();
  pthread_mutex_unlock(&mutex_trop_long);

  /* Permet d'enlever la fonction unlock_mutex dans la pile d'appel */
  pthread_cleanup_pop(1);
  
  pthread_exit(NULL);
}

/* Gère la phase d'enchere */
void* thread_enchere(void *arg){

  printf("PHASE D'ENCHERE\n\n");
  
  /* Permet de relacher le mutex quand la thread est canceled */
  pthread_cleanup_push(unlock_mutex, 2);
  
  /* Phase courante = PHASE_ENCHERE */
  set_phase("enchere");

  /* Lancement du timer */
  if(pthread_create(&tid_timer, NULL, thread_timer, (void *) TEMPS_ENCHERE) != 0){
    perror("pthread_create thread_timer in thread_enchere");
    exit(1);
  }

  /* Attente de la du signal en provenance du timer */
  pthread_mutex_lock(&mutex_cond_enchere);
  pthread_cond_wait(&cond_enchere, &mutex_cond_enchere);
  pthread_mutex_unlock(&mutex_cond_enchere);

  /* Annulation de la thread timer dans tous les cas */
  pthread_cancel(tid_timer);

  /* Fin de la phase d'enchere */
  finenchere();

  /* Initialisation de la phase de résolution */  
  pthread_mutex_lock(&mutex_phase);
  set_phase("resolution");
  pthread_mutex_unlock(&mutex_phase);

  /* Mise à jour du joueur actif */
  update_joueur_actif();
  
  /* Création de la thread de résolution */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_resolution, NULL) != 0){
    perror("pthread_create thread_resolution");
    exit(1);
  }
  pthread_mutex_unlock(&mutex_phase);
  
  /* Permet d'enlever la fonction unlock_mutex dans la pile d'appel */
  pthread_cleanup_pop(1);
  
  pthread_exit(NULL);
}

/* Gère la phase de réflexion */
void* thread_reflexion(void *arg){
  printf("PHASE DE REFLEXION\n\n");

  /* Permet de relacher le mutex quand la thread est canceled */
  pthread_cleanup_push(unlock_mutex, 1);
  
  /* Phase courante = PHASE_REFLEXION */
  set_phase("reflexion");
  
  /* Création de la thread timer */
  if(pthread_create(&tid_timer, NULL, thread_timer, (void *) TEMPS_REFLEXION) != 0){
    perror("pthread_create thread_timer in thread_reflexion");
    exit(1);
  }

  /* Attente du signal en provenance du timer ou d'un client (après avoir saisi un nombre de coups) */
  pthread_mutex_lock(&mutex_cond_reflexion);
  pthread_cond_wait(&cond_reflexion, &mutex_cond_reflexion);
  pthread_mutex_unlock(&mutex_cond_reflexion); 

  /* Annulation de la thread timer dans tous les cas */
  pthread_cancel(tid_timer);
   
  /* Fin de la phase de reflexion */
  pthread_mutex_lock(&mutex_joueur_solution);
  if(joueur_solution == 0)
    finreflexion();
  pthread_mutex_unlock(&mutex_joueur_solution);
 
  /* Lancement de la phase d'enchere */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_enchere, (void *) NULL) != 0){
    perror("pthread_create thread_enchere in thread_reflexion");
    exit(1);
  }
  pthread_mutex_unlock(&mutex_phase);

  /* Permet d'enlever la fonction unlock_mutex dans la pile d'appel */
  pthread_cleanup_pop(1);
  
  pthread_exit(NULL);
}

/* traitement à effectuer pour chaque client */
void* client_thread(void *arg){
  int sock_com = (int) arg;
  pthread_t tid;
  int nb_clients;
  char cmd[20], user[MAX_SIZE], coups[200];
  int coups_int;
  char buffer[MAX_SIZE];

  pthread_mutex_lock(&mutex_attente);
  file_attente = add_new_client(file_attente, pthread_self(), sock_com);
  pthread_mutex_unlock(&mutex_attente);

  while(read(sock_com, &buffer, MAX_SIZE) > 0){
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
      printf("nb client = %d\n", nb_clients);
      pthread_mutex_unlock(&mutex_clients);
      if(nb_clients == 2 && get_phase() == -1){
	/* Lorsque le nombre de clients est de 2 et que le jeu n'est pas dans une phase, lancer une session */
	session();
      }else if(get_phase() != -1){
	/* Pour tout ceux qui se connectent en milieu de tour, envoyer juste le plateau */
	session_attente(sock_com);
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
        vainqueur();
	/* Relance d'une session si la liste d'attente est non vide */
	nouvellesession();
      }
      pthread_exit(NULL);
    }

    /* SOLUTION */
    else if(strcmp(cmd, "SOLUTION") == 0){
      if(get_phase() == PHASE_REFLEXION){
	/* Si on est dans la phase de réflexion alors c'est une proposition de nombre de coups */
	pthread_mutex_lock(&mutex_conflit);
	tuastrouve(user, coups_int);
	pthread_mutex_unlock(&mutex_conflit);
      }else if(get_phase() == PHASE_RESOLUTION){
	/* Si on est dans la phase de résolution alors c'est une proposition de solution */
	/* Annulation du timer */
	pthread_cancel(tid_timer);
	pthread_mutex_lock(&mutex_trop_long);
	trop_long = 0;
	pthread_mutex_unlock(&mutex_trop_long);
	/* Signalement à la thread phase résolution que la solution a été proposée */
	pthread_mutex_lock(&mutex_cond_resolution);
	pthread_cond_signal(&cond_resolution);
	pthread_mutex_unlock(&mutex_cond_resolution);

	sasolution(user, coups);
      }
    }

    /* ENCHERE */
    else if(strcmp(cmd, "ENCHERE") == 0){
      if(get_phase() == PHASE_ENCHERE){
	/* Protection avec un mutex car deux enchère peuvent arriver en meme temps mais ne peuvent etre traitées au meme moment (soucis de cohérence) */
	pthread_mutex_lock(&mutex_conflit);
	traitement_enchere(user, coups_int);
	pthread_mutex_unlock(&mutex_conflit);
      }
    }

    /* SEND */
    else if(strcmp(cmd, "SEND") == 0){
      traitement_chat(user, coups);
    }

    else{
      printf("Commande reçu au mauvais moment !\n");
    }

    memset(cmd, '\0', 20);
    memset(user, '\0', MAX_SIZE);
    memset(coups, '\0', 200);
    memset(buffer, '\0', MAX_SIZE);
  }
  pthread_exit((void*) 0);
 

  pthread_exit((void*) 0);
}

