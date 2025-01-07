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

t_metadata read_metadata(FILE *file);
t_tupletable *insert_tuple(t_tupletable *table, t_tuple tuple);
t_tupletable *load_tuples(FILE *file, t_metadata *meta);
int search_key(t_tupletable *table, t_key key, int *comparisons);
void print_result(t_metadata *meta, t_tuple *tuple, int comparisons)

// LECTURE DES METADONNEES

t_metadata read_metadata(FILE *file) {
    t_metadata meta = {0};
    char temp[MAXLEN];

    while (fgets(temp, MAXLEN, file)) {
        if (temp[0] == '#') continue;
        if (meta.sep == '\0') {
            meta.sep = temp[0];
            continue;
        }
        if (meta.nbFields == 0) {
            meta.nbFields = atoi(temp);
            meta.fieldNames = malloc(meta.nbFields * sizeof(t_field));
            continue;
        }
        char *token = strtok(temp, &meta.sep);
        for (int i = 0; i < meta.nbFields && token != NULL; i++) {
            strncpy(meta.fieldNames[i], token, MAXLEN);
            token = strtok(NULL, &meta.sep);
        }
        break;
    }
    return meta;
}

// INSERTION DE TUPLE DANS LE TABLEAU

t_tupletable *insert_tuple(t_tupletable *table, t_tuple tuple) {
    if (table->nbTuples >= table->sizeTab) {
        table->sizeTab *= 2;
        table->tuples = realloc(table->tuples, table->sizeTab * sizeof(t_tuple));
        if (!table->tuples) {
            perror("Erreur de redimensionnement du tableau");
            exit(1);
        }
    }
    table->tuples[table->nbTuples++] = tuple;
    return table;
}

// CHARGEMENT DES TUPLES 

t_tupletable *load_tuples(FILE *file, t_metadata *meta) {
    t_tupletable *table = malloc(sizeof(t_tupletable));
    table->tuples = malloc(100 * sizeof(t_tuple));
    table->sizeTab = 100;
    table->nbTuples = 0;

    char line[MAXLEN];
    while (fgets(line, MAXLEN, file)) {
        if (line[0] == '#') continue;

        t_tuple tuple;
        char *token = strtok(line, &meta->sep);
        strncpy(tuple.key, token, MAXLEN);

        for (int i = 0; i < meta->nbFields - 1; i++) {
            token = strtok(NULL, &meta->sep);
            if (token) {
                strncpy(tuple.value[i], token, MAXLEN);
            } else {
                tuple.value[i][0] = '\0';
            }
        }
        insert_tuple(table, tuple);
    }
    return table;
}

// RECHERCHE DE CLE

int search_key(t_tupletable *table, t_key key, int *comparisons) {
    *comparisons = 0;
    for (int i = 0; i < table->nbTuples; i++) {
        (*comparisons)++;
        if (strcmp(table->tuples[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

// AFFICHAGE DES RESULTATS

void print_result(t_metadata *meta, t_tuple *tuple, int comparisons) {
    printf("Recherche de %s : trouvé ! nb comparaisons : %d\n", tuple->key, comparisons);
    for (int i = 0; i < meta->nbFields - 1; i++) {
        printf("%s: %s\n", meta->fieldNames[i + 1], tuple->value[i][0] ? tuple->value[i] : "X");
    }
}

// BOUCLE PRINCIPALE

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return 1;
    }

    t_metadata meta = read_metadata(file);
    t_tupletable *table = load_tuples(file, &meta);
    fclose(file);

    printf("%d mots indexés\n", table->nbTuples);
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
    free(table->tuples);
    free(table);
    return 0;
}
