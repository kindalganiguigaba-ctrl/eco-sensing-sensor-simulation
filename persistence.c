#include <stdio.h>
#include <stdlib.h>
#include "capteur.h"

/* Format de sauvegarde (binaire):
   - batterie (float)
   - x (float)
   - y (float)
   - buffer_usage (int)
   - next_id (int)
   - paquets_transmis (int)
   - suivi par buffer_usage fois : (id int, valeur float, timestamp long)
   La sauvegarde n'écrit pas les pointeurs bruts.
*/

int sauvegarder_etat(Capteur *c, const char *filename) {
    if (!c || !filename) return -1;
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("sauvegarder_etat: fopen");
        return -1;
    }

    if (fwrite(&c->batterie, sizeof(float), 1, f) != 1) goto err;
    if (fwrite(&c->x, sizeof(float), 1, f) != 1) goto err;
    if (fwrite(&c->y, sizeof(float), 1, f) != 1) goto err;
    if (fwrite(&c->buffer_usage, sizeof(int), 1, f) != 1) goto err;
    if (fwrite(&c->next_id, sizeof(int), 1, f) != 1) goto err;
    if (fwrite(&c->paquets_transmis, sizeof(int), 1, f) != 1) goto err;

    Paquet *cur = c->buffer_tete;
    while (cur) {
        if (fwrite(&cur->id, sizeof(int), 1, f) != 1) goto err;
        if (fwrite(&cur->valeur, sizeof(float), 1, f) != 1) goto err;
        if (fwrite(&cur->timestamp, sizeof(long), 1, f) != 1) goto err;
        cur = cur->suivant;
    }

    fclose(f);
    return 0;
err:
    perror("sauvegarder_etat: fwrite");
    fclose(f);
    return -1;
}

int charger_etat(Capteur *c, const char *filename) {
    if (!c || !filename) return -1;
    FILE *f = fopen(filename, "rb");
    if (!f) {
        /* Pas de fichier : signaler avec -1 (caller peut démarrer une simulation neuve) */
        return -1;
    }

    /* Avant de charger, s'assurer que la liste existante est libérée */
    liberer_liste(c);

    if (fread(&c->batterie, sizeof(float), 1, f) != 1) goto err;
    if (fread(&c->x, sizeof(float), 1, f) != 1) goto err;
    if (fread(&c->y, sizeof(float), 1, f) != 1) goto err;
    if (fread(&c->buffer_usage, sizeof(int), 1, f) != 1) goto err;
    if (fread(&c->next_id, sizeof(int), 1, f) != 1) goto err;
    if (fread(&c->paquets_transmis, sizeof(int), 1, f) != 1) goto err;

    Paquet *last = NULL;
    for (int i = 0; i < c->buffer_usage; ++i) {
        Paquet *p = (Paquet*)malloc(sizeof(Paquet));
        if (!p) {
            perror("charger_etat: malloc");
            fclose(f);
            return -1;
        }
        if (fread(&p->id, sizeof(int), 1, f) != 1) { free(p); goto err; }
        if (fread(&p->valeur, sizeof(float), 1, f) != 1) { free(p); goto err; }
        if (fread(&p->timestamp, sizeof(long), 1, f) != 1) { free(p); goto err; }
        p->suivant = NULL;
        if (!last) {
            c->buffer_tete = p;
        } else {
            last->suivant = p;
        }
        last = p;
    }
    c->buffer_queue = last;
    fclose(f);
    return 0;
err:
    perror("charger_etat: fread");
    fclose(f);
    return -1;
}
