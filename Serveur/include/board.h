#ifndef _BOARD_H_
#define _BOARD_H_

#include <pthread.h>

extern int compteur_coups; /* compteur de coups effectués actuellement dans simulation */
extern pthread_mutex_t mutex_compteur_coups;
int cibles[16][16]; /* Tableau des cibles possibles */

/* Structure de case */
typedef struct square{
  int top_wall;
  int left_wall;
  int right_wall;
  int bottom_wall;
  /* int is_target; */
  char robot; /* ici on met 'R','V','G','B' si un robot est présent sur la case, '0' sinon */
} square;

/* Structure de plateau */
typedef struct board{
  square* tab[16][16];
  /* coordonnées des robots */
  int xr;
  int yr;
  int xb;
  int yb;
  int xj;
  int yj;
  int xv;
  int yv;
  /* coordonnées de la cible */
  int xc;
  int yc;
  /* couleur du robot à amener à la cible */
  char color;
} board;

/* Création d'une square */
square* create_square();
/* Création d'un plateau avec énigme */
board* create_terrain(char *plateau);
/* Création d'un plateau sans énigme */
board* create_board(char* plateau, char* enigme);
/* Permet de savoir si une solution est bonne ou fausse en fonction du terrain et d'une énigme */
int simulation(char *desc_plateau, char *enigme, char *solution);
/* Permet d'obtenir la matrice des positions de cibles possibles */
void init_matrice();
/* Récupération d'une case cible aléatoire à partir de la matrice */
int *get_random_cible();
/* Génération d'une énigme aléatoire */
char *genererate_random_enigme();  
/* Free plateau */
void destroy_plateau(board *plateau);

#endif
