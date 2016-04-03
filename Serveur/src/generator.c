#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int cibles[16][16];

void init_matrice() {
  printf("DEBUT init_matrice\n");
  int i, j;
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      cibles[i][j] = 0;
    }
  }
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
  printf("DEBUT get_random_cible\n");
  int *cible = (int*) malloc(2*sizeof(int));
  srand(time(NULL));
  int limite = rand()%34;
  int i, j, cpt = 0;
  for(i = 0; i<16; i++) {
    for(j = 0; j<16; j++) {
      if (cibles[i][j] == 0) 
	continue;
      else {
	printf("i = %d, j = %d\n", i, j);
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
    i = 5;
  if (j < 16 && j >= 0) 
    cible[1] = j;
  else
    j = 8;
  printf("[generator.c] get_random_cible : i = %d; j = %d\n", i, j);
  printf("FIN get_random_cible\n");
  return cible;
}

char *genererate_random_enigme() {

  printf("DEBUT generate_random_cible\n");
  init_matrice();
  srand(getpid());
  int xr, yr ,xb, yb, xj, yj, xv, yv, xc, yc;
  xr = rand()%16;
  yr = rand()%16;
  xb = rand()%16;
  yb = rand()%16;
  xj = rand()%16;
  yj = rand()%16;
  xv = rand()%16;
  yv = rand()%16;
  int *cible = get_random_cible();
  xc = cible[0];
  yc = cible[1];
  printf("[generator.c] genererate : xc = %d; yc = %d\n", xc, yc);
  char *color;
  double alea = rand()%20;
  printf("alea = %f\n", alea);
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
