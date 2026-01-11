#ifndef CAPTEUR_H
#define CAPTEUR_H

#include <stdio.h>
#include <stdlib.h>

/* Config matérielle / simulation */
#define MAX_BUFFER 5
#define SAVE_FILE "save.bin"
#define E_ELEC 0.05f   /* Joules */
#define E_AMP  0.01f   /* Joules / m^2 */

/* Structures */
typedef struct Paquet {
    int id;
    float valeur;
    long timestamp;
    struct Paquet *suivant;
} Paquet;

typedef struct {
    float batterie;       /* en Joules */
    float x, y;           /* position du capteur */
    Paquet *buffer_tete;  /* tête de la file FIFO (plus ancien) */
    int buffer_usage;     /* nombre actuel de paquets */
    int next_id;          /* identifiant à donner au prochain paquet */
} Capteur;

/* --- Prototypes --- */

/* capteur / init */
void init_capteur(Capteur *c, float batterie_init, float x, float y);

/* memoire.c */
Paquet* creer_paquet(int id, float valeur, long timestamp);
void ajouter_paquet_fin(Capteur *c, Paquet *p);
int supprimer_tete_alerte(Capteur *c); /* supprime la tête, affiche ALERTE et retourne id supprimé ou -1 */
void liberer_buffer(Capteur *c);

/* simulation.c */
void produire_paquet(Capteur *c);
void afficher_buffer(const Capteur *c);
float cout_envoi(const Capteur *c);
int envoyer_paquets(Capteur *c);
void print_separator(void);

/* fichier.c */
void sauvegarder_etat(const Capteur *c);
int charger_etat(Capteur *c);

#endif /* CAPTEUR_H */