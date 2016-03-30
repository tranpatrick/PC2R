#ifndef _FUNC_SERVER_H_
#define _FUNC_SERVER_H_

#include "../include/clientlist.h"
#include <pthread.h>

extern int etat;
extern client_list *clients; /* Liste des clients connectés */
extern client_list *file_attente; /* Liste des clients en attente d'un nouveau tour */
extern int num_tour;
extern int min_coups;
extern const char *plateau;
extern const char *enigme;
extern pthread_t tid_phase;
extern pthread_t tid_timer;
extern int joueur_solution;

extern pthread_mutex_t mutex_clients;
extern pthread_mutex_t mutex_attente;
extern pthread_mutex_t mutex_etat;
extern pthread_mutex_t mutex_tour;
extern pthread_mutex_t mutex_phase;
extern pthread_mutex_t mutex_mincoups;
extern pthread_mutex_t mutex_joueur_solution;

extern pthread_mutex_t mutex_cond_reflexion;
extern pthread_mutex_t mutex_cond_enchere;
extern pthread_mutex_t mutex_cond_resolution;
extern pthread_cond_t cond_reflexion;
extern pthread_cond_t cond_enchere;
extern pthread_cond_t cond_resolution;

/* Permet d'obtenir le bilan de la partie en cours */
char* bilan();

/* Permet de changer l'état actuel du serveur */
void set_phase(char *phase);

/* Permet d'obtenir la phase actuelle du jeu */
int get_phase();

/* validation de la connexion d'un utilisateur */
void bienvenue(char *name, int sock_com);

/* notification de validation de connexion d'un joueur à tous les autres joueurs */
void connecte(char *name);

/* notification de déconnexion d'un joueur */
void sorti(char *name);

/* annonce de début de session avec envoi du tableau */
/* ATTENTION ajouter plateau en param */
void session(char *plateau);

/* fin de la session courante, et annonce du vainqueur */
void vainqueur(char *name);

/* tour donne le bilan de la session courant (tour courant; (joueur,score)) et l'enigme (position des robots et du point d'arrivé) */
/* ATTENTION ajouter bilan et enigme en param */
void tour(char *enigme);

/* validation de l'annonce d'une solution par le serveur, fin de la phase de reflexion */
void tuastrouve(char *name, int coups);

/* signalement de l'annonce d'une solution à tous les joueurs */
void ilatrouve(char *name, int coups);

/* expiration du delai de reflexion, fin de la phase de reflexion */
void finreflexion();

/* validation d'une enchere */
void tuenchere(int sock_com);

/* signalement aux autres joueurs d'une enchere */
void ilenchere(client_list *l);

/* fin de la phase d'enchere, le joueur actif est user */
void finenchere(client_list *l, char *name, int coups);

/* signalement aux clients de la solution proposée */
void sasolution(client_list *l, char *solution);

/* notification de solution acceptée à tous les clients */
void bonne(client_list *l);

/* notification de solution refusée à tous les clients */
void mauvaise(client_list *l);

/* plus de joueurs ayant proposé une solution restant, fin du tour */
void finreso(client_list *l);

/* temps depasse, nouvelle phase de resolution (joueur suivant), affichage du joueur actif */
void troplong(client_list *l, char *name);

#endif