#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== Structures ===== */
typedef struct Cellule {
    int dest;
    float proba;
    struct Cellule *suiv;
} Cellule;

typedef struct {
    Cellule *tete;
} Liste;

typedef struct {
    int n;
    Liste *tab;
} GrapheLA;

/* ===== Fonctions listes ===== */
static Cellule* creer_cellule(int dest, float proba) {
    Cellule *c = (Cellule*)malloc(sizeof(Cellule));
    if (c == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
    c->dest = dest;
    c->proba = proba;
    c->suiv = NULL;
    return c;
}

static void initialiser_liste(Liste *l) { l->tete = NULL; }

static void ajouter_fin(Liste *l, int dest, float proba) {
    Cellule *c = creer_cellule(dest, proba);
    if (l->tete == NULL) {
        l->tete = c;
    } else {
        Cellule *p = l->tete;
        while (p->suiv != NULL) p = p->suiv;
        p->suiv = c;
    }
}

static void afficher_liste(const Liste *l) {
    const Cellule *p = l->tete;
    printf("head -> ");
    while (p != NULL) {
        printf("(%d, %.2f) -> ", p->dest, p->proba);
        p = p->suiv;
    }
    printf("NULL\n");
}

/* ===== Fonctions graphe (liste d'adjacence) ===== */
static GrapheLA* creer_graphe(int n) {
    GrapheLA *g = (GrapheLA*)malloc(sizeof(GrapheLA));
    if (g == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
    g->n = n;
    g->tab = (Liste*)malloc(n * sizeof(Liste));
    if (g->tab == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < n; ++i) initialiser_liste(&g->tab[i]);
    return g;
}

static void ajouter_arete(GrapheLA *g, int depart, int arrivee, float proba) {
    if (depart < 1 || depart > g->n || arrivee < 1 || arrivee > g->n) {
        fprintf(stderr, "Arete hors bornes: %d -> %d\n", depart, arrivee);
        return;
    }
    ajouter_fin(&g->tab[depart - 1], arrivee, proba);
}

static void afficher_graphe(const GrapheLA *g) {
    for (int i = 0; i < g->n; ++i) {
        printf("Liste du sommet %d: ", i + 1);
        afficher_liste(&g->tab[i]);
    }
}

static void liberer_graphe(GrapheLA *g) {
    if (g == NULL) return;
    for (int i = 0; i < g->n; ++i) {
        Cellule *p = g->tab[i].tete;
        while (p != NULL) {
            Cellule *tmp = p->suiv;
            free(p);
            p = tmp;
        }
    }
    free(g->tab);
    free(g);
}

/* ===== Etape 2 : Vérifier le graphe de Markov ===== */
static float somme_sortante(const GrapheLA *g, int v) {
    float s = 0.0f;
    const Cellule *p = g->tab[v - 1].tete;
    while (p != NULL) { s += p->proba; p = p->suiv; }
    return s;
}

static int verifier_graphe_markov(const GrapheLA *g, float bas, float haut) {
    int ok = 1;
    for (int v = 1; v <= g->n; ++v) {
        float s = somme_sortante(g, v);
        if (s < bas || s > haut) {
            if (ok) printf("Le graphe n’est pas un graphe de Markov\n");
            printf("la somme des probabilites du sommet %d est %.2f\n", v, s);
            ok = 0;
        }
    }
    if (ok) printf("Le graphe est un graphe de Markov\n");
    return ok;
}

/* ===== Etape 3 : Générer le fichier Mermaid =====
   getId(num) : 1->A, 2->B, ..., 26->Z, 27->AA, etc.
*/
static void toId(int num, char *out) {
    char tmp[32];
    int k = 0;
    while (num > 0) {
        num--;
        tmp[k++] = (char)('A' + (num % 26));
        num /= 26;
    }
    for (int i = 0; i < k; ++i) out[i] = tmp[k - 1 - i];
    out[k] = '\0';
}

static char* getId(int num) {
    static char buf[32];
    toId(num, buf);
    return buf;
}

static int ecrire_mermaid(const GrapheLA *g, const char *chemin) {
    FILE *f = fopen(chemin, "wt");
    if (f == NULL) { perror("open output"); return 0; }

    fprintf(f, "---\n");
    fprintf(f, "config:\n");
    fprintf(f, "  layout: elk\n");
    fprintf(f, "  theme: neo\n");
    fprintf(f, "  look: neo\n");
    fprintf(f, "---\n\n");
    fprintf(f, "flowchart LR\n\n");

    for (int i = 1; i <= g->n; ++i) {
        char id[32];
        toId(i, id);
        fprintf(f, "%s((%d))\n", id, i);
    }
    fprintf(f, "\n");

    for (int i = 1; i <= g->n; ++i) {
        char from[32];
        toId(i, from);
        const Cellule *p = g->tab[i - 1].tete;
        while (p != NULL) {
            char to[32];
            toId(p->dest, to);
            fprintf(f, "%s -->|%.2f| %s\n", from, p->proba, to);
            p = p->suiv;
        }
    }

    fclose(f);
    return 1;
}

/* ===== Lecture du fichier ===== */
static GrapheLA* lire_graphe(const char *chemin) {
    FILE *f = fopen(chemin, "rt");
    if (f == NULL) { perror("open file"); exit(EXIT_FAILURE); }

    int n;
    if (fscanf(f, "%d", &n) != 1) {
        fprintf(stderr, "Impossible de lire n\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    GrapheLA *g = creer_graphe(n);

    int d, a;
    float p;
    while (fscanf(f, "%d %d %f", &d, &a, &p) == 3) {
        ajouter_arete(g, d, a, p);
    }

    fclose(f);
    return g;
}

/* ===== Programme principal ===== */
int main(int argc, char *argv[]) {
    const char *fichier_in = (argc >= 2) ? argv[1] : "exemple1.txt";
    GrapheLA *g = lire_graphe(fichier_in);

    printf("Graphe charge (n = %d)\n", g->n);
    afficher_graphe(g);

    verifier_graphe_markov(g, 0.99f, 1.00f);

    const char *fichier_out = "mermaid_output.txt";
    if (ecrire_mermaid(g, fichier_out)) {
        printf("Fichier Mermaid genere: %s\n", fichier_out);
        printf("Copiez/collez son contenu dans l'editeur en ligne Mermaid.\n");
    }

    liberer_graphe(g);
    return 0;
}
