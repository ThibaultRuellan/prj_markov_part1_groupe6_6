#include "graph.h"
#include "utils.h"
#include <string.h>
#include <math.h>

// Créer une cellule
t_cell *createCell(int destination, float probability) {
    t_cell *cell = (t_cell *)malloc(sizeof(t_cell));
    if (cell == NULL) {
        perror("Failed to allocate memory for cell");
        exit(EXIT_FAILURE);
    }
    cell->destination = destination;
    cell->probability = probability;
    cell->next = NULL;
    return cell;
}

// Libérer une cellule
void freeCell(t_cell *cell) {
    if (cell != NULL) {
        free(cell);
    }
}

// Créer une liste vide
t_list *createEmptyList() {
    t_list *list = (t_list *)malloc(sizeof(t_list));
    if (list == NULL) {
        perror("Failed to allocate memory for list");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    return list;
}

// Ajouter une cellule à une liste
void addCellToList(t_list *list, int destination, float probability) {
    t_cell *new_cell = createCell(destination, probability);
    new_cell->next = list->head;
    list->head = new_cell;
}

// Afficher une liste
void displayList(t_list *list) {
    t_cell *current = list->head;
    printf("[head @] -> ");
    while (current != NULL) {
        printf("(%d, %.2f) ", current->destination, current->probability);
        if (current->next != NULL) {
            printf("@-> ");
        }
        current = current->next;
    }
    printf("\n");
}

// Libérer une liste
void freeList(t_list *list) {
    if (list == NULL) return;

    t_cell *current = list->head;
    while (current != NULL) {
        t_cell *temp = current;
        current = current->next;
        freeCell(temp);
    }
    free(list);
}

// Créer une liste d'adjacence vide
t_adjacency_list createAdjacencyList(int nb_vertices) {
    t_adjacency_list adj_list;
    adj_list.nb_vertices = nb_vertices;
    adj_list.lists = (t_list *)malloc(nb_vertices * sizeof(t_list));

    if (adj_list.lists == NULL) {
        perror("Failed to allocate memory for adjacency list");
        exit(EXIT_FAILURE);
    }

    // Initialiser chaque liste comme vide
    for (int i = 0; i < nb_vertices; i++) {
        adj_list.lists[i].head = NULL;
    }

    return adj_list;
}

// Ajouter une arête
void addEdge(t_adjacency_list *adj_list, int start, int end, float probability) {
    // Les sommets sont numérotés de 1 à n, on ajuste pour l'indexation (0 à n-1)
    addCellToList(&adj_list->lists[start - 1], end, probability);
}

// Afficher la liste d'adjacence
void displayAdjacencyList(t_adjacency_list adj_list) {
    printf("\n=== Liste d'adjacence du graphe ===\n");
    for (int i = 0; i < adj_list.nb_vertices; i++) {
        printf("Liste pour le sommet %d:", i + 1);
        displayList(&adj_list.lists[i]);
    }
    printf("===================================\n\n");
}

// Libérer la liste d'adjacence
void freeAdjacencyList(t_adjacency_list *adj_list) {
    for (int i = 0; i < adj_list->nb_vertices; i++) {
        t_cell *current = adj_list->lists[i].head;
        while (current != NULL) {
            t_cell *temp = current;
            current = current->next;
            freeCell(temp);
        }
    }
    free(adj_list->lists);
}

// Lire un graphe depuis un fichier
t_adjacency_list readGraph(const char *filename) {
    FILE *file = fopen(filename, "rt"); // read-only, text
    int nbvert, depart, arrivee;
    float proba;
    t_adjacency_list adj_list;

    if (file == NULL) {
        perror("Could not open file for reading");
        exit(EXIT_FAILURE);
    }

    // Première ligne contient le nombre de sommets
    if (fscanf(file, "%d", &nbvert) != 1) {
        perror("Could not read number of vertices");
        exit(EXIT_FAILURE);
    }

    // Initialiser une liste d'adjacence vide
    adj_list = createAdjacencyList(nbvert);

    // Lire chaque arête
    while (fscanf(file, "%d %d %f", &depart, &arrivee, &proba) == 3) {
        addEdge(&adj_list, depart, arrivee, proba);
    }

    fclose(file);
    return adj_list;
}

// Vérifier si c'est un graphe de Markov
int isMarkovGraph(t_adjacency_list adj_list) {
    int is_valid = 1;

    printf("\n=== Vérification du graphe de Markov ===\n");

    for (int i = 0; i < adj_list.nb_vertices; i++) {
        float sum = 0.0f;
        t_cell *current = adj_list.lists[i].head;

        // Calculer la somme des probabilités sortantes
        while (current != NULL) {
            sum += current->probability;
            current = current->next;
        }

        // Vérifier si la somme est entre 0.99 et 1.01 (tolérance pour float)
        if (sum < 0.99f || sum > 1.01f) {
            printf("Le graphe n'est pas un graphe de Markov\n");
            printf("La somme des probabilités du sommet %d est %.2f\n", i + 1, sum);
            is_valid = 0;
        }
    }

    if (is_valid) {
        printf("Le graphe est un graphe de Markov\n");
    }
    printf("========================================\n\n");

    return is_valid;
}

// Générer un fichier au format Mermaid
void generateMermaidFile(t_adjacency_list adj_list, const char *output_filename) {
    FILE *file = fopen(output_filename, "w");
    if (file == NULL) {
        perror("Could not open file for writing");
        exit(EXIT_FAILURE);
    }

    // Écrire l'en-tête Mermaid
    fprintf(file, "---\n");
    fprintf(file, "config:\n");
    fprintf(file, "  layout: elk\n");
    fprintf(file, "  theme: neo\n");
    fprintf(file, "  look: neo\n");
    fprintf(file, "---\n\n");
    fprintf(file, "flowchart LR\n");

    // Déclarer les sommets
    for (int i = 0; i < adj_list.nb_vertices; i++) {
        char *id = getId(i + 1);
        fprintf(file, "%s((%d))\n", id, i + 1);
    }

    fprintf(file, "\n");

    // Écrire les arêtes
    for (int i = 0; i < adj_list.nb_vertices; i++) {
        t_cell *current = adj_list.lists[i].head;

        while (current != NULL) {
            char id_start_buf[10], id_end_buf[10];
            strcpy(id_start_buf, getId(i + 1));
            strcpy(id_end_buf, getId(current->destination));
            fprintf(file, "%s -->|%.2f|%s\n", id_start_buf, current->probability, id_end_buf);
            current = current->next;
        }
    }

    fclose(file);
    printf("Fichier Mermaid généré: %s\n", output_filename);
}