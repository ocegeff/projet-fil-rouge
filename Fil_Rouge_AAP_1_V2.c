// Livrable Fil Rouge AAP
// Écrit par : Petit Amélie, Geffroy Océane, Schwerdorffer Dimitri, Durand Roehner Lydie
// Groupe G
//Programme 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_LEN 1000
#define MAX_FIELDS 15
#define MAX_TUPLES 60000

// 1. DEFINTION DES DIFFERENTS TYPES ET STRUCTURES

// 1.1 TYPES PRIMAIRES

typedef char t_field[MAX_LEN];       // Champ : chaîne de caractère (max 1000)
typedef t_field t_key;               // Clé : champ
typedef t_field t_value[MAX_FIELDS]; // t_value : tableau de 6000 champs maximum

typedef struct {                     //Définition des métadonnées
    char sep;                        // Séparateur (entre deux champs)
    int nbFields;                    // Nombre maximum de champs sur une ligne
    t_key key;                       // Label des champs
    t_field *fieldNames;             // FieldNames est un pointeur vers un tableau de champs
} t_metadata;

// 1.2 TYPES SECONDAIRES

typedef struct {
    t_key key;
    t_value val;
} t_tuple;                           // Tuple : couple (clé, valeur)

typedef struct {
    t_tuple tuples[MAX_TUPLES];      // Tableau de tuples
    int nbTuples;                    // Nombre de mots saisis
} t_tupletable;

// typedef int (*function)(t_key, t_hashtable);


// 2. PROTOTYPES

void split(char sep, char *txt, int nbFields, t_key key, t_field *field_table);         // Documentation : 1
void print_tuples(t_metadata data, t_tupletable *dico, char *key);                      // Documentation : 2

// 3. FONCTIONS

void split(char sep, char *txt, int nbFields, t_key key, t_field *field_table) {
    int l = strlen(txt);                                       // "strlen" calcule le nombre de caractères de la chaîne de caractères passée en argument.
    int k = 0, i = 0, j = 0;

    while ((txt[i] != sep) && (txt[i] != '\0')) {              // Recherche des clés
        key[i] = txt[i];
        i++;
    }

    key[i] = '\0';

    i++;                                                       //Positionne le pointeur de lecture au début du prochain champ.

    while ((i < l) && (k < nbFields)) {                        // 	Extraction de tous les champs en découpant la chaîne à chaque occurrence du séparateur, en ignorant les caractères de nouvelle ligne et de fin de chaîne.
        while ((txt[i] != sep) && (txt[i] != '\0') && (txt[i] != '\n') && (txt[i] != '\r')) {
            field_table[k][j] = txt[i];
            j++;
            i++;
        }
        k++;
        i++;
        j = 0;
    }
}

void print_tuples(t_metadata data, t_tupletable *dico, char *key) {
    int index_key = 0; int found = 0;
    for (int i = 0; i < dico->nbTuples; i++) { // Recherche de la clé dans le dictionnaire. La fonction strcmp compare deux chaînes de caractères et retourne 0 si elles sont identiques.  
        if (strcmp(key, dico->tuples[i].key) == 0) {
            found = 1;
            break;
        }
        index_key++;
    }
    if (!found) {
        printf("Recherche de %s : échec ! nb comparaisons : %d\n", key, index_key);
    } else {
        printf("Recherche de %s : trouvé ! nb comparaisons : %d\n", key, index_key + 1);
        printf("%s : %s\n", data.key, key); // mot : key

        for (int k = 0; k < data.nbFields - 1; k++) {
            if ((dico->tuples[index_key].val[k][0] != '\0')) {
                printf("%s: %s\n", data.fieldNames[k], dico->tuples[index_key].val[k]);
            } else if (strcmp(data.fieldNames[k], "") != 0) {
                printf("%s: X\n", data.fieldNames[k]);
            }
        }
    }
}

int main(int argc, char **argv) {

    FILE *anagrammeFile = fopen(argv[1], "r"); //Ouverture du fichier.

    // IMPLEMENTATION DES METADONNEES

    char temp[MAX_LEN]; // Chaîne de caractères temporaire qui va contenir le texte du fichier.
	
    // SEPARATEUR
	
    fgets(temp, MAX_LEN, anagrammeFile);
    while (strcspn(temp, "\n\r") > 1) { // Une ligne ne sera lue que si elle ne contient qu'un caractère, qui sera le séparateur.
        fgets(temp, MAX_LEN, anagrammeFile);
    }

    t_metadata meta;
    meta.sep = *temp; // Contient le séparateur.

    // NOMBRE DE CHAMPS ET NOMS DE CHAMPS
    if (meta.sep != '#') { // si "#" n'est pas le séparateur, on passe chaque ligne qui commence avec ce caractère.
        fgets(temp, MAX_LEN, anagrammeFile);
        while (temp[0] == '#') {
            fgets(temp, MAX_LEN, anagrammeFile);
        }

        meta.nbFields = atoi(temp); // Obtient le nombre de champs.

        t_field *field_table = malloc(sizeof(t_field) * meta.nbFields);

        fgets(temp, MAX_LEN, anagrammeFile);
        while (temp[0] == '#') {
            fgets(temp, MAX_LEN, anagrammeFile);
        }

        split(meta.sep, temp, meta.nbFields, meta.key, field_table);
        meta.fieldNames = field_table; // Obtient le nom des champs.
		
    } else { // Si "#" est le séparateur. On suppose qu'il n'y a pas de commentaire après la déclaration du séparateur.
        fgets(temp, MAX_LEN, anagrammeFile);
        meta.nbFields = atoi(temp);
        fgets(temp, MAX_LEN, anagrammeFile);

        t_field *field_table = malloc(sizeof(t_field) * meta.nbFields); //Liste de champs

        split(meta.sep, temp, meta.nbFields, meta.key, field_table);
        meta.fieldNames = field_table;
    }

    // CREATION DU DICTIONNAIRE
    t_tupletable *dico = malloc(sizeof(t_tupletable)); 
    int len_dic = 0;

    while (fgets(temp, MAX_LEN, anagrammeFile) != NULL) { // Lit jusqu'à la fin du fichier.
        split(meta.sep, temp, meta.nbFields, dico->tuples[len_dic].key, dico->tuples[len_dic].val);
        len_dic++; //Remplissage dico
    }
    fclose(anagrammeFile);

    dico->nbTuples = len_dic; //nombre de clés ajoutées dans dico 

    char finding[MAX_LEN];
    printf("Mots indexés : %d\n", len_dic);
    printf("Donnez le mot à trouver : \n");
    // printf("%s", finding);
    while (scanf("%99s", finding) != EOF) {
        print_tuples(meta, dico, finding);
        printf("\nDonnez le mot à trouver : \n");
    }

}
