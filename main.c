/*
 * main.c
 *
 * Point d'entrée et boucle principale de la simulation.
 *
 * Compilation:
 *   gcc -std=c11 -Wall -Wextra -o sensor_simulation main.c simulation.c memoire.c fichier.c -lm
 *
 * Usage:
 *   ./sensor_simulation [batterie_init] [x y]
 */
#include "capteur.h"
#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    Capteur c;
    /* Valeurs par défaut ; on peut ajuster via argv si souhaité */
    float init_battery = 1.0f; /* Joules par défaut */
    float pos_x = 10.0f;
    float pos_y = 5.0f;

    if (argc >= 2) init_battery = (float)atof(argv[1]);
    if (argc >= 4) {
        pos_x = (float)atof(argv[2]);
        pos_y = (float)atof(argv[3]);
    }

    init_capteur(&c, init_battery, pos_x, pos_y);

    /* Proposition de charger l'état si save.bin existe */
    FILE *fcheck = fopen(SAVE_FILE, "rb");
    if (fcheck) {
        fclose(fcheck);
        printf("Fichier '%s' detecte. Charger l'etat precedent ? (y/n) : ", SAVE_FILE);
        int ch = getchar();
        while (getchar() != '\n'); /* vider stdin */
        if (ch == 'y' || ch == 'Y') {
            if (!charger_etat(&c)) {
                printf("Impossible de charger l'etat. Demarrage d'une nouvelle simulation.\n");
                init_capteur(&c, init_battery, pos_x, pos_y);
            }
        } else {
            printf("Demarrage d'une nouvelle simulation.\n");
        }
    } else {
        printf("Aucun etat précédent trouvé. Demarrage d'une nouvelle simulation.\n");
    }

    print_separator();
    printf("Simulation demarree : batterie=%.4f J, position=(%.2f,%.2f)\n", c.batterie, c.x, c.y);
    print_separator();

    long total_transmis = 0;
    int cycle = 0;

    /* Boucle de simulation principale */
    while (c.batterie > 0.0f) {
        cycle++;
        printf("[Cycle %d] Debut. Batterie=%.4f J\n", cycle, c.batterie);

        /* Produire un paquet à chaque cycle */
        produire_paquet(&c);

        /* Afficher buffer avant transmission */
        afficher_buffer(&c);

        /* Tentative d'envoi */
        int envoyes = envoyer_paquets(&c);
        total_transmis += envoyes;
        if (envoyes > 0) {
            printf("%d paquet(s) transmis ce cycle. Batterie restante=%.4f J\n", envoyes, c.batterie);
        }

        /* Sauvegarder l'état à la fin de chaque cycle */
        sauvegarder_etat(&c);

        print_separator();

        /* Petite sécurité : éviter boucle infinie imprévue (optionnel) */
        if (cycle > 1000000) {
            fprintf(stderr, "Nombre de cycles trop eleve, arrêt de securite.\n");
            break;
        }
    }

    printf("Capteur mort. Nombre total de paquets transmis avant panne : %ld\n", total_transmis);

    /* Nettoyage final */
    liberer_buffer(&c);

    return 0;
}
