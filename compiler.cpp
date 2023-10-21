#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"
#include "colors.h"

#define PARSE_ERROR(error)                  \
        if (error.err_code != CORRECT)      \
        {                                   \
            return error;                   \
        }

#define PARSE_ERROR_MAIN(error)             \
        if (error.err_code != CORRECT)      \
        {                                   \
            print_error_asm (error);        \
            return 1;                       \
        }

#define RETURN_ERROR(code, line_asm, message) \
        return Error {code, __LINE__, line_asm, file_name_read, __FILE__, __func__, message}

enum Errors_asm
{
    CORRECT =       -1,
    OPEN_FILE_ERR =  1,
    SYNTAX_ERR    =  2,
    MEM_ALLOC_ERR =  3,
    NULL_POINTER =   4,
    WRITE_FILE_ERR = 5,
    UNKNOWN_LABEL =  6
};

struct Error
{
    Errors_asm err_code;
    int err_line;
    int err_line_asm;
    const char* asm_file;
    const char* err_file;
    const char* err_func;
    const char* err_message;
};

struct Label_Var
{
    char name[MAX_COMMAND_LEN];
    int num_ip;
};

struct Label_Arr
{
    Label_Var labels[NUM_OF_LABELS];
    int num_labels;
    bool need_recompile;
};

struct Commands_Arr
{
    int* commands;
    int num_commands;
    int capacity;
    int str_asm;
};

const int REALLOC_STEP = 10;
const char* FILE_NAME_READ_DEF = "asm.txt";
const char* FILE_NAME_PRINT_DEF = "byte_code.txt";
const char* BIN_FILE_NAME_PRINT_DEF = "byte_code.bin";

char file_name_read[MAX_NAME_LEN] = "";

Error get_commands_arr (const char* name_file_read,
                         Commands_Arr* commands_struct,
                         Label_Arr* labels_struct);
Error parse_command (char str[],
                      Commands_Arr* commands_struct,
                      Label_Arr* labels_struct);
Error parse_arg (char str[],
                  Commands_Arr* commands_struct,
                  Label_Arr* labels_struct,
                  Arg_Types* arg_type_real);
int found_label (char arg[], Label_Arr* labels_struct);
#ifdef TXT_BYTE_CODE
Error print_commands_txt (const char* name_file_print,
                           int* commands_int,
                           File_Header* header);
#endif
Error print_commands_bin (const char* name_file_print,
                           int* commands_int,
                           File_Header* header);
Error header_ctor (File_Header* header, int num_comm);
void parse_cmd_args (int argc,
                     char* argv[],
                     char bin_file_name_print[],
                     char file_name_read[],
                     char file_name_print[]);
void print_error_asm (Error error);
void del_comment (char* str);
void del_slash_n (char* str);

int main (int argc, char* argv[])
{
    char file_name_print[MAX_NAME_LEN] = "";
    char bin_file_name_print[MAX_NAME_LEN] = "";
    parse_cmd_args (argc, argv, bin_file_name_print, file_name_read, file_name_print);

    Commands_Arr commands_struct = {};
    Label_Arr labels_struct = {};
    labels_struct.need_recompile = false;
    Error error = get_commands_arr (file_name_read, &commands_struct, &labels_struct);
    PARSE_ERROR_MAIN(error);

    if (labels_struct.need_recompile)
    {
        labels_struct.need_recompile = false;
        commands_struct = {};
        error = get_commands_arr (file_name_read, &commands_struct, &labels_struct);
        PARSE_ERROR_MAIN(error);
    }

    if (labels_struct.need_recompile)
    {
        print_error_asm (Error {UNKNOWN_LABEL, __LINE__, -1, file_name_read, __FILE__, __func__, "Unknown label."});
        return 1;
    }

    File_Header header = {};
    header_ctor (&header, commands_struct.num_commands);

#ifdef TXT_BYTE_CODE
    error = print_commands_txt (file_name_print, commands_struct.commands, &header);
    PARSE_ERROR_MAIN(error);
#endif

    error = print_commands_bin (bin_file_name_print, commands_struct.commands, &header);
    PARSE_ERROR_MAIN(error);

    printf (GREEN_COL "Correct compilation\n" OFF_COL);
    return 0;
}

void print_error_asm (Error error)
{
    printf (RED_COL);
    printf ("%s Code of error = %d\n"
            "In %s::%d\n"
            "In file: %s, function: %s, line: %d\n",
            error.err_message, error.err_code,
            error.asm_file, error.err_line_asm,
            error.err_file, error.err_func, error.err_line);
    printf (OFF_COL);
}

Error get_commands_arr (const char* name_file_read,
                        Commands_Arr* commands_struct,
                        Label_Arr* labels_struct)
{
    if (!name_file_read || !commands_struct || !labels_struct)
        RETURN_ERROR(NULL_POINTER, -1, "Null pointer.");

    FILE* file_read = fopen (name_file_read, "r");
    if (!file_read)
        RETURN_ERROR(OPEN_FILE_ERR, -1, "Error in opening file.");

    char str[MAX_COMMAND_LEN] = "";
    commands_struct->capacity = REALLOC_STEP;
    commands_struct->commands = (int*) calloc (REALLOC_STEP, sizeof (int));
    if (!(commands_struct->commands))
        RETURN_ERROR(MEM_ALLOC_ERR, -1, "Error in allocation of memory.");

    while (fgets (str, MAX_COMMAND_LEN, file_read))
    {
        (commands_struct->str_asm)++;
        del_comment (str);
        del_slash_n (str);
        if (str[0] == ':')
        {
            strcpy ((labels_struct->labels)[labels_struct->num_labels].name, str + 1);
            (labels_struct->labels)[labels_struct->num_labels].num_ip = commands_struct->num_commands;
            (labels_struct->num_labels)++;
            continue;
        }

        if (commands_struct->num_commands + 2 >= commands_struct->capacity)
        {
            commands_struct->capacity += REALLOC_STEP;
            (commands_struct->commands) = (int*) realloc ((commands_struct->commands), commands_struct->capacity * sizeof (int));
            if (!(commands_struct->commands))
                RETURN_ERROR(MEM_ALLOC_ERR, -1, "Error in allocation of memory.");
        }
        Error error = parse_command (str, commands_struct, labels_struct);
        PARSE_ERROR(error);
    }

    fclose (file_read);
    RETURN_ERROR(CORRECT, -1, "");
}

Error parse_command (char str[],
                     Commands_Arr* commands_struct,
                     Label_Arr* labels_struct)
{
    if (!str || !commands_struct || !labels_struct)
        RETURN_ERROR(NULL_POINTER, -1, "Null pointer.");

    bool is_read_command = false;
    (commands_struct->commands)[commands_struct->num_commands] = 0;
    (commands_struct->commands)[commands_struct->num_commands + 1] = 0;
    Arg_Types arg_type_real = NO_ARG;
    Error error = {};

    char command[MAX_COMMAND_LEN] = "";
    sscanf (str, "%s", command);
    for (int i = 0; i < NUM_OF_COMMANDS; i++)
    {
        if (strcmp (command, COMMANDS_LIST[i].name) ==  0)
        {
            is_read_command = true;
            (commands_struct->commands)[commands_struct->num_commands] |= COMMANDS_LIST[i].code;
            error = parse_arg (str, commands_struct, labels_struct, &arg_type_real);
            PARSE_ERROR(error);

            if ((arg_type_real & COMMANDS_LIST[i].arg_type) == 0)
                RETURN_ERROR(SYNTAX_ERR, commands_struct->str_asm, "Incorrect argument.");
        }
    }
    if (!is_read_command)
        RETURN_ERROR(SYNTAX_ERR, commands_struct->str_asm, "Incorrect command.");

    return error;
}

Error parse_arg (char str[],
                 Commands_Arr* commands_struct,
                 Label_Arr* labels_struct,
                 Arg_Types* arg_type_real)
{
    double number = 0.0;
    char command[MAX_COMMAND_LEN] = "";
    char arg[MAX_COMMAND_LEN] = "";
    if (sscanf (str, "%s %s", command, arg) != 2)
    {
        *arg_type_real = NO_ARG;
        (commands_struct->num_commands)++;
        RETURN_ERROR(CORRECT, -1, "");
    }

    if ((strlen (arg) == 3) &&
        (arg[0] == 'r' && ('a' <= arg[1] && arg[1] <= 'd') && arg[2] == 'x'))
    {
        (commands_struct->commands)[commands_struct->num_commands] |= BIT_REGISTER;
        (commands_struct->num_commands)++;
        (commands_struct->commands)[commands_struct->num_commands] = (int) (arg[1] - 'a' + 1);
        *arg_type_real = REG_ARG;
    }
    else if (sscanf (arg, "%lf", &number) == 1)
    {
        (commands_struct->commands)[commands_struct->num_commands] |= BIT_IMM_CONST;
        (commands_struct->num_commands)++;
        (commands_struct->commands)[commands_struct->num_commands] = (int) (number * PRECISION);
        *arg_type_real = NUM_ARG;
    }
    else //label
    {
        int pos = -1;
        (commands_struct->num_commands)++;
        if ((pos = found_label (arg, labels_struct)) >= 0)
        {
            (commands_struct->commands)[commands_struct->num_commands] = (labels_struct->labels)[pos].num_ip;
        }
        else
        {
            (commands_struct->commands)[commands_struct->num_commands] = -1;
            labels_struct->need_recompile = true;
        }
        *arg_type_real = LABEL;
    }
    (commands_struct->num_commands)++;
    RETURN_ERROR(CORRECT, -1, "");
}

int found_label (char arg[], Label_Arr* labels_struct)
{
    for (int i = 0; i < labels_struct->num_labels; i++)
        if (strcmp (arg, (labels_struct->labels)[i].name) == 0)
            return i;
    return -1;
}

Error print_commands_bin (const char* name_file_print,
                          int* commands_int,
                          File_Header* header)
{
    if (!name_file_print || !commands_int || !header)
        RETURN_ERROR(NULL_POINTER, -1, "Null pointer.");

    FILE* file_print = fopen (name_file_print, "wb");
    if (!file_print)
        RETURN_ERROR(OPEN_FILE_ERR, -1, "Error in opening file.");

    if (fwrite (header, sizeof (File_Header), 1, file_print) != 1)
        RETURN_ERROR(WRITE_FILE_ERR, -1, "Error in writing header in file.");

    if (fwrite (commands_int, sizeof (int), header->num_commands, file_print) != header->num_commands)
        RETURN_ERROR(WRITE_FILE_ERR, -1, "Error in writing commands in file.");

    fclose (file_print);
    RETURN_ERROR(CORRECT, -1, "");
}

#ifdef TXT_BYTE_CODE
Error print_commands_txt (const char* name_file_print,
                           int* commands_int,
                           File_Header* header)
{
    if (!name_file_print || !commands_int || !header)
        RETURN_ERROR(NULL_POINTER, -1, "Null pointer.");

    FILE* file_print = fopen (name_file_print, "w");
    if (!file_print)
        RETURN_ERROR(OPEN_FILE_ERR, -1, "Error in opening file.");

    fprintf (file_print, "%s\n", header->signature);
    fprintf (file_print, "%d\n", header->version);
    fprintf (file_print, "%d\n", header->num_commands);

    for (int i = 0; i < header->num_commands; i++)
    {
        fprintf (file_print, "%d\n", commands_int[i]);
    }
    fclose (file_print);
    RETURN_ERROR(CORRECT, -1, "");
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

Error header_ctor (File_Header* header, int num_comm)
{
    if (!header)
        RETURN_ERROR(NULL_POINTER, -1, "Null pointer.");

    strcpy (header->signature, SIGNATURE);
    header->version = VERSION;
    header->num_commands = num_comm;
    RETURN_ERROR(CORRECT, -1, "");
}

void parse_cmd_args (int argc,
                     char* argv[],
                     char bin_file_name_print[],
                     char file_name_read[],
                     char file_name_print[])
{
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
}
