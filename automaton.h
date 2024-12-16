#include "str.h"

typedef struct
{
  string *elements;
  int card;
} set;

typedef struct
{
  string origin;
  char symbol;
  string destination;
} transition;

typedef struct
{
  transition *transitions;
  int size;
} Delta;

typedef struct
{
  set *elements;
  int card;
} path_list;

typedef struct
{
  set *Q;
  set *Sigma;
  set *s;
  set *F;
  Delta *Delta;
} info_automaton;

// Reads a row of the .txt file
string read_line(FILE *file)
{
  int b_size = 1;
  int length = 0;
  int ch;

  char *buffer = (string)malloc(b_size * sizeof(char));
  if (buffer == NULL)
  {
    printf("Error allocating memory\n");
    return NULL;
  }

  while ((ch = fgetc(file)) != EOF && ch != '\n')
  {
    if (length >= b_size - 1)
    {
      b_size *= 2;
      char *temp = realloc(buffer, b_size * sizeof(char));
      if (temp == NULL)
      {
        printf("Error allocating memory\n");
        free(buffer);
        return NULL;
      }
      buffer = temp;
    }
    buffer[length++] = (char)ch;
  }

  if (length == 0 && ch == EOF)
  {
    free(buffer);
    return NULL;
  }

  buffer[length] = '\0';
  return buffer;
}

// Prints a set
void print_set(set *s)
{
  if (s == NULL || s->elements == NULL)
  {
    printf("Empty set.\n");
    return;
  }

  for (int i = 0; i < s->card; i++)
  {
    printf("%s", s->elements[i]);
    if (i < s->card - 1)
    {
      printf(", ");
    }
  }
}

// Prints a set (path)
void print_path(set *s)
{
  if (s == NULL || s->elements == NULL)
  {
    printf("Empty set.\n");
    return;
  }

  for (int i = 0; i < s->card; i++)
  {
    printf("%s", s->elements[i]);
    if (i < s->card - 1)
    {
      printf("->");
    }
  }
}

// Checks if the element already exist
int check_unicity(set *new_set, string aux)
{
  for (int i = 0; i < new_set->card; i++)
  {
    if (str_comp(new_set->elements[i], aux) == 0)
    {
      return 1;
    }
  }
  return 0;
}

// Returns a set given line-string and delimiter
set *get_set(string line, int opt)
{
  char delimiter = ',';
  int i = 0, k = 0;

  string aux = (string)malloc(2);
  if (aux == NULL)
  {
    printf("Error allocating memory\n");
    return NULL;
  }
  aux[0] = '\0';

  set *new_set = (set *)malloc(sizeof(set));
  if (new_set == NULL)
  {
    printf("Error allocating memory\n");
    free(aux);
    return NULL;
  }
  new_set->card = 0;
  new_set->elements = NULL;

  while (line[i] != '\0')
  {
    if (line[i] != delimiter)
    {
      char *temp = realloc(aux, k + 2);
      if (temp == NULL)
      {
        printf("Error allocating memory\n");
        free(aux);
        for (int m = 0; m < new_set->card; m++)
        {
          free(new_set->elements[m]);
        }
        free(new_set->elements);
        free(new_set);
        return NULL;
      }
      aux = temp;
      aux[k++] = line[i];
      aux[k] = '\0';
    }
    else
    {
      if (check_unicity(new_set, aux) == 0 || opt == 0)
      {
        string *temp_elements =
            realloc(new_set->elements, (new_set->card + 1) * sizeof(string));
        if (temp_elements == NULL)
        {
          printf("Error allocating memory\n");
          free(aux);
          for (int m = 0; m < new_set->card; m++)
          {
            free(new_set->elements[m]);
          }
          free(new_set->elements);
          free(new_set);
          return NULL;
        }
        new_set->elements = temp_elements;

        new_set->elements[new_set->card] =
            (string)malloc((k + 1) * sizeof(char));
        if (new_set->elements[new_set->card] == NULL)
        {
          printf("Error allocating memory\n");
          free(aux);
          for (int m = 0; m < new_set->card; m++)
          {
            free(new_set->elements[m]);
          }
          free(new_set->elements);
          free(new_set);
          return NULL;
        }
        copy_str(new_set->elements[new_set->card], aux);
        new_set->card++;
      }
      k = 0;
      aux[0] = '\0';
    }
    i++;
  }

  if (k > 0 && (check_unicity(new_set, aux) == 0 || opt == 0))
  {
    string *temp_elements =
        realloc(new_set->elements, (new_set->card + 1) * sizeof(string));
    if (temp_elements == NULL)
    {
      printf("Error allocating memory\n");
      free(aux);
      for (int m = 0; m < new_set->card; m++)
      {
        free(new_set->elements[m]);
      }
      free(new_set->elements);
      free(new_set);
      return NULL;
    }
    new_set->elements = temp_elements;

    new_set->elements[new_set->card] = (string)malloc((k + 1) * sizeof(char));
    if (new_set->elements[new_set->card] == NULL)
    {
      printf("Error allocating memory\n");
      free(aux);
      for (int m = 0; m < new_set->card; m++)
      {
        free(new_set->elements[m]);
      }
      free(new_set->elements);
      free(new_set);
      return NULL;
    }
    copy_str(new_set->elements[new_set->card], aux);
    new_set->card++;
  }

  free(aux);
  return new_set;
}

// Reads the initial state
set *get_initial(FILE *file)
{
  string line = read_line(file);
  if (line == NULL)
  {
    printf("Error reading initial state\n");
    exit(1);
  }

  set *initial_state = get_set(line, 1);
  free(line);

  if (initial_state == NULL)
  {
    printf("Error allocating memory\n");
    exit(1);
  }

  if (initial_state->card != 1)
  {
    printf("Incorrect state format\n");
    for (int i = 0; i < initial_state->card; i++)
    {
      free(initial_state->elements[i]);
    }
    free(initial_state->elements);
    free(initial_state);
    exit(1);
  }
  return initial_state;
}

// Reads the transitions and returns afnd t. array (delta)
Delta *get_transitions(FILE *file)
{
  Delta *transitions = (Delta *)malloc(sizeof(Delta));
  if (transitions == NULL)
  {
    printf("Error allocating memory\n");
    return NULL;
  }
  transitions->size = 0;
  transitions->transitions = NULL;

  string line;
  while ((line = read_line(file)) != NULL && line[0] != '\0')
  {
    transition new_transition;
    char origin[50], destination[50];

    int parsed = sscanf(line, "%[^,],%c,%[^,]", origin, &new_transition.symbol,
                        destination);
    if (parsed == 3)
    {
      new_transition.origin =
          (string)malloc((str_length(origin) + 1) * sizeof(char));
      new_transition.destination =
          (string)malloc((str_length(destination) + 1) * sizeof(char));
      if (new_transition.origin == NULL || new_transition.destination == NULL)
      {
        printf("Error allocating memory\n");
        free(line);
        for (int i = 0; i < transitions->size; i++)
        {
          free(transitions->transitions[i].origin);
          free(transitions->transitions[i].destination);
        }
        free(transitions->transitions);
        free(transitions);
        exit(1);
      }
      copy_str(new_transition.origin, origin);
      copy_str(new_transition.destination, destination);

      transition *temp_trans =
          realloc(transitions->transitions,
                  (transitions->size + 1) * sizeof(transition));
      if (temp_trans == NULL)
      {
        printf("Error allocating memory\n");
        free(new_transition.origin);
        free(new_transition.destination);
        free(line);
        for (int i = 0; i < transitions->size; i++)
        {
          free(transitions->transitions[i].origin);
          free(transitions->transitions[i].destination);
        }
        free(transitions->transitions);
        free(transitions);
        exit(1);
      }
      transitions->transitions = temp_trans;
      transitions->transitions[transitions->size] = new_transition;
      transitions->size++;
    }
    else
    {
      printf("Invalid transition format at: %s\n line", line);
    }
    free(line);
  }
  return transitions;
}

// Print in the console the automaton info
void show_info(info_automaton *automaton)
{
  printf("States:\n");
  print_set(automaton->Q);

  printf("\nAlphabet:\n");
  print_set(automaton->Sigma);

  printf("\nInit State:\n");
  print_set(automaton->s);

  printf("\nFinal States:\n");
  print_set(automaton->F);

  printf("\nTransitions:\n");
  for (int i = 0; i < automaton->Delta->size; i++)
  {
    printf("(%s, %c) -> %s\n", automaton->Delta->transitions[i].origin,
           automaton->Delta->transitions[i].symbol,
           automaton->Delta->transitions[i].destination);
  }
}

// returns the processed afnd

info_automaton *get_AFND(FILE *file)
{
  info_automaton *AFND = (info_automaton *)malloc(sizeof(info_automaton));
  if (AFND == NULL)
  {
    printf("Error allocating Memory\n");
    return NULL;
  }

  string q_line = read_line(file);
  if (q_line == NULL)
  {
    printf("Error reading Q\n");
    free(AFND);
    return NULL;
  }
  AFND->Q = get_set(q_line, 1);
  free(q_line);
  if (AFND->Q == NULL)
  {
    printf("Error allocating Memory\n");
    free(AFND);
    return NULL;
  }

  string sigma_line = read_line(file);
  if (sigma_line == NULL)
  {
    printf("Error reading alphabet\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);
    free(AFND);
    return NULL;
  }
  AFND->Sigma = get_set(sigma_line, 1);
  free(sigma_line);
  if (AFND->Sigma == NULL)
  {
    printf("Error allocating Memory\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);
    free(AFND);
    return NULL;
  }

  AFND->s = get_initial(file);
  if (AFND->s == NULL)
  {
    printf("Error allocating Memory\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);

    for (int i = 0; i < AFND->Sigma->card; i++)
    {
      free(AFND->Sigma->elements[i]);
    }
    free(AFND->Sigma->elements);
    free(AFND->Sigma);

    free(AFND);
    return NULL;
  }

  string f_line = read_line(file);
  if (f_line == NULL)
  {
    printf("Error reading final states F\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);

    for (int i = 0; i < AFND->Sigma->card; i++)
    {
      free(AFND->Sigma->elements[i]);
    }
    free(AFND->Sigma->elements);
    free(AFND->Sigma);

    for (int i = 0; i < AFND->s->card; i++)
    {
      free(AFND->s->elements[i]);
    }
    free(AFND->s->elements);
    free(AFND->s);

    free(AFND);
    return NULL;
  }
  AFND->F = get_set(f_line, 1);
  free(f_line);
  if (AFND->F == NULL)
  {
    printf("Error allocating Memory F\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);

    for (int i = 0; i < AFND->Sigma->card; i++)
    {
      free(AFND->Sigma->elements[i]);
    }
    free(AFND->Sigma->elements);
    free(AFND->Sigma);

    for (int i = 0; i < AFND->s->card; i++)
    {
      free(AFND->s->elements[i]);
    }
    free(AFND->s->elements);
    free(AFND->s);

    free(AFND);
    return NULL;
  }

  AFND->Delta = get_transitions(file);
  if (AFND->Delta == NULL)
  {
    printf("Error allocating Memory\n");
    for (int i = 0; i < AFND->Q->card; i++)
    {
      free(AFND->Q->elements[i]);
    }
    free(AFND->Q->elements);
    free(AFND->Q);

    for (int i = 0; i < AFND->Sigma->card; i++)
    {
      free(AFND->Sigma->elements[i]);
    }
    free(AFND->Sigma->elements);
    free(AFND->Sigma);

    for (int i = 0; i < AFND->s->card; i++)
    {
      free(AFND->s->elements[i]);
    }
    free(AFND->s->elements);
    free(AFND->s);

    for (int i = 0; i < AFND->F->card; i++)
    {
      free(AFND->F->elements[i]);
    }
    free(AFND->F->elements);
    free(AFND->F);

    free(AFND);
    return NULL;
  }

  return AFND;
}

// checks that a state is final
int is_final_st(string state, info_automaton *automata)
{
  for (int i = 0; i < automata->F->card; i++)
  {
    if (str_comp(automata->F->elements[i], state) == 0)
    {
      return 1;
    }
  }
  return 0;
}

// push a path to the path list
void push_path(path_list *plist, set *new_path)
{
  set *temp = realloc(plist->elements, sizeof(set) * (plist->card + 1));
  if (!temp)
  {
    printf("Error, memory allocation failed \n");
    exit(EXIT_FAILURE);
  }
  plist->elements = temp;
  plist->elements[plist->card] = *new_path;
  plist->card++;
}

// prints the whole list
void print_path_list(const path_list *plist)
{
  for (int i = 0; i < plist->card; i++)
  {
    printf("Path %d:\n", i + 1);
    print_path(&plist->elements[i]);
  }
}

// Recursive function to process a string in the automata with epsilon transitions
void process_string(string current_state, string input_string, set *current_path, path_list *path_list, info_automaton *automata)
{
  // Allocate memory for the new element in the path
  current_path->elements = realloc(current_path->elements, sizeof(string) * (current_path->card + 1));
  if (!current_path->elements)
  {
    printf("Memory allocation failed\n");
    exit(1);
  }
  current_path->elements[current_path->card] = malloc(str_length(current_state) + 1); // Allocate memory for the string
  if (!current_path->elements[current_path->card])
  {
    printf("Memory allocation failed\n");
    exit(1);
  }
  copy_str(current_path->elements[current_path->card], current_state);
  current_path->card++;

  if (*input_string == '\0')
  {
    if (is_final_st(current_state, automata))
    {
      push_path(path_list, current_path);
    }
  }

  char symbol = *input_string;

  for (int i = 0; i < automata->Delta->size; i++)
  {
    if (str_comp(automata->Delta->transitions[i].origin, current_state) == 0)
    {

      if (automata->Delta->transitions[i].symbol == 'E')
      {
        set new_path;
        new_path.card = current_path->card;
        new_path.elements = malloc(sizeof(string) * new_path.card);
        if (!new_path.elements)
        {
          printf("Memory allocation failed\n");
          exit(1);
        }

        for (int j = 0; j < current_path->card; j++)
        {
          new_path.elements[j] = malloc(str_length(current_path->elements[j]) + 1);
          if (!new_path.elements[j])
          {
            printf("Memory allocation failed\n");
            exit(1);
          }
          copy_str(new_path.elements[j], current_path->elements[j]);
        }

        process_string(automata->Delta->transitions[i].destination, input_string, &new_path, path_list, automata);
      }
      if (automata->Delta->transitions[i].symbol == symbol)
      {
        set new_path;
        new_path.card = current_path->card;
        new_path.elements = malloc(sizeof(string) * new_path.card);
        if (!new_path.elements)
        {
          printf("Memory allocation failed\n");
          exit(1);
        }
        for (int j = 0; j < current_path->card; j++)
        {
          new_path.elements[j] = malloc(str_length(current_path->elements[j]) + 1);
          if (!new_path.elements[j])
          {
            printf("Memory allocation failed\n");
            exit(1);
          }
          copy_str(new_path.elements[j], current_path->elements[j]);
        }
        process_string(automata->Delta->transitions[i].destination, input_string + 1, &new_path, path_list, automata);
      }
    }
  }
}

void string_validation(info_automaton *afnd)
{

  /* String and path processing */
  printf("///////////////////////   STRING VALIDATION - (NFA-ε)   ////////////////////////////////////\n");
  printf("Enter the string: \n");
  printf("=> ");
  string test_string = (string)malloc(1000);
  scanf("%[^\n]", test_string);
  getchar();

  path_list paths;
  paths.elements = NULL;
  paths.card = 0;

  set initial_path;
  initial_path.elements = NULL;
  initial_path.card = 0;

  process_string(afnd->s->elements[0], test_string, &initial_path, &paths, afnd);

  printf("Paths for string '%s':\n\n", test_string);
  if (paths.card == 0)
  {
    printf("Result: Invalid String, No paths found.\n");
  }
  else
  {
    printf("Result: Valid String\n");
    print_path_list(&paths);
  }

  // Free allocated memory
  for (int i = 0; i < paths.card; i++)
  {
    for (int j = 0; j < paths.elements[i].card; j++)
    {
      free(paths.elements[i].elements[j]);
    }
    free(paths.elements[i].elements);
  }
  free(paths.elements);
}