#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/board.h"
#include "../include/funcserver.h"

int cibles[16][16];
int pos[16][16];

void init_matrice() {

  printf("DEBUT init_matrice\n");
  int i, j;
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      cibles[i][j] = 0;
      pos[i][j] = 0;
    }
  }
 
  pos[7][7] = 1;
  pos[7][8] = 1;
  pos[8][7] = 1;
  pos[8][8] = 1;
  /*board *tmp = create_terrain(plateau);
  if (tmp == NULL)
    printf("NULLLLL\n");
 
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      printf("allo\n");
      if(tmp->tab[i][j]->top_wall == 1){
	if(tmp->tab[i][j]->left_wall == 1 || tmp->tab[i][j]->right_wall == 1){
	  cibles[i][j] = 1;
	}
      }else if(tmp->tab[i][j]->bottom_wall == 1){
	if(tmp->tab[i][j]->left_wall == 1 || tmp->tab[i][j]->right_wall == 1){
	  cibles[i][j] = 1;
	}
      }
    }
  }
printf("dsfsdfdsf\n");
  cibles[7][7] = 0;
  cibles[7][8] = 0;
  cibles[8][7] = 0;
  cibles[8][8] = 0;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      if(cibles[i][j] == 1)
	printf("cibles[%d][%d]\n", i, j);
    }
  }
  */
  cibles[0][0] = 1;
  cibles[0][3] = 1;
  cibles[0][4] = 1;
  cibles[0][11] = 1;
  cibles[0][12] = 1;
  cibles[1][13] = 1;
  cibles[2][5] = 1;
  cibles[2][9] = 1;
  cibles[3][15] = 1;
  cibles[4][0] = 1;
  cibles[4][2] = 1;
  cibles[4][15] = 1;
  cibles[5][7] = 1;
  cibles[5][14] = 1;
  cibles[6][1] = 1;
  cibles[6][11] = 1;
  cibles[8][5] = 1;
  cibles[9][1] = 1;
  cibles[9][12] = 1;
  cibles[9][15] = 1;
  cibles[10][4] = 1;
  cibles[10][15] = 1;
  cibles[11][0] = 1;
  cibles[12][0] = 1;
  cibles[12][9] = 1;
  cibles[13][5] = 1;
  cibles[14][3] = 1;
  cibles[14][11] = 1;
  cibles[15][0] = 1;
  cibles[15][6] = 1;
  cibles[15][7] = 1;
  cibles[15][14] = 1;
  cibles[15][13] = 1;
  cibles[15][15] = 1;
  printf("FIN init_matrice\n");

}

int *get_random_cible() {

  int *cible = (int*) malloc(2*sizeof(int));
  srand(time(NULL));
  int limite = rand()%34;
  int i, j, cpt = 0;
  for(i = 0; i<16; i++) {
    for(j = 0; j<16; j++) {
      if (cibles[i][j] == 0) 
	continue;
      else {
	cpt++;
      }
      if (cpt == limite)
	break;
    }
    if (cpt == limite)
      break;
  }
  if (i < 16 && i >= 0) 
    cible[0] = i;
  else
    cible[0] = 5;
  if (j < 16 && j >= 0) 
    cible[1] = j;
  else
    cible[1] = 7;
  if (pos[i][j] == 1) {
    cible[0] = 14;
    cible[1] = 11;
  }
  return cible;
}

char *genererate_random_enigme() {

  printf("DEBUT generate_random_cible\n");
  init_matrice();
  srand(time(NULL));
  int xr, yr ,xb, yb, xj, yj, xv, yv, xc, yc;
  xr = rand()%16;
  yr = rand()%16;
  while (cibles[xr][yr] == 1 || pos[xr][yr] == 1) {
    xr = rand()%16;
  }
  pos[xr][yr] = 1;
  xb = rand()%16;
  yb = rand()%16;
  while (cibles[xb][yb] == 1 || pos[xb][yb] == 1) {
    xb = rand()%16;
  }
  pos[xb][yb] = 1;

  xj = rand()%16;
  yj = rand()%16;
  while (cibles[xj][yj] == 1 || pos[xj][yj] == 1) {
    xj = rand()%16;
  }
  pos[xj][yj] = 1;
  xv = rand()%16;
  yv = rand()%16;
  while (cibles[xv][yv] == 1 || pos[xv][yv] == 1) {
    xv = rand()%16;
  }
  pos[xv][yv] = 1;
  int *cible = get_random_cible();
  xc = cible[0];
  yc = cible[1];
  char *color;
  double alea = rand()%20;
  if (alea < 5) {
    color = strdup("R");
  }
  else if (alea < 10) {
    color = strdup("B");
  }
  else if (alea < 15) {
    color = strdup("J");
  }
  else {
    color = strdup("V");
  }
  char *enigme = (char*) malloc(40*sizeof(char));
  sprintf(enigme, "(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s)",
	  xr, yr, xb, yb, xj, yj, xv, yv, xc, yc, color);
  printf("[generator.c] generate : %s\n", enigme); 
  printf("FIN generate_random_cible\n");
  return enigme;
}

/*int main() {
  init_matrice();
  char *enigme = genererate_random_enigme();
  printf("enigme generee : %s\n", enigme);
  free(enigme);
  }
*/
