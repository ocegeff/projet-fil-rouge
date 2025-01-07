//Inclusion des bibliothèques nécessaires au bon fonctionnement du programme.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Définition des grandeurs globales.

#define MAXLEN 1000
#define MAXFIELDS 15

//Création des structures

typedef char t_field[MAXLEN];      // Un champ est une chaîne de caractères
typedef t_field t_key;             // Une clé est un champ
typedef t_field t_value[MAXFIELDS];

typedef struct {
    t_key key;
    t_value value;
} t_tuple;                         // Un tuple est un ensemble (clé, valeur)

typedef struct {
    char sep;                      // Séparateur
    int nbFields;                  // Nombre de champs
    t_field *fieldNames;           // Noms des champs
} t_metadata;

typedef struct {
    t_tuple *tuples;               // Tableau des tuples
    int sizeTab;                   // Taille allouée pour le tableau
    int nbTuples;                  // Nombre de tuples insérés
} t_tupletable;

// PROTOTYPES

t_metadata read_metadata(FILE *file); //lecture des métadonnées à partir du fichier
t_tupletable *insert_tuple(t_tupletable *table, t_tuple tuple); //Insérer un tuple dans la table
t_tupletable *load_tuples(FILE *file, t_metadata *meta); //Charger les tuples depuis le fichier en utilisant les métadonnées
int search_key(t_tupletable *table, t_key key, int *comparisons); //Rechercher une clé dans la table et compter les comparaisons effectuées
void print_result(t_metadata *meta, t_tuple *tuple, int comparisons) 

// LECTURE DES METADONNEES

t_metadata read_metadata(FILE *file) {
    t_metadata meta = {0}; //Initialisation de la structure des métadonnées
    char temp[MAXLEN];

    while (fgets(temp, MAXLEN, file)) { //Lire chaque ligne du fichier
        if (temp[0] == '#') continue;
        if (meta.sep == '\0') { //lire le séparateur
            meta.sep = temp[0];
            continue;
        }
        if (meta.nbFields == 0) { //lire le nb de champs 
            meta.nbFields = atoi(temp); //le convertir en entier
            meta.fieldNames = malloc(meta.nbFields * sizeof(t_field)); //allouer de la mémoire pour les noms de champs
            continue;
        }
        char *token = strtok(temp, &meta.sep); //scinder la ligne avec le séparateur
        for (int i = 0; i < meta.nbFields && token != NULL; i++) {
            strncpy(meta.fieldNames[i], token, MAXLEN); //copier chaque nom de champ
            token = strtok(NULL, &meta.sep); //passer au champ suivant
        }
        break;
    }
    return meta;
}

// INSERTION DE TUPLE DANS LE TABLEAU

t_tupletable *insert_tuple(t_tupletable *table, t_tuple tuple) {
    if (table->nbTuples >= table->sizeTab) { //si la table est complète
        table->sizeTab *= 2; //on double la taille allouée
        table->tuples = realloc(table->tuples, table->sizeTab * sizeof(t_tuple)); //on réalloue la mémoire
        if (!table->tuples) { //on vérifie l'échec de la réallocation
            perror("Erreur de redimensionnement du tableau");
            exit(1); //on quitte si on a une erreur
        }
    }
    table->tuples[table->nbTuples++] = tuple; //on ajoute le tuple et on fait +1 sur le compteur
    return table;
}

// CHARGEMENT DES TUPLES 

t_tupletable *load_tuples(FILE *file, t_metadata *meta) {
    t_tupletable *table = malloc(sizeof(t_tupletable)); //on alloue la mémoire 
    table->tuples = malloc(100 * sizeof(t_tuple)); //on alloue la mémoire initiale pour 100 tuples
    table->sizeTab = 100;
    table->nbTuples = 0;

    char line[MAXLEN];
    while (fgets(line, MAXLEN, file)) {
        if (line[0] == '#') continue;

        t_tuple tuple; //création tuple temporaire 
        char *token = strtok(line, &meta->sep); //on découpe avec le séparateur
        strncpy(tuple.key, token, MAXLEN); //lecture de la clé

        for (int i = 0; i < meta->nbFields - 1; i++) { //lecture des valeurs restantes
            token = strtok(NULL, &meta->sep); //on passe au champ suivant
            if (token) { 
                strncpy(tuple.value[i], token, MAXLEN);//on copie la valeur dans le tuple
            } else {
                tuple.value[i][0] = '\0'; //le champ reste vide si NULL
            }
        }
        insert_tuple(table, tuple);
    }
    return table;
}

// RECHERCHE DE CLE

int search_key(t_tupletable *table, t_key key, int *comparisons) {
    *comparisons = 0; //on initialise le compteur de comparaisons à 0
    for (int i = 0; i < table->nbTuples; i++) {
        (*comparisons)++;
        if (strcmp(table->tuples[i].key, key) == 0) { //on compare les clés 
            return i; //on retourne l'indice si on a trouvé 
        }
    }
    return -1; //sinon -1
}

// AFFICHAGE DES RESULTATS

void print_result(t_metadata *meta, t_tuple *tuple, int comparisons) {
    printf("Recherche de %s : trouvé ! nb comparaisons : %d\n", tuple->key, comparisons);
    for (int i = 0; i < meta->nbFields - 1; i++) {
        printf("%s: %s\n", meta->fieldNames[i + 1], tuple->value[i][0] ? tuple->value[i] : "X"); //on affiche les valeurs ou X si c'est vide
    }
}

// BOUCLE PRINCIPALE

int main(int argc, char **argv) {
    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r"); //on ouvre le fichier en lecture 
    if (!file) { //on vérifie s'il y a un échec à l'ouverture 
        perror("Erreur d'ouverture du fichier");
        return 1;
    }

    t_metadata meta = read_metadata(file); //on lit les métadonnées
    t_tupletable *table = load_tuples(file, &meta); //on charge les tuples
    fclose(file);

    printf("%d mots indexés\n", table->nbTuples); //on affiche le nb de tuples
    printf("Saisir les mots recherchés :\n");

    char key[MAXLEN];
    while (scanf("%s", key) != EOF) {
        int comparisons = 0;
        int idx = search_key(table, key, &comparisons);
        if (idx == -1) {
            printf("Recherche de %s : échec ! nb comparaisons : %d\n", key, comparisons);
        } else {
            print_result(&meta, &table->tuples[idx], comparisons);
        }
    }

    free(meta.fieldNames); //Libération des mémoires (éviter les fuites mémoires)
    free(table->tuples); //libération mémoire des tuples 
    free(table); //libération mémoire de la table
    return 0;
}
