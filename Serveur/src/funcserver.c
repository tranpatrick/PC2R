#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "../include/funcserver.h"
#include "../include/clientlist.h"

#define MAX_SIZE 256

/* Etats du jeu */
#define PHASE_NULL -1
#define PHASE_REFLEXION 1
#define PHASE_ENCHERE 2
#define PHASE_RESOLUTION 3

pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_attente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_etat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tour = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phase = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_cond_reflexion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cond_enchere = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cond_resolution = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_reflexion = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_enchere = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_resolution = PTHREAD_COND_INITIALIZER;

int etat;
client_list *clients = NULL; /* Liste des clients connectés */
client_list *file_attente = NULL; /* Liste des clients en attente d'un nouveau tour */

int num_tour;

/* plateau par défaut pour l'instant */
const char *plateau = 
  "(0,3,D)(0,11,D)(1,13,G)(1,13,H)(2,5,D)(2,5,B)(2,9,D)(2,9,B)(4,0,B)(4,2,D)(4,2,H)(4,15,H)(5,7,G)(5,7,B)(5,14,G)(5,14,B)(6,1,G)(6,1,H)(6,7,B)(6,8,B)(6,11,H)(6,11,D)(7,6,D)(7,9,G)(8,5,H)(8,5,D)(8,6,D)(8,9,G)(9,1,B)(9,1,D)(9,12,H)(9,12,D)(10,4,G)(10,4,B)(10,15,H)(12,0,H)(12,9,G)(12,9,H)(13,5,H)(13,5,D)(13,14,G)(13,14,B)(14,3,G)(14,3,H)(14,11,D)(14,11,B)(15,6,D)(15,13,D)";

pthread_t thread_phase;


/* enigme temporaire */
const char *enigme = "(13,5,9,12,6,1,5,14,8,5,R)";

/* Permet d'obtenir le bilan de la partie en cours */
void bilan(){
  char buffer[MAX_SIZE];
  char tmp[MAX_SIZE];
  pthread_mutex_lock(&mutex_tour);
  client_list *aux = clients;
  sprintf(buffer, "%d", tour);
  while(aux != NULL){
    memset(tmp, '\0', MAX_SIZE);
    sprintf(tmp, "(%s,%d)", aux->name, aux->score); 
    strcat(buffer, tmp);
    aux = aux->next;
  }
  
  aux = clients;
  while(aux != NULL){
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }

  pthread_mutex_unlock(&mutex_tour);
}

/* Permet de changer l'état actuel du serveur */
void set_phase(char *phase){
  pthread_mutex_lock(&mutex_etat);
  if(strcmp(phase, "nulle") == 0)
    etat = PHASE_NULL;
  else if(strcmp(phase, "reflexion")){
    etat = PHASE_REFLEXION;  
    pthread_mutex_lock(&mutex_tour);
    num_tour = 0;
    pthread_mutex_unlock(&mutex_tour);
  }else if(strcmp(phase, "enchere"))
    etat = PHASE_ENCHERE;
  else if(strcmp(phase, "resolution"))
    etat = PHASE_RESOLUTION;
  pthread_mutex_unlock(&mutex_etat);
}

/* Permet d'obtenir la phase actuelle du jeu */
int get_phase(){
  int res;
  pthread_mutex_lock(&mutex_etat);
  res = etat;
  pthread_mutex_unlock(&mutex_etat);
  return res;
}

/* validation de la connexion d'un utilisateur */
void bienvenue(char *name, int sock_com){
  char buffer[MAX_SIZE];
  client_list *tmp;
  
  pthread_mutex_lock(&mutex_clients);
  if(client_exists(clients, name)){
    /* si le nom du joueur est déjà pris */
    sprintf(buffer, "%s already used\n", name);
    write(sock_com, buffer, strlen(buffer));
    pthread_mutex_unlock(&mutex_clients);
  }else{
    /* ajout dans la file d'attente tant que le client n'est pas nommé */
    pthread_mutex_lock(&mutex_attente);
    file_attente = add_name_client(file_attente, sock_com, name);
    if(get_phase() == -1){
      /* transfert du client dans la file d'attente vers la liste des clients */
      tmp = get_client(file_attente, name);
      file_attente = suppr_client(file_attente, name);
      tmp->next = clients;
      clients = tmp;
      pthread_mutex_unlock(&mutex_attente);
      pthread_mutex_unlock(&mutex_clients);
      sprintf(buffer, "BIENVENUE/%s/\n", name);
      write(sock_com, buffer, strlen(buffer));
      connecte(name);
    }
  }
}

/* notification de validation de connexion d'un joueur à tous les autres joueurs */
void connecte(char *name){
  char buffer[MAX_SIZE];
  client_list *l = clients;
  client_list *l2 = file_attente;
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    memset(buffer, '\0', MAX_SIZE);
    if(l->name != NULL){
      if(strcmp(l->name, name) != 0){
        sprintf(buffer, "CONNECTE/%s/\n", name);
        write(l->socket, buffer, strlen(buffer));
      }
    }
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  while(l2 != NULL){
    memset(buffer, '\0', MAX_SIZE);
    if(l2->name != NULL){
      if(strcmp(l2->name, name) != 0){
        sprintf(buffer, "CONNECTE/%s/\n", name);
        write(l2->socket, buffer, strlen(buffer));
      }
    }
    l2 = l2->next;
  }
  pthread_mutex_unlock(&mutex_attente);
}

/* notification de déconnexion d'un joueur */
void sorti(char *name){
  char buffer[MAX_SIZE];
  sprintf(buffer, "DECONNEXION/%s/\n", name);
  client_list *l = clients;
  client_list *l2 = file_attente;
  pthread_t tid;

  client_list *tmp = get_client(clients, name);
  if(tmp == NULL){
    tmp = get_client(file_attente, name);
  }
  if(tmp == NULL){
    fprintf(stderr, "sorti : utilisateur inconnu\n");
    return;
  }
  tid = tmp->tid;
  
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    memset(buffer, '\0', MAX_SIZE);
    if(strcmp(l->name, name) != 0){
      write(l->socket, buffer, strlen(buffer));
    }
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  while(l2 != NULL){
    memset(buffer, '\0', MAX_SIZE);
    if(strcmp(l2->name, name) != 0){
      write(l2->socket, buffer, strlen(buffer));
    }
    l2 = l2->next;
  }
  pthread_mutex_unlock(&mutex_attente);

  /* suppression du client des listes et annulation des threads associées */
    close(tmp->socket);

    if(tid != NULL){
    pthread_mutex_lock(&mutex_clients);
    clients = suppr_client(clients, name);
    pthread_mutex_unlock(&mutex_clients);
    pthread_cancel(tid);
    pthread_exit(NULL);
  }else{
    if(tid != NULL){
      pthread_mutex_lock(&mutex_attente);
      file_attente = suppr_client(file_attente, name);
      pthread_mutex_unlock(&mutex_attente);
      pthread_cancel(tid);
      pthread_exit(NULL);
    }else{
      fprintf(stderr, "sorti : impossible de déconnecter %s\n", name);
    }
  }
}

/* annonce de début de session avec envoi du tableau */
/* ATTENTION ajouter plateau en param */
void session(char* plateau){
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer,"SESSION/%s/" ,plateau);
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    l->score = 0; /* remise à 0 des scores (nouvelle session) */
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  
}

/* fin de la session courante, et annonce du vainqueur */
void vainqueur(char *name){
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer,"VAINQUEUR/%s/", name);
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* tour donne le bilan de la session courant (tour courant; (joueur,score)) et l'enigme (position des robots et du point d'arrivé) */
/* ATTENTION ajouter bilan et enigme en param */
void tour(char *enigme){return;}

/* validation de l'annonce d'une solution par le serveur, fin de la phase de reflexion */
void tuastrouve(int sock_com, char *name, int coups){return;}

/* signalement de l'annonce d'une solution à tous les joueurs */
void ilatrouve(client_list *l, char *name, int coups){return;}

/* expiration du delai de reflexion, fin de la phase de reflexion */
void finreflexion(client_list *l){return;}

/* validation d'une enchere */
void tuenchere(int sock_com){return;}

/* signalement aux autres joueurs d'une enchere */
void ilenchere(client_list *l){return;}

/* fin de la phase d'enchere, le joueur actif est user */
void finenchere(client_list *l, char *name, int coups){return;}

/* signalement aux clients de la solution proposée */
void sasolution(client_list *l, char *solution){return;}

/* notification de solution acceptée à tous les clients */
void bonne(client_list *l){return;}

/* notification de solution refusée à tous les clients */
void mauvaise(client_list *l){return;}

/* plus de joueurs ayant proposé une solution restant, fin du tour */
void finreso(client_list *l){return;}

/* temps depasse, nouvelle phase de resolution (joueur suivant), affichage du joueur actif */
void troplong(client_list *l, char *name){
  return;
}
