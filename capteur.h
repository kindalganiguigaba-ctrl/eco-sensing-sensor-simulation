#ifndef CAPTEUR_H
#define CAPTEUR_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_MAX 5

typedef struct Paquet {
    int id;
    float valeur;
    long timestamp;
    struct Paquet *suivant;
} Paquet;

typedef struct Capteur {
    float batterie;        /* Niveau d'energie (Joules) */
    float x, y;            /* Position */
    Paquet *buffer_tete;   /* Tete (plus ancien) */
    Paquet *buffer_queue;  /* Queue (plus recent) pour ajout rapide */
    int buffer_usage;      /* Nombre de paquets en memoire */
    int next_id;           /* Prochain id de paquet */
    int paquets_transmis;  /* Compteur de paquets transmis */
    FILE *fichier_log;     /* Pointeur vers log.txt */
} Capteur;

/* memoire.c */
void produire_paquet(Capteur *c);
void liberer_liste(Capteur *c);
void afficher_buffer(Capteur *c);

/* simulation.c */
float cout_transmission(Capteur *c);
int transmettre_paquet(Capteur *c);
void afficher_etat(Capteur *c, long temps);

/* persistence.c */
int sauvegarder_etat(Capteur *c, const char *filename);
int charger_etat(Capteur *c, const char *filename);

/* logger.c */
void log_append(FILE *f, const char *fmt, ...);

#endif /* CAPTEUR_H */
