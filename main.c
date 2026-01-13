#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "capteur.h"

/* Mode unique: ACCUMULER jusqu'a buffer plein (5) puis sliding-window.
   On produit 1 paquet par cycle uniquement. On ne transmet pas (mode buffer-only).
   Consommation d'energie de base par cycle pour vider la batterie progressivement. */

#define SAVE_FILENAME "save.bin"
#define LOG_FILENAME  "log.txt"

/* consommation non-radio par cycle (Joules) */
#define CONSO_BASE 0.55f

static int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int main(void) {
    srand((unsigned int)time(NULL));
    Capteur c;
    memset(&c, 0, sizeof(Capteur));

    /* Si save.bin existe, proposer reprise (ASCII only) */
    int resume = 0;
    if (file_exists(SAVE_FILENAME)) {
        char answer[8] = {0};
        printf("Fichier de sauvegarde '%s' trouve. Voulez-vous reprendre la session existante ? [O/n] ", SAVE_FILENAME);
        if (fgets(answer, sizeof(answer), stdin) != NULL) {
            if (answer[0] == 'n' || answer[0] == 'N') {
                resume = 0;
                if (remove(SAVE_FILENAME) == 0) {
                    printf("Ancienne sauvegarde supprimee, nouvelle session demarrera.\n");
                }
            } else {
                resume = 1;
            }
        }
    }

    if (resume) {
        if (charger_etat(&c, SAVE_FILENAME) != 0) {
            fprintf(stderr, "Erreur lors du chargement : demarrage d'une nouvelle session.\n");
            c.batterie = 70.0f;
            c.x = 10.0f; c.y = 10.0f;
            c.next_id = 1;
            c.buffer_usage = 0;
            c.paquets_transmis = 0;
        } else {
            printf("Etat charge depuis '%s' : Batterie = %.4f J | Paquets = %d | Next id = %d\n",
                   SAVE_FILENAME, c.batterie, c.buffer_usage, c.next_id);
        }
    } else {
        /* Nouvelle simulation par defaut */
        c.batterie = 70.0f; /* 70 Joules */
        c.x = 10.0f;
        c.y = 10.0f;
        c.next_id = 1;
        c.buffer_usage = 0;
        c.paquets_transmis = 0;
        printf("Nouvelle simulation : Batterie = %.2f J\n", c.batterie);
    }

    /* Ouverture du log */
    c.fichier_log = fopen(LOG_FILENAME, "a");
    if (c.fichier_log) {
        if (resume) log_append(c.fichier_log, "--- Reprise de session (save.bin charge) ---");
        else log_append(c.fichier_log, "--- Nouvelle Session ---");
    } else {
        perror("Impossible d'ouvrir log.txt");
    }

    long temps = 0;
    /* Boucle de simulation : mode buffer-only */
    while (c.batterie > 0.0f) {
        temps++;
        printf("\n--- CYCLE %ld | Buffer: %d/%d | Batterie: %.4f J ---\n", temps, c.buffer_usage, BUFFER_MAX, c.batterie);

        /* PAS de transmission en mode buffer-only : on n'appelle pas transmettre_paquet */

        /* Production : ajoute 1 paquet, gestion de la saturation (suppression avant ajout) */
        produire_paquet(&c);

        /* Affichage pour tracabilite/video */
        afficher_buffer(&c);

        /* Journalisation (Crash Test) : ligne lisible et espacee */
        if (c.fichier_log) {
            log_append(c.fichier_log, "Temps: %lds | Batterie: %.4fJ | Paquets en attente: %d | Transmis: %d",
                       temps, c.batterie, c.buffer_usage, c.paquets_transmis);
        }

        /* Sauvegarde binaire */
        if (sauvegarder_etat(&c, SAVE_FILENAME) != 0) {
            fprintf(stderr, "Attention : echec de la sauvegarde vers '%s'\n", SAVE_FILENAME);
        }

        /* Consommation de base par cycle */
        c.batterie -= CONSO_BASE;
        if (c.batterie < 0.0f) c.batterie = 0.0f;
    }

    printf("\n--- CAPTEUR MORT A %lds ---\n", temps);
    printf("Total transmis : %ld paquets.\n", temps);

    if (c.fichier_log) {
        log_append(c.fichier_log, "--- SESSION TERMINEE : CAPTEUR MORT A %lds | Transmis: %d ---", temps, c.paquets_transmis);
        fclose(c.fichier_log);
    }

    liberer_liste(&c);
    return 0;
}
