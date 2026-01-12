#include <stdio.h>
#include <math.h>
#include "capteur.h"

/* Paramètres énergétiques demandés */
#define E_ELEC 0.05f   /* Joules */
#define E_AMP  0.01f   /* Joules / m^2 */
/* Petite consommation de base par cycle (simule tâches non-radio) */
#define CONSO_BASE 0.001f

/* Calcule le coût d'une transmission (distance 2D vers 0,0) */
float cout_transmission(Capteur *c) {
    if (!c) return 0.0f;
    double dx = (double)c->x;
    double dy = (double)c->y;
    double d2 = dx*dx + dy*dy;
    double energie = (double)E_ELEC + (double)E_AMP * d2;
    return (float)energie;
}

/* Tente d'envoyer les paquets en file d'attente.
   Envoie autant de paquets que la batterie le permet (FIFO).
   Retourne le nombre de paquets transmis lors de cet appel.
*/

/* Tente d'envoyer UN SEUL paquet par cycle pour permettre au buffer de se remplir */
int transmettre_paquet(Capteur *c) {
    if (!c || !c->buffer_tete) return 0;
    int transmis = 0;

    float cout = cout_transmission(c);

    /* On utilise 'if' au lieu de 'while' pour limiter à 1 envoi par cycle */
    if (c->batterie >= cout) {
        /* Consommer l'énergie */
        c->batterie -= cout;

        /* Détacher le paquet de la tête (le plus ancien) */
        Paquet *p = c->buffer_tete;
        c->buffer_tete = p->suivant;

        /* Si la liste devient vide, on réinitialise la queue */
        if (c->buffer_tete == NULL) {
            c->buffer_queue = NULL;
        }

        c->buffer_usage--;
        c->paquets_transmis++;

        printf("[TX] Envoi paquet ID %d | cout=%.4f J | batterie restante=%.4f J\n",
               p->id, cout, c->batterie);

        free(p); // Libération impérative
        transmis = 1;
    } else {
        printf("[TX] Batterie insuffisante (%.4f J) pour envoyer (coût=%.4f J)\n",
               c->batterie, cout);
    }

    /* Consommation de base du circuit à chaque cycle */
    c->batterie -= CONSO_BASE;
    if (c->batterie < 0.0f) c->batterie = 0.0f;

    return transmis;
}

/* Affiche l'état succinct du capteur */
void afficher_etat(Capteur *c, long temps) {
    if (!c) return;
    printf("Temps: %lds | Batterie: %.4fJ | Paquets en attente: %d | Transmis: %d\n",
           temps, c->batterie, c->buffer_usage, c->paquets_transmis);
}
