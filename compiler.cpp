#include <stdio.h>
#include <string.h>

enum Errors
{
    CORRECT =      -1,
    OPEN_FILE_ERR = 1,
    SYNTAX_ERR    = 2
};

const int MAX_COMMAND_LEN = 20;
const char* FILE_NAME_READ = "asm.txt";
const char* FILE_NAME_PRINT = "byte_code.txt";

Errors compiling (const char* name_file_read, const char* name_file_print);
void print_error (Errors error);

int main ()
{
    Errors error = compiling (FILE_NAME_READ, FILE_NAME_PRINT);
    print_error (error);
    if (error != CORRECT)
        return 1;
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
    case CORRECT:
        printf ("Correct compilation\n");
        break;
    default:
        break;
    }
}

Errors compiling (const char* name_file_read, const char* name_file_print)
{
    FILE* file_read = fopen (name_file_read, "r");
    FILE* file_print = fopen (name_file_print, "w");
    if (!(file_read && file_print))
        return OPEN_FILE_ERR;

    char command[MAX_COMMAND_LEN] = "";
    int number = 0;
    while (true)
    {
        if (fscanf (file_read, "%s", command) != 1)
        {
            break;
        }
        else
        {
            if (strcmp (command, "HLT") == 0)
            {
                fprintf (file_print, "%d\n", -1);
            }
            else if (strcmp (command, "OUT") == 0)
            {
                fprintf (file_print, "%d\n", 9);
            }
            else if (strcmp (command, "IN") == 0)
            {
                fprintf (file_print, "%d\n", 10);
            }
            else if (strcmp (command, "DIV") == 0)
            {
                fprintf (file_print, "%d\n", 2);
            }
            else if (strcmp (command, "SUB") == 0)
            {
                fprintf (file_print, "%d\n", 3);
            }
            else if (strcmp (command, "MUL") == 0)
            {
                fprintf (file_print, "%d\n", 4);
            }
            else if (strcmp (command, "ADD") == 0)
            {
                fprintf (file_print, "%d\n", 5);
            }
            else if (strcmp (command, "SQRT") == 0)
            {
                fprintf (file_print, "%d\n", 6);
            }
            else if (strcmp (command, "SIN") == 0)
            {
                fprintf (file_print, "%d\n", 7);
            }
            else if (strcmp (command, "COS") == 0)
            {
                fprintf (file_print, "%d\n", 8);
            }
            else if (strcmp (command, "PUSH") == 0)
            {
                fprintf (file_print, "%d ", 1);
                if (fscanf (file_read, "%d", &number) != 1)
                    return SYNTAX_ERR;
                else
                    fprintf (file_print, "%d\n", number);
            }
            else
            {
                return SYNTAX_ERR;
            }
        }
    }
    fclose (file_read);
    fclose (file_print);
    return CORRECT;
}
