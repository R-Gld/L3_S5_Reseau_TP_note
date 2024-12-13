#define _DEFAULT_SOURCE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

/* cst pour les coordonnées */
#define MIN_C 0
#define MAX_C 9
#define DIV 10.0

/**
 * Tire et renvoie un entier pseudo-aléatoire entre min et max bornes incluses.
 *
 * @param min valeur minimale de l'entier
 * @param max valeur maximale de l'entier
 */
int gen_int_rand(int min, int max) {
    return (int)((random()/(INT_MAX+1.0))*(max-min+1)+min);
}

/**
 * Génère aléatoirement les coordonnées d'un nombre de points (dans un repère orthonormé et ici entre 0 - inclus et 1 - non inclus, à la précision d'une décimale).
 *
 * @param tab_pt tableau des coordonnées des points
 * @param n nombre de points
 */
void pts_gen(double *tab_pt, int n){
  for (int i=0; i<2*n; i+=2){
    tab_pt[i] = gen_int_rand(MIN_C, MAX_C)/DIV;// abscisse
    tab_pt[i+1] = gen_int_rand(MIN_C, MAX_C)/DIV;// ordonnée
  }  
}

/**
 * Affiche les coordonnées d'un point (abscisse et ordonnée).
 *
 * @param abs abscisse
 * @param ord ordonnée
 */
void pt_print(double abs, double ord){
  printf("(%.2f,%.2f) ", abs, ord);
}

/**
 * Affiche les coordonnées des points d'un tableau (abscisse et ordonnée).
 *
 * @param tab_pt tableau des coordonnées des points
 * @param n nombre de points
 */
void pts_print(const double *tab_pt, int n){
  for (int i=0; i<2*n; i+=2){
    pt_print(tab_pt[i], tab_pt[i+1]);
  }
  printf("\n");
}

/**
 * Calcule les coordonnées du centre de gravité d'un ensemble de points dont les coordonnées (abscisse et ordonnée) sont mémorisées dans un tableau.
 *
 * @param tab_pt tableau des coordonnées des points
 * @param n nombre de points
 * @param cg_pt pointeur sur le centre de gravité
 */
void pts_cg(const double *tab_pt, int n, double *cg_pt){
  cg_pt[0]=0; cg_pt[1]=0;
  for (int i=0; i<2*n; i+=2) {
    cg_pt[0] += tab_pt[i]; cg_pt[1] += tab_pt[i+1];
  }
  cg_pt[0] /= n; cg_pt[1] /= n;
}

int main(int argc, char *argv[]){
  srandom(time(NULL));
  int n;    // nombre de points
  double  *pts_coord; // coordonnées des points
  // vérifier argument ligne de commande
  if (argc!=2) { fprintf(stderr,"%s nbPt\n", argv[0]); return 1; }
  // vérifier format argument ligne de commande
  n = atoi(argv[1]);
  if (n <= 0) { fprintf(stderr, "%s nbPt (int)\n", argv[0]); return 2; }

  // réserver mémoire
  pts_coord = calloc(n*2, sizeof(double));
  if (pts_coord == NULL) { perror("Allocation mémoire"); return 3; }
  
  // générer coordonnées des points et les afficher
  pts_gen(pts_coord, n);
  pts_print(pts_coord, n);
  
  // centre de gravité
  double cg[2];
  pts_cg(pts_coord, n, cg);
  printf("CG : "); pt_print(cg[0], cg[1]); printf("\n");
  
  // libérer mémoire
  free(pts_coord);
  
  return 0; 
}
