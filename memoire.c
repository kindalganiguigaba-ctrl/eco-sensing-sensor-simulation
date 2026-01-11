#include "capteur.h"
#include <time.h>

/* Crée un paquet alloué dynamiquement */
Paquet* creer_paquet(int id, float valeur, long timestamp) {
    Paquet *p = (Paquet*) malloc(sizeof(Paquet));
    if (!p) {
        fprintf(stderr, "Erreur malloc : impossible d'allouer un paquet\n");
        return NULL;
    }
    p->id = id;
    p->valeur = valeur;
    p->timestamp = timestamp;
    p->suivant = NULL;
    return p;
}

/* Ajoute un paquet en fin de liste et met à jour buffer_usage */
void ajouter_paquet_fin(Capteur *c, Paquet *p) {
    if (!p) return;
    p->suivant = NULL;
    if (c->buffer_tete == NULL) {
        c->buffer_tete = p;
    } else {
        Paquet *cur = c->buffer_tete;
        while (cur->suivant) cur = cur->suivant;
        cur->suivant = p;
    }
    c->buffer_usage++;
}

/* Supprime la tête (paquet le plus ancien). Affiche l'alerte demandée et free.
 * Retourne l'ID supprimé ou -1 si vide.
 */
int supprimer_tete_alerte(Capteur *c) {
    if (!c->buffer_tete) return -1;
    Paquet *ancien = c->buffer_tete;
    int id = ancien->id;
    printf("ALERTE : Memoire saturee. Suppression du paquet ID [%d] pour liberer de l'espace.\n", id);
    c->buffer_tete = ancien->suivant;
    free(ancien);
    c->buffer_usage--;
    if (c->buffer_usage < 0) c->buffer_usage = 0;
    return id;
}

/* Libère tous les paquets du buffer */
void liberer_buffer(Capteur *c) {
    Paquet *cur = c->buffer_tete;
    while (cur) {
        Paquet *tmp = cur->suivant;
        free(cur);
        cur = tmp;
    }
    c->buffer_tete = NULL;
    c->buffer_usage = 0;
}
