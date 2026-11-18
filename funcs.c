#include "header.h"

/*=====================================================*/
/* BLOCCO 1: PARSING DEI PARAMETRI DA LINEA DI COMANDO */
/*=====================================================*/

// Legge gli argomenti scitti in terminale (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
Parameters *parsing_parameters(int argc, char *argv[]) {

  // Memoria dinamica per gestire i comandi da terminale
  Parameters *par = malloc(sizeof(Parameters));
  if (par == NULL) {
    fprintf(stderr, "ERRORE: allocazione della memoria.\n");
    return NULL;
  }

  par->file_input = NULL;
  par->file_output = NULL;
  par->verbose = 0;

  // File input, se è -i o --in
  for (int i = 1; i < argc; i++) {

    // Confronto i caratteri con strcmp, se è -i o --in setto il file di input
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0) {
      if (i + 1 < argc) {
        // Copiamo la stringa 
        par->file_input = strdup(argv[i + 1]);
        i++;       
      } else {
        fprintf(stderr, "ERRORE: parametro -i richiede un argomento.\n");
        free(par->file_input);
        free(par->file_output);
        free(par); // Liberazione memoria
        return NULL;
      }

    // File output, se è -o o --out
    } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
      if (i + 1 < argc) {
        par->file_output = strdup(argv[i + 1]);
        i++;
      } else {
        fprintf(stderr, "ERRORE: parametro -o richiede un argomento.\n");
        free(par->file_input);
        free(par->file_output);
        free(par);
        return NULL;
      } 
    }
    // Flag verbose, se è -v o --verbose output dettagliato
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      par->verbose = 1;
    }
    // Combinazioni -vi o -iv per input e verbose insieme
    else if (strncmp(argv[i], "-vi", 3) == 0 || strncmp(argv[i], "-iv", 3) == 0) {
      par->verbose = 1;

      if (i + 1 < argc) {
        par->file_input = strdup(argv[i + 1]);
        i++;
      } else {
        fprintf(stderr, "ERRORE: -vi o -iv richiede un file di input.\n");
        free(par->file_input);
        free(par->file_output);
        free(par);
        return NULL;
      }
    }
  }

  return par; // Ritorna la struct Parameters con i valori letti da terminale
}

/*==========================*/
/* BLOCCO 2: LISTA DEI TIPI */
/*==========================*/

// Crea una lista vuota
ListType *create_ListType() {
  ListType *list = malloc(sizeof(ListType));
  if (list) {
    list->head = NULL;
    list->num_types = 0;
  }
  return list;
}

// Aggiunte un tipo alla lista, creando un nuovo nodo e inserendolo in testa
void add_type(ListType *list,char *type) 
{
  if (!list || !type) return;
  NodeType *new_node = malloc(sizeof(NodeType)); // Allochiamo memoria per un nuovo nodo
  new_node->types = strdup(type); // Copia la stringa
  new_node->next = list->head;    // Il nuovo nodo punta al vecchio head
  list->head = new_node;          // Il nuovo nodo diventa la nuova testa della lista
  list->num_types++;
}

// Ritorna 1 se il tipo esiste già nella lista, altrimenti ritorna 0
int flag_type_exists(ListType *list, char *type) {
  if (!list || !type) return 0; // se la lista o il tipo sono NULL, il tipo non esiste

  // Scorriamo la lista per trovare il tipo o arrivare alla fine della lista
  NodeType *current = list->head;
  while (current != NULL) { 
    if (strcmp(current->types, type) ==0)
      return 1;
    current = current->next;
  }
  return 0;
}

// Libera la memoria della lista
void freeListType(ListType *list) {
  if (!list) return;

  // Itereremo attraverso la lista e libereremo ogni nodo e la stringa associata
  NodeType *current = list->head; 
  while (current != NULL) {
    NodeType *temp = current; // Creamo un nodo temporaneo per liberare la memoria
    current = current->next;  
    free(temp->types);        
    free(temp);               
  }
  free(list);
}

/*===============================*/
/* BLOCCO 3: PULIZIA DELLE RIGHE */
/*===============================*/

//Rimuove i commenti dalla riga letta, sia commenti a linea singola che commenti multilinea
void rmv_comments(char *line, int *in_m_comment) {
  for (int i = 0; line[i] != '\0'; i++) {
    // Non dentro commento multiriga
    if (*in_m_comment == 0) {
      // Commento "//" taglio la riga e esco
      if (line[i] == '/' && line[i + 1] == '/') {
        line[i] = '\0'; 
        break;          

      // Inizio commento multiriga
      } else if (line[i] == '/' && line[i + 1] == '*') { 
        *in_m_comment = 1;
        line[i] = ' ';
        line[i + 1] = ' ';
        i++; // Salta asterisco
      }
    // Dentro commento multiriga
    } else {
      // Se fine commento multiriga "*/", chiudo il commento e pulisco la riga
      if (line[i] == '*' && line[i + 1] == '/') {
        *in_m_comment = 0;
        line[i] = ' ';
        line[i + 1] = ' ';
        i++;   // Salta slash
      } else { 
        // Se siamo dentro un commento multiriga, sostituisco i caratteri con spazi
        line[i] = ' ';
      }
    }
  }
}

// Funzione per tokenizzare la riga e aggiungere spazi intorno a = , ;
void cleaned_line(char *line) {
  if (line == NULL) return;

  char temp_buffer[1024] = ""; // Buffer temporaneo per la riga modificata
  int j = 0;

  for (int i = 0; line[i] != '\0'; i++) {
    if (line[i] == '=' || line[i] == ',' || line[i] == ';') {
      temp_buffer[j++] = ' ';
      temp_buffer[j++] = line[i];
      temp_buffer[j++] = ' ';
    } else {
      temp_buffer[j++] = line[i];
    }
  }
  temp_buffer[j] = '\0';     // Termina la stringa

  strcpy(line, temp_buffer); // Rimpiazziamo la riga originale con la versione modificata
}

/*===========================================*/
/* BLOCCO 4: VALIDAZIONE DEI NOMI E DEI TIPI */
/*===========================================*/

// Ritorna 1 se il nome è valido, 0 se non è valido
int is_valid_name(char *name, ListType *custom_types) {

  // Controllo se il nome è vuoto o NULL
  if (!name || strlen(name) == 0) return 0;

  // Controllo se il nome è un tipo
  if (is_valid_type(name, custom_types)) return 0;
  
  // Controllo primo carattere deve essere una lettera o un underscore
  if (!isalpha(name[0]) && name[0] != '_') return 0; 
  
  // Controllo che tutti i caratteri successivi siano alfanumerici o underscore
  for (int i = 1; name[i] != '\0'; i++) { 
    if (!isalnum(name[i]) && name[i] != '_')
      return 0;
  }
  return 1; // Controllo superato :)
}

// Ritorna 1 se il tipo è valido, 0 se non è valido
int is_valid_type(char *type, ListType *custom_types) {
  if (!type) return 0;

  // Tipi standard
  const char *standard_types[] = {
      "int",      "float",  "double", "char", "void",  "short", "long",
      "signed", "unsigned", "struct", "enum", "union", "FILE"};

  // Calcoliamo il numero di tipi standard nell'array
  int num_std = sizeof(standard_types) /sizeof(standard_types[0]); 

  // Controlliamo se il tipo è uno dei tipi standard
  for (int i = 0; i < num_std; i++) {
    if (strcmp(type, standard_types[i]) == 0) 
      return 1;
  }

  // Se non è uno dei tipi standard, controlliamo se è un typedef personalizzato
  return flag_type_exists( custom_types, type);
}

// Funzione che riconosce se stiamo entrando nel corpo del codice
int is_start_of_code(char *token, Variabile *array_vars, int num_vars) {
  if (token == NULL) return 0;

  // Lista parole chieve che indicano l'inizio del codice
  const char *keywords[] = {"if",     "for",    "while", "do",   "switch",
                            "return", "printf", "scanf", "else", "main"};
  
  int num_keywords = sizeof(keywords) / sizeof(keywords[0]); // Numero di parole chiave nell'array

  // Controlliamo se il token è una parola chiave o il nome di una variabile
  for (int i = 0; i < num_keywords; i++) {
    if (strcmp(token, keywords[i]) == 0) 
      // Se il token è una parola chiave, ritorniamo 1
      return 1;
  }

  // Controlliamo se il token è il nome di una variabile già dichiarata
  for (int i = 0; i < num_vars; i++) {
    if (strcmp(token, array_vars[i].name) == 0) 
      // Se il token è il nome di una variabile, ritorniamo 1
      return 1;
  }
  return 0; // Se il token non è né una parola chiave né il nome di una variabile
}

/*=================================*/
/* BLOCCO 5: GESTIONE DEGLI ERRORI */
/*=================================*/

// Registra il numero di riga in cui si è verificato un errore,
// espandendo dinamicamente l'array se necessario
void add_error_line(Statistics *stats, int line) {
  if (stats->error_lines_count >= stats->error_lines_capacity) {
    // Se l'array è pieno, raddoppiamo la capacità
    stats->error_lines_capacity *= 2;
    // Allochiamo memoria per il nuovo array degli errori
    int *temp = realloc(stats->error_lines,
                stats->error_lines_capacity *sizeof(int));
    // Se realloc fallisce, manteniamo l'array originale e stampiamo un errore
    if (temp) stats->error_lines = temp;
  }
  // Aggiungiamo la riga dell'errore all'array
  stats->error_lines[stats->error_lines_count++] = line; 
}

/*========================*/
/* BLOCCO 6: ANALISI RIGA */
/*========================*/

// Funzione che analizza una riga del file, tokenizzandola e aggiornando le statistiche
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr,
                  int *num_vars, int *capacity_array, Statistics *stats,
                  int *reading_declarations, ListType *custom_types,
                  Analyser *analyser) {

  Variabile *array_vars = *array_vars_ptr; 

  cleaned_line(line);

  // Definiamo i delimitatori per tokenizzare la riga
  const char *delimiters = " \t\n\r(){}[]*+-/!&|<>\"'";

  // Tokenizziamo la riga usando strtok, che divide la stringa in base ai delimitatori
  char *token = strtok( line, delimiters);

  if (token == NULL)
    return;
  // Se il token inizia con '#', è una direttiva del preprocessore, quindi ignoriamo la riga
  if (token[0] == '#')
    return; 

  // Se siamo nel blocco dichiarazioni e troviamo l'inizio del codice,
  // usciamo dal blocco dichiarazioni
  if (*reading_declarations == 1 && is_start_of_code(
      token, array_vars, *num_vars)) {
    *reading_declarations = 0;
  }

  // Cerca le variabili usate
  if (*reading_declarations == 0) {
    while (token != NULL) {
      // Saltiamo i simboli isolati
      if (strcmp(token, "=") == 0 || strcmp(token, ";") == 0 ||
          strcmp(token, ",") == 0) {
        token = strtok(NULL, delimiters);
        continue;
      }

      // Cerchiamo se il token corrisponde a una variabile used
      for (int i = 0; i < *num_vars; i++) {
        // Se il token corrisponde al nome di una variabile, impostiamo il flag "used" a 1
        if (strcmp(token, array_vars[i].name) == 0){
          array_vars[i].used = 1; // flag
          break; // Usciamo dal cilco
        }
      }
      // Passiamo al prossimo token
      token = strtok(NULL, delimiters);
    }
    return;
  }

  // Cerca le variabili dichiarate
  // Uso alis per non ->
  #define in_declaration (analyser->in_declaration)
  #define in_assegnment (analyser->in_assegnment)
  #define is_typedef (analyser->is_typedef)
  char *type_found = analyser->type_found;   

  // Ciclo per analizzare ogni token della riga
  while (token != NULL) {
    // Gestione dei simboli isolati, come =, ;, e , 
    // Se troviamo un '=' in una dichiarazione, significa che stiamo entrando in un'assegnazione
    if (strcmp(token, "=") == 0) {
      in_assegnment = 1;
      token = strtok(NULL, delimiters);
      continue;
    }
    // Se troviamo un ';', significa che la dichiarazione è terminata, quindi resettiamo i flag
    if (strcmp(token, ";") == 0) {
      in_declaration = 0;
      in_assegnment = 0;
      is_typedef = 0;
      token = strtok(NULL, delimiters);
      continue;
    }
    // Se troviamo una ',', significa che stiamo dichiarando più variabili dello stesso tipo
    if (strcmp(token, ",") == 0) {
      in_assegnment = 0;
      token = strtok(NULL, delimiters);
      continue;
    }

    // Analisi parola
    if (strcmp(token, "typedef") == 0) {
      is_typedef = 1;
      in_declaration = 1;
    
    // Se non siamo in dichiarazione, controlliamo se il token è un tipo valido
    } else if (in_declaration == 0){ 
      if (is_valid_type(token, custom_types)) {
        in_declaration = 1;        // Settiamo che siamo in dichiarazione
        strcpy(type_found, token); // Salviamo il tipo trovato

      // Se il token non è un tipo valido, incrementiamo i contatori di errori
      } else {
        stats->tot_errors++;
        stats->types_not_ok++;
        add_error_line(stats, current_line);

        // Salta tutti i token fino al prossimo ";"
        while (token != NULL && strcmp(token, ";") != 0) {
          token = strtok(NULL, delimiters);
        }
      }
    } else if (in_declaration == 1 && in_assegnment == 0) {
      if (is_typedef == 1) {
        if (!is_valid_type(token,NULL)) {
          // Aggiungiamo il nuovo tipo alla lista dei tipi validi
          add_type(custom_types, token);
        }
      } else {
        if (isdigit(token[0])) {
          int is_pure_number = 1;
          for (int k = 1; token[k] != '\0'; k++) {
            if (!isdigit(token[k]) && token[k] != '.' && token[k] != 'f') {        
              is_pure_number = 0; // flag a 0
              break;
            }
          }
          if (is_pure_number) { // Se è un numero puro, ignoralo e passa al prossimo token
            token = strtok(NULL, delimiters);
            continue;
          }
        }

        if (is_valid_name(token, custom_types)) {
          
          if (strcmp(token, "main") == 0) {
            in_declaration = 0; // Ignoriamo main, non è una variabile da salvare
            type_found[0] = '\0';
            break; // Interrompe il parsing di questa riga, così ignora
                   // argomenti come int argc che creano falsi positivi
          }
          stats->tot_vars++;

          // Array dinamico per le variabili
          if (*num_vars >= *capacity_array) {
            *capacity_array *= 2; // Raddoppiamo la capacità
            Variabile *temp_array = realloc( array_vars, (*capacity_array) *
                                    sizeof(Variabile)); // Riallochiamo la memoria
            if (temp_array == NULL) { // Se la memoria non è stata allocata
              fprintf(stderr, "ERRORE allocazione memoria: realloc fallito.\n");
              return; // Usciamo evitando di impostare array_vars a NULL
            }
            // Se realloc ha successo, aggiorniamo il puntatore e l'array
            array_vars = temp_array;
            *array_vars_ptr = array_vars;
          }

          array_vars[*num_vars].name = strdup(token); // Copiamo il token (allocazione dinamica con strdup)
          array_vars[*num_vars].type = strdup(type_found); // Copiamo il tipo (allocazione dinamica con strdup)
          array_vars[*num_vars].line = current_line; // Salviamo la riga
          array_vars[*num_vars].used = 0; //Inizializziamo il flag a 0
          (*num_vars)++;

        // Altrimenti non è un nome valido
        } else { 
          stats->tot_errors++;
          stats->names_not_ok++;
          add_error_line(stats, current_line); // Aggiungiamo la riga agli errori
        }
      }
    }
    token = strtok(NULL, delimiters); // Passiamo al prossimo token
  }

// Liberiamo i define
#undef in_declaration
#undef in_assegnment
#undef is_typedef
}

/*====================================*/
/* BLOCCO 7: LETTURA FILE E RISULTATI */
/*====================================*/

// funzione che legge il file e restituisce un array di variabili
Variabile *read_file(char *filename, int *num_vars, Statistics *stats, ListType *custom_types) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    fprintf(stderr, "ERRORE: impossibile aprire il file '%s'\n", filename);
    return NULL;
  }

  // Controlliamo se il file è vuoto
  fseek(file, 0, SEEK_END);
  if (ftell(file) == 0) {
    fprintf(stderr, "ERRORE: il file %s e' vuoto.\n", filename);
    fclose(file);
    return NULL;
  }
  // Torniamo all'inizio del file per iniziare la lettura
  rewind(file);

  // Array dinamico per salvare le variabili (token) trovati
  int capacity_array = 20;
  Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));

  *num_vars = 0;        // Contatore di variabili trovate
  int current_line = 0; // Per salvare dove si trova la variabile o l'errore

  char line[256]; // array di caratteri per memorizzare la riga letta dal file
  int in_m_comment = 0; // flag per capire se siamo dentro un commento multilinea

  // Flag per capire se siamo ancora nel blocco dichiarazioni o siamo entrati nel corpo del codice
  int reading_declarations = 1;

  Analyser analyser = {0, 0, "", 0}; // Mette a zero lo stato del parser lexicale

  // cicliamo per ogni riga del file IMPORTANTE**
  while (fgets(line, sizeof(line), file) != NULL) {
    current_line++; // Incrementa il contatore di linee per l'errore

    rmv_comments(line, &in_m_comment); // puntatore a in_m_comment per
                                       // modificare il flag dentro la funzione

    // passiamo &array_vars (puntatore a puntatore) e &capacity_array
    analyze_line(line, current_line, &array_vars, num_vars, &capacity_array,
                 stats, &reading_declarations, custom_types,
                 &analyser); // analizza la riga e aggiorna l'array di variabili
                             // e le statistiche
  }

  // Errore di lettura file (corrotto o problemi I/O)
  if (ferror(file)) { // controlla se c'è stato un errore durante la lettura
    fprintf(stderr,
            "Errore: si è verificato un errore durante la lettura del file!\n");
  }

  if (fclose(file) ==
      EOF) { // chiude il file e verifica che la chiusura sia andata a buon fine
    fprintf(stderr, "Errore durante la chiusura del file!\n");
  }

  return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}

// calcolo statistice di variabili non usate e totale errori
void calculate_stats(Variabile *vars, int num_vars, Statistics *stats) {
  for (int i = 0; i < num_vars; i++) {
    if (vars[i].used == 0) {  // se la variabile non è usata
      stats->vars_not_used++; // incrementa il contatore di variabili non usate
      stats->tot_errors++;    // incrementa il contatore di errori totali
      add_error_line(stats, vars[i].line); // aggiunge la riga agli errori
    }
  }
}

// funzione che stampa i risultati
void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param) {

  if (param->file_output != NULL) {
    FILE *file_out =
        fopen(param->file_output, "w"); // apre il file in scrittura
    if (file_out) { // se il file è stato aperto con successo
      fprintf(file_out, "=== RESOCONTO ANALISI ===\n");
      fprintf(file_out, "Variabili trovate: %d\n", stats->tot_vars);
      fprintf(file_out, "Errori totali: %d\n", stats->tot_errors);
      fprintf(file_out, "Nomi non validi: %d\n", stats->names_not_ok);
      fprintf(file_out, "Tipi non validi: %d\n", stats->types_not_ok);
      fprintf(file_out, "Variabili non usate: %d\n", stats->vars_not_used);
      fprintf(file_out, "-------------------------\n");

      fprintf(file_out, "\n --- CRONOLOGIA ERRORI --- \n");
      for (int e = 0; e < stats->error_lines_count;
           e++) { // scorre tutte le righe con errori
        fprintf(file_out, "Rilevato errore alla riga n: %d nel file.\n", stats->error_lines[e]);
      }

      fprintf(file_out, "\n --- VARIABILI NON USATE --- \n");
      for (int i = 0; i < num_vars; i++) { // scorre tutte le variabili
        if (vars[i].used == 0)
          fprintf(file_out, "Nome: %s\n", vars[i].name);
      }

      fprintf(file_out, "\n --- DETTAGLIO GENERALE VARIABILI --- \n");
      for (int i = 0; i < num_vars; i++) {
        fprintf(
            file_out,
            "Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
            vars[i].line, vars[i].type, vars[i].name,
            vars[i].used ? "USATA" : "NON USATA [!]");
      }

      // Controllo errori di scrittura su file come richiesto da specifiche
      if (ferror(file_out)) {
        fprintf(stderr, "ERRORE: si è verificato un problema durante la "
                        "scrittura sul file di output.\n");
      }

      if (fclose(file_out) == EOF) {
        fprintf(stderr,
                "ERRORE: problema durante la chiusura del file di output!\n");
      } else {
        printf("Analisi completata, Risultati salvati in '%s'\n",
               param->file_output);
      }
    } else {
      fprintf(stderr, "ERRORE: impossibile creare il file di output.\n");
    }
  }

  if (param->verbose == 1 || param->file_output == NULL) { 
    printf("\n=== RESOCONTO ANALISI ===\n");
    printf("Variabili trovate: %d\n", stats->tot_vars);
    printf("Errori totali: %d\n", stats->tot_errors);
    printf(" - Nomi non validi: %d\n", stats->names_not_ok);
    printf(" - Tipi non validi: %d\n", stats->types_not_ok);
    printf(" - Variabili non usate: %d\n", stats->vars_not_used);
    printf("=========================\n");

    printf("\n --- CRONOLOGIA ERRORI --- \n");
    for (int e = 0; e < stats->error_lines_count; e++) {
      printf("Rilevato errore alla riga n: %d nel file.\n", stats->error_lines[e]);
    }

    printf("\n --- VARIABILI NON USATE --- \n");
    for (int i = 0; i < num_vars; i++) {
      if (vars[i].used == 0)
        printf("Nome: %s\n", vars[i].name);
    }
    if (param->verbose == 1) {
      printf("\n --- DETTAGLIO GENERALE VARIABILI --- \n");
      for (int i = 0; i < num_vars; i++) {
        printf(
            "Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
            vars[i].line, vars[i].type, vars[i].name,
            vars[i].used ? "USATA" : "NON USATA");
      }
    }
  }
}


/*===============================*/
/* BLOCCO 8: LIBERAZIONE MEMORIA */
/*===============================*/

// Funzione per liberare la memoria allocata per le variabili e i parametri
void free_memory(Variabile *vars, int num_vars, Parameters *param) {
  // Liberiamo dalla memoria nomi e tipi delle variabili
  for (int i = 0; i < num_vars; i++) {
    if (vars[i].name)
      free(vars[i].name);
    if (vars[i].type)
      free(vars[i].type);
  }
  free(vars);

  // Liberiamo dalla memoria, i file in input e output
  if (param) {
    if (param->file_input)
      free(param->file_input);
    if (param->file_output)
      free(param->file_output);
    free(param);
  }
}
