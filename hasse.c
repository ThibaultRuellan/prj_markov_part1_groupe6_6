#include "hasse.h"
#include <string.h>

// ============ Fonctions pour les liens ============

t_link_array createLinkArray() {
    t_link_array links;
    links.capacity = 20;
    links.nb_links = 0;
    links.links = (t_link *)malloc(links.capacity * sizeof(t_link));
    if (links.links == NULL) {
        perror("Failed to allocate memory for link array");
        exit(EXIT_FAILURE);
    }
    return links;
}

void addLink(t_link_array *links, int from_class, int to_class) {
    if (links->nb_links >= links->capacity) {
        links->capacity *= 2;
        links->links = (t_link *)realloc(links->links, links->capacity * sizeof(t_link));
        if (links->links == NULL) {
            perror("Failed to reallocate memory for link array");
            exit(EXIT_FAILURE);
        }
    }
    links->links[links->nb_links].from_class = from_class;
    links->links[links->nb_links].to_class = to_class;
    links->nb_links++;
}

int linkExists(t_link_array links, int from_class, int to_class) {
    for (int i = 0; i < links.nb_links; i++) {
        if (links.links[i].from_class == from_class &&
            links.links[i].to_class == to_class) {
            return 1;
        }
    }
    return 0;
}

void displayLinks(t_link_array links) {
    printf("\n=== Liens entre classes ===\n");
    for (int i = 0; i < links.nb_links; i++) {
        printf("C%d -> C%d\n", links.links[i].from_class + 1,
               links.links[i].to_class + 1);
    }
    printf("===========================\n\n");
}

void freeLinkArray(t_link_array *links) {
    if (links->links != NULL) {
        free(links->links);
        links->links = NULL;
    }
}

// ============ Trouver les liens entre classes ============

t_link_array findClassLinks(t_adjacency_list adj_list, t_partition partition) {
    t_link_array links = createLinkArray();

    // Créer la table de correspondance sommet -> classe
    int *vertex_to_class = createVertexToClassMap(partition, adj_list.nb_vertices);

    // Pour chaque sommet du graphe
    for (int i = 0; i < adj_list.nb_vertices; i++) {
        int vertex_id = i + 1;
        int class_i = vertex_to_class[i];

        // Parcourir tous les successeurs de ce sommet
        t_cell *current = adj_list.lists[i].head;
        while (current != NULL) {
            int successor = current->destination;
            int class_j = vertex_to_class[successor - 1];

            // Si les classes sont différentes et le lien n'existe pas encore
            if (class_i != class_j && !linkExists(links, class_i, class_j)) {
                addLink(&links, class_i, class_j);
            }

            current = current->next;
        }
    }

    free(vertex_to_class);
    return links;
}

// ============ Générer le diagramme de Hasse ============

void generateHasseDiagram(t_partition partition, t_link_array links, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Could not open file for writing Hasse diagram");
        exit(EXIT_FAILURE);
    }

    // En-tête Mermaid
    fprintf(file, "---\n");
    fprintf(file, "config:\n");
    fprintf(file, "  layout: elk\n");
    fprintf(file, "  theme: neo\n");
    fprintf(file, "  look: neo\n");
    fprintf(file, "---\n\n");
    fprintf(file, "flowchart TD\n");

    // Déclarer les classes
    for (int i = 0; i < partition.nb_classes; i++) {
        fprintf(file, "%s[\"%s: {", partition.classes[i].name, partition.classes[i].name);
        for (int j = 0; j < partition.classes[i].nb_vertices; j++) {
            fprintf(file, "%d", partition.classes[i].vertices[j]);
            if (j < partition.classes[i].nb_vertices - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "}\"]\n");
    }

    fprintf(file, "\n");

    // Écrire les liens
    for (int i = 0; i < links.nb_links; i++) {
        fprintf(file, "C%d --> C%d\n",
                links.links[i].from_class + 1,
                links.links[i].to_class + 1);
    }

    fclose(file);
    printf("Diagramme de Hasse généré: %s\n", filename);
}

// ============ Analyser les caractéristiques du graphe ============

void analyzeGraphCharacteristics(t_adjacency_list adj_list, t_partition partition,
                                t_link_array links) {
    printf("\n=== Caractéristiques du graphe de Markov ===\n\n");

    // Déterminer quelles classes sont transitoires ou persistantes
    int *is_transient = (int *)calloc(partition.nb_classes, sizeof(int));

    // Une classe est transitoire s'il existe un lien sortant
    for (int i = 0; i < links.nb_links; i++) {
        is_transient[links.links[i].from_class] = 1;
    }

    // Afficher les classes transitoires et persistantes
    int nb_persistent = 0;
    for (int i = 0; i < partition.nb_classes; i++) {
        printf("Classe C%d: ", i + 1);
        displayClass(partition.classes[i]);

        if (is_transient[i]) {
            printf("  -> Cette classe est TRANSITOIRE\n");
            printf("  -> Tous les états de cette classe sont transitoires\n");
        } else {
            printf("  -> Cette classe est PERSISTANTE\n");
            printf("  -> Tous les états de cette classe sont persistants\n");
            nb_persistent++;

            // Vérifier si c'est un état absorbant
            if (partition.classes[i].nb_vertices == 1) {
                printf("  -> L'état %d est ABSORBANT\n", partition.classes[i].vertices[0]);
            }
        }
        printf("\n");
    }

    // Vérifier si le graphe est irréductible
    printf("Le graphe est ");
    if (partition.nb_classes == 1) {
        printf("IRRÉDUCTIBLE (une seule classe)\n");
    } else {
        printf("NON IRRÉDUCTIBLE (%d classes)\n", partition.nb_classes);
    }

    printf("\n============================================\n\n");

    free(is_transient);
}

// ============ Supprimer les liens transitifs (OPTIONNEL) ============

void removeTransitiveLinks(t_link_array *p_link_array) {
    int n = p_link_array->nb_links;
    if (n == 0) return;

    // Marquer les liens à supprimer
    int *to_remove = (int *)calloc(n, sizeof(int));

    // Pour chaque paire de liens (i->j) et (j->k)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) continue;

            // Si on a i->j
            if (p_link_array->links[j].from_class == p_link_array->links[i].to_class) {
                // Chercher si on a i->k où k = destination de j
                for (int k = 0; k < n; k++) {
                    if (k == i || k == j) continue;

                    if (p_link_array->links[k].from_class == p_link_array->links[i].from_class &&
                        p_link_array->links[k].to_class == p_link_array->links[j].to_class) {
                        // Marquer le lien i->k comme redondant
                        to_remove[k] = 1;
                    }
                }
            }
        }
    }

    // Créer un nouveau tableau sans les liens marqués
    t_link_array new_array = createLinkArray();
    for (int i = 0; i < n; i++) {
        if (!to_remove[i]) {
            addLink(&new_array, p_link_array->links[i].from_class,
                   p_link_array->links[i].to_class);
        }
    }

    // Remplacer l'ancien tableau
    free(p_link_array->links);
    *p_link_array = new_array;

    free(to_remove);
}
