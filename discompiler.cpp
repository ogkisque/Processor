#include <stdio.h>
#include <string.h>

#include "commands.h"

enum Errors
{
    CORRECT =      -1,
    OPEN_FILE_ERR = 1,
    SYNTAX_ERR    = 2
};

const int MAX_COMMAND_LEN = 20;
const char* FILE_NAME_READ = "byte_code.txt";
const char* FILE_NAME_PRINT = "asm1.txt";

Errors discompiling (const char* name_file_read, const char* name_file_print);
void print_error (Errors error);

int main ()
{
    Errors error = discompiling (FILE_NAME_READ, FILE_NAME_PRINT);
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
        printf ("Correct discompilation\n");
        break;
    default:
        break;
    }
}

Errors discompiling (const char* name_file_read, const char* name_file_print)
{
    FILE* file_read = fopen (name_file_read, "r");
    FILE* file_print = fopen (name_file_print, "w");
    if (!(file_read && file_print))
        return OPEN_FILE_ERR;

    int command = 0;
    int number = 0;
    while (true)
    {
        if (fscanf (file_read, "%d", &command) != 1)
        {
            break;
        }
        else
        {
            switch (command)
            {
            case HLT:
                fprintf (file_print, "HLT\n");
                break;
            case OUT:
                fprintf (file_print, "OUT\n");
                break;
            case IN:
                fprintf (file_print, "IN\n");
                break;
            case DIV:
                fprintf (file_print, "DIV\n");
                break;
            case SUB:
                fprintf (file_print, "SUB\n");
                break;
            case MUL:
                fprintf (file_print, "MUL\n");
                break;
            case ADD:
                fprintf (file_print, "ADD\n");
                break;
            case SQRT:
                fprintf (file_print, "SQRT\n");
                break;
            case SIN:
                fprintf (file_print, "SIN\n");
                break;
            case COS:
                fprintf (file_print, "COS\n");
                break;
            case PUSH:
                if (fscanf (file_read, "%d", &number) != 1)
                    return SYNTAX_ERR;
                else
                    fprintf (file_print, "PUSH %d\n", number);
                break;
            default:
                return SYNTAX_ERR;
                break;
            }
        }
    }
    fclose (file_read);
    fclose (file_print);
    return CORRECT;
}
