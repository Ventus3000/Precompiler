#include "header.h"

int main(int argc, char *argv[]) { // args è il numero di argomenti, e argv l'array
  Parameters *par;
  Variabile *var;
  Statistics stats;
  int num_vars;

  // Partiamo gestendo prima l'input dell'utente da terminale (-i, -o, -v)
  par = parsing_parameters(argc, argv);

  if (par == NULL || par->file_input == NULL) {
    fprintf(stderr, "ERRORE: parametro -i obbligatorio.\n");
    fprintf(stderr,
            "Uso appropriato: %s -i <file_input> [-o <file_output>] [-v]\n",
            argv[0]);
    return 1;
  }

  // Inizializziamo le statistiche a zero
  stats.tot_vars = 0;
  stats.tot_errors = 0;
  stats.vars_not_used = 0;
  stats.names_not_ok = 0;
  stats.types_not_ok = 0;

  // Array dinamico di righe d'errore
  stats.error_lines_capacity = 20; // capacita' iniziale che si raddoppia dinamicamente
  stats.error_lines = malloc(stats.error_lines_capacity * sizeof(int));
  stats.error_lines_count = 0;

  ListType *custom_types = create_ListType(); // Creiamo la lista dei typedef

  // Lettura file C
  var = read_file(par->file_input, &num_vars, &stats, custom_types);

  if (var == NULL) {
    fprintf(stderr, "ERRORE: file impossibile leggere il file '%s'\n",
            par->file_input);
    freeListType(custom_types);
    free_memory(NULL, 0, par);
    return 1;
  }

  calculate_stats(var, num_vars, &stats);

  print_result(&stats, var, num_vars, par);

  free_memory(var, num_vars, par);

  freeListType(custom_types);

  if (stats.error_lines)
    free(stats.error_lines);

  return 0;
}
