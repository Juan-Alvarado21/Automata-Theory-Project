
#include <stdio.h>
#include <stdlib.h>
#include "automaton.h"

// struct to map a set
typedef struct
{
    char letter;
    set set;
    transition *transition;
    int cardinality;
} mapping;

// Definition of global lists for destinations
set *destination_lists; // An array of destination lists, one for each symbol of the alphabet

void set_append(set *set, string element)
{
    // Memory allocation for additional space to allocate a new string pointer
    string *new_elements = realloc(set->elements, (set->card + 1) * sizeof(string));
    if (new_elements == NULL)
    {
        printf("Error allocating memory\n");
        return;
    }
    set->elements = new_elements;

    // Allocates memory for the new string
    set->elements[set->card] = malloc((str_length(element) + 1) * sizeof(char));
    if (set->elements[set->card] == NULL)
    {
        printf("Error allocating memory for string\n");
        return;
    }
    // Copy the content of 'element' to the reserved space
    for (int i = 0; i < str_length(element); i++)
    {
        set->elements[set->card][i] = element[i];
    }
    set->elements[set->card][str_length(element)] = '\0';

    set->card++;
}

set *get_Target(info_automaton *afnd)
{
    int n = afnd->Sigma->card - 1; // subtract 1 to skip transitions with epsilon

    // Create an array of target-sets, one for each symbol of the alphabet
    set *targets = (set *)malloc(n * sizeof(set));

    // Create an array of sets for the destinations
    destination_lists = (set *)malloc(n * sizeof(set));

    // Initialize each set in the destinations and targets array
    for (int i = 0; i < n; i++)
    {
        targets[i].elements = (string *)malloc(afnd->Delta->size * sizeof(string));
        targets[i].card = 0;

        destination_lists[i].elements = (string *)malloc(afnd->Delta->size * sizeof(string));
        destination_lists[i].card = 0;
    }

    // Temporary buffer to treat individual symbols as strings
    char symbol_buffer[10];
    symbol_buffer[1] = '\0';

    // Iterate over alphabet symbols
    for (int i = 0; i < n; i++)
    {
        string current_symbol = afnd->Sigma->elements[i];

        // Iteration over the automaton transitions
        for (int j = 0; j < afnd->Delta->size; j++)
        {
            transition current_transition = afnd->Delta->transitions[j];

            // get the current transition symbol
            symbol_buffer[0] = current_transition.symbol;
            // If the transition symbol matches the current symbol
            if (str_comp(symbol_buffer, current_symbol) == 0)
            {
                // Check if the source state is already in the set
                if (!check_unicity(&targets[i], current_transition.origin))
                {
                    // If it does not exist, add it to the corresponding set
                    set_append(&targets[i], current_transition.origin);
                }

                // Add the destination to the corresponding list
                if (!check_unicity(&destination_lists[i], current_transition.destination))
                {
                    set_append(&destination_lists[i], current_transition.destination);
                }
            }
        }
    }

    targets->card = n;
    return targets; // Return the array of sets
}

// prints the target array
void print_Targets(set *targets)
{
    for (int i = 0; i < targets->card; i++)
    {
        print_set(&targets[i]);
    }
}

// Function to map the subset index to a letter
char get_letter(int index)
{
    return 'A' + index;
}

// Function to compare if two sets are equal
int sets_are_equal(set *set1, set *set2)
{
    if (set1->card != set2->card)
        return 0;

    for (int i = 0; i < set1->card; i++)
    {
        if (!check_unicity(set2, set1->elements[i]))
            return 0;
    }

    return 1;
}

// Function to create a copy of a set
set create_copy_of_set(set *original)
{
    set copy;
    copy.card = original->card;
    copy.elements = malloc(sizeof(string) * copy.card);
    if (copy.elements == NULL)
    {
        printf("Error allocating memory for set elements\n");
        copy.card = 0;
        return copy;
    }
    for (int i = 0; i < copy.card; i++)
    {
        int len = str_length(original->elements[i]);
        copy.elements[i] = malloc((len + 1) * sizeof(char));
        if (copy.elements[i] == NULL)
        {
            printf("Error allocating memory for string\n");
            // free allocated memory
            for (int j = 0; j < i; j++)
            {
                free(copy.elements[j]);
            }
            free(copy.elements);
            copy.card = 0;
            return copy;
        }
        // Copy the string one by one
        for (int j = 0; j < len; j++)
        {
            copy.elements[i][j] = original->elements[i][j];
        }
        copy.elements[i][len] = '\0';
    }
    return copy;
}

void epsilon_closure(mapping *map, set *current, info_automaton *afnd, int sigma_index, int map_index)
{
    int processed_index = 0;
    int total_mappings = map[0].cardinality; // cardinality of mappings

    // it Applies epsilon closure
    while (processed_index < current->card)
    {
        string current_state = current->elements[processed_index];

        for (int j = 0; j < afnd->Delta->size; j++)
        {
            transition t = afnd->Delta->transitions[j];

            // It checks epsilon transitions
            if (str_comp(t.origin, current_state) == 0 && t.symbol == 'E')
            {
                if (!check_unicity(current, t.destination))
                {
                    set_append(current, t.destination);
                }
            }
        }

        processed_index++;
    }

    // If the epsilon closure was empty, return
    if (current->card == 0)
    {
        return;
    }

    // If both indices are passed as -1, we do not create transitions
    if (sigma_index == -1 && map_index == -1)
    {
        // We check if the set 'current' already exists in 'map'
        int exists = 0;
        int index_in_map = -1;

        for (int i = 0; i < total_mappings; i++)
        {
            if (sets_are_equal(&map[i].set, current))
            {
                exists = 1;
                index_in_map = i;
                break;
            }
        }

        // If it does not exist, we add it but without creating transitions
        if (!exists)
        {
            mapping *temp = (mapping *)realloc(map, (total_mappings + 1) * sizeof(mapping));
            if (!temp)
            {
                perror("Error allocating memory ");
                exit(EXIT_FAILURE);
            }
            map = temp;

            char subset_letter = get_letter(total_mappings);

            map[total_mappings].letter = subset_letter;
            map[total_mappings].set = *current;

            // Memory allocation for the new mapping
            map[total_mappings].transition = (transition *)malloc((afnd->Sigma->card - 1) * sizeof(transition));
            if (!map[total_mappings].transition)
            {
                perror("Error allocating memory");
                exit(EXIT_FAILURE);
            }

            // We initialize the transitions
            for (int k = 0; k < afnd->Sigma->card - 1; k++)
            {
                map[total_mappings].transition[k].origin = NULL;
                map[total_mappings].transition[k].destination = NULL;
                map[total_mappings].transition[k].symbol = '\0';
            }

            map[0].cardinality = total_mappings + 1;
            total_mappings++;
        }

        return; // We do not create transitions because indices -1, -1 were requested
    }

    // We check if the set 'current' already exists in 'map'
    int exists = 0;
    int index_in_map = -1;

    for (int i = 0; i < total_mappings; i++)
    {
        if (sets_are_equal(&map[i].set, current))
        {
            exists = 1;
            index_in_map = i;
            break;
        }
    }

    // If it does not exist, we add it
    if (!exists)
    {
        mapping *temp = (mapping *)realloc(map, (total_mappings + 1) * sizeof(mapping));
        if (!temp)
        {
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        map = temp;

        char subset_letter = get_letter(total_mappings);

        map[total_mappings].letter = subset_letter;
        map[total_mappings].set = *current;

        // We reserve memory for the arrangement of transitions
        map[total_mappings].transition = (transition *)malloc((afnd->Sigma->card - 1) * sizeof(transition));
        if (!map[total_mappings].transition)
        {
            perror("Error al reservar memoria para las transiciones");
            exit(EXIT_FAILURE);
        }

        // We initialize the transitions
        for (int k = 0; k < afnd->Sigma->card - 1; k++)
        {
            map[total_mappings].transition[k].origin = NULL;
            map[total_mappings].transition[k].destination = NULL;
            map[total_mappings].transition[k].symbol = '\0';
        }

        map[0].cardinality = total_mappings + 1;
        index_in_map = total_mappings;
        total_mappings++;
    }

    // Create the transition with origin and symbol always
    map[map_index].transition[sigma_index].origin = (char *)malloc(2);
    map[map_index].transition[sigma_index].origin[0] = map[map_index].letter;
    map[map_index].transition[sigma_index].origin[1] = '\0';

    map[map_index].transition[sigma_index].symbol = afnd->Sigma->elements[sigma_index][0];

    // If there is no destination, " " (a space) is assigned
    if (index_in_map == -1)
    {
        map[map_index].transition[sigma_index].destination = (char *)malloc(2);
        map[map_index].transition[sigma_index].destination[0] = ' ';
        map[map_index].transition[sigma_index].destination[1] = '\0';
    }
    else
    {
        map[map_index].transition[sigma_index].destination = (char *)malloc(2);
        map[map_index].transition[sigma_index].destination[0] = map[index_in_map].letter;
        map[map_index].transition[sigma_index].destination[1] = '\0';
    }
}

// Function to calculate the intersection of two sets
set *intersection(set *a, set *b)
{
    int max_size = a->card < b->card ? a->card : b->card;
    set *intersection = (set *)malloc(sizeof(set));
    if (intersection == NULL)
    {
        return NULL;
    }
    intersection->elements = (string *)malloc(max_size * sizeof(string));
    if (intersection->elements == NULL)
    {
        free(intersection);
        return NULL;
    }
    intersection->card = 0;

    // Compare both sets
    for (int i = 0; i < a->card; i++)
    {
        for (int j = 0; j < b->card; j++)
        {
            if (str_comp(a->elements[i], b->elements[j]) == 0)
            {
                // Add the matching element to the intersection set
                intersection->elements[intersection->card] = str_dup(a->elements[i]);
                intersection->card++;
                break;
            }
        }
    }

    return intersection;
}

// Function to calculate the union of two sets
set *union_set(set *a, set *b)
{
    int max_size = a->card + b->card;
    set *union_set = (set *)malloc(sizeof(set));
    if (union_set == NULL)
    {
        return NULL;
    }
    union_set->elements = (string *)malloc(max_size * sizeof(string));
    if (union_set->elements == NULL)
    {
        free(union_set);
        return NULL;
    }
    union_set->card = 0;

    for (int i = 0; i < a->card; i++)
    {
        union_set->elements[union_set->card] = str_dup(a->elements[i]);
        if (union_set->elements[union_set->card] == NULL)
        {
            for (int k = 0; k < union_set->card; k++)
            {
                free(union_set->elements[k]);
            }
            free(union_set->elements);
            free(union_set);
            return NULL;
        }
        union_set->card++;
    }

    for (int i = 0; i < b->card; i++)
    {
        int exists = 0;
        // Check if the element already exists in the union
        for (int j = 0; j < union_set->card; j++)
        {
            if (str_comp(b->elements[i], union_set->elements[j]) == 0)
            {
                exists = 1;
                break;
            }
        }
        // If it does not exist, add it to the union set
        if (!exists)
        {
            union_set->elements[union_set->card] = str_dup(b->elements[i]);
            if (union_set->elements[union_set->card] == NULL)
            {
                for (int k = 0; k < union_set->card; k++)
                {
                    free(union_set->elements[k]);
                }
                free(union_set->elements);
                free(union_set);
                return NULL;
            }
            union_set->card++;
        }
    }

    return union_set;
}

// Check if a given state is final
int is_final_state(mapping *map, info_automaton *afnd)
{
    for (int i = 0; i < map->set.card; i++)
    {
        // Check if the current state is in afnd->F (final states)
        for (int j = 0; j < afnd->F->card; j++)
        {
            // Compare the map state with the final state in afnd->F
            if (str_comp(map->set.elements[i], afnd->F->elements[j]) == 0)
            {
                return 1; // Founded
            }
        }
    }
    return 0; // No mapping state is final
}

// prints the whole mapping & converted DFA info into the output file
void print_map(FILE *fp, mapping *map, int n, info_automaton *afnd)
{
    if (n <= 0)
    {
        return; // No mappings yet
    }

    // 1. Print the letters of each mapping separated by commas (first line)
    for (int i = 0; i < n; i++)
    {
        fprintf(fp, "%c", map[i].letter);
        if (i < n - 1)
            fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    // 2. Print the automaton alphabet (except epsilon) separated by commas (second line)
    for (int i = 0; i < afnd->Sigma->card - 1; i++)
    {
        fprintf(fp, "%c", afnd->Sigma->elements[i][0]);
        if (i < afnd->Sigma->card - 2)
            fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    // 3. Print initial state (third line)
    fprintf(fp, "%c", map[0].letter);
    fprintf(fp, "\n");

    // 4. Print the letters of the final states separated by commas (fourth line)

    int first_final = 1;
    for (int i = 0; i < n; i++)
    {
        if (is_final_state(&map[i], afnd))
        {
            if (!first_final)
                fprintf(fp, ",");
            fprintf(fp, "%c", map[i].letter);
            first_final = 0;
        }
    }
    fprintf(fp, "\n");

    // 5. Print the transitions for each mapping
    for (int i = 0; i < n; i++)
    {
        for (int t = 0; t < afnd->Sigma->card - 1; t++)
        {
            const char *dest_str = (map[i].transition[t].destination == NULL) ? " " : map[i].transition[t].destination;
            fprintf(fp, "%c,%c,%s\n", map[i].letter, afnd->Sigma->elements[t][0], dest_str);
        }

        // Print the set in terminal and check if it is final
        printf("%c={", map[i].letter);
        print_set(&map[i].set);
        printf("}");

        if (is_final_state(&map[i], afnd))
        {
            printf(" (final)");
        }

        printf("\n");
    }
}

// Push mapping & new set to the array of mappings (DFA states)
void push(mapping **map, int *n, char letter, set new_set)
{
    *map = (mapping *)realloc(*map, sizeof(mapping) * (*n + 1));
    if (*map == NULL)
    {
        printf("Error en realloc, memoria insuficiente.\n");
        exit(1);
    }

    // We insert the new mapping at the end of the array.
    (*map)[*n].letter = letter;
    (*map)[*n].set = new_set;
    (*map)[*n].cardinality = new_set.card;

    (*n)++;
}

// Function to obtain the transition relationship between sets with common targets and their destinations
set *get_destinations(info_automaton *afnd, set *Targets)
{
    int n = afnd->Sigma->card - 1; // subtract 1 to skip transitions with epsilon

    set *destinations = (set *)malloc(sizeof(set));
    destinations->elements = NULL;
    destinations->card = 0;

    char symbol_buffer[10];
    symbol_buffer[1] = '\0';

    // Iterate over the states in Targets (source states)
    for (int i = 0; i < Targets->card; i++)
    {
        string estado_origen = Targets->elements[i];

        // Iterate over alphabet symbols (Sigma)
        for (int j = 0; j < n; j++) // Iterate over the alphabet
        {
            string current_symbol = afnd->Sigma->elements[j];

            // Iterate over transitions
            for (int k = 0; k < afnd->Delta->size; k++)
            {
                transition current_transition = afnd->Delta->transitions[k];

                // Convert the transition symbol to a string
                symbol_buffer[0] = current_transition.symbol;

                // If we find a transition with the current symbol
                if (str_comp(symbol_buffer, current_symbol) == 0)
                {
                    // Check if the current source state is in the transition
                    if (str_comp(current_transition.origin, estado_origen) == 0)
                    {
                        // Add the destination to the destination list
                        if (!check_unicity(destinations, current_transition.destination))
                        {
                            set_append(destinations, current_transition.destination);
                        }
                    }
                }
            }
        }
    }

    return destinations;
}

// Function to apply a movement (returns a set of destinations, which are obtained from the intersection of targets with states of a set)
set *move_to(set *target, set *actual, info_automaton *afnd)
{
    set *new = get_destinations(afnd, intersection(target, actual));
    return new;
}

// applies epsilon closure, ask for the alphabet index, asks for the index of mapped set
void go_to(mapping *map, set *target, info_automaton *afnd, int sigma_index, int map_index)
{
    epsilon_closure(map, move_to(&target[sigma_index], &map[map_index].set, afnd), afnd, sigma_index, map_index);
}

// push and returns the first state of the automata
set *push_fstate(info_automaton *afnd)
{

    set *current = (set *)malloc(sizeof(set));
    if (current == NULL)
    {
        printf("Error allocating memory for set\n");
        exit(-1);
    }
    current->elements = (string *)malloc(sizeof(string) * 1);
    if (current->elements == NULL)
    {
        printf("Error allocating memory for set elements\n");
        exit(-1);
    }
    current->elements[0] = afnd->Q->elements[0];
    current->card = 1;

    return current;
}

void thompson(info_automaton *afnd, FILE *file)
{
    printf("///////////////////////   THOMPSON CONSTRUCTION - (NFA-DFA)   ////////////////////////////////////\n");
    /* Algorithm main variables */
    set *Targets = get_Target(afnd);          // set of target states
    set *current = push_fstate(afnd);         // Initialization of the initial state
    int sigma_size = (afnd->Sigma->card) - 1; // Cardinality of the alphabet without epsilon

    // Initialization of the mapping set
    mapping *map = (mapping *)malloc(sizeof(mapping) * 1000);
    epsilon_closure(map, current, afnd, -1, -1);

    // Now a loop will be made that repeats as long as new mappings appear
    int previous_cardinality = 0;
    int current_cardinality = map->cardinality;

    // While new states are discovered
    while (current_cardinality > previous_cardinality)
    {
        previous_cardinality = current_cardinality;

        // Loop through all currently known mappings
        for (int i = 0; i < current_cardinality; i++)
        {
            for (int j = 0; j < sigma_size; j++) // it explores every possibility in the alphabet
            {
                go_to(map, Targets, afnd, j, i);
            }
        }

        // Update the mapping count after this lap
        current_cardinality = map->cardinality;
    }

    // Once no new mappings appear, we print the result
    print_map(file, map, map->cardinality, afnd);
    // free(map);
}
