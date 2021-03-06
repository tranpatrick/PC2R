#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "../include/funcserver.h"
#include "../include/clientlist.h"
#include "../include/board.h"
#include "../include/server.h"

#define MAX_SIZE 512

/* Etats du jeu */
#define PHASE_NULL -1
#define PHASE_REFLEXION 1
#define PHASE_ENCHERE 2
#define PHASE_RESOLUTION 3
#define TEMPS_AFFICHAGE_SOLUTION 8
#define OBJECTIF 10

/* Description des variables partagées dans funcserver.h */

pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_attente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_etat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tour = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phase = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_joueur_solution = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_joueur_actif = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_trop_long = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_conflit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_compteur_coups = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_num_plateau = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_cond_reflexion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cond_enchere = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cond_resolution = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_reflexion = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_enchere = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_resolution = PTHREAD_COND_INITIALIZER;

int etat;
client_list *clients = NULL; /* Liste des clients connectés */
client_list *file_attente = NULL; /* Liste des clients en attente d'un nouveau tour */
client_list *joueur_actif = NULL; /* le joueur qui doit proposer sa solution */

int num_tour;
int joueur_solution;
int trop_long;
int compteur_coups = 0;
int num_plateau = 0;

/* liste des plateaux */
char **plateaux; 

/* enigme */
char *enigme = NULL; /*"(13,5,9,12,6,1,5,14,8,5,R)";  */


pthread_t tid_phase;
pthread_t tid_timer;

/* Permet d'obtenir le bilan de la partie en cours */
char* bilan(){
  char *buffer = (char*) malloc(MAX_SIZE*sizeof(char));
  char tmp[MAX_SIZE];
  client_list *aux;

  /* Récupération du numéro de tour courant */
  pthread_mutex_lock(&mutex_tour);
  sprintf(buffer, "%d", num_tour);
  pthread_mutex_unlock(&mutex_tour);

  /* Construction du bilan en parcourant tous les joueurs */
  pthread_mutex_lock(&mutex_clients);
  aux = clients;
  while(aux != NULL){
    memset(tmp, '\0', MAX_SIZE);
    sprintf(tmp, "(%s,%d)", aux->name, aux->score); 
    strcat(buffer, tmp);
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  return buffer;
}

/* Permet de changer l'état actuel du serveur */
void set_phase(char *phase){
  pthread_mutex_lock(&mutex_etat);
  if(strcmp(phase, "nulle") == 0)
    etat = PHASE_NULL;
  else if(strcmp(phase, "reflexion") == 0){
    etat = PHASE_REFLEXION;  
  }else if(strcmp(phase, "enchere") == 0)
    etat = PHASE_ENCHERE;
  else if(strcmp(phase, "resolution") == 0)
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

/* permet de mettre à jour le joueur actif pour la phase de résolution */
void update_joueur_actif(){
  client_list *aux = clients;
  client_list *actif = NULL;
  int min = 999999999;

  /* Parcours de tous les joueurs pour récupérer celui qui a une proposition de valeur minimum */
  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    if(aux->proposition != -1 && aux->proposition < min){
      min = aux->proposition;
      actif = aux;
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Mise à jour du joueur actif */
  pthread_mutex_lock(&mutex_joueur_actif);
  joueur_actif = actif;
  pthread_mutex_unlock(&mutex_joueur_actif);
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
    }
    sprintf(buffer, "BIENVENUE/%s/\n", name);
    write(sock_com, buffer, strlen(buffer));
    pthread_mutex_unlock(&mutex_clients);
    pthread_mutex_unlock(&mutex_attente);
    /* Notification à tous les clients de la connexion de ce joueur */
    connecte(name);
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* notification de validation de connexion d'un joueur à tous les autres joueurs */
void connecte(char *name){
  char buffer[MAX_SIZE];
  sprintf(buffer, "CONNECTE/%s/\n", name);

  /* Notification pour les utilisateurs dans la liste des joueurs */
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  while(l != NULL){
    if(strcmp(l->name, name) != 0)
      write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Notification pour les utilisateurs dans la file d'attente */
  pthread_mutex_lock(&mutex_attente);
  client_list *l2 = file_attente;
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
  client_list *l;
  client_list *l2;
  
  /* Récupération du client à supprimer */
  client_list *tmp = get_client(clients, name);
  if(tmp == NULL){
    tmp = get_client(file_attente, name);
  }
  if(tmp == NULL){
    fprintf(stderr, "sorti : utilisateur inconnu\n");
    return;
  }
    
  /* Notification pour les utilisateurs dans la liste des joueurs */
  pthread_mutex_lock(&mutex_clients);
  l = clients;
  while(l != NULL){
    if(strcmp(l->name, name) != 0){
      write(l->socket, buffer, strlen(buffer));
    }
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Notification pour les utilisateurs dans la file d'attente */
  pthread_mutex_lock(&mutex_attente);
  l2 = file_attente;
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
  
  pthread_mutex_lock(&mutex_clients);
  clients = suppr_client(clients, name);
  pthread_mutex_unlock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  file_attente = suppr_client(file_attente, name);
  pthread_mutex_unlock(&mutex_attente);
}

/* Envoie du plateau de la session pour le client en attente */
void session_attente(int socket){
  char buffer[MAX_SIZE];
  pthread_mutex_lock(&mutex_num_plateau);
  int num = num_plateau;
  pthread_mutex_unlock(&mutex_num_plateau);
  sprintf(buffer,"SESSION/%s/\n",plateaux[num_plateau]);
  write(socket, buffer, strlen(buffer));
}

/* annonce de début de session avec envoi du tableau */
void session(){
  /* Selection d'un plateau aléatoire */
  srand(time(NULL));
  int alea = rand()%3;
  pthread_mutex_lock(&mutex_num_plateau);
  num_plateau = alea;
  pthread_mutex_unlock(&mutex_num_plateau);

  /* Envoi du plateau à tous les joueurs */
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer,"SESSION/%s/\n",plateaux[alea]);
  while(l != NULL){
    l->score = 0; /*remise à 0 des scores (nouvelle session)*/
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Reinitialisation du numero de tour */
  pthread_mutex_lock(&mutex_tour);
  num_tour = 0;
  pthread_mutex_unlock(&mutex_tour);  

  /* Lancement du premier tour */
  tour();
}

/* fin de la session courante, et annonce du vainqueur */
void vainqueur(){
  client_list *l = clients;
  char buffer[MAX_SIZE];

  /* Annulation des threads de phase et de timer */
  pthread_mutex_lock(&mutex_phase);
  pthread_cancel(tid_phase);
  pthread_cancel(tid_timer);
  set_phase("nulle");
  pthread_mutex_unlock(&mutex_phase);

  /* Envoi du nom de vainqueur à tous les joueurs */
  sprintf(buffer, "VAINQUEUR/%s/\n", bilan());
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* tour donne le bilan de la session courant (tour courant; (joueur,score)) et l'enigme (position des robots et du point d'arrivé) */
void tour(){
  client_list *aux = clients;
  char buffer[MAX_SIZE];
  sleep(1); /* permet au client d'avoir le temps d'afficher le plateau avant le tour */
  
  /* Incrementation du numero de tour */
  pthread_mutex_lock(&mutex_tour);
  num_tour++;
  pthread_mutex_unlock(&mutex_tour);
  
  /* Génération d'une nouvelle énigme */
  if (enigme != NULL) {
    free(enigme);
  }
  enigme = strdup(genererate_random_enigme());

  sprintf(buffer, "TOUR/%s/%s/\n", enigme, bilan());

  /* Remise à -1 de toutes les propositions d'enchères */
  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    aux->proposition = -1;
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Reinitialisation du boolean joueur_solution */
  pthread_mutex_lock(&mutex_joueur_solution);
  joueur_solution = 0;
  pthread_mutex_unlock(&mutex_joueur_solution);

  /* Création d'une thread qui va gérer la phase de reflexion */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_reflexion, NULL) != 0){
    perror("pthread_create thread_reflexion");
    exit(1);
  }
  pthread_mutex_unlock(&mutex_phase);
}

/* validation de l'annonce d'une solution par le serveur, fin de la phase de reflexion */
void tuastrouve(char *name, int coups){  
  pthread_mutex_lock(&mutex_clients);
  client_list *aux = clients;
  client_list *user = get_client(clients, name);

  /* Notification à l'utilisateur */
  write(user->socket, "TUASTROUVE/\n", strlen("TUASTROUVE/\n"));
  
  /* Mise à jour des information du client */
  while(aux != NULL){
    if(strcmp(aux->name, name) == 0){
      aux->proposition = coups;
      user->proposition = coups;
      break;
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);      

  /* joueur_solution = 1 (une solution a été trouvée) */
  pthread_mutex_lock(&mutex_joueur_solution);
  joueur_solution = 1;
  pthread_mutex_unlock(&mutex_joueur_solution);

  /* Annonce qu'une solution a été trouvée */
  ilatrouve(name, coups);
  
}

/* signalement de l'annonce d'une solution à tous les joueurs */
void ilatrouve(char *name, int coups){

  /* Notification à tous les joueurs qu'une solution a été trouvée */
  pthread_mutex_lock(&mutex_clients);
  client_list *aux = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "ILATROUVE/%s/%d/\n", name, coups); 
  while(aux != NULL){
    if(strcmp(aux->name, name) != 0){
      write(aux->socket, buffer, strlen(buffer));
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Arret de la phase de reflexion, envoie d'un cond pour signaler à la thread */
  pthread_mutex_lock(&mutex_cond_reflexion);
  pthread_cond_signal(&cond_reflexion);
  pthread_mutex_unlock(&mutex_cond_reflexion);
}

/* Expiration du delai de reflexion, fin de la phase de reflexion */
void finreflexion(){
  pthread_mutex_lock(&mutex_clients);
  char buffer[MAX_SIZE];
  client_list *aux = clients;
  sprintf(buffer, "FINREFLEXION/\n");
  while(aux != NULL){
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  return;
}

/* traitement d'une enchere */
void traitement_enchere(char *name, int coups){
  pthread_mutex_lock(&mutex_clients);
  client_list *cl = get_client(clients, name);
  
  if(cl->proposition != -1 && coups >= cl->proposition){
    /* Echec si un joueur essaye d'encherir une valeur plus grande que celle qu'il a déjà encheri */
    echec(cl->socket, name);
  }else{
    client_list *aux = clients;
    int bool = 0;
    char *user_incoherent;
    
    /* Vérification qu'il n'existe pas une enchere de même valeur */
    while(aux != NULL){
      if(aux->proposition == coups){
	user_incoherent = aux->name;
	bool = 1;
	break;
      }
      aux = aux->next;
    }

    if(bool == 1){
      /* Echec */      
      echec(cl->socket, user_incoherent);
    }else{
      /* Validation */
      cl->proposition = coups;
      validation(cl->socket);
      pthread_mutex_unlock(&mutex_clients);
      nouvelleenchere(name, coups);
    }
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* validation d'une enchere */
void validation(int socket){
  write(socket, "VALIDATION/\n", 12);
}

/* signalement aux autres joueurs d'une enchere */
void echec(int socket, char *user){
  char buffer[MAX_SIZE];
  sprintf(buffer, "ECHEC/%s/\n", user);
  write(socket, buffer, strlen(buffer));
}

/* signalement aux autres joueurs d'une nouvelle enchere */
void nouvelleenchere(char *user, int coups){
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "NOUVELLEENCHERE/%s/%d/\n", user, coups);
  while(l != NULL){
    if(strcmp(l->name, user) != 0)
      write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* fin de la phase d'enchere, le joueur actif est user */
void finenchere(){
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  char *user;
  int min = 999999;
  char buffer[MAX_SIZE];

  /* Recherche du joueur actif */
  while(l != NULL){
    if(l->proposition != -1 && l->proposition < min){
      user = l->name;
      min = l->proposition;
    }
    l = l->next;
  }

  /* S'il n'y a pas eu de solution proposée, envoi de finenchere avec null null */
  if(min == 999999)
    sprintf(buffer, "FINENCHERE/null/null/\n");
  else
    sprintf(buffer, "FINENCHERE/%s/%d/\n", user, min);

  /* Notification à tous les clients */
  l = clients;
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* signalement aux clients de la solution proposée */
void sasolution(char *user, char *solution){
  char buffer[MAX_SIZE];
  client_list *aux = clients;
  pthread_mutex_lock(&mutex_joueur_actif);
  if(strcmp(user, joueur_actif->name) == 0){
    sprintf(buffer, "SASOLUTION/%s/%s/\n", user, solution);
    pthread_mutex_lock(&mutex_clients);  
    while(aux != NULL){
      write(aux->socket, buffer, strlen(buffer));
      aux = aux->next;
    }
    pthread_mutex_unlock(&mutex_clients);
  }
  pthread_mutex_unlock(&mutex_joueur_actif);
  
  /* traitement de la solution */
  traitement_solution(solution);
}

/* notification de solution acceptée à tous les clients */
void bonne(){
  char buffer[MAX_SIZE];
  sprintf(buffer, "BONNE/\n");
  pthread_mutex_lock(&mutex_clients);
  client_list *aux = clients; 
  while(aux != NULL){
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Vérification des scores pour voir si le score objectif a été atteint */
  if(check_score() == 1){
    vainqueur();
    nouvellesession();
  }else{
    /* Lancement du nouveau tour */
    nouveautour();
  }

}

/* notification de solution refusée à tous les clients */
void mauvaise(){
  char buffer[MAX_SIZE];
  char *tmp_name;

  /* Mise à jour du joueur actif */
  update_joueur_actif();
  
  /* S'il n'y a plus de joueur actif, finreso sinon envoi de MAUVAISE avec le nom du joueur actif */
  pthread_mutex_lock(&mutex_joueur_actif);  
  if(joueur_actif == NULL){
    pthread_mutex_unlock(&mutex_joueur_actif);
    finreso();
    return;
  }else{
    tmp_name = joueur_actif->name;
  }
  pthread_mutex_unlock(&mutex_joueur_actif);
  
  sprintf(buffer, "MAUVAISE/%s/\n", tmp_name);
  
  /* Notification à tous les clients */
  client_list *aux = clients;
  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Lancement d'une nouvelle phase de resolution */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_resolution, (void *) NULL) != 0){
    perror("pthread_create thread_resolution in thread_resolution");
    exit(1);
  }
  pthread_mutex_unlock(&mutex_phase);
}

/* verification de la solution */
void traitement_solution(char *solution){
  /* Récupération du nombre de coups que le joueur actif avait proposé et reset de cette variable */
  pthread_mutex_lock(&mutex_joueur_actif);
  client_list* tmp = joueur_actif;
  int prop = joueur_actif->proposition;
  joueur_actif->proposition = -1;
  pthread_mutex_unlock(&mutex_joueur_actif);
  
  /* Récupération du numéro de plateau */
  pthread_mutex_lock(&mutex_num_plateau);
  int num = num_plateau;
  pthread_mutex_unlock(&mutex_num_plateau);

  /* Simulation de la solution de l'utilisateur */
  if(simulation(plateaux[num], enigme, solution) == 1 ){ 
    /* Si la solution est bonne, incrément du score du joueur et envoi de BONNE */
    pthread_mutex_lock(&mutex_compteur_coups);   
    if(prop >= compteur_coups) {
      tmp->score = tmp->score + 1;
      pthread_mutex_unlock(&mutex_compteur_coups);    
      /* Ce sleep permet au client d'avoir le temps d'afficher la solution avant que le serveur n'envoie d'autres commandes */
      sleep(TEMPS_AFFICHAGE_SOLUTION);
      bonne();
    }
    else{
      /* Si la solution est bonne mais ne respecte pas le nombre de coups annoncé, appel à mauvaise() */
      pthread_mutex_unlock(&mutex_compteur_coups);    
      sleep(TEMPS_AFFICHAGE_SOLUTION);
      mauvaise();
    }
  }else{
    /* Si la solution est mauvaise appel à mauvaise() */
    sleep(TEMPS_AFFICHAGE_SOLUTION);
    mauvaise();
  }
}

/* nouveau tour */
void nouveautour(){
  /* transfert de tous les clients dans la file d'attente (ayant un nom) vers clients */
  pthread_mutex_lock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  client_list *att = file_attente;
  client_list *tmp;
  while(att != NULL && att->name != NULL){
    file_attente = att->next;
    tmp = att->next;
    att->next = clients;
    clients = att;
    att = tmp;
  }
  pthread_mutex_unlock(&mutex_attente);
  pthread_mutex_unlock(&mutex_clients);

  /* Lancement du tour */
  tour(enigme);
}

/* plus de joueurs ayant proposé une solution restant, fin du tour */
void finreso(){
  /* Notification de fin de phase de resolution à tous les clients */
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "FINRESO/\n");

  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Lancement du prochain tour */
  nouveautour();
  
}

/* temps depasse, nouvelle phase de resolution (joueur suivant), affichage du joueur actif */
void troplong(){
  client_list *l;
  char buffer[MAX_SIZE];
  char *tmp_name;

  /* Mise à jour du joueur actif */
  pthread_mutex_lock(&mutex_joueur_actif);
  joueur_actif->proposition = -1;
  pthread_mutex_unlock(&mutex_joueur_actif);
  update_joueur_actif();
  
  /* Récupération du nom du joueur suivant (s'il n'y en a pas finreso) */
  pthread_mutex_lock(&mutex_joueur_actif);
  if(joueur_actif == NULL){
    pthread_mutex_unlock(&mutex_joueur_actif);
    finreso();
    return;
  }else{
    tmp_name = joueur_actif->name;
  }
  pthread_mutex_unlock(&mutex_joueur_actif);

  /* Envoie de TROPLONG suivi du nom du joueur suivant */
  sprintf(buffer, "TROPLONG/%s/\n", tmp_name);
  pthread_mutex_lock(&mutex_clients);
  l = clients;
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Lancement d'une nouvelle phase de resolution */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_resolution, (void *) NULL) != 0){
    perror("pthread_create thread_resolution in thread_resolution");
    exit(1);
  }
  pthread_mutex_unlock(&mutex_phase);
}

/* Fonction de traitement des message du chat */
void traitement_chat(char *user, char *coups) {
  pthread_mutex_lock(&mutex_clients);
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "CHAT/%s/%s/\n", user, coups);
  while(l != NULL){
    if(strcmp(l->name, user) != 0)
      write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* relance une session lorsqu'une session se termine et si les conditions le permettent */
void nouvellesession(){
  int nb_clients;

  /* transfert de tous les clients dans la file d'attente (ayant un nom) vers clients */
  pthread_mutex_lock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  client_list *att = file_attente;
  client_list *tmp;

  while(att != NULL && att->name != NULL){
    file_attente = att->next;
    tmp = att->next;
    att->next = clients;
    clients = att;
    att = tmp;
  }
  
  pthread_mutex_unlock(&mutex_attente);
  pthread_mutex_unlock(&mutex_clients);

  /* Si le nombre de clients est supérieur à 2, lancer une session */
  pthread_mutex_lock(&mutex_clients);
  nb_clients = client_list_length(clients);
  pthread_mutex_unlock(&mutex_clients);

  if(nb_clients >= 2){
    session();
  }
}

/* vérifie les scores, si un des scores a atteint le score objectif, fin de la session (dans bonne()) */
int check_score(){
  pthread_mutex_lock(&mutex_clients);
  client_list *aux = clients;
  while(aux != NULL){
    if(aux->score == OBJECTIF){
      pthread_mutex_unlock(&mutex_clients);
      return 1;
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);
  return 0;
}
