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
    return (lst == NULL); //retourne vrai si la liste est vide
}

t_tuple getFirstVal(t_list lst) {
    assert(!isEmpty(lst)); //vérifie que la liste n'est pas vide
    return lst->data; //retourne les données du premier noeud
}

t_list newList() {
    return NULL;
}

t_list addHeadNode(t_tuple data, t_list lst) {
    t_node *n = malloc(sizeof(t_node)); //alloue mémoire pour un nouveau noeud
    n->data = data; //affecte les données au noeud
    n->pNext = lst; //pointeur vers l'ancienne tete de liste
    return n; //retourne le nouveau noeud comme tête 
}

t_list removeHeadNode(t_list lst) {
    assert(!isEmpty(lst));
    t_list p = lst->pNext;//conserve le pointeur vers le noeud suivant
    free(lst);//libère la mémoire
    return p; //retourne la nouvelle tête de liste
}

// Fonction de hachage par défaut
int hash1(char *key, int nbSlots) {
    int sum = 0;
    for (int i = 0; key[i] != '\0'; i++) { //parcourt chaque caractère de la clé 
        sum += key[i]; 
    }
    return sum % nbSlots; //retourne l'indice de slot
}

// Initialisation d'une table de hachage
t_hashtable *initHashTable(int nbSlots, char *hashfunction) {
    t_hashtable *table = malloc(sizeof(t_hashtable)); //alloue la mémoire
    table->hashfunction = hashfunction;//stocke la fonction de hachage
    table->nbSlots = nbSlots; //nb de slots dans le tableau
    table->slots = malloc(nbSlots * sizeof(t_list)); //alloue le tableau de listes chainées 
    for (int i = 0; i < nbSlots; i++) {
        table->slots[i] = newList();
    }
    return table; //retourne tableau de hachage
}

// Insertion d'un tuple dans la table de hachage
void insertIntoHashTable(t_hashtable *table, t_tuple data) {
    int index = hash1(data.key, table->nbSlots); // Utilise hash1 comme fonction de hachage
    table->slots[index] = addHeadNode(data, table->slots[index]); //ajoute le tuple en tête de liste
}

// Recherche dans la table de hachage
t_tuple *searchInHashTable(t_hashtable *table, char *key, int *comparisons) {
    int index = hash1(key, table->nbSlots); // Utilise hash1 comme fonction de hachage
    t_list current = table->slots[index];//liste chainée correspondante

    *comparisons = 0;

    while (!isEmpty(current)) { //parcourt la liste chainée 
        (*comparisons)++;
        if (strcmp(getFirstVal(current).key, key) == 0) { //compare les clés
            return &current->data; // Retourne un pointeur valide vers la donnée
        }
        current = current->pNext; //on passe au noeud suivant
    }

    return NULL; // Clé non trouvée
}

// Affichage d'une table de hachage
void displayHashTable(t_hashtable *table) {
    for (int i = 0; i < table->nbSlots; i++) {
        printf("Slot %d:\n", i); //affichage de l'indice du slot
        t_list current = table->slots[i]; //accès à la liste chainée du slot 
        while (!isEmpty(current)) {
            printf("  Key: %s\n", current->data.key); //affiche la clé
            printf("  Value: ");
            for (int j = 0; current->data.value[j][0] != '\0'; j++) { 
                printf("%s ", current->data.value[j]);
            }
            printf("\n");
            current = current->pNext; //passe au noeud suivant 
        }
    }
}

// Libération de la mémoire
void freeHashTable(t_hashtable *table) {
    for (int i = 0; i < table->nbSlots; i++) {
        t_list current = table->slots[i];
        while (!isEmpty(current)) { //libere chaque noeud de la liste
            current = removeHeadNode(current);
        }
    }
    free(table->slots); //libere tableau des listes chainées 
    free(table); //libere structure du tableau de hachage
}

// Programme principal
int main(int argc, char *argv[]) {
    if (argc < 3) { //vérifie que les arguments nécessaires sont fournis
        printf("Usage: %s <fichier> <nombre d'alvéoles>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1]; //récupère le nom du fichier
    int nbSlots = atoi(argv[2]);//récupère nb de slots

    // Initialisation de la table de hachage
    t_hashtable *table = initHashTable(nbSlots, "hash1");

    // Lecture du fichier
    FILE *file = fopen(filename, "r");//ouvre le fichier en lecture
    if (!file) {
        perror("Error opening file");//si le fichier a un échec à l'ouverture
        return 1;
    }

    char buffer[MAXLEN];
    char separator;
    int nbFields;

    // Lecture des métadonnées
    while (fgets(buffer, MAXLEN, file)) {
        if (buffer[0] == '#') continue; // Ignorer les commentaires
        separator = buffer[0];//le premier caractère est le séparateur
        break;
    }

    fgets(buffer, MAXLEN, file);//lit le nb de champs 
    nbFields = atoi(buffer);

    fgets(buffer, MAXLEN, file); // Champs ignorés ici

    // Lecture des tuples
    while (fgets(buffer, MAXLEN, file)) {
        if (buffer[0] == '#') continue;

        t_tuple tuple;
        char *token = strtok(buffer, &separator); //découpe la ligne avec le séparateur
        strcpy(tuple.key, token);//récupère la clé pour le tuple 

        int i = 0;
        while ((token = strtok(NULL, &separator)) != NULL && i < MAXFIELDS) {
            strcpy(tuple.value[i], token);//copie chaque valeur dans le tableau des valeurs
            i++;
        }

        while (i < MAXFIELDS) { //complète les champs restants avec des chaines vides si nécessaire
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

    while (scanf("%s", key) != EOF) { //lit clés saisies par l'utilisateur
        t_tuple *result = searchInHashTable(table, key, &comparisons);//recherche la clé dans le tableau
        if (result) {
            printf("Key: %s trouvé après %d comparaisons\n", key, comparisons);
            printf("Values:\n");
            for (int i = 0; result->value[i][0] != '\0'; i++) {
                printf("  %s\n", result->value[i]); //affiche toutes les valeurs associées
            }
        } else {
            printf("Key: %s non trouvé après %d comparaisons\n", key, comparisons);
        }
    }

    // Libération de la mémoire
    freeHashTable(table);

    return 0;
}
