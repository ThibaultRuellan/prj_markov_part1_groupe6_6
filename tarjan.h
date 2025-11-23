#ifndef TARJAN_H
#define TARJAN_H

#include "graph.h"

// Structure pour un sommet dans l'algorithme de Tarjan
typedef struct {
    int id;                // Identifiant du sommet dans le graphe
    int num;               // Numéro dans l'ordre de parcours
    int accessible;        // Numéro accessible (lowlink)
    int in_stack;          // Indicateur si le sommet est dans la pile
} t_tarjan_vertex;

// Structure pour une classe (composante fortement connexe)
typedef struct {
    char name[10];         // Nom de la classe (C1, C2, etc.)
    int *vertices;         // Tableau des sommets dans cette classe
    int nb_vertices;       // Nombre de sommets dans cette classe
    int capacity;          // Capacité du tableau
} t_class;

// Structure pour une partition (ensemble de classes)
typedef struct {
    t_class *classes;      // Tableau de classes
    int nb_classes;        // Nombre de classes
    int capacity;          // Capacité du tableau
} t_partition;

// Pile pour l'algorithme de Tarjan
typedef struct s_stack_node {
    int vertex_id;
    struct s_stack_node *next;
} t_stack_node;

typedef struct {
    t_stack_node *top;
} t_stack;

// Fonctions pour la pile
t_stack *createStack();
void push(t_stack *stack, int vertex_id);
int pop(t_stack *stack);
int isEmpty(t_stack *stack);
void freeStack(t_stack *stack);

// Fonctions pour les classes
t_class createClass(const char *name);
void addVertexToClass(t_class *classe, int vertex_id);
void displayClass(t_class classe);
void freeClass(t_class *classe);

// Fonctions pour la partition
t_partition createPartition();
void addClassToPartition(t_partition *partition, t_class classe);
void displayPartition(t_partition partition);
void freePartition(t_partition *partition);

// Fonctions pour l'algorithme de Tarjan
t_tarjan_vertex *initTarjanVertices(t_adjacency_list adj_list);
void parcours(int vertex_id, t_adjacency_list adj_list, t_tarjan_vertex *vertices,
              t_stack *stack, int *num_counter, t_partition *partition);
t_partition tarjan(t_adjacency_list adj_list);

// Fonction pour créer un tableau associant chaque sommet à sa classe
int *createVertexToClassMap(t_partition partition, int nb_vertices);

#endif // TARJAN_H
