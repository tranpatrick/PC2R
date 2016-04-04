#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"
#include "../include/funcserver.h"

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
 int i,j, x,y;
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

  while(solution[i] != '\0'){
       if(i%2 != 0){
      pthread_mutex_lock(&mutex_compteur_coups);
      compteur_coups++;
      pthread_mutex_unlock(&mutex_compteur_coups);
      direction = solution[i];
      
      /* Déplacement du robot */
           switch(couleur){
      case 'R':
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
