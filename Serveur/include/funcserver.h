#ifndef _FUNC_SERVER_H_
#define _FUNC_SERVER_H_

#include "../include/clientlist.h"
#include <pthread.h>

extern int etat; /* phase courant du jeu (NECESSAIRE OU PAS ?) */
extern client_list *clients; /* Liste des clients connectés */
extern client_list *file_attente; /* Liste des clients en attente d'un nouveau tour */
extern int num_tour; /* numero de tour courant */
extern int min_enchere; /* enchere minimum */
extern const char *plateau; /* plateau de jeu */
extern const char *enigme; /* disposition des pions */
extern pthread_t tid_phase; /* tid de la thread qui gère la phase courante */
extern pthread_t tid_timer; /* tid de la thread qui gère le timer courant */
extern int joueur_solution; /* booleen pour savoir si un joueur a émis une solution */
extern client_list *joueur_actif; /* le joueur qui doit proposer sa solution */
extern int trop_long;  /* permet de savoir si l'utilisateur qui a émit la solution, l'a émis à temps */

extern pthread_mutex_t mutex_clients;
extern pthread_mutex_t mutex_attente;
extern pthread_mutex_t mutex_etat;
extern pthread_mutex_t mutex_tour;
extern pthread_mutex_t mutex_phase;
extern pthread_mutex_t mutex_min_enchere;
extern pthread_mutex_t mutex_joueur_solution;
extern pthread_mutex_t mutex_joueur_actif;
extern pthread_mutex_t mutex_trop_long;
extern pthread_mutex_t mutex_conflit; /* évite de traiter deux solutions ou enchere en même temps */

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

/* permet de mettre à jour le joueur actif pour la phase de résolution */
void update_joueur_actif();

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
void vainqueur();

/* tour donne le bilan de la session courant (tour courant; (joueur,score)) et l'enigme (position des robots et du point d'arrivé) */
/* ATTENTION ajouter bilan et enigme en param */
void tour(char *enigme);

/* validation de l'annonce d'une solution par le serveur, fin de la phase de reflexion */
void tuastrouve(char *name, int coups);

/* signalement de l'annonce d'une solution à tous les joueurs */
void ilatrouve(char *name, int coups);

/* expiration du delai de reflexion, fin de la phase de reflexion */
void finreflexion();

/* traitement d'une enchere */
void traitement_enchere(char *name, int coups);

/* validation d'une enchere */
void validation(int socket);

/* signalement au joueur de l'echec d'une enchere */
void echec(int socket, char *user);

/* signalement aux autres joueurs d'une nouvelle enchere */
void nouvelleenchere(char *user, int coups);

/* fin de la phase d'enchere, le joueur actif est user */
void finenchere();

/* signalement aux clients de la solution proposée */
void sasolution(char *user, char *solution);

/* notification de solution acceptée à tous les clients */
void bonne();

/* notification de solution refusée à tous les clients */
void mauvaise();

/* verification de la solution */
void traitement_solution(char *solution);

/* plus de joueurs ayant proposé une solution restant, fin du tour */
void finreso();

/* temps depasse, nouvelle phase de resolution (joueur suivant), affichage du joueur actif */
void troplong();

/* relance une session lorsqu'une session se termine et si les conditions le permettent */
void nouvellesession();

/* vérifie les scores, si un des scores a atteint le score objectif, fin de la session */
int check_score();

#endif
