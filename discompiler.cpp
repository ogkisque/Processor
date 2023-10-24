#include "discompiler.h"

int main (int argc, char* argv[])
{
    char file_name_read[MAX_NAME_LEN] = "";
    char file_name_print[MAX_NAME_LEN] = "";
    bool is_txt_file = parse_cmd_args (file_name_read, file_name_print, argc, argv);

    Commands_Arr commands = {};
    if (is_txt_file)
        read_txt_byte_code (file_name_read, &commands);
    else
        read_bin_byte_code (file_name_read, &commands);

    discompile (file_name_print, &commands);

    return 0;
}

void read_bin_byte_code (char* file_name_read, Commands_Arr* commands)
{
    FILE* file_read = fopen (file_name_read, "r");

    File_Header header = {};
    fread (&header, sizeof (header), 1, file_read);

    commands->num_commands = header.num_commands;
    commands->commands = (int*) calloc (commands->num_commands, sizeof (int));
    fread (commands->commands, sizeof (int), header.num_commands, file_read);

    fclose (file_read);
}

#ifdef TXT_BYTE_CODE
void read_txt_byte_code (char* file_name_read, Commands_Arr* commands)
{
    FILE* file_read = fopen (file_name_read, "r");

    File_Header header = {};
    fscanf (file_read, "%s", header.signature);
    fscanf (file_read, "%d", &(header.version));
    fscanf (file_read, "%d", &(header.num_commands));

    commands->num_commands = header.num_commands;
    commands->commands = (int*) calloc (commands->num_commands, sizeof (int));

    int number = 0;
    int i = 0;
    while (fscanf (file_read, "%d", &number) == 1)
    {
        (commands->commands)[i] = number;
        i++;
    }

    fclose (file_read);
}
#endif

void discompile (char* file_name_print, Commands_Arr* commands)
{
    int command = 0;
    FILE* file_print = fopen (file_name_print, "w");

    #define DEF_CMD(name, num, args, ...)                               \
            if (((commands->commands)[i] & CODE_COMMAND_MASK) == num)   \
            {                                                           \
                fprintf (file_print, "%s ", #name);                     \
                if ((commands->commands)[i] & BIT_REGISTER)             \
                {                                                       \
                    i++;                                                \
                    command = (commands->commands)[i];                  \
                    fprintf (file_print, "%c", 'r');                    \
                    fprintf (file_print, "%c", 'a' + command - 1);      \
                    fprintf (file_print, "%c\n", 'x');                  \
                }                                                       \
                else if ((commands->commands)[i] & BIT_MEM_OPER_NUM)    \
                {                                                       \
                    i++;                                                \
                    command = (commands->commands)[i];                  \
                    fprintf (file_print, "[%d]\n", command);            \
                }                                                       \
                else if ((commands->commands)[i] & BIT_MEM_OPER_REG)    \
                {                                                       \
                    i++;                                                \
                    command = (commands->commands)[i];                  \
                    fprintf (file_print, "[%c", 'r');                   \
                    fprintf (file_print, "%c", 'a' + command - 1);      \
                    fprintf (file_print, "%c]\n", 'x');                 \
                }                                                       \
                else if ((commands->commands)[i] & BIT_IMM_CONST)       \
                {                                                       \
                    i++;                                                \
                    command = (commands->commands)[i];                  \
                    fprintf (file_print, "%d\n", command);              \
                }                                                       \
                else if ((args) & LABEL)                                \
                {                                                       \
                    i++;                                                \
                    command = (commands->commands)[i];                  \
                    fprintf (file_print, "%d\n", command);              \
                }                                                       \
                else                                                    \
                {                                                       \
                    fprintf (file_print, "\n");                         \
                }                                                       \
            }


    for (int i = 0; i < commands->num_commands; i++)
    {
        #include "code_generate.h"
    }
    fclose (file_print);
    #undef DEF_CMD
}

bool parse_cmd_args (char file_name_read[], char file_name_print[], int argc, char* argv[])
{
    if (argc < 3)
    {
        strcpy (file_name_read, BIN_FILE_NAME_READ_DEF);
        strcpy (file_name_print, FILE_NAME_PRINT_DEF);
        return false;
    }
    else
    {
        strcpy (file_name_read, argv[1]);
        strcpy (file_name_print, argv[2]);
        return check_type_file (file_name_read);
    }
}

bool check_type_file (char file_name_read[])
{
    int i = 0;
    while (file_name_read[i] != '.')
        i++;

    if (strncmp (file_name_read + i + 1, "txt", 3) == 0)
        return true;
    return false;
}
