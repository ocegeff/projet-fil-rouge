#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 1000
#define MAXFIELDS 15

typedef char t_field[MAXLEN];

typedef struct {
    t_field key;
    t_field value[MAXFIELDS];
} t_tuple;

typedef struct node {
    t_tuple data;
    struct node *pNext;
} t_node;

typedef t_node* t_list;

typedef struct {
    char *hashfunction;
    int nbSlots;
    t_list *slots;
} t_hashtable;

unsigned int hash_function(const char *key, int nbSlots) {
    unsigned int hash = 0; //initialisation du hash
    while (*key) {
        hash += (unsigned char)(*key); //additionne la valeur du caractère
        key++;
    }
    return hash % nbSlots; //retourne l'indice dans le tableau
}

t_hashtable* create_hashtable(int nbSlots) {
    t_hashtable *table = malloc(sizeof(t_hashtable)); //alloue la mémoire pour le tableau
    table->nbSlots = nbSlots; //définit le nb d'alvéoles
    table->slots = calloc(nbSlots, sizeof(t_list)); 
    return table; //retourne le tableau initialisé
}

void insert_into_hashtable(t_hashtable *table, const char *key, t_field *values, int nbFields) {
    unsigned int index = hash_function(key, table->nbSlots); //calcule l'indice de l'alvéole 
    t_node *new_node = malloc(sizeof(t_node)); //alloue la mémoire pour un nouveau noeud
    strcpy(new_node->data.key, key); //copie la clé dans le noeud
    for (int i = 0; i < nbFields; i++) { //copie les valeurs associées 
        strcpy(new_node->data.value[i], values[i]);
    }
    new_node->pNext = table->slots[index]; //insère le noeud en tête de liste chainée 
    table->slots[index] = new_node;
}

void save_hashtable(t_hashtable *table, const char *filename) {
    FILE *file = fopen(filename, "wb"); //ouvre le fichier en écriture binaire
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    fwrite(&table->nbSlots, sizeof(int), 1, file);
    for (int i = 0; i < table->nbSlots; i++) { //parcourt chaque alvéole et sauvegarde les tuples 
        t_node *node = table->slots[i];
        while (node) {
            fwrite(&node->data, sizeof(t_tuple), 1, file); //écrit les tuples dans le fichier
            node = node->pNext;
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 5) { //vérification de la validité des arguments
        fprintf(stderr, "Usage: %s -i<input_file> -o<output_file> -s<size> [-h<hash_function>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_file = NULL, *output_file = NULL, *hash_function = "default"; 
    int size = 0; 

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-i", 2) == 0) { //fichier d'entrée
            input_file = argv[i] + 2;
        } else if (strncmp(argv[i], "-o", 2) == 0) { //fichier de sortie
            output_file = argv[i] + 2;
        } else if (strncmp(argv[i], "-s", 2) == 0) { //taille de la table
            size = atoi(argv[i] + 2);
        } else if (strncmp(argv[i], "-h", 2) == 0) { //fonction de hachage
            hash_function = argv[i] + 2;
        }
    }

    if (!input_file || !output_file || size <= 0) {
        fprintf(stderr, "Arguments invalides.\n");
        return EXIT_FAILURE;
    }

    t_hashtable *table = create_hashtable(size); //création du tableau de hachage 

    FILE *input = fopen(input_file, "r");
    if (!input) {
        perror("Erreur d'ouverture du fichier d'entrée");
        return EXIT_FAILURE;
    }

    char line[MAXLEN];
    while (fgets(line, sizeof(line), input)) {
        if (line[0] == '#' || strlen(line) < 2) continue; // Ignorer commentaires ou lignes vides
        char *key = strtok(line, "#"); //relève la clé
        t_field values[MAXFIELDS];//tableau pour les valeurs 
        int field_count = 0;
        char *token = strtok(NULL, "#");
        while (token) { //découpe les valeurs
            strncpy(values[field_count++], token, MAXLEN);
            token = strtok(NULL, "#");
        }
        insert_into_hashtable(table, key, values, field_count); //insère le tuple dans le tableau de hachage
    
    }
    fclose(input);

    save_hashtable(table, output_file);

    printf("Table de hachage enregistrée dans %s.\n", output_file); //affichage de message de confirmation
    return EXIT_SUCCESS;
}
