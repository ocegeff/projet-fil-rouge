#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAXLEN 1000
#define MAXFIELDS 15

typedef char t_field[MAXLEN];
typedef t_field t_key;
typedef t_field t_value[MAXFIELDS];

typedef struct {
    t_key key;
    t_value value;
} t_tuple;

typedef struct node {
    t_tuple data;
    struct node *pNext;
} t_node;

typedef t_node *t_list;

typedef struct {
    char *hashfunction; // nom de la fonction de hachage
    int nbSlots;        // nombre d’alvéoles
    t_list *slots;      // tableau de listes chaînées
} t_hashtable;

// Fonctions utilitaires pour les listes chaînées
int isEmpty(t_list lst) {
    return (lst == NULL);
}

t_tuple getFirstVal(t_list lst) {
    assert(!isEmpty(lst));
    return lst->data;
}

t_list newList() {
    return NULL;
}

t_list addHeadNode(t_tuple data, t_list lst) {
    t_node *n = malloc(sizeof(t_node));
    n->data = data;
    n->pNext = lst;
    return n;
}

t_list removeHeadNode(t_list lst) {
    assert(!isEmpty(lst));
    t_list p = lst->pNext;
    free(lst);
    return p;
}

// Fonction de hachage par défaut
int hash1(char *key, int nbSlots) {
    int sum = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        sum += key[i];
    }
    return sum % nbSlots;
}

// Initialisation d'une table de hachage
t_hashtable *initHashTable(int nbSlots, char *hashfunction) {
    t_hashtable *table = malloc(sizeof(t_hashtable));
    table->hashfunction = hashfunction;
    table->nbSlots = nbSlots;
    table->slots = malloc(nbSlots * sizeof(t_list));
    for (int i = 0; i < nbSlots; i++) {
        table->slots[i] = newList();
    }
    return table;
}

// Insertion d'un tuple dans la table de hachage
void insertIntoHashTable(t_hashtable *table, t_tuple data) {
    int index = hash1(data.key, table->nbSlots); // Utilise hash1 comme fonction de hachage
    table->slots[index] = addHeadNode(data, table->slots[index]);
}

// Recherche dans la table de hachage
t_tuple *searchInHashTable(t_hashtable *table, char *key, int *comparisons) {
    int index = hash1(key, table->nbSlots);
    t_list current = table->slots[index];

    *comparisons = 0;

    while (!isEmpty(current)) {
        (*comparisons)++;
        if (strcmp(getFirstVal(current).key, key) == 0) {
            return &current->data; // Retourne un pointeur valide vers la donnée
        }
        current = current->pNext;
    }

    return NULL; // Clé non trouvée
}

// Affichage d'une table de hachage
void displayHashTable(t_hashtable *table) {
    for (int i = 0; i < table->nbSlots; i++) {
        printf("Slot %d:\n", i);
        t_list current = table->slots[i];
        while (!isEmpty(current)) {
            printf("  Key: %s\n", current->data.key);
            printf("  Value: ");
            for (int j = 0; current->data.value[j][0] != '\0'; j++) {
                printf("%s ", current->data.value[j]);
            }
            printf("\n");
            current = current->pNext;
        }
    }
}

// Libération de la mémoire
void freeHashTable(t_hashtable *table) {
    for (int i = 0; i < table->nbSlots; i++) {
        t_list current = table->slots[i];
        while (!isEmpty(current)) {
            current = removeHeadNode(current);
        }
    }
    free(table->slots);
    free(table);
}

// Programme principal
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <fichier> <nombre d'alvéoles>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    int nbSlots = atoi(argv[2]);

    // Initialisation de la table de hachage
    t_hashtable *table = initHashTable(nbSlots, "hash1");

    // Lecture du fichier
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    char buffer[MAXLEN];
    char separator;
    int nbFields;

    // Lecture des métadonnées
    while (fgets(buffer, MAXLEN, file)) {
        if (buffer[0] == '#') continue; // Ignorer les commentaires
        separator = buffer[0];
        break;
    }

    fgets(buffer, MAXLEN, file);
    nbFields = atoi(buffer);

    fgets(buffer, MAXLEN, file); // Champs ignorés ici

    // Lecture des tuples
    while (fgets(buffer, MAXLEN, file)) {
        if (buffer[0] == '#') continue;

        t_tuple tuple;
        char *token = strtok(buffer, &separator);
        strcpy(tuple.key, token);

        int i = 0;
        while ((token = strtok(NULL, &separator)) != NULL && i < MAXFIELDS) {
            strcpy(tuple.value[i], token);
            i++;
        }

        while (i < MAXFIELDS) {
            tuple.value[i][0] = '\0';
            i++;
        }

        insertIntoHashTable(table, tuple);
    }

    fclose(file);

    // Rechercher des clés
    printf("Table créée. Entrez les mots recherchés:\n");
    char key[MAXLEN];
    int comparisons;

    while (scanf("%s", key) != EOF) {
        t_tuple *result = searchInHashTable(table, key, &comparisons);
        if (result) {
            printf("Key: %s trouvé après %d comparaisons\n", key, comparisons);
            printf("Values:\n");
            for (int i = 0; result->value[i][0] != '\0'; i++) {
                printf("  %s\n", result->value[i]);
            }
        } else {
            printf("Key: %s non trouvé après %d comparaisons\n", key, comparisons);
        }
    }

    // Libération de la mémoire
    freeHashTable(table);

    return 0;
}