#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"

#define PARSE_ERROR(error)          \
        if (error != CORRECT)       \
        {                           \
            print_error (error);    \
            return 1;               \
        }

enum Errors
{
    CORRECT =      -1,
    OPEN_FILE_ERR = 1,
    SYNTAX_ERR    = 2,
    MEM_ALLOC_ERR = 3,
    NULL_POINTER =  4
};

const int REALLOC_STEP = 10;
const char* FILE_NAME_READ_DEF = "asm.txt";
const char* FILE_NAME_PRINT_DEF = "byte_code.txt";
const char* BIN_FILE_NAME_PRINT_DEF = "byte_code.bin";

Errors get_commands_arr (const char* name_file_read, int** commands_int, int* num_com);
#ifdef TXT_BYTE_CODE
Errors print_commands_txt (const char* name_file_print, int* commands_int, File_Header* header);
#endif
Errors print_commands_bin (const char* name_file_print, int* commands_int, File_Header* header);
Errors header_ctor (File_Header* header, int num_comm);
void print_error (Errors error);
void del_comment (char* str);
void del_slash_n (char* str);

int main (int argc, char* argv[])
{

    char file_name_read[MAX_NAME_LEN] = "";
    char file_name_print[MAX_NAME_LEN] = "";
    char bin_file_name_print[MAX_NAME_LEN] = "";
    if (argc < 4)
    {
        strcpy (file_name_read, FILE_NAME_READ_DEF);
        strcpy (file_name_print, FILE_NAME_PRINT_DEF);
        strcpy (bin_file_name_print, BIN_FILE_NAME_PRINT_DEF);
    }
    else
    {
        strcpy (file_name_read, argv[1]);
        strcpy (file_name_print, argv[2]);
        strcpy (bin_file_name_print, argv[3]);
    }

    int* commands_int;
    int num_commands = 0;
    Errors error = get_commands_arr (file_name_read, &commands_int, &num_commands);
    PARSE_ERROR(error);

    File_Header header = {};
    header_ctor (&header, num_commands);

#ifdef TXT_BYTE_CODE
    error = print_commands_txt (file_name_print, commands_int, &header);
    PARSE_ERROR(error);
#endif

    error = print_commands_bin (bin_file_name_print, commands_int, &header);
    PARSE_ERROR(error);
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
    case NULL_POINTER:
        printf ("Null pointer\n");
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

        bool is_read_command = false;
        for (int i = 0; i < NUM_OF_COMMANDS; i++)
        {
            if (strncmp (str, COMMANDS_LIST[i].name, strlen (COMMANDS_LIST[i].name)) ==  0)
            {
                is_read_command = true;
                switch (COMMANDS_LIST[i].arg_type)
                {
                    case NO_ARG:
                        (*commands_int)[num_commands - 2] = COMMANDS_LIST[i].code;
                        num_commands--;
                        break;
                    case REG_ARG:
                        (*commands_int)[num_commands - 2] = 0;
                        (*commands_int)[num_commands - 1] = 0;
                        (*commands_int)[num_commands - 2] |= COMMANDS_LIST[i].code;
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
                        break;
                    case NUM_ARG:
                        break;
                    case NUM_OR_REG_ARG:
                        (*commands_int)[num_commands - 2] = 0;
                        (*commands_int)[num_commands - 1] = 0;
                        (*commands_int)[num_commands - 2] |= COMMANDS_LIST[i].code;
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
                        break;
                    default:
                        break;
                }
                break;
            }
        }
        if (!is_read_command)
            return SYNTAX_ERR;
    }
    fclose (file_read);
    *num_com = num_commands;
    return CORRECT;
}

Errors print_commands_bin (const char* name_file_print, int* commands_int, File_Header* header)
{
    if (!name_file_print || !commands_int || !header)
        return NULL_POINTER;
    FILE* file_print = fopen (name_file_print, "wb");
    if (!file_print)
        return OPEN_FILE_ERR;

    fwrite (header, sizeof (File_Header), 1, file_print);
    fwrite (commands_int, sizeof (int), header->num_commands, file_print);

    fclose (file_print);
    return CORRECT;
}

#ifdef TXT_BYTE_CODE
Errors print_commands_txt (const char* name_file_print, int* commands_int, File_Header* header)
{
    FILE* file_print = fopen (name_file_print, "w");
    if (!file_print)
        return OPEN_FILE_ERR;

    fprintf (file_print, "%s\n", header->signature);
    fprintf (file_print, "%d\n", header->version);
    fprintf (file_print, "%d\n", header->num_commands);

    for (int i = 0; i < header->num_commands; i++)
    {
        fprintf (file_print, "%d\n", commands_int[i]);
    }
    fclose (file_print);
    return CORRECT;
}
#endif

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

Errors header_ctor (File_Header* header, int num_comm)
{
    if (!header)
        return NULL_POINTER;
    strcpy (header->signature, SIGNATURE);
    header->version = VERSION;
    header->num_commands = num_comm;
    return CORRECT;
}
