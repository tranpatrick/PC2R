#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
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
/*int min_enchere;*/
/*client_list *liste_encherisseurs = NULL;*/
int joueur_solution;
int trop_long;
int compteur_coups = 0;

/* plateau par défaut pour l'instant */
const char *plateau = 
  "(0,3,D)(0,11,D)(1,13,G)(1,13,H)(2,5,D)(2,5,B)(2,9,D)(2,9,B)(4,0,B)(4,2,D)(4,2,H)(4,15,H)(5,7,G)(5,7,B)(5,14,G)(5,14,B)(6,1,G)(6,1,H)(6,7,B)(6,8,B)(6,11,H)(6,11,D)(7,6,D)(7,9,G)(8,5,H)(8,5,D)(8,6,D)(8,9,G)(9,1,B)(9,1,D)(9,12,H)(9,12,D)(10,4,G)(10,4,B)(10,15,H)(12,0,H)(12,9,G)(12,9,H)(13,5,H)(13,5,D)(13,14,G)(13,14,B)(14,3,G)(14,3,H)(14,11,D)(14,11,B)(15,6,D)(15,13,D)";
/* enigme temporaire */
const char *enigme = "(13,5,9,12,6,1,5,14,8,5,R)";

pthread_t tid_phase;
pthread_t tid_timer;


/* Permet d'obtenir le bilan de la partie en cours */
char* bilan(){

  printf("DEBUT BILAN\n");

  char *buffer = (char*) malloc(MAX_SIZE*sizeof(char));
  char tmp[MAX_SIZE];
  pthread_mutex_lock(&mutex_tour);
  client_list *aux = clients;
  sprintf(buffer, "%d", num_tour);
  pthread_mutex_unlock(&mutex_tour);

  printf("1\n");

  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    memset(tmp, '\0', MAX_SIZE);
    printf("while.1\n");
    sprintf(tmp, "(%s,%d)", aux->name, aux->score); 
    printf("while.2\n");
    strcat(buffer, tmp);
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  printf("FIN BILAN\n");

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
  printf("entree update_joueur_actif\n");
  client_list *aux = clients;
  client_list *actif = NULL;
  int min = 999999999;
  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    if(aux->proposition != -1 && aux->proposition < min){
      min = aux->proposition;
      actif = aux;
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  printf("ABLABALBA\n");

  pthread_mutex_lock(&mutex_joueur_actif);
  printf("BABLABALBA\n"); 
  joueur_actif = actif;
  pthread_mutex_unlock(&mutex_joueur_actif);
  printf("SORTIE update_joueur_actif\n");
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
    connecte(name);
    pthread_mutex_unlock(&mutex_attente);
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* notification de validation de connexion d'un joueur à tous les autres joueurs */
void connecte(char *name){
  char buffer[MAX_SIZE];
  client_list *l = clients;
  client_list *l2 = file_attente;
  sprintf(buffer, "CONNECTE/%s/\n", name);
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    if(strcmp(l->name, name) != 0)
      write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* ICI si on veut notifier les utilisateurs dans la file d'attente */
  /*pthread_mutex_lock(&mutex_attente);
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
  pthread_mutex_unlock(&mutex_attente); */
}

/* notification de déconnexion d'un joueur */
void sorti(char *name){
  printf("DEBUT SORTI\n");

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
    if(strcmp(l->name, name) != 0){
      write(l->socket, buffer, strlen(buffer));
    }
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* ICI si on veut notifier les utilisateurs dans la file d'attente */
  /*pthread_mutex_lock(&mutex_attente);
  while(l2 != NULL){
    memset(buffer, '\0', MAX_SIZE);
    if(strcmp(l2->name, name) != 0){
      write(l2->socket, buffer, strlen(buffer));
    }
    l2 = l2->next;
  }
  pthread_mutex_unlock(&mutex_attente);*/

  /* suppression du client des listes et annulation des threads associées */
  close(tmp->socket);
  
  pthread_mutex_lock(&mutex_clients);
  clients = suppr_client(clients, name);
  pthread_mutex_unlock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  file_attente = suppr_client(file_attente, name);
  pthread_mutex_unlock(&mutex_attente);
  /*pthread_cancel(tid);
    pthread_exit(NULL);*/

  /*else{
    if(tid != NULL){
      pthread_mutex_lock(&mutex_attente);
      file_attente = suppr_client(file_attente, name);
      pthread_mutex_unlock(&mutex_attente);
      pthread_cancel(tid);
      pthread_exit(NULL);
    }else{
      fprintf(stderr, "sorti : impossible de déconnecter %s\n", name);
    }
    }*/

  printf("FIN SORTI\n");

}

void session_attente(char *plateau, int socket){
  char buffer[MAX_SIZE];
  sprintf(buffer,"SESSION/%s/\n",plateau);
  write(socket, buffer, strlen(buffer));
}

/* annonce de début de session avec envoi du tableau */
/* ATTENTION ajouter plateau en param */
void session(char* plateau){
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer,"SESSION/%s/\n",plateau);
  pthread_mutex_lock(&mutex_clients);
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

  /* Création d'une thread qui va gérer la phase de reflexion */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_reflexion, NULL) != 0){
    perror("pthread_create thread_reflexion");
    return EXIT_FAILURE;
  }
  pthread_mutex_unlock(&mutex_phase);

}

/* fin de la session courante, et annonce du vainqueur */
void vainqueur(){
  printf("DEBUT VAINQUEUR\n");

  client_list *l = clients;
  char buffer[MAX_SIZE];

  /* Annulation des threads */
  pthread_mutex_lock(&mutex_phase);
  pthread_cancel(tid_timer);
  pthread_cancel(tid_phase);
  set_phase("nulle");
  pthread_mutex_unlock(&mutex_phase);

  sprintf(buffer, "VAINQUEUR/%s/\n", bilan());
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  printf("FIN VAINQUEUR\n");

}

/* tour donne le bilan de la session courant (tour courant; (joueur,score)) et l'enigme (position des robots et du point d'arrivé) */
/* ATTENTION ajouter bilan et enigme en param */
void tour(char *enigme){

  printf("DEBUT TOUR\n");

  client_list *aux = clients;
  char buffer[MAX_SIZE];
  sleep(1);
  
  printf("0\n");

  /* Incrementation du numero de tour */
  pthread_mutex_lock(&mutex_tour);
  num_tour++;
  pthread_mutex_unlock(&mutex_tour);
  
  printf("1\n");  

  /* Notification début de tour avec enigme */
  sprintf(buffer, "TOUR/%s/%s/\n", enigme, bilan());

  printf("1.5\n");

  /* Remise à -1 de toutes les propositions d'enchères */
  pthread_mutex_lock(&mutex_clients);
  while(aux != NULL){
    aux->proposition = -1;
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  printf("2\n");

  /* Reinitialisation du boolean joueur_solution */
  pthread_mutex_lock(&mutex_joueur_solution);
  joueur_solution = 0;
  pthread_mutex_unlock(&mutex_joueur_solution);

  printf("FIN TOUR\n");
  
}

/* validation de l'annonce d'une solution par le serveur, fin de la phase de reflexion */
void tuastrouve(char *name, int coups){
  client_list *aux = clients;
  client_list *tmp;
  
  pthread_mutex_lock(&mutex_clients);

  client_list *user = get_client(clients, name);
  write(user->socket, "TUASTROUVE/\n", strlen("TUASTROUVE/\n"));
  /* user->proposition = coups; ladi a ecrit ca, proposition = -1 dans traitement solution */
  
  while(aux != NULL){
    if(strcmp(aux->name, name) == 0){
      aux->proposition = coups;
      
      /* Sauvegarde du nb coups que le joueur propose */
      /*      pthread_mutex_lock(&mutex_min_enchere);
	      min_enchere = coups;*/
      tmp = get_client(clients, name);
      tmp->proposition = coups;

      /* Ajout du client dans la liste des clients qui vont jouer */
      /*liste_encherisseurs = add_client(liste_encherisseurs, client_copy(get_client(clients, name)));*/

      /*pthread_mutex_unlock(&mutex_min_enchere);*/
      pthread_mutex_unlock(&mutex_clients);

      /* joueur_solution = 1 */
      pthread_mutex_lock(&mutex_joueur_solution);
      joueur_solution = 1;
      pthread_mutex_unlock(&mutex_joueur_solution);
      
      /* Annonce qu'une solution a été trouvée */
      ilatrouve(name, coups);
      break;
    }
    aux = aux->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* signalement de l'annonce d'une solution à tous les joueurs */
void ilatrouve(char *name, int coups){
  client_list *aux = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "ILATROUVE/%s/%d/\n", name, coups); /** A DEBUG ICI **/
  printf("buffer : %s\n", buffer);
  pthread_mutex_lock(&mutex_clients);
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

/* expiration du delai de reflexion, fin de la phase de reflexion */
void finreflexion(){
  char buffer[MAX_SIZE];
  client_list *aux = clients;
  sprintf(buffer, "FINREFLEXION/\n");
  while(aux != NULL){
    write(aux->socket, buffer, strlen(buffer));
    aux = aux->next;
  }
  return;
}

/* traitement d'une enchere */
void traitement_enchere(char *name, int coups){
  pthread_mutex_lock(&mutex_clients);
  client_list *cl = get_client(clients, name);
  if(cl->proposition != -1 && coups >= cl->proposition){
    /* Echec */
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
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "NOUVELLEENCHERE/%s/%d/\n", user, coups);
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    if(strcmp(l->name, user) != 0)
      write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);
}

/* fin de la phase d'enchere, le joueur actif est user */
void finenchere(){
  client_list *l = clients;
  char *user;
  int min = 999999;
  char buffer[MAX_SIZE];
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    if(l->proposition != -1 && l->proposition < min){
      user = l->name;
      min = l->proposition;
    }
    l = l->next;
  }
  sprintf(buffer, "FINENCHERE/%s/%d/\n", user, min);
  l = clients;
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  printf("SORTIE de ENCHERE\n");
}

/* signalement aux clients de la solution proposée */
void sasolution(char *user, char *solution){
  printf("DEBUT SASOLUTION\n");
  char buffer[MAX_SIZE];
  client_list *aux = clients;
  printf("1\n");
  pthread_mutex_lock(&mutex_joueur_actif);
  printf("1\n");
  if (joueur_actif == NULL) {
    printf("joueur Actif = null");
  }
  if(strcmp(user, joueur_actif->name) == 0){
    printf("2\n");
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
  printf("FIN sasolution\n");
}

/* notification de solution acceptée à tous les clients */
void bonne(){
  char buffer[MAX_SIZE];
  sprintf(buffer, "BONNE/\n");
  client_list *aux = clients;
  pthread_mutex_lock(&mutex_clients);
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
  printf("Update_joueur_actif dans mauvaise\n");
  update_joueur_actif();
  
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
    return EXIT_FAILURE;
  }
  pthread_mutex_unlock(&mutex_phase);
  
  
}

/* verification de la solution */
void traitement_solution(char *solution){
  char *tmp_name;
  
  pthread_mutex_lock(&mutex_joueur_actif);
  client_list* tmp = joueur_actif;
  tmp_name = joueur_actif->name;
  int prop = joueur_actif->proposition;
  int nbCoupsSolution = strlen(solution)/2;
  pthread_mutex_unlock(&mutex_joueur_actif);
  
  /* chercher pourquoi proposition = -1 */
  printf("proposition = %d\n",prop);

  pthread_mutex_lock(&mutex_joueur_actif);
  joueur_actif->proposition = -1;
  /*  printf("joueur_actif->proposition = %d\n", joueur_actif->proposition);
  client_list *pat = get_client(clients, "pat");
  printf("pat->proposition = %d\n", pat->proposition);*/
  pthread_mutex_unlock(&mutex_joueur_actif);
  
  /* && nbCoupsSolution <= prop */
  if(simulation(plateau, enigme, solution) == 1 ){ 
    printf("if simulation == 1)\n");

    pthread_mutex_lock(&mutex_compteur_coups);
    if(prop >= compteur_coups)
      tmp->score = tmp->score + 1;
    pthread_mutex_unlock(&mutex_compteur_coups);    
    
    printf("score de %s = %d\n",tmp->name, tmp->score);
    sleep(TEMPS_AFFICHAGE_SOLUTION);
    bonne();
  }else{
    printf("simulation != 1\n");
    sleep(TEMPS_AFFICHAGE_SOLUTION);
    mauvaise();
  }
  printf("FIN TRAITEMENT SOLUTION\n");
}

/* nouveau tour */
nouveautour(){

  printf("DEBUT NOUVEAUTOUR\n");

  /* transfert de tous les clients dans la file d'attente (ayant un nom) vers clients */
  pthread_mutex_lock(&mutex_clients);
  pthread_mutex_lock(&mutex_attente);
  client_list *att = file_attente;
  client_list *tmp;

  printf("Nbre de clients dans file_attente: %d\n", client_list_length(file_attente));

  while(att != NULL && att->name != NULL){
    file_attente = att->next;
    tmp = att->next;
    att->next = clients;
    clients = att;
    att = tmp;
  }
  
  printf("Nbre de clients: %d\n", client_list_length(clients));

  print_client_list(clients);

  pthread_mutex_unlock(&mutex_attente);
  pthread_mutex_unlock(&mutex_clients);

  /* création d'une thread qui va gérer la phase de reflexion */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_reflexion, NULL) != 0){
    perror("pthread_create thread_reflexion");
    return EXIT_FAILURE;
  }

  printf("HELLO\n");

  pthread_mutex_unlock(&mutex_phase);

  printf("FIN NOUVEAUTOUR\n");
  
}

/* plus de joueurs ayant proposé une solution restant, fin du tour */
void finreso(){
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "FINRESO/\n");
  pthread_mutex_lock(&mutex_clients);
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
  printf("ENTREE troplong()\n");
  client_list *l = clients;
  char buffer[MAX_SIZE];
  char *tmp_name;

  /* Mise à jour du joueur actif */
  pthread_mutex_lock(&mutex_joueur_actif);
  joueur_actif->proposition = -1;
  pthread_mutex_unlock(&mutex_joueur_actif);
  printf("Update_joueur_actif dans troplong\n");
  update_joueur_actif();
  
  pthread_mutex_lock(&mutex_joueur_actif);
  if(joueur_actif == NULL){
    pthread_mutex_unlock(&mutex_joueur_actif);
    finreso();
    return;
  }else{
    tmp_name = joueur_actif->name;
  }
  pthread_mutex_unlock(&mutex_joueur_actif);
  sprintf(buffer, "TROPLONG/%s/\n", tmp_name);
  pthread_mutex_lock(&mutex_clients);
  while(l != NULL){
    write(l->socket, buffer, strlen(buffer));
    l = l->next;
  }
  pthread_mutex_unlock(&mutex_clients);

  /* Lancement d'une nouvelle phase de resolution */
  pthread_mutex_lock(&mutex_phase);
  if(pthread_create(&tid_phase, NULL, thread_resolution, (void *) NULL) != 0){
    perror("pthread_create thread_resolution in thread_resolution");
    return EXIT_FAILURE;
  }
  pthread_mutex_unlock(&mutex_phase);
    
  printf("FIN troplong()\n");
}

/* fonction de traitement des message du chat */
void traitement_chat(char *user, char *coups) {
  client_list *l = clients;
  char buffer[MAX_SIZE];
  sprintf(buffer, "CHAT/%s/%s/\n", user, coups);
  pthread_mutex_lock(&mutex_clients);
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
    session(plateau);
  }

}

/* vérifie les scores, si un des scores a atteint le score objectif, fin de la session */
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
