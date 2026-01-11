#include "capteur.h"
#include <stdio.h>
#include <string.h>

/* Sauvegarde binaire de l'état : capteur (primitives) puis paquets en ordre */
void sauvegarder_etat(const Capteur *c) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) {
        perror("Erreur ouverture fichier pour sauvegarde");
        return;
    }

    /* Écrire champs simples */
    if (fwrite(&c->batterie, sizeof(c->batterie), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
    if (fwrite(&c->x, sizeof(c->x), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
    if (fwrite(&c->y, sizeof(c->y), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
    if (fwrite(&c->buffer_usage, sizeof(c->buffer_usage), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
    if (fwrite(&c->next_id, sizeof(c->next_id), 1, f) != 1) { perror("fwrite"); fclose(f); return; }

    /* Écrire paquets successifs (sans pointeurs) */
    Paquet *cur = c->buffer_tete;
    while (cur) {
        if (fwrite(&cur->id, sizeof(cur->id), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
        if (fwrite(&cur->valeur, sizeof(cur->valeur), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
        if (fwrite(&cur->timestamp, sizeof(cur->timestamp), 1, f) != 1) { perror("fwrite"); fclose(f); return; }
        cur = cur->suivant;
    }

    fclose(f);
    printf("Etat sauvegarde dans '%s' (battery=%.4f, buffer_usage=%d)\n", SAVE_FILE, c->batterie, c->buffer_usage);
}

/* Charge l'état depuis save.bin ; retourne 1 = OK, 0 = pas de fichier ou erreur */
int charger_etat(Capteur *c) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) {
        return 0; /* pas de fichier */
    }

    int saved_buffer_usage = 0;

    /* Lire champs simples */
    if (fread(&c->batterie, sizeof(c->batterie), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&c->x, sizeof(c->x), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&c->y, sizeof(c->y), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&saved_buffer_usage, sizeof(saved_buffer_usage), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&c->next_id, sizeof(c->next_id), 1, f) != 1) { fclose(f); return 0; }

    /* Libérer tout buffer existant avant reconstruction */
    liberer_buffer(c);
    c->buffer_tete = NULL;
    c->buffer_usage = 0;

    /* Lire paquets et reconstruire la liste dans le même ordre */
    for (int i = 0; i < saved_buffer_usage; ++i) {
        int id;
        float valeur;
        long timestamp;
        if (fread(&id, sizeof(id), 1, f) != 1) { fclose(f); liberer_buffer(c); return 0; }
        if (fread(&valeur, sizeof(valeur), 1, f) != 1) { fclose(f); liberer_buffer(c); return 0; }
        if (fread(&timestamp, sizeof(timestamp), 1, f) != 1) { fclose(f); liberer_buffer(c); return 0; }

        Paquet *p = creer_paquet(id, valeur, timestamp);
        if (!p) { fclose(f); liberer_buffer(c); return 0; }
        ajouter_paquet_fin(c, p);
    }

    fclose(f);
    printf("Etat charge depuis '%s' (battery=%.4f, buffer_usage=%d)\n", SAVE_FILE, c->batterie, c->buffer_usage);
    return 1;
}
