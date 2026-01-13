#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "capteur.h"

/* Generation de valeur aleatoire entre 0.0 et 100.0 */
static float gen_valeur(void) {
    return ((float)rand() / (float)RAND_MAX) * 100.0f;
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

/* Supprime la tete (plus ancien) et free() : affiche l'alerte + log file
   IMPORTANT : suppression faite AVANT l'ajout du nouveau paquet, donc
   l'alerte apparait immediatement au moment de la tentative d'ajout. */
static void supprimer_tete(Capteur *c) {
    if (!c || !c->buffer_tete) return;
    Paquet *old = c->buffer_tete;
    c->buffer_tete = old->suivant;
    if (c->buffer_tete == NULL) c->buffer_queue = NULL;

    /* Message console (ASCII only) */
    printf("ALERTE : Memoire saturee. Suppression du paquet ID [%d] pour liberer de l'espace.\n", old->id);

    /* Egalement loguer dans log.txt si ouvert, avec une mise en forme lisible */
    if (c->fichier_log) {
        log_append(c->fichier_log, "ALERTE : Memoire saturee. Suppression du paquet ID [%d] pour liberer de l'espace.", old->id);
    }

    free(old);
    if (c->buffer_usage > 0) c->buffer_usage--;
}

/* Ajoute a la queue en respectant BUFFER_MAX */
static void ajouter_queue(Capteur *c, Paquet *p) {
    if (!c || !p) return;

    /* Si plein : supprimer la tete AVANT d'insere le nouveau paquet.
       Cela garantit que l'alerte est immediatement visible au moment de l'ajout. */
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

    /* Log de production (lisible) */
    if (c->fichier_log) {
        log_append(c->fichier_log, "PROD : paquet ID %d ajoute. Buffer: %d/%d", p->id, c->buffer_usage, BUFFER_MAX);
    }
}

/* Affiche le contenu du buffer (pour tracabilite) */
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

/* Libere toute la liste chainee (appele a la fin ou avant rechargement) */
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
