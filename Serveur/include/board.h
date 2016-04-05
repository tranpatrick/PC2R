#ifndef _BOARD_H_
#define _BOARD_H_

#include <pthread.h>

extern int compteur_coups; /* compteur de coups effectués actuellement dans simulation */
extern pthread_mutex_t mutex_compteur_coups;
int cibles[16][16]; /* Tableau des cibles possibles */

typedef struct square{
  int top_wall;
  int left_wall;
  int right_wall;
  int bottom_wall;
  /* int is_target; */
  char robot; /* ici on met 'R','V','G','B' si un robot est présent sur la case, '0' sinon */
} square;

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

square* create_square();
board* create_board(char* plateau, char* enigme);
int simulation(char *desc_plateau, char *enigme, char *solution);
void init_matrice();
int *get_random_cible();
char *genererate_random_enigme();  

#endif
