#include "thompson.h"

int count_bits(int n) // acum. Least significant bit (cardinality of subsets)
{
    int count = 0;
    while (n)
    {
        count += n & 1; // Add 1 if the last bit is 1
        n >>= 1;        // right bit shift
    }
    return count;
}

// Function to mapping subset indices to letters
char *label_subsets(int combinations, int q_card, info_automaton *afnd)
{
    char *subset_labels = (char *)calloc(combinations, sizeof(char));
    if (subset_labels == NULL)
    {
        printf("Error allocating memory\n");
        return NULL;
    }
    int label_index = 0;
    for (int i = 1; i < combinations; i++)
    {
        if (count_bits(i) > 1)
        { // Only assign letters to subsets with more than one element
            subset_labels[i] = get_letter(label_index++);

            // Print the mapping to the console in the specified format
            int first = 1;
            printf("{");
            for (int j = 0; j < q_card; j++)
            {
                if (i & (1 << j)) // if j is in i
                {
                    if (!first)
                        printf(",");
                    printf("%s", afnd->Q->elements[j]);
                    first = 0;
                }
            }
            printf("} = %c\n", subset_labels[i]); // Output the mapping
        }
    }
    return subset_labels;
}

int *final_indexed(int combinations, info_automaton *afnd)
{
    int *subset_is_final = (int *)calloc(combinations, sizeof(int));
    int total_subsets = 1 << afnd->Q->card;

    if (subset_is_final == NULL)
    {
        printf("Error allocating memory\n");
        return NULL;
    }

    // Locate final states
    for (int i = 0; i < total_subsets; i++)
    {
        for (int j = 0; j < afnd->Q->card; j++)
        {
            if (i & (1 << j)) // left shifting to comparing
            {
                for (int k = 0; k < afnd->F->card; k++)
                {
                    if (str_comp(afnd->Q->elements[j], afnd->F->elements[k]) == 0)
                    {
                        subset_is_final[i] = 1;
                        break;
                    }
                }
            }
        }
    }
    return subset_is_final;
}

void show_Q(info_automaton *afnd, FILE *output, int combinations, char *subset_labels)
{
    for (int i = 1; i < combinations; i++)
    {
        if (subset_labels[i] != 0)
        {
            fprintf(output, "%c,", subset_labels[i]); // prints composed states
        }
        else
        {
            int first = 1;
            for (int j = 0; j < afnd->Q->card; j++)
            {
                if (i & (1 << j))
                {
                    if (!first)
                        fprintf(output, ",");
                    fprintf(output, "%s", afnd->Q->elements[j]); // prints atomic states
                    first = 0;
                }
            }
            fprintf(output, ",");
        }
    }
    fprintf(output, "\n");
}

void show_sigma(info_automaton *afnd, FILE *output)
{
    for (int i = 0; i < afnd->Sigma->card; i++)
    {
        fprintf(output, "%s,", afnd->Sigma->elements[i]);
    }
    fprintf(output, "\n");
}

void show_s(info_automaton *afnd, FILE *output)
{
    fprintf(output, "%s\n", afnd->s->elements[0]);
}

void show_F(info_automaton *afnd, FILE *output, int combinations, int *subset_is_final, char *subset_labels)
{
    int first_final = 1;
    for (int i = 1; i < combinations; i++)
    {
        if (subset_is_final[i])
        {
            if (!first_final)
                fprintf(output, ",");
            if (subset_labels[i] != 0)
            {
                fprintf(output, "%c", subset_labels[i]);
            }
            else
            {
                int first_state = 1;
                for (int j = 0; j < afnd->Q->card; j++)
                {
                    if (i & (1 << j))
                    {
                        if (!first_state)
                            fprintf(output, ",");
                        fprintf(output, "%s", afnd->Q->elements[j]);
                        first_state = 0;
                    }
                }
            }
            first_final = 0;
        }
    }
    fprintf(output, "\n");
}

void show_delta(info_automaton *afnd, FILE *output, int total_subsets, char *subset_labels)
{
    for (int i = 1; i < total_subsets; i++)
    {
        for (int s = 0; s < afnd->Sigma->card; s++)
        {
            char symbol = afnd->Sigma->elements[s][0]; // bcs elements is a string
            int destination_subset = 0;

            for (int j = 0; j < afnd->Q->card; j++)
            {
                if (i & (1 << j))
                {
                    for (int t = 0; t < afnd->Delta->size; t++)
                    {
                        if (str_comp(afnd->Delta->transitions[t].origin, afnd->Q->elements[j]) == 0 &&
                            afnd->Delta->transitions[t].symbol == symbol)
                        {
                            for (int d = 0; d < afnd->Q->card; d++)
                            {
                                if (str_comp(afnd->Delta->transitions[t].destination, afnd->Q->elements[d]) == 0)
                                {
                                    destination_subset |= (1 << d);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (destination_subset != 0)
            {
                if (subset_labels[i] != 0)
                {
                    fprintf(output, "%c", subset_labels[i]);
                }
                else
                {
                    int first = 1;
                    for (int j = 0; j < afnd->Q->card; j++)
                    {
                        if (i & (1 << j))
                        {
                            if (!first)
                                fprintf(output, ",");
                            fprintf(output, "%s", afnd->Q->elements[j]);
                            first = 0;
                        }
                    }
                }
                fprintf(output, ",%c,", symbol);

                if (subset_labels[destination_subset] != 0)
                {
                    fprintf(output, "%c", subset_labels[destination_subset]);
                }
                else
                {
                    int first_dest = 1;
                    for (int j = 0; j < afnd->Q->card; j++)
                    {
                        if (destination_subset & (1 << j))
                        {
                            if (!first_dest)
                                fprintf(output, ",");
                            fprintf(output, "%s", afnd->Q->elements[j]);
                            first_dest = 0;
                        }
                    }
                }
                fprintf(output, "\n");
            }
        }
    }
}

void convert_to_AFD(info_automaton *afnd, FILE *output)
{
    int total_subsets = 1 << afnd->Q->card;
    int *subset_is_final = final_indexed(total_subsets, afnd);
    char *subset_labels = label_subsets(total_subsets, afnd->Q->card, afnd);

    show_Q(afnd, output, total_subsets, subset_labels);
    show_sigma(afnd, output);
    show_s(afnd, output);
    show_F(afnd, output, total_subsets, subset_is_final, subset_labels);
    show_delta(afnd, output, total_subsets, subset_labels);

    free(subset_is_final);
    free(subset_labels);
}

/*

Author: Juan Manuel Alvarado Sandoval
Subject: Automata theory & Formal Languages
                                   ^
Past assignment: Practica 4        |

NFA to DFA by subset algorithm

///////////////////////////////////////////

Current assignment: Practica 5     |
                                   v

- NFA to DFA using Thompson's Construction
- String-(AFN-ε) validation
*/

int main()
{
    FILE *input = fopen("AFND.txt", "r");
    if (input == NULL)
    {
        printf("Input file couldn't be opened\n");
        return 1;
    }

    info_automaton *afnd = get_AFND(input);
    if (afnd == NULL)
    {
        printf("Error reading AFND file\n");
        fclose(input);
        return 1;
    }
    fclose(input);

    FILE *output = fopen("afd.txt", "w");
    if (output == NULL)
    {
        printf("Output file couldn't be opened\n");
        return 1;
    }
    thompson(afnd, output);
    string_validation(afnd);
    fclose(output);
    printf("\nConversion from AFND to AFD was successful in afd.txt\n");

    return 0;
}
