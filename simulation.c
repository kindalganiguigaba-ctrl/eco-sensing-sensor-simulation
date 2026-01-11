#include "capteur.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

/* Initialise le capteur */
void init_capteur(Capteur *c, float batterie_init, float x, float y) {
    c->batterie = batterie_init;
    c->x = x;
    c->y = y;
    c->buffer_tete = NULL;
    c->buffer_usage = 0;
    c->next_id = 1;
}

/* Produire un paquet : creation dynamique, gestion saturation (sliding window),
 * ajout en fin de liste. Utilise les fonctions de memoire.c.
 */
void produire_paquet(Capteur *c) {
    /* Génération aléatoire de la mesure (0.0 - 100.0) */
    float v = ((float)rand() / (float)RAND_MAX) * 100.0f;
    long ts = (long)time(NULL);
    int id = c->next_id++;

    Paquet *nouveau = creer_paquet(id, v, ts);
    if (!nouveau) return; /* erreur allocation */

    /* Si buffer plein : supprimer le paquet le plus ancien (tete) */
    if (c->buffer_usage >= MAX_BUFFER) {
        /* suppression et message d'alerte */
        supprimer_tete_alerte(c);
    }

    /* Ajout en fin de liste */
    ajouter_paquet_fin(c, nouveau);
    printf("Produit paquet ID=%d valeur=%.2f timestamp=%ld (buffer=%d)\n", id, v, ts, c->buffer_usage);
}

/* Affiche rapidement le contenu du buffer (IDs) */
void afficher_buffer(const Capteur *c) {
    printf("Buffer (usage=%d):", c->buffer_usage);
    Paquet *cur = c->buffer_tete;
    while (cur) {
        printf(" [%d]", cur->id);
        cur = cur->suivant;
    }
    printf("\n");
}

/* Calcule le coût d'envoi d'un paquet selon la distance */
float cout_envoi(const Capteur *c) {
    double d = sqrt(pow((double)c->x, 2.0) + pow((double)c->y, 2.0));
    double cost = (double)E_ELEC + (double)E_AMP * d * d;
    return (float)cost;
}

/* Tente d'envoyer les paquets du buffer (FIFO). Retourne le nombre de paquets transmis dans cet appel */
int envoyer_paquets(Capteur *c) {
    int transmet = 0;
    float cost = cout_envoi(c);

    /* On envoie tant qu'il y a des paquets ET assez d'énergie pour envoyer un paquet */
    while (c->buffer_tete != NULL) {
        if (c->batterie <= 0.0f) {
            c->batterie = 0.0f;
            break;
        }
        if (c->batterie >= cost) {
            /* transmission réussie */
            Paquet *env = c->buffer_tete;
            c->buffer_tete = env->suivant;
            printf("Transmis paquet ID=%d (coût=%.4f J). Batterie avant=%.4f J\n", env->id, cost, c->batterie);
            c->batterie -= cost;
            free(env);
            c->buffer_usage--;
            transmet++;
        } else {
            /* Pas assez d'énergie pour envoyer ce paquet */
            printf("Energie insuffisante pour envoyer le paquet ID=%d (besoin=%.4f J, restant=%.4f J). Capteur mort.\n",
                   c->buffer_tete->id, cost, c->batterie);
            c->batterie = 0.0f;
            break;
        }
    }
    return transmet;
}

void print_separator(void) {
    printf("------------------------------------------------------------\n");
}