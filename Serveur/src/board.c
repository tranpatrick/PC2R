#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"
#include "../include/funcserver.h"

int cibles[16][16]; /* Tableau des cibles possibles */
int pos[16][16]; /* Tableau des positions de robot et cibles impossibles */

square* create_square(){
  square *res = (square*) malloc(sizeof(square));
  res->top_wall = 0;
  res->left_wall = 0;
  res->right_wall = 0;
  res->bottom_wall = 0;
  res->robot = '0';
  return res;
}

board* create_terrain(char *plateau){
  board *res = (board*) malloc(sizeof(board));
  /*res->tab = (square***) malloc(16*sizeof(square**));*/
  int i,j, x,y;
  char mur;
  char buffer[10];

  res->xc = 18;

  /*  for(i=0; i<16; i++){
      res->tab[i] = malloc(sizeof(square***)*16);
      }*/

  /* Création et initialisation des cases à vide */
  for(i=0; i<16; i++){
    for(j=0; j<16; j++){
      res->tab[i][j] = create_square();
    }
  }

  /* Initialisation des murs */
  i = 0;
  j = 0; /* j sera l'indice de fin de la chaine buffer */
  while(plateau[i] != '\0'){
    if(plateau[i] != ')'){
      /* ajout du char dans buffer */
      buffer[j] = plateau[i];
      j++;
    }else{
      sscanf(buffer, "(%d,%d,%c)", &x, &y, &mur);
      switch(mur){
      case 'G':
	res->tab[x][y]->left_wall = 1;
	break;
      case 'D':
	res->tab[x][y]->right_wall = 1;
	break;
      case 'H':
	res->tab[x][y]->top_wall = 1;
	break;
      case 'B':
	res->tab[x][y]->bottom_wall = 1;
	break;
      default:
	break;
      }
      memset(buffer, '\0', 10);
      j = 0;
    }
    i++;
  }

  /*  for(i=0; i<16; i++){
      for(j=0; j<16; j++){
      printf("%d ", res->tab[i][j]->left_wall);
      }
      printf("\n");
      } */ 

  return res;
}

board* create_board(char* plateau, char* enigme){
  board *res = (board*) malloc(sizeof(board));
  int i, j, x, y;
  int xr, yr, xb, yb, xj, yj, xv, yv, xc, yc;
  char color;
  char mur;
  char buffer[10];
  
  /* Création et initialisation des cases à vide */
  for(i=0; i<16; i++){
    for(j=0; j<16; j++){
      res->tab[i][j] = create_square();
    }
  }

  /* Initialisation des murs */
  i = 0;
  j = 0; /* j sera l'indice de fin de la chaine buffer */
  while(plateau[i] != '\0'){
    if(plateau[i] != ')'){
      /* ajout du char dans buffer */
      buffer[j] = plateau[i];
      j++;
    }else{
      sscanf(buffer, "(%d,%d,%c)", &x, &y, &mur);
      switch(mur){
      case 'G':
	res->tab[x][y]->left_wall = 1;
	break;
      case 'D':
	res->tab[x][y]->right_wall = 1;
	break;
      case 'H':
	res->tab[x][y]->top_wall = 1;
	break;
      case 'B':
	res->tab[x][y]->bottom_wall = 1;
	break;
      default:
	break;
      }
      memset(buffer, '\0', 10);
      j = 0;
    }
    i++;
  }

  /* Initialisation des positions des robots et de la cible */
  sscanf(enigme, "(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%c)",
	 &xr, &yr, &xb, &yb, &xj, &yj, &xv, &yv, &xc, &yc, &color);
  res->tab[xr][yr]->robot = 'R';
  res->tab[xb][yb]->robot = 'B';
  res->tab[xj][yj]->robot = 'J';
  res->tab[xv][yv]->robot = 'V';
  /*res->tab[xc][yc]->is_target = 1;*/

  /* Spécifier la position des robots */
  res->xr = xr;
  res->yr = yr;
  res->xb = xb;
  res->yb = yb;
  res->xj = xj;
  res->yj = yj;
  res->xv = xv;
  res->yv = yv;

  /* Spécifier la couleur la case cible */
  res->xc = xc;
  res->yc = yc;
  /* Spécifier la couleur du robot à amener sur la cible */
  res->color = color;
  return res;
}

int simulation(char *desc_plateau, char *enigme, char *solution){
  printf("[board.c] DEBUT SIMULATION\n");
  char couleur, direction;
  int i = 0;
  int xr, yr, xb, yb, xj, yj, xv, yv, xc, yc;
  
  pthread_mutex_lock(&mutex_compteur_coups);
  compteur_coups = 0;
  pthread_mutex_unlock(&mutex_compteur_coups);
  
  board *plateau = create_board(desc_plateau, enigme);
  xc = plateau->xc;
  yc = plateau->yc;
  xr = plateau->xr;
  yr = plateau->yr;
  xb = plateau->xb;
  yb = plateau->yb;
  xj = plateau->xj;
  yj = plateau->yj;
  xv = plateau->xv;
  yv = plateau->yv;

  printf("Simulation : Après initialisation\n");

  while(solution[i] != '\0'){
    if(i%2 != 0){
      pthread_mutex_lock(&mutex_compteur_coups);
      compteur_coups++;
      pthread_mutex_unlock(&mutex_compteur_coups);
      direction = solution[i];
      
      /* Déplacement du robot */
      switch(couleur){
      case 'R':
	printf("R\n");
	switch(direction){
	case 'H':
	  xr = plateau->xr;
	  yr = plateau->yr;
	  while(xr > 0 && plateau->tab[xr][yr]->top_wall == 0
		&& plateau->tab[xr-1][yr]->bottom_wall == 0
		&& plateau->tab[xr-1][yr]->robot == '0'){
	    plateau->tab[xr][yr]->robot = '0';
	    xr--;
	    plateau->tab[xr][yr]->robot = 'R';
	    plateau->xr--;
	  }
	  break;
	case 'B':
	  xr = plateau->xr;
	  yr = plateau->yr;
	  while(xr < 15 && plateau->tab[xr][yr]->bottom_wall == 0
		&& plateau->tab[xr+1][yr]->top_wall == 0
		&& plateau->tab[xr+1][yr]->robot == '0'){
	    plateau->tab[xr][yr]->robot = '0';
	    xr++;
	    plateau->tab[xr][yr]->robot = 'R';
	    plateau->xr++;
	  }
	  break;
	case 'D':
	  xr = plateau->xr;
	  yr = plateau->yr;
	  while(yr < 15 && plateau->tab[xr][yr]->right_wall == 0
		&& plateau->tab[xr][yr+1]->left_wall == 0
		&& plateau->tab[xr][yr+1]->robot == '0'){
	    plateau->tab[xr][yr]->robot = '0';
	    yr++;
	    plateau->tab[xr][yr]->robot = 'R';
	    plateau->yr++;
	  }
	  break;
	case 'G':
	  xr = plateau->xr;
	  yr = plateau->yr;
	  while(yr > 0 && plateau->tab[xr][yr]->left_wall == 0
		&& plateau->tab[xr][yr-1]->right_wall == 0
		&& plateau->tab[xr][yr-1]->robot == '0'){
	    plateau->tab[xr][yr]->robot = '0';
	    yr--;
	    plateau->tab[xr][yr]->robot = 'R';
	    plateau->yr--;
	  }
	  break;
	default:
	  break;
	}
	break;
	
      case 'B':
	switch(direction){
	case 'H':
	  xb = plateau->xb;
	  yb = plateau->yb;
	  while(xb > 0 && plateau->tab[xb][yb]->top_wall == 0
		&& plateau->tab[xb-1][yb]->bottom_wall == 0
		&& plateau->tab[xb-1][yb]->robot == '0'){
	    plateau->tab[xb][yb]->robot = '0';
	    xb--;
	    plateau->tab[xb][yb]->robot = 'B';
	    plateau->xb--;
	  }
	  break;
	case 'B':
	  xb = plateau->xb;
	  yb = plateau->yb;
	  while(xb < 15 && plateau->tab[xb][yb]->bottom_wall == 0
		&& plateau->tab[xb+1][yb]->top_wall == 0
		&& plateau->tab[xb+1][yb]->robot == '0'){
	    plateau->tab[xb][yb]->robot = '0';
	    xb++;
	    plateau->tab[xb][yb]->robot = 'B';
	    plateau->xb++;
	  }
	  break;
	case 'D':
	  xb = plateau->xb;
	  yb = plateau->yb;
	  while(yb < 15 && plateau->tab[xb][yb]->right_wall == 0
		&& plateau->tab[xb][yb+1]->left_wall == 0
		&& plateau->tab[xb][yb+1]->robot == '0'){
	    plateau->tab[xb][yb]->robot = '0';
	    yb++;
	    plateau->tab[xb][yb]->robot = 'B';
	    plateau->yb++;
	  }
	  break;
	case 'G':
	  xb = plateau->xb;
	  yb = plateau->yb;
	  while(yb > 0 && plateau->tab[xb][yb]->left_wall == 0
		&& plateau->tab[xb][yb-1]->right_wall == 0
		&& plateau->tab[xb][yb-1]->robot == '0'){
	    plateau->tab[xb][yb]->robot = '0';
	    yb--;
	    plateau->tab[xb][yb]->robot = 'B';
	    plateau->yb--;
	  }
	  break;
	default:
	  break;
	}
	break;
	
      case 'J':
	switch(direction){
	case 'H':
	  xj = plateau->xj;
	  yj = plateau->yj;
	  while(xj > 0 && plateau->tab[xj][yj]->top_wall == 0
		&& plateau->tab[xj-1][yj]->bottom_wall == 0
		&& plateau->tab[xj-1][yj]->robot == '0'){
	    plateau->tab[xj][yj]->robot = '0';
	    xj--;
	    plateau->tab[xj][yj]->robot = 'J';
	    plateau->xj--;
	  }
	  break;
	case 'B':
	  xj = plateau->xj;
	  yj = plateau->yj;
	  while(xj < 15 && plateau->tab[xj][yj]->bottom_wall == 0
		&& plateau->tab[xj+1][yj]->top_wall == 0
		&& plateau->tab[xj+1][yj]->robot == '0'){
	    plateau->tab[xj][yj]->robot = '0';
	    xj++;
	    plateau->tab[xj][yj]->robot = 'J';
	    plateau->xj++;
	  }
	  break;
	case 'D':
	  xj = plateau->xj;
	  yj = plateau->yj;
	  while(yj < 15 && plateau->tab[xj][yj]->right_wall == 0
		&& plateau->tab[xj][yj+1]->left_wall == 0
		&& plateau->tab[xj][yj+1]->robot == '0'){
	    plateau->tab[xj][yj]->robot = '0';
	    yj++;
	    plateau->tab[xj][yj]->robot = 'J';
	    plateau->yj++;
	  }
	  break;
	case 'G':
	  xj = plateau->xj;
	  yj = plateau->yj;
	  while(yj > 0 && plateau->tab[xj][yj]->left_wall == 0
		&& plateau->tab[xj][yj-1]->right_wall == 0
		&& plateau->tab[xj][yj-1]->robot == '0'){
	    plateau->tab[xj][yj]->robot = '0';
	    yj--;
	    plateau->tab[xj][yj]->robot = 'J';
	    plateau->yj--;
	  }
	  break;
	default:
	  break;
	}
	break;

      case 'V':
	switch(direction){
	case 'H':
	  xv = plateau->xv;
	  yv = plateau->yv;
	  while(xv > 0 && plateau->tab[xv][yv]->top_wall == 0
		&& plateau->tab[xv-1][yv]->bottom_wall == 0
		&& plateau->tab[xv-1][yv]->robot == '0'){
	    plateau->tab[xv][yv]->robot = '0';
	    xv--;
	    plateau->tab[xv][yv]->robot = 'V';
	    plateau->xv--;
	  }
	  break;
	case 'B':
	  xv = plateau->xv;
	  yv = plateau->yv;
	  while(xv < 15 && plateau->tab[xv][yv]->bottom_wall == 0
		&& plateau->tab[xv+1][yv]->top_wall == 0
		&& plateau->tab[xv+1][yv]->robot == '0'){
	    plateau->tab[xv][yv]->robot = '0';
	    xv++;
	    plateau->tab[xv][yv]->robot = 'V';
	    plateau->xv++;
	  }
	  break;
	case 'D':
	  xv = plateau->xv;
	  yv = plateau->yv;
	  while(yv < 15 && plateau->tab[xv][yv]->right_wall == 0
		&& plateau->tab[xv][yv+1]->left_wall == 0
		&& plateau->tab[xv][yv+1]->robot == '0'){
	    plateau->tab[xv][yv]->robot = '0';
	    yv++;
	    plateau->tab[xv][yv]->robot = 'V';
	    plateau->yv++;
	  }
	  break;
	case 'G':
	  xv = plateau->xv;
	  yv = plateau->yv;
	  while(yv > 0 && plateau->tab[xv][yv]->left_wall == 0
		&& plateau->tab[xv][yv-1]->right_wall == 0
		&& plateau->tab[xv][yv-1]->robot == '0'){
	    plateau->tab[xv][yv]->robot = '0';
	    yv--;
	    plateau->tab[xv][yv]->robot = 'V';
	    plateau->yv--;
	  }
	  break;
	default:
	  break;
	}
	break;
      default:
	break;
      }    
    }else{
      couleur = solution[i];
    }
    i++;
  }

  printf("Avant détermination de solution\n");

  /* Déterminer si la solution est bonne */
  switch(plateau->color){
  case 'R':
    if(xr == xc && yr == yc) {
      return 1;
    }
    else {
      return 0;
    }
    break;
  case 'B':
    if(xb == xc && yb == yc) {
      return 1;
    }
    else {
      return 0;
    }
    break;
  case 'J':
    if(xj == xc && yj == yc) {
      return 1;
    }
    else {
      return 0;
    }
    break;
  case 'V':
    if(xv == xc && yv == yc) {
      return 1;
    }
    else {
      return 0;
    }
    break;
  default:
    break;
  }
  return 0;
  
}


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
  
  pthread_mutex_lock(&mutex_num_plateau);
  int num = num_plateau;
  pthread_mutex_unlock(&mutex_num_plateau);

  board *tmp = create_terrain(plateaux[num_plateau]);
  
  /*  for(i=0; i<16; i++){
      for(j=0; j<16; j++){
      printf("%d ", tmp->tab[i][j]->left_wall);
      }
      printf("\n");
      } */ 

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
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
  
  /* cibles[0][0] = 1;
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
     cibles[15][15] = 1;*/
  printf("FIN init_matrice\n");

}

int *get_random_cible() {

  int *cible = (int*) malloc(2*sizeof(int));
  srand(time(NULL));
  
  int limite;
  int i, j, cpt = 0;
  
  for(i = 0; i<16; i++) {
    for(j = 0; j<16; j++) {
      if (cibles[i][j] == 0) 
	continue;
      else {
	cpt++;
      }
    }
  }
  limite = rand()%cpt;
  cpt = 0;

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

  if (!(i < 16 && i >= 0) || !(j >= 0 && j < 16) || pos[i][j]  == 1) {
    return NULL;
  }
  cible[0] = i;
  cible[1] = j;
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
  int *cible = NULL;
  while (cible == NULL) {
    cible  = get_random_cible();
  };
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
