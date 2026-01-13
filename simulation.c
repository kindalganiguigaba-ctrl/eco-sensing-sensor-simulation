/* fichier conserve (non utilise en mode buffer-only) mais garde la fonction
   de cout de transmission au cas ou tu souhaites la reutiliser plus tard. */
#include <math.h>
#include "capteur.h"

#define E_ELEC 0.05f
#define E_AMP  0.01f

float cout_transmission(Capteur *c) {
    if (!c) return 0.0f;
    double dx = (double)c->x;
    double dy = (double)c->y;
    double d2 = dx*dx + dy*dy;
    double energie = (double)E_ELEC + (double)E_AMP * d2;
    return (float)energie;
}

/* Placeholder pour transmissions si besoin futur */
int transmettre_paquet(Capteur *c) {
    (void)c;
    return 0;
}

void afficher_etat(Capteur *c, long temps) {
    if (!c) return;
    printf("Temps: %lds | Batterie: %.4fJ | Paquets en attente: %d | Transmis: %d\n",
           temps, c->batterie, c->buffer_usage, c->paquets_transmis);
}
