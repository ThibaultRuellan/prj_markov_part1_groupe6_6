#ifndef MATRIX_H
#define MATRIX_H

#include "graph.h"
#include "tarjan.h"

// Structure pour une matrice
typedef struct {
    float **data;          // Tableau 2D de floats
    int rows;              // Nombre de lignes
    int cols;              // Nombre de colonnes
} t_matrix;

// Fonctions de base pour les matrices
t_matrix createMatrix(int n);
t_matrix createEmptyMatrix(int n);
void freeMatrix(t_matrix *matrix);
void displayMatrix(t_matrix matrix);

// Création de matrice depuis un graphe
t_matrix adjacencyListToMatrix(t_adjacency_list adj_list);

// Opérations matricielles
void copyMatrix(t_matrix dest, t_matrix src);
void multiplyMatrices(t_matrix a, t_matrix b, t_matrix result);
float matrixDifference(t_matrix m, t_matrix n);

// Calcul de puissance de matrice
t_matrix matrixPower(t_matrix matrix, int power);

// Extraction de sous-matrice pour une classe
t_matrix subMatrix(t_matrix matrix, t_partition part, int compo_index);

// Calcul de distribution stationnaire
void computeStationaryDistribution(t_adjacency_list adj_list, t_partition partition,
                                  float epsilon);

// Calcul de période (BONUS)
int gcd(int *vals, int nbvals);
int getPeriod(t_matrix sub_matrix);

#endif // MATRIX_H
