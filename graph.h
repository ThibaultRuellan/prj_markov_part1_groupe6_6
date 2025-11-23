#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>

// Structure représentant une cellule (arête) dans la liste d'adjacence
typedef struct s_cell {
    int destination;           // Sommet d'arrivée
    float probability;         // Probabilité de transition
    struct s_cell *next;      // Pointeur vers la cellule suivante
} t_cell;

// Structure représentant une liste chaînée
typedef struct {
    t_cell *head;             // Tête de la liste
} t_list;

// Structure représentant une liste d'adjacence (le graphe)
typedef struct {
    t_list *lists;            // Tableau de listes
    int nb_vertices;          // Nombre de sommets
} t_adjacency_list;

// Fonctions pour les cellules
t_cell *createCell(int destination, float probability);
void freeCell(t_cell *cell);

// Fonctions pour les listes
t_list *createEmptyList();
void addCellToList(t_list *list, int destination, float probability);
void displayList(t_list *list);
void freeList(t_list *list);

// Fonctions pour la liste d'adjacence
t_adjacency_list createAdjacencyList(int nb_vertices);
void addEdge(t_adjacency_list *adj_list, int start, int end, float probability);
void displayAdjacencyList(t_adjacency_list adj_list);
void freeAdjacencyList(t_adjacency_list *adj_list);

// Fonction de lecture depuis un fichier
t_adjacency_list readGraph(const char *filename);

// Fonction de vérification du graphe de Markov
int isMarkovGraph(t_adjacency_list adj_list);

// Fonction de génération du fichier Mermaid
void generateMermaidFile(t_adjacency_list adj_list, const char *output_filename);

#endif // GRAPH_H