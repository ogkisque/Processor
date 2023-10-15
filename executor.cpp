#include <stdio.h>
#include <math.h>
#include <string.h>

#include "stack.h"
#include "commands.h"

#define SPU_CTOR(sp, num_commands) \
        spu_ctor ((sp), num_commands, #sp, __FILE__, __func__, __LINE__)

#define PARSE_ERROR(sp, error)              \
        if (error.err_code != CORRECT)      \
        {                                   \
            print_error_spu (sp, error);    \
            return 1;                       \
        }

#define RETURN_ERROR(code, message) \
        return Error {code, __LINE__, __FILE__, __func__, message}

struct Error
{
    int err_code;
    int err_line;
    const char* err_file;
    const char* err_func;
    const char* err_message;
};

struct Spu
{
    Stack* stk;
    int rax;
    int rbx;
    int rcx;
    int rdx;
    int num_comm;
    int* code;
    int ip;

    const char* name;
    const char* file;
    const char* func;
    int line;
};

enum Errors_spu
{
    CORRECT =           0,
    STACK_ERR =         1,
    NULL_POINTER_SPU =  2,
    RUBBISH_SPU =       3,
    MEM_ALLOC_ERR_SPU = 4,
    OPEN_FILE_ERR =     5,
    INPUT_NUM_ERR =     6,
    SYNTAX_ERR =        7,
    VERSION_ERR =       8,
    SIGNATURE_ERR =     9,
    VERIFY_ERR =        10
};

const char* FILE_NAME_READ_DEF = "byte_code.txt";
const char* BIN_FILE_NAME_READ_DEF = "byte_code.bin";
const int PI = 3.1415926;
const int NUM_ERR_STACK = 1;
const int NUM_ERR_SPU = 2;
const int CODE_ERROR_MASK = 0x00FFFFFF;
const int TYPE_ERROR_MASK = 0xFF000000;

#ifdef TXT_BYTE_CODE
Error read_txt_byte_code (char* file_name_read, Spu* sp);
#endif
Error read_bin_byte_code (char* file_name_read, Spu* sp);
Error execute (Spu* sp);
void print_error_spu (Spu* sp, Error error);
Error spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line);
Error spu_dtor (Spu* sp);
Error spu_verify (Spu* sp);
void spu_dump (Spu* sp, Error error);

int main (int argc, char* argv[])
{
    char file_name_read[MAX_NAME_LEN] = "";
    char bin_file_name_read[MAX_NAME_LEN] = "";
    if (argc < 3)
    {
        strcpy (bin_file_name_read, BIN_FILE_NAME_READ_DEF);
        strcpy (file_name_read, FILE_NAME_READ_DEF);
    }
    else
    {
        strcpy (file_name_read, argv[1]);
        strcpy (bin_file_name_read, argv[2]);
    }

    Error error = {};
#ifdef TXT_BYTE_CODE
    printf ("With .txt file\n");
    Spu sp1 = {};
    error = read_txt_byte_code (file_name_read, &sp1);
    PARSE_ERROR(&sp1, error);
    error = execute (&sp1);
    PARSE_ERROR(&sp1, error);
#endif

    printf ("With .bin file\n");
    Spu sp2 = {};
    error = read_bin_byte_code (bin_file_name_read, &sp2);
    PARSE_ERROR(&sp2, error);
    error = execute (&sp2);
    PARSE_ERROR(&sp2, error);

    return 0;
}

Error read_bin_byte_code (char* file_name_read, Spu* sp)
{
    if (!file_name_read || !sp)
        RETURN_ERROR(NULL_POINTER_SPU, "Null pointer\n");

    FILE* file_read = fopen (file_name_read, "rb");
    if (!file_read)
        RETURN_ERROR(OPEN_FILE_ERR, "Error of opening file\n");

    File_Header header = {};
    fread (&header, sizeof (header), 1, file_read);

    if (strcmp (header.signature, SIGNATURE) != 0)
        RETURN_ERROR(SIGNATURE_ERR, "Incorrect signature");
    if (header.version != VERSION)
        RETURN_ERROR(VERSION_ERR, "Incorrect version");

    Error error = SPU_CTOR(sp, header.num_commands);
    if (error.err_code != CORRECT)
        spu_dump (sp, error);

    fread (sp->code, sizeof (int), header.num_commands, file_read);

    fclose (file_read);
    RETURN_ERROR(CORRECT, "");
}

#ifdef TXT_BYTE_CODE
Error read_txt_byte_code (char* file_name_read, Spu* sp)
{
    if (!file_name_read || !sp)
        RETURN_ERROR(NULL_POINTER, "Null pointer\n");

    FILE* file_read = fopen (file_name_read, "r");
    if (!file_read)
        RETURN_ERROR(OPEN_FILE_ERR, "Error of opening file\n");

    char str[MAX_COMMAND_LEN] = "";
    int number = 0;
    int i = 0;
    while (fscanf (file_read, "%s", str) == 1)
    {
        if (i == 0)
        {
            if (strcmp (str, SIGNATURE) != 0)
                RETURN_ERROR(SIGNATURE_ERR, "Incorrect signature");
        }
        else
        {
            if (sscanf (str, "%d", &number) != 1)
            {
                RETURN_ERROR(SYNTAX_ERR, "Error in syntax");
            }
            else
            {
                if (i == 1)
                {
                    if (number != VERSION)
                        RETURN_ERROR(VERSION_ERR, "Incorrect version");
                }
                else if (i == 2)
                {
                    if (number < 0)
                        RETURN_ERROR(SYNTAX_ERR, "Negative number of commands");
                    Error err = SPU_CTOR(sp, number);
                    if (err.err_code != CORRECT)
                        spu_dump (sp, err);
                }
                else
                {
                    sp->code[i - 3] = number;
                }
            }
        }
        i++;
    }

    fclose (file_read);
    RETURN_ERROR(CORRECT, "");
}
#endif

void print_error_spu (Spu* sp, Error error)
{
    printf (BOLD_RED_COL);
    if (error.err_code == STACK_ERR)
    {
        STACK_DUMP(sp->stk, stack_verify (sp->stk));
    }
    else
    {
        printf ("%s\n"
                "In file: %s, function: %s, line: %d",
                error.err_message, error.err_file, error.err_func, error.err_line);
    }
    printf (OFF_COL);
}

Error execute (Spu* sp)
{
    Error error = spu_verify (sp);
    if (error.err_code != CORRECT)
    {
        spu_dump (sp, error);
        RETURN_ERROR(VERIFY_ERR, "Error in verifying of spu\n");
    }

    int i = 0;
    int command = 0;
    int number = 0;
    int number1 = 0;
    double num_double = 0.0;
    while (i < sp->num_comm)
    {
        command = (sp->code)[i];
        switch (command & CODE_COMMAND_MASK)
        {
            case HLT:
                RETURN_ERROR(CORRECT, "");
            case OUT:
                if (stack_pop (sp->stk, &number) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    printf ("%lf\n", (double) number / PRECISION);
                break;
            case IN:
                printf ("Enter the number: ");
                if (scanf ("%lf", &num_double) == 1)
                    stack_push (sp->stk, (int) (num_double * PRECISION));
                else
                    RETURN_ERROR(INPUT_NUM_ERR, "Error with input data");
                break;
            case DIV:
                if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, (int) (((double) number1 / (double) number) * PRECISION));
                break;
            case SUB:
                if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, number1 - number);
                break;
            case MUL:
                if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, number * number1 / PRECISION);
                break;
            case ADD:
                if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, number + number1);
                break;
            case SQRT:
                if (stack_pop (sp->stk, &number) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, (int) (sqrt ((double) number / PRECISION) * PRECISION));
                break;
            case SIN:
                if (stack_pop (sp->stk, &number) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, (int) (sin ((double) number / PRECISION * PI / 180) * PRECISION));
                break;
            case COS:
                if (stack_pop (sp->stk, &number) != 0)
                    RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                else
                    stack_push (sp->stk, (int) (cos ((double) number / PRECISION * PI / 180) * PRECISION));
                break;
            case PUSH:
                i++;
                if (command & BIT_IMM_CONST)
                {
                    number = (sp->code)[i];
                    stack_push (sp->stk, number);
                }
                else if (command & BIT_REGISTER)
                {
                    number = (sp->code)[i];
                    switch (number)
                    {
                        case 1:
                            number = sp->rax;
                            break;
                        case 2:
                            number = sp->rbx;
                            break;
                        case 3:
                            number = sp->rcx;
                            break;
                        case 4:
                            number = sp->rdx;
                            break;
                        default:
                            RETURN_ERROR(SYNTAX_ERR, "Incorrect name of register");
                    }
                    stack_push (sp->stk, number);
                }
                else
                {
                    RETURN_ERROR(SYNTAX_ERR, "Error in syntax");
                }
                break;
            case POP:
                i++;
                if (command & BIT_REGISTER)
                {
                    number = (sp->code)[i];
                    int tmp = 0;
                    if (stack_pop (sp->stk, &tmp) != 0)
                    {
                        RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
                    }
                    else
                    {
                        switch (number)
                        {
                            case 1:
                                sp->rax = tmp;
                                break;
                            case 2:
                                sp->rbx = tmp;
                                break;
                            case 3:
                                sp->rcx = tmp;
                                break;
                            case 4:
                                sp->rdx = tmp;
                                break;
                            default:
                                RETURN_ERROR(SYNTAX_ERR, "Incorrect name of register");
                        }
                    }
                }
                else
                {
                    RETURN_ERROR(SYNTAX_ERR, "Error in syntax");
                }
                break;
            default:
                RETURN_ERROR(SYNTAX_ERR, "Incorrect code of command");
                break;
        }
        i++;
    }
    RETURN_ERROR(CORRECT, "");
}

Error spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line)
{
    if (!sp)
        RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of spu");

    int error = 0;
    if (MAKE_STACK(&(sp->stk)) != 0)
        error |= STACK_ERR;

    sp->rax = 0;
    sp->rbx = 0;
    sp->rcx = 0;
    sp->rdx = 0;
    sp->ip = 0;
    sp->num_comm = num_commands;
    sp->name = name;
    sp->file = file;
    sp->func = func;
    sp->line = line;

    sp->code = (int*) calloc (num_commands + 1, sizeof (int));
    if (!sp->code)
        RETURN_ERROR(MEM_ALLOC_ERR_SPU, "Error in memory allocation of array of commands");

    RETURN_ERROR(CORRECT, "");
}

Error spu_dtor (Spu** sp)
{
    if (!sp || !(*sp))
        RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of spu");

    delete_stack (&((*sp)->stk));
    (*sp)->rax = INT_MAX;
    (*sp)->rbx = INT_MAX;
    (*sp)->rcx = INT_MAX;
    (*sp)->rdx = INT_MAX;
    (*sp)->ip = -1;
    (*sp)->num_comm = -1;
    (*sp)->name = NULL;
    (*sp)->file = NULL;
    (*sp)->func = NULL;
    (*sp)->line = -1;
    free ((*sp)->code);
    (*sp)->code = NULL;
    free (*sp);
    *sp = NULL;
    RETURN_ERROR(CORRECT, "");
}

Error spu_verify (Spu* sp)
{
    if (!sp)                                                                                    RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of spu");
    if (stack_verify (sp->stk) != 0)                                                            RETURN_ERROR(STACK_ERR, "Error with stack");
    if (sp->rax == INT_MAX || sp->rbx == INT_MAX || sp->rcx == INT_MAX || sp->rbx == INT_MAX)   RETURN_ERROR(RUBBISH_SPU, "Rubbish in registers");
    if (!(sp->code) || !(sp->file) || !(sp->func) || !(sp->name))                               RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of elements in spu");
    RETURN_ERROR(CORRECT, "");
}

void spu_dump (Spu* sp, Error error)
{
    printf (RED_COL);
    if (!sp)
    {
        printf ("Null pointer of spu\n");
        return;
    }

    printf ("Error is in spu: name - %s, file - %s, function - %s, line - %d\n",
            sp->name, sp->file, sp->func, sp->line);
    print_error_spu (sp, error);
    printf (RED_COL);
    print_stack (sp->stk);
    printf ("rax = %d, rbx = %d, rcx = %d, rdx = %d\n",
            sp->rax, sp->rbx, sp->rcx, sp->rdx);
    for (int i = 0; i < sp->num_comm; i++)
    {
        if (i < 10)
            printf ("0%d ", i);
        else
            printf ("%d ", i);
    }
    printf ("\n");
    for (int i = 0; i < sp->num_comm; i++)
    {
        if ((sp->code)[i] < 10)
            printf ("%d  ", (sp->code)[i]);
        else
            printf ("%d ", (sp->code)[i]);
    }
    printf ("\n");
    for (int i = 0; i < sp->ip; i++)
        printf ("   ");
    printf ("^\n");
    printf (OFF_COL);
}
