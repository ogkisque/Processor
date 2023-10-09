#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"

enum Errors
{
    CORRECT =      -1,
    OPEN_FILE_ERR = 1,
    SYNTAX_ERR    = 2,
    MEM_ALLOC_ERR = 3
};

const int REALLOC_STEP = 10;
const char* FILE_NAME_READ_DEF = "asm.txt";
const char* FILE_NAME_PRINT_DEF = "byte_code.txt";

Errors get_commands_arr (const char* name_file_read, int** commands_int, int* num_com);
Errors print_commands_arr (const char* name_file_print, int* commands_int, int num_commands);
void print_error (Errors error);
void del_comment (char* str);
void del_slash_n (char* str);

int main (int argc, char* argv[])
{
    char file_name_read[MAX_NAME_LEN] = "";
    char file_name_print[MAX_NAME_LEN] = "";
    if (argc < 3)
    {
        strcpy (file_name_read, FILE_NAME_READ_DEF);
        strcpy (file_name_print, FILE_NAME_PRINT_DEF);
    }
    else
    {
        strcpy (file_name_read, argv[1]);
        strcpy (file_name_print, argv[2]);
    }

    int* commands_int;
    int num_commands = 0;
    Errors error = get_commands_arr (file_name_read, &commands_int, &num_commands);
    if (error != CORRECT)
    {
        print_error (error);
        return 1;
    }

    error = print_commands_arr (file_name_print, commands_int, num_commands);
    if (error != CORRECT)
    {
        print_error (error);
        return 1;
    }
    return 0;
}

void print_error (Errors error)
{
    switch (error)
    {
    case OPEN_FILE_ERR:
        printf ("Error is in opening file\n");
        break;
    case SYNTAX_ERR:
        printf ("Error is in syntax\n");
        break;
    case MEM_ALLOC_ERR:
        printf ("Error is in allocation of memory\n");
        break;
    case CORRECT:
        printf ("Correct compilation\n");
        break;
    default:
        break;
    }
}

Errors get_commands_arr (const char* name_file_read, int** commands_int, int* num_com)
{
    FILE* file_read = fopen (name_file_read, "r");
    if (!file_read)
        return OPEN_FILE_ERR;

    char command[MAX_COMMAND_LEN] = "";
    char str[MAX_COMMAND_LEN] = "";
    double number = 0.0;
    int num_commands = 0;
    int size_arr = REALLOC_STEP;
    (*commands_int) = (int*) calloc (REALLOC_STEP, sizeof (int));
    if (!(*commands_int))
        return MEM_ALLOC_ERR;

    while (fgets (str, MAX_COMMAND_LEN, file_read))
    {
        del_comment (str);
        del_slash_n (str);
        num_commands += 2;
        if (num_commands == size_arr)
        {
            size_arr += REALLOC_STEP;
            (*commands_int) = (int*) realloc ((*commands_int), size_arr * sizeof (int));
            if (!(*commands_int))
                return MEM_ALLOC_ERR;
        }

        if (strcmp (str, "hlt") == 0)
        {
            (*commands_int)[num_commands - 2] = HLT;
            num_commands--;
        }
        else if (strcmp (str, "out") == 0)
        {
            (*commands_int)[num_commands - 2] = OUT;
            num_commands--;
        }
        else if (strcmp (str, "in") == 0)
        {
            (*commands_int)[num_commands - 2] = IN;
            num_commands--;
        }
        else if (strcmp (str, "div") == 0)
        {
            (*commands_int)[num_commands - 2] = DIV;
            num_commands--;
        }
        else if (strcmp (str, "sub") == 0)
        {
            (*commands_int)[num_commands - 2] = SUB;
            num_commands--;
        }
        else if (strcmp (str, "mul") == 0)
        {
            (*commands_int)[num_commands - 2] = MUL;
            num_commands--;
        }
        else if (strcmp (str, "add") == 0)
        {
            (*commands_int)[num_commands - 2] = ADD;
            num_commands--;
        }
        else if (strcmp (str, "sqrt") == 0)
        {
            (*commands_int)[num_commands - 2] = SQRT;
            num_commands--;
        }
        else if (strcmp (str, "sin") == 0)
        {
            (*commands_int)[num_commands - 2] = SIN;
            num_commands--;
        }
        else if (strcmp (str, "cos") == 0)
        {
            (*commands_int)[num_commands - 2] = COS;
            num_commands--;
        }
        else if (strncmp (str, "push", 4) == 0)
        {
            (*commands_int)[num_commands - 2] = 0;
            (*commands_int)[num_commands - 1] = 0;
            (*commands_int)[num_commands - 2] |= PUSH;
            if (sscanf (str + 5, "%s", command) != 1)
                return SYNTAX_ERR;

            if ((strlen (command) == 3) &&
                (command[0] == 'r' && command[2] == 'x' && command[1] >= 'a' && command[1] <= 'd'))
            {
                (*commands_int)[num_commands - 2] |= BIT_REGISTER;
                (*commands_int)[num_commands - 1] = (int) (command[1] - 'a' + 1);
            }
            else if (sscanf (command, "%lf", &number) == 1)
            {
                (*commands_int)[num_commands - 2] |= BIT_IMM_CONST;
                (*commands_int)[num_commands - 1] = (int) (number * PRECISION);
            }
            else
            {
                return SYNTAX_ERR;
            }
        }
        else if (strncmp (str, "pop", 3) == 0)
        {
            (*commands_int)[num_commands - 2] = 0;
            (*commands_int)[num_commands - 1] = 0;
            (*commands_int)[num_commands - 2] |= POP;
            if (sscanf (str + 4, "%s", command) != 1)
                return SYNTAX_ERR;
            if (strlen (command) == 3)
            {
                if (command[0] == 'r' && command[2] == 'x' && command[1] >= 'a' && command[1] <= 'd')
                {
                    (*commands_int)[num_commands - 2] |= BIT_REGISTER;
                    (*commands_int)[num_commands - 1] = (int) (command[1] - 'a' + 1);
                }
            }
            else
            {
                return SYNTAX_ERR;
            }
        }
        else
        {
            return SYNTAX_ERR;
        }
    }
    fclose (file_read);
    *num_com = num_commands;
    return CORRECT;
}

Errors print_commands_arr (const char* name_file_print, int* commands_int, int num_commands)
{
    FILE* file_print = fopen (name_file_print, "w");
    if (!file_print)
        return OPEN_FILE_ERR;

    fprintf (file_print, "%s\n", SIGNATURE);
    fprintf (file_print, "%d\n", VERSION);
    fprintf (file_print, "%d\n", num_commands);

    for (int i = 0; i < num_commands; i++)
    {
        fprintf (file_print, "%d\n", commands_int[i]);
    }
    fclose (file_print);
    return CORRECT;
}

void del_comment (char* str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ';')
        {
            str[i] = '\0';
            break;
        }
    }
}

void del_slash_n (char* str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }
    }
}
