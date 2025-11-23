#ifndef HASSE_H
#define HASSE_H

#include "graph.h"
#include "tarjan.h"

// Structure pour un lien entre classes
typedef struct {
    int from_class;        // Classe de départ
    int to_class;          // Classe d'arrivée
} t_link;

// Structure pour stocker tous les liens
typedef struct {
    t_link *links;         // Tableau de liens
    int nb_links;          // Nombre de liens
    int capacity;          // Capacité du tableau
} t_link_array;

// Fonctions pour les liens
t_link_array createLinkArray();
void addLink(t_link_array *links, int from_class, int to_class);
int linkExists(t_link_array links, int from_class, int to_class);
void displayLinks(t_link_array links);
void freeLinkArray(t_link_array *links);

// Fonction pour trouver les liens entre classes
t_link_array findClassLinks(t_adjacency_list adj_list, t_partition partition);

// Fonction pour générer le diagramme de Hasse au format Mermaid
void generateHasseDiagram(t_partition partition, t_link_array links, const char *filename);

// Fonction pour supprimer les liens transitifs (OPTIONNEL)
void removeTransitiveLinks(t_link_array *p_link_array);

// Fonctions pour déterminer les caractéristiques du graphe
void analyzeGraphCharacteristics(t_adjacency_list adj_list, t_partition partition,
                                t_link_array links);

#endif // HASSE_H
