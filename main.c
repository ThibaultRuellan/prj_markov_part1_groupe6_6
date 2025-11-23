#include "graph.h"
#include "tarjan.h"
#include "hasse.h"
#include "matrix.h"
#include "utils.h"
#include <string.h>

void printUsage() {
    printf("\n=== Programme d'analyse de graphes de Markov ===\n\n");
    printf("Usage: ./markov <fichier_graphe> [options]\n\n");
    printf("Options:\n");
    printf("  --partie1     : Afficher le graphe et le vérifier (PARTIE 1)\n");
    printf("  --partie2     : Analyser les composantes connexes (PARTIE 2)\n");
    printf("  --partie3     : Calculer les distributions stationnaires (PARTIE 3)\n");
    printf("  --all         : Exécuter toutes les parties\n");
    printf("  --help        : Afficher cette aide\n\n");
    printf("Exemples:\n");
    printf("  ./markov exemple1.txt --all\n");
    printf("  ./markov exemple_meteo.txt --partie3\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erreur: Fichier graphe manquant\n");
        printUsage();
        return EXIT_FAILURE;
    }

    // Vérifier l'option --help
    if (strcmp(argv[1], "--help") == 0) {
        printUsage();
        return EXIT_SUCCESS;
    }

    const char *filename = argv[1];

    // Déterminer quelles parties exécuter
    int run_partie1 = 0;
    int run_partie2 = 0;
    int run_partie3 = 0;

    if (argc == 2) {
        // Par défaut, exécuter toutes les parties
        run_partie1 = run_partie2 = run_partie3 = 1;
    } else {
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--partie1") == 0) {
                run_partie1 = 1;
            } else if (strcmp(argv[i], "--partie2") == 0) {
                run_partie2 = 1;
            } else if (strcmp(argv[i], "--partie3") == 0) {
                run_partie3 = 1;
            } else if (strcmp(argv[i], "--all") == 0) {
                run_partie1 = run_partie2 = run_partie3 = 1;
            }
        }
    }

    printf("\n========================================\n");
    printf("   ANALYSE DE GRAPHE DE MARKOV\n");
    printf("========================================\n");
    printf("Fichier: %s\n", filename);
    printf("========================================\n\n");

    // ========== PARTIE 1 : Créer et vérifier le graphe ==========

    printf("Chargement du graphe...\n");
    t_adjacency_list adj_list = readGraph(filename);
    printf("Graphe chargé: %d sommets\n", adj_list.nb_vertices);

    if (run_partie1) {
        printf("\n========== PARTIE 1 ==========\n");

        // Afficher la liste d'adjacence
        displayAdjacencyList(adj_list);

        // Vérifier si c'est un graphe de Markov
        int is_valid = isMarkovGraph(adj_list);

        if (!is_valid) {
            printf("\nAttention: Le graphe n'est pas valide!\n");
        }

        // Générer le fichier Mermaid
        char mermaid_file[256];
        snprintf(mermaid_file, sizeof(mermaid_file), "%s_graph.mmd", filename);
        generateMermaidFile(adj_list, mermaid_file);

        printf("\n========== FIN PARTIE 1 ==========\n\n");
    }

    // ========== PARTIE 2 : Algorithme de Tarjan et Hasse ==========

    t_partition partition;
    t_link_array links;

    if (run_partie2 || run_partie3) {
        printf("\n========== PARTIE 2 ==========\n");

        // Appliquer l'algorithme de Tarjan
        printf("Application de l'algorithme de Tarjan...\n");
        partition = tarjan(adj_list);

        // Afficher la partition
        displayPartition(partition);

        // Trouver les liens entre classes
        printf("Recherche des liens entre classes...\n");
        links = findClassLinks(adj_list, partition);
        displayLinks(links);

        // Générer le diagramme de Hasse
        char hasse_file[256];
        snprintf(hasse_file, sizeof(hasse_file), "%s_hasse.mmd", filename);
        generateHasseDiagram(partition, links, hasse_file);

        // Analyser les caractéristiques
        analyzeGraphCharacteristics(adj_list, partition, links);

        printf("\n========== FIN PARTIE 2 ==========\n\n");
    }

    // ========== PARTIE 3 : Calculs matriciels et distributions ==========

    if (run_partie3) {
        printf("\n========== PARTIE 3 ==========\n");

        // Créer la matrice de transition
        t_matrix M = adjacencyListToMatrix(adj_list);
        printf("Matrice de transition créée\n");
        displayMatrix(M);

        // Calculer M^3
        printf("Calcul de M^3:\n");
        t_matrix M3 = matrixPower(M, 3);
        displayMatrix(M3);
        freeMatrix(&M3);

        // Calculer M^7
        printf("Calcul de M^7:\n");
        t_matrix M7 = matrixPower(M, 7);
        displayMatrix(M7);
        freeMatrix(&M7);

        // Trouver la convergence
        printf("Recherche de la convergence (epsilon = 0.01)...\n");
        t_matrix Mn_prev = createEmptyMatrix(M.rows);
        copyMatrix(Mn_prev, M);

        int n = 1;
        float diff;
        do {
            n++;
            t_matrix Mn = matrixPower(M, n);
            diff = matrixDifference(Mn, Mn_prev);

            if (diff < 0.01f) {
                printf("Convergence atteinte à M^%d (différence = %.6f)\n", n, diff);
                displayMatrix(Mn);
            }

            copyMatrix(Mn_prev, Mn);
            freeMatrix(&Mn);

            if (n > 100) {
                printf("Pas de convergence après 100 itérations\n");
                break;
            }
        } while (diff >= 0.01f);

        freeMatrix(&Mn_prev);

        // Calculer les distributions stationnaires par classe
        computeStationaryDistribution(adj_list, partition, 0.01f);

        // BONUS: Calculer les périodes
        printf("\n=== BONUS: Calcul des périodes ===\n");
        for (int i = 0; i < partition.nb_classes; i++) {
            t_matrix sub = subMatrix(M, partition, i);
            int period = getPeriod(sub);
            printf("Classe C%d: période = %d\n", i + 1, period);
            freeMatrix(&sub);
        }
        printf("===================================\n\n");

        freeMatrix(&M);

        printf("\n========== FIN PARTIE 3 ==========\n\n");
    }

    // Libérer la mémoire
    if (run_partie2 || run_partie3) {
        freeLinkArray(&links);
        freePartition(&partition);
    }
    freeAdjacencyList(&adj_list);

    printf("\n========================================\n");
    printf("   ANALYSE TERMINÉE\n");
    printf("========================================\n\n");

    return EXIT_SUCCESS;
}
