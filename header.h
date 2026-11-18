#ifndef HEADER_H    
#define HEADER_H    
#include <ctype.h>  // isalpha e isalnum
#include <stdio.h>  // input/output
#include <stdlib.h> // Per funzioni di allocazione dinamica, gestione file, ecc.
#include <string.h> // Manipolazione stringhe, come strcmp, strcpy, ecc.

// Struttura dati per rappresentare una variabile
typedef struct {
  char *name;     // Nome della variabile
  char *type;     // Tipo della variabile
  int line;       // Riga in cui è stata dichiarata
  int used;       // Flag con 1 o 0 se usata o no
  int num_errors; // Contatore di errori
} Variabile;

typedef struct {
  char *file_input;  // nome file da analizzare
  char *file_output; // nome file di output
  int verbose;       // flag per output dettagliato (0 o 1)
} Parameters;

// Struttura di stato dell'analizzatore
typedef struct {
  int in_declaration;  // flag per segnare l'inizio di una dichiarazione di una variabile
  int in_assegnment;   // flag per segnare l'inizio di un assegnamento di una variabile
  char type_found[30]; // tipo trovato e salvo qui le variabili temporaneamente
  int is_typedef;      // flag per segnare l'inizio di un typedef
} Analyser;

// Struttura dati per memorizzare le statistiche per gli errori
typedef struct {
  int tot_vars;      // Numero totale di variabili controllate
  int tot_errors;    // Numero totale di errori rilevati
  int vars_not_used; // Numero di variabili non utilizzate
  int names_not_ok;  // Numero di nomi di variabili non corretti
  int types_not_ok;  // Numero di tipi di dato non corretti

  // Array dinamico per memorizzare le righe in cui si sono verificati errori
  int *error_lines;         // array dinamico di righe d'errore
  int error_lines_count;    // numero di righe con errori
  int error_lines_capacity; // capacita' iniziale che si raddoppia dinamicamente
} Statistics;


// Nodo della lista per memorizzare i tipi validi
typedef struct NodeType {
  char *types;           // Il tipo memorizzato
  struct NodeType *next; // Puntatore al PROSSIMO nodo
} NodeType;

// Struttura della lista per memorizzare i tipi validi
typedef struct {
  NodeType *head; // Punta al primo nodo
  int num_types;  // Conta quanti tipi
} ListType;

// gestione della lista di tipi validi
ListType *create_ListType(); // Crea una lista vuota  (per ogni tipo nuovo
                             // chiamo questa funzione)
void add_type(ListType *list, char *type); // Aggiunge un tipo alla lista
int flag_type_exists(ListType *list,
                     char *type);  // Controlla se un tipo esiste già (flag 1 o
                                   // 0) tramite ricerca volendo binaria
void freeListType(ListType *list); // Libera la memoria con free


// Parsing dei parametri da linea di comando (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
// argv legge gli argomenti passati da terminale (array di stringhe)
Parameters *parsing_parameters(int argc, char *argv[]);

// Lettura e analisi del file di input
// Carica tutte le variabili trovate in un array
// Ritorna l'array di Variabile e il numero totale in *num_vars
Variabile *read_file(char *filename, int *num_vars, Statistics *stats,
                     ListType *custom_types);

// Validazione del nome di una variabile
// Ritorna 1 se valido, 0 se non valido
int is_valid_name(char *name, ListType *custom_types);

// Validazione del tipo di una variabile /typedef/
// Ritorna 1 se valido, 0 se non valido
int is_valid_type(char *type, ListType *custom_types);

// Rimozione dei commenti dalla riga letta
void rmv_comments(char *line, int *in_m_comment);

// Riconosce se stiamo entrando nel corpo del codice
int is_start_of_code(char *token, Variabile *array_vars, int num_vars);

// Analizza la singola riga di codice
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr,
                  int *num_vars, int *capacity_array, Statistics *stats,
                  int *reading_declarations, ListType *custom_types,
                  Analyser *analyser);

// Calcolo delle statistiche finali
// Aggiorna la struct Statistiche con i risultati
void calculate_stats(Variabile *vars, int num_vars, Statistics *stats);

// Stampa dei risultati (su file e/o stdout)
void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param);

// Liberazione memoria
void free_memory(Variabile *vars, int num_vars, Parameters *param);

#endif