#include "tarjan.h"
#include "utils.h"
#include <string.h>

// ============ Fonctions pour la pile ============

t_stack *createStack() {
    t_stack *stack = (t_stack *)malloc(sizeof(t_stack));
    if (stack == NULL) {
        perror("Failed to allocate memory for stack");
        exit(EXIT_FAILURE);
    }
    stack->top = NULL;
    return stack;
}

void push(t_stack *stack, int vertex_id) {
    t_stack_node *node = (t_stack_node *)malloc(sizeof(t_stack_node));
    if (node == NULL) {
        perror("Failed to allocate memory for stack node");
        exit(EXIT_FAILURE);
    }
    node->vertex_id = vertex_id;
    node->next = stack->top;
    stack->top = node;
}

int pop(t_stack *stack) {
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: trying to pop from empty stack\n");
        exit(EXIT_FAILURE);
    }
    t_stack_node *node = stack->top;
    int vertex_id = node->vertex_id;
    stack->top = node->next;
    free(node);
    return vertex_id;
}

int isEmpty(t_stack *stack) {
    return stack->top == NULL;
}

void freeStack(t_stack *stack) {
    while (!isEmpty(stack)) {
        pop(stack);
    }
    free(stack);
}

// ============ Fonctions pour les classes ============

t_class createClass(const char *name) {
    t_class classe;
    strncpy(classe.name, name, 9);
    classe.name[9] = '\0';
    classe.capacity = 10;
    classe.nb_vertices = 0;
    classe.vertices = (int *)malloc(classe.capacity * sizeof(int));
    if (classe.vertices == NULL) {
        perror("Failed to allocate memory for class vertices");
        exit(EXIT_FAILURE);
    }
    return classe;
}

void addVertexToClass(t_class *classe, int vertex_id) {
    if (classe->nb_vertices >= classe->capacity) {
        classe->capacity *= 2;
        classe->vertices = (int *)realloc(classe->vertices, classe->capacity * sizeof(int));
        if (classe->vertices == NULL) {
            perror("Failed to reallocate memory for class vertices");
            exit(EXIT_FAILURE);
        }
    }
    classe->vertices[classe->nb_vertices++] = vertex_id;
}

void displayClass(t_class classe) {
    printf("Composante %s: {", classe.name);
    for (int i = 0; i < classe.nb_vertices; i++) {
        printf("%d", classe.vertices[i]);
        if (i < classe.nb_vertices - 1) {
            printf(",");
        }
    }
    printf("}\n");
}

void freeClass(t_class *classe) {
    if (classe->vertices != NULL) {
        free(classe->vertices);
        classe->vertices = NULL;
    }
}

// ============ Fonctions pour la partition ============

t_partition createPartition() {
    t_partition partition;
    partition.capacity = 10;
    partition.nb_classes = 0;
    partition.classes = (t_class *)malloc(partition.capacity * sizeof(t_class));
    if (partition.classes == NULL) {
        perror("Failed to allocate memory for partition");
        exit(EXIT_FAILURE);
    }
    return partition;
}

void addClassToPartition(t_partition *partition, t_class classe) {
    if (partition->nb_classes >= partition->capacity) {
        partition->capacity *= 2;
        partition->classes = (t_class *)realloc(partition->classes,
                                                partition->capacity * sizeof(t_class));
        if (partition->classes == NULL) {
            perror("Failed to reallocate memory for partition");
            exit(EXIT_FAILURE);
        }
    }
    partition->classes[partition->nb_classes++] = classe;
}

void displayPartition(t_partition partition) {
    printf("\n=== Partition du graphe (Composantes fortement connexes) ===\n");
    for (int i = 0; i < partition.nb_classes; i++) {
        displayClass(partition.classes[i]);
    }
    printf("=============================================================\n\n");
}

void freePartition(t_partition *partition) {
    for (int i = 0; i < partition->nb_classes; i++) {
        freeClass(&partition->classes[i]);
    }
    free(partition->classes);
}

// ============ Algorithme de Tarjan ============

t_tarjan_vertex *initTarjanVertices(t_adjacency_list adj_list) {
    t_tarjan_vertex *vertices = (t_tarjan_vertex *)malloc(
        adj_list.nb_vertices * sizeof(t_tarjan_vertex));

    if (vertices == NULL) {
        perror("Failed to allocate memory for Tarjan vertices");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < adj_list.nb_vertices; i++) {
        vertices[i].id = i + 1;
        vertices[i].num = -1;
        vertices[i].accessible = -1;
        vertices[i].in_stack = 0;
    }

    return vertices;
}

void parcours(int vertex_id, t_adjacency_list adj_list, t_tarjan_vertex *vertices,
              t_stack *stack, int *num_counter, t_partition *partition) {
    int index = vertex_id - 1;  // Conversion de 1-indexé à 0-indexé

    vertices[index].num = *num_counter;
    vertices[index].accessible = *num_counter;
    (*num_counter)++;

    push(stack, vertex_id);
    vertices[index].in_stack = 1;

    // Parcourir les successeurs
    t_cell *current = adj_list.lists[index].head;
    while (current != NULL) {
        int successor = current->destination;
        int succ_index = successor - 1;

        if (vertices[succ_index].num == -1) {
            // Successeur pas encore visité
            parcours(successor, adj_list, vertices, stack, num_counter, partition);
            vertices[index].accessible = min(vertices[index].accessible,
                                            vertices[succ_index].accessible);
        } else if (vertices[succ_index].in_stack) {
            // Successeur dans la pile
            vertices[index].accessible = min(vertices[index].accessible,
                                            vertices[succ_index].num);
        }

        current = current->next;
    }

    // Si c'est une racine de composante fortement connexe
    if (vertices[index].accessible == vertices[index].num) {
        char class_name[10];
        sprintf(class_name, "C%d", partition->nb_classes + 1);
        t_class new_class = createClass(class_name);

        int popped;
        do {
            popped = pop(stack);
            vertices[popped - 1].in_stack = 0;
            addVertexToClass(&new_class, popped);
        } while (popped != vertex_id);

        addClassToPartition(partition, new_class);
    }
}

t_partition tarjan(t_adjacency_list adj_list) {
    t_partition partition = createPartition();
    t_tarjan_vertex *vertices = initTarjanVertices(adj_list);
    t_stack *stack = createStack();
    int num_counter = 0;

    // Parcourir tous les sommets
    for (int i = 0; i < adj_list.nb_vertices; i++) {
        if (vertices[i].num == -1) {
            parcours(i + 1, adj_list, vertices, stack, &num_counter, &partition);
        }
    }

    freeStack(stack);
    free(vertices);

    return partition;
}

// Créer un tableau qui associe chaque sommet à sa classe
int *createVertexToClassMap(t_partition partition, int nb_vertices) {
    int *map = (int *)malloc(nb_vertices * sizeof(int));
    if (map == NULL) {
        perror("Failed to allocate memory for vertex to class map");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < partition.nb_classes; i++) {
        for (int j = 0; j < partition.classes[i].nb_vertices; j++) {
            int vertex = partition.classes[i].vertices[j];
            map[vertex - 1] = i;  // Classe i pour le sommet vertex
        }
    }

    return map;
}