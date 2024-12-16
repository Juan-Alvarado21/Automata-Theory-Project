#include <stdio.h>
#include <stdlib.h>

typedef char *string;

// Function to get the length of the string
int str_length(const char *str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

// Function to compare two strings
int str_comp(const char *str1, const char *str2)
{
    int i = 0;
    while (str1[i] == str2[i] && str1[i] != '\0' && str2[i] != '\0')
    {
        i++;
    }
    return str1[i] - str2[i];
}

// Function to compare two strings
int str_val(char *str1, char *str2)
{
    int i = 0;

    if (str1[str_length(str1)] != '\0')
    {
        str1[str_length(str1)] = '\0';
    }
    if (str2[str_length(str2)] != '\0')
    {
        str2[str_length(str2)] = '\0';
    }

    // Compare characters one by one until you find a difference or the end of a string
    while (str1[i] == str2[i] && str1[i] != '\0' && str2[i] != '\0')
    {
        i++;
    }

    // If we reach the end of both strings without differences, then they are equal
    if (str1[i] == '\0' && str2[i] == '\0')
    {
        return 0; // The strings are equals
    }

    // Else return index difference s
    return str1[i] - str2[i];
}

// Function to copy one string to another
void copy_str(string dest, string src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Implementation of strdup (duplicate string)
string str_dup(const char *str)
{
    int len = str_length(str);
    string new_str = (string)malloc(len + 1); // +1 para el carácter '\0'
    if (new_str != NULL)
    {
        for (int i = 0; i <= len; i++) // Incluye el '\0'
        {
            new_str[i] = str[i];
        }
    }
    return new_str;
}

// Implementation of strchr (search for a character in a string)
const char *str_chr(const char *str, char c)
{
    while (*str != '\0')
    {
        if (*str == c)
        {
            return str;
        }
        str++;
    }
    return NULL;
}

// Implementation of strtok (tokenize string)
string str_tok(string str, const char *delim)
{
    static string last = NULL;
    if (str != NULL)
    {
        last = str;
    }

    if (last == NULL)
    {
        return NULL;
    }

    // Find first token
    int i = 0;
    while (last[i] != '\0' && str_chr(delim, last[i]) == NULL) // Mientras no sea delimitador
    {
        i++;
    }

    if (last[i] == '\0')
    {
        last = NULL;
        return NULL;
    }

    last[i] = '\0'; // Ends the first token

    string token = last;
    last = last + i + 1;

    return token;
}

// Implementation of strcat (concatenate strings)
void str_cat(string dest, const char *src)
{
    int dest_len = str_length(dest);
    int src_len = str_length(src);
    int i;

    for (i = 0; i <= src_len; i++)
    {
        dest[dest_len + i] = src[i];
    }
}
