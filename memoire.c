#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "capteur.h"

static float gen_valeur() {
    return ((float)rand() / (float)RAND_MAX) * 100.0f; /* 0.0 - 100.0 */
}

static Paquet *creer_paquet_interne(int id) {
    Paquet *p = (Paquet*)malloc(sizeof(Paquet));
    if (!p) return NULL;
    p->id = id;
    p->valeur = gen_valeur();
    p->timestamp = (long)time(NULL);
    p->suivant = NULL;
    return p;
}

/* Supprime la tête (plus ancien) et free() : affiche l'alerte */
static void supprimer_tete(Capteur *c) {
    if (!c || !c->buffer_tete) return;
    Paquet *old = c->buffer_tete;
    c->buffer_tete = old->suivant;
    if (c->buffer_tete == NULL) c->buffer_queue = NULL;
    printf("ALERTE : Memoire saturee. Suppression du paquet ID [%d] pour liberer de l'espace.\n", old->id);
    free(old);
    c->buffer_usage--;
}

/* Ajoute à la queue en respectant BUFFER_MAX */
static void ajouter_queue(Capteur *c, Paquet *p) {
    if (!c || !p) return;
    if (c->buffer_usage >= BUFFER_MAX) {
        supprimer_tete(c);
    }
    if (c->buffer_tete == NULL) {
        c->buffer_tete = p;
        c->buffer_queue = p;
    } else {
        c->buffer_queue->suivant = p;
        c->buffer_queue = p;
    }
    c->buffer_usage++;
}

/* API public : produit un paquet (malloc) et l'ajoute au buffer */
void produire_paquet(Capteur *c) {
    if (!c) return;
    Paquet *p = creer_paquet_interne(c->next_id++);
    if (!p) {
        fprintf(stderr, "Erreur malloc lors de la creation du paquet\n");
        return;
    }
    ajouter_queue(c, p);
}

/* Affiche le contenu du buffer (pour traçabilité) */
void afficher_buffer(Capteur *c) {
    if (!c) return;
    printf("\n--- CONTENU DU BUFFER (%d/%d) ---\n", c->buffer_usage, BUFFER_MAX);
    Paquet *cur = c->buffer_tete;
    int idx = 1;
    while (cur) {
        printf("  %d. ID: [%3d] | Valeur: %6.2f | T: %ld\n", idx++, cur->id, cur->valeur, cur->timestamp);
        cur = cur->suivant;
    }
    if (c->buffer_usage == 0) printf("  (Buffer vide)\n");
    printf("----------------------------------\n");
}

/* Libère toute la liste chaînée (appelé à la fin ou avant rechargement) */
void liberer_liste(Capteur *c) {
    if (!c) return;
    Paquet *cur = c->buffer_tete;
    while (cur) {
        Paquet *n = cur->suivant;
        free(cur);
        cur = n;
    }
    c->buffer_tete = NULL;
    c->buffer_queue = NULL;
    c->buffer_usage = 0;
}
