#include "matrix.h"
#include <math.h>
#include <string.h>

// ============ Fonctions de base pour les matrices ============

t_matrix createMatrix(int n) {
    t_matrix matrix;
    matrix.rows = n;
    matrix.cols = n;

    matrix.data = (float **)malloc(n * sizeof(float *));
    if (matrix.data == NULL) {
        perror("Failed to allocate memory for matrix rows");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        matrix.data[i] = (float *)malloc(n * sizeof(float));
        if (matrix.data[i] == NULL) {
            perror("Failed to allocate memory for matrix columns");
            exit(EXIT_FAILURE);
        }
    }

    return matrix;
}

t_matrix createEmptyMatrix(int n) {
    t_matrix matrix = createMatrix(n);

    // Initialiser à 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix.data[i][j] = 0.0f;
        }
    }

    return matrix;
}

void freeMatrix(t_matrix *matrix) {
    if (matrix->data != NULL) {
        for (int i = 0; i < matrix->rows; i++) {
            if (matrix->data[i] != NULL) {
                free(matrix->data[i]);
            }
        }
        free(matrix->data);
        matrix->data = NULL;
    }
}

void displayMatrix(t_matrix matrix) {
    printf("\nMatrice %dx%d:\n", matrix.rows, matrix.cols);
    for (int i = 0; i < matrix.rows; i++) {
        for (int j = 0; j < matrix.cols; j++) {
            printf("%.2f ", matrix.data[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// ============ Création de matrice depuis un graphe ============

t_matrix adjacencyListToMatrix(t_adjacency_list adj_list) {
    int n = adj_list.nb_vertices;
    t_matrix matrix = createEmptyMatrix(n);

    // Remplir la matrice avec les probabilités
    for (int i = 0; i < n; i++) {
        t_cell *current = adj_list.lists[i].head;

        while (current != NULL) {
            int dest = current->destination - 1;  // Conversion à 0-indexé
            matrix.data[i][dest] = current->probability;
            current = current->next;
        }
    }

    return matrix;
}

// ============ Opérations matricielles ============

void copyMatrix(t_matrix dest, t_matrix src) {
    if (dest.rows != src.rows || dest.cols != src.cols) {
        fprintf(stderr, "Error: matrix dimensions don't match for copy\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            dest.data[i][j] = src.data[i][j];
        }
    }
}

void multiplyMatrices(t_matrix a, t_matrix b, t_matrix result) {
    if (a.cols != b.rows || a.rows != result.rows || b.cols != result.cols) {
        fprintf(stderr, "Error: incompatible matrix dimensions for multiplication\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < b.cols; j++) {
            result.data[i][j] = 0.0f;
            for (int k = 0; k < a.cols; k++) {
                result.data[i][j] += a.data[i][k] * b.data[k][j];
            }
        }
    }
}

float matrixDifference(t_matrix m, t_matrix n) {
    if (m.rows != n.rows || m.cols != n.cols) {
        fprintf(stderr, "Error: matrix dimensions don't match for difference\n");
        exit(EXIT_FAILURE);
    }

    float diff = 0.0f;
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            diff += fabsf(m.data[i][j] - n.data[i][j]);
        }
    }

    return diff;
}

// ============ Calcul de puissance de matrice ============

t_matrix matrixPower(t_matrix matrix, int power) {
    if (power < 0) {
        fprintf(stderr, "Error: power must be non-negative\n");
        exit(EXIT_FAILURE);
    }

    t_matrix result = createEmptyMatrix(matrix.rows);

    if (power == 0) {
        // Matrice identité
        for (int i = 0; i < matrix.rows; i++) {
            result.data[i][i] = 1.0f;
        }
        return result;
    }

    // Copier la matrice pour power = 1
    copyMatrix(result, matrix);

    // Multiplier power-1 fois
    t_matrix temp = createEmptyMatrix(matrix.rows);
    for (int p = 1; p < power; p++) {
        multiplyMatrices(result, matrix, temp);
        copyMatrix(result, temp);
    }

    freeMatrix(&temp);
    return result;
}

// ============ Extraction de sous-matrice ============

t_matrix subMatrix(t_matrix matrix, t_partition part, int compo_index) {
    if (compo_index < 0 || compo_index >= part.nb_classes) {
        fprintf(stderr, "Error: invalid component index\n");
        exit(EXIT_FAILURE);
    }

    t_class *classe = &part.classes[compo_index];
    int n = classe->nb_vertices;
    t_matrix sub = createEmptyMatrix(n);

    // Extraire les lignes et colonnes correspondant aux sommets de la classe
    for (int i = 0; i < n; i++) {
        int vertex_i = classe->vertices[i] - 1;  // Conversion à 0-indexé
        for (int j = 0; j < n; j++) {
            int vertex_j = classe->vertices[j] - 1;
            sub.data[i][j] = matrix.data[vertex_i][vertex_j];
        }
    }

    return sub;
}

// ============ Calcul de distribution stationnaire ============

void computeStationaryDistribution(t_adjacency_list adj_list, t_partition partition,
                                  float epsilon) {
    printf("\n=== Calcul des distributions stationnaires ===\n\n");

    // Créer la matrice de transition
    t_matrix M = adjacencyListToMatrix(adj_list);

    printf("Matrice de transition M:\n");
    displayMatrix(M);

    // Déterminer quelles classes sont persistantes
    int *vertex_to_class = createVertexToClassMap(partition, adj_list.nb_vertices);

    // Pour chaque classe persistante
    for (int c = 0; c < partition.nb_classes; c++) {
        // Vérifier si la classe est persistante (pas de sommet qui sort)
        int is_persistent = 1;
        for (int v = 0; v < partition.classes[c].nb_vertices; v++) {
            int vertex = partition.classes[c].vertices[v] - 1;

            // Vérifier les successeurs de ce sommet
            t_cell *current = adj_list.lists[vertex].head;
            while (current != NULL) {
                int succ_class = vertex_to_class[current->destination - 1];
                if (succ_class != c) {
                    is_persistent = 0;
                    break;
                }
                current = current->next;
            }
            if (!is_persistent) break;
        }

        if (!is_persistent) {
            printf("Classe C%d est transitoire - distribution limite nulle\n\n", c + 1);
            continue;
        }

        printf("Classe C%d est persistante - calcul de la distribution stationnaire...\n", c + 1);

        // Extraire la sous-matrice pour cette classe
        t_matrix sub = subMatrix(M, partition, c);

        // Calculer les puissances successives jusqu'à convergence
        t_matrix prev = createEmptyMatrix(sub.rows);
        copyMatrix(prev, sub);

        int power = 1;
        float diff;

        do {
            power++;
            t_matrix next = matrixPower(sub, power);
            diff = matrixDifference(next, prev);

            copyMatrix(prev, next);
            freeMatrix(&next);

            if (power > 1000) {
                printf("Attention: pas de convergence après 1000 itérations\n");
                break;
            }
        } while (diff > epsilon);

        printf("Convergence atteinte après %d itérations (différence = %.6f)\n", power, diff);
        printf("Distribution stationnaire (première ligne de M^%d):\n", power);

        printf("  Pi* = (");
        for (int j = 0; j < prev.rows; j++) {
            printf("%.4f", prev.data[0][j]);
            if (j < prev.rows - 1) printf(", ");
        }
        printf(")\n\n");

        freeMatrix(&prev);
        freeMatrix(&sub);
    }

    free(vertex_to_class);
    freeMatrix(&M);

    printf("==============================================\n\n");
}

// ============ Calcul de période (BONUS) ============

int gcd(int *vals, int nbvals) {
    if (nbvals == 0) return 0;
    int result = vals[0];
    for (int i = 1; i < nbvals; i++) {
        int a = result;
        int b = vals[i];
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        result = a;
    }
    return result;
}

int getPeriod(t_matrix sub_matrix) {
    int n = sub_matrix.rows;
    int *periods = (int *)malloc(n * sizeof(int));
    int period_count = 0;

    t_matrix power_matrix = createEmptyMatrix(n);
    t_matrix result_matrix = createEmptyMatrix(n);
    copyMatrix(power_matrix, sub_matrix);

    for (int cpt = 1; cpt <= n; cpt++) {
        int diag_nonzero = 0;
        for (int i = 0; i < n; i++) {
            if (power_matrix.data[i][i] > 0.0f) {
                diag_nonzero = 1;
            }
        }

        if (diag_nonzero) {
            periods[period_count] = cpt;
            period_count++;
        }

        multiplyMatrices(power_matrix, sub_matrix, result_matrix);
        copyMatrix(power_matrix, result_matrix);
    }

    int period = gcd(periods, period_count);

    free(periods);
    freeMatrix(&power_matrix);
    freeMatrix(&result_matrix);

    return period;
}
