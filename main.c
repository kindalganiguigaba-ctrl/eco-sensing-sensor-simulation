#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "capteur.h"

#define SAVE_FILENAME "save.bin"
#define LOG_FILENAME  "log.txt"

int main(void) {
    srand((unsigned int)time(NULL));
    Capteur c;
    memset(&c, 0, sizeof(Capteur));

    /* 1. Chargement ou Initialisation */
    if (charger_etat(&c, SAVE_FILENAME) != 0) {
        c.batterie = 50.0f; // 50 Joules pour durer longtemps
        c.x = 10.0f;
        c.y = 10.0f;
        c.next_id = 1;
        c.buffer_usage = 0;
        c.paquets_transmis = 0;
        printf("Nouvelle simulation : Batterie = %.2f J\n", c.batterie);
    }

    /* 2. Ouverture du log */
    c.fichier_log = fopen(LOG_FILENAME, "a");
    if (c.fichier_log) {
        fprintf(c.fichier_log, "--- Nouvelle Session ---\n");
    }

    long temps = 0;
    /* 3. Boucle de simulation */
    while (c.batterie > 0.0f) {
        temps++;
        printf("\n--- CYCLE %ld | Buffer: %d/%d ---\n", temps, c.buffer_usage, BUFFER_MAX);

        /* Production (ajoute 1 paquet, gère la saturation de 5) */
        produire_paquet(&c);

        /* Affichage pour ta vidéo/debug */
        afficher_buffer(&c);

        /* Transmission (envoie 1 seul paquet max grâce au 'if' dans simulation.c) */
        transmettre_paquet(&c);

        /* Journalisation (Crash Test) */
        if (c.fichier_log) {
            fprintf(c.fichier_log, "Temps: %lds | Batterie: %.4fJ | Paquets en attente: %d\n",
                    temps, c.batterie, c.buffer_usage);
            fflush(c.fichier_log);
        }

        /* Sauvegarde binaire */
        sauvegarder_etat(&c, SAVE_FILENAME);
    }

    printf("\n--- CAPTEUR MORT A %lds ---\n", temps);
    printf("Total transmis : %d paquets.\n", c.paquets_transmis);

    if (c.fichier_log) fclose(c.fichier_log);
    liberer_liste(&c);
    return 0;
}
