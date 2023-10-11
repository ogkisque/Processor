#include <stdio.h>
#include <math.h>
#include <string.h>

#include "stack.h"
#include "commands.h"

#define READ_TXT_FILE

#define SPU_CTOR(sp, num_commands) \
        spu_ctor ((sp), num_commands, #sp, __FILE__, __func__, __LINE__)

#define PARSE_ERROR(sp, error)              \
        if (error != 0)                     \
        {                                   \
            print_error_spu (sp, error);    \
            return 1;                       \
        }

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
    STACK_ERR =         1 << 1,
    NULL_POINTER_SPU =  1 << 2,
    RUBBISH_SPU =       1 << 3,
    MEM_ALLOC_ERR_SPU = 1 << 4,
    OPEN_FILE_ERR =     1 << 5,
    INPUT_NUM_ERR =     1 << 6,
    SYNTAX_ERR =        1 << 7,
    VERSION_ERR =       1 << 8,
    SIGNATURE_ERR =     1 << 9,
    VERIFY_ERR =        1 << 10
};

const char* FILE_NAME_READ_DEF = "byte_code.txt";
const char* BIN_FILE_NAME_READ_DEF = "byte_code.bin";
const int PI = 3.1415926;
const int NUM_ERR_STACK = 1;
const int NUM_ERR_SPU = 2;
const int CODE_ERROR_MASK = 0x00FFFFFF;
const int TYPE_ERROR_MASK = 0xFF000000;

int read_byte_code (char* file_name_read, Spu* sp);
int read_bin_byte_code (char* file_name_read, Spu* sp);
int execute (Spu* sp);
void print_error_spu (Spu* sp, int error);
int spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line);
int spu_dtor (Spu* sp);
int spu_verify (Spu* sp);
void spu_dump (Spu* sp, int error);

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

    int error = 0;
#ifdef READ_TXT_FILE
    printf ("With .txt file\n");
    Spu sp1 = {};
    error = read_byte_code (file_name_read, &sp1);
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

int read_bin_byte_code (char* file_name_read, Spu* sp)
{
    int error = 0;

    FILE* file_read = fopen (file_name_read, "rb");
    if (!file_read)
        return OPEN_FILE_ERR;

    char sign[MAX_COMMAND_LEN] = "";
    int read = fread (sign, sizeof (char), sizeof (SIGNATURE) / sizeof (char), file_read);
    if (strcmp (sign, SIGNATURE) != 0)
        return SIGNATURE_ERR;

    int vers = 0;
    fread (&vers, sizeof (int), 1, file_read);
    if (vers != VERSION)
        return VERSION_ERR;

    int num_commands = 0;
    fread (&num_commands, sizeof (int), 1, file_read);
    error = SPU_CTOR(sp, num_commands);
    if (error != 0)
        spu_dump (sp, error);

    fread (sp->code, sizeof (int), num_commands, file_read);

    fclose (file_read);
    return error;
}

int read_byte_code (char* file_name_read, Spu* sp)
{
    int error = 0;

    FILE* file_read = fopen (file_name_read, "r");
    if (!file_read)
        return OPEN_FILE_ERR;

    char str[MAX_COMMAND_LEN] = "";
    int number = 0;
    int i = 0;
    while (fscanf (file_read, "%s", str) == 1)
    {
        if (i == 0)
        {
            if (strcmp (str, SIGNATURE) != 0)
                return SIGNATURE_ERR;
        }
        else
        {
            if (sscanf (str, "%d", &number) != 1)
            {
                return SYNTAX_ERR;
            }
            else
            {
                if (i == 1)
                {
                    if (number != VERSION)
                        return VERSION_ERR;
                }
                else if (i == 2)
                {
                    if (number < 0)
                        return SYNTAX_ERR;
                    int err = SPU_CTOR(sp, number);
                    if (err != 0)
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
    return error;
}

void print_error_spu (Spu* sp, int error)
{
    printf (BOLD_RED_COL);

    if (error & NULL_POINTER_SPU)
        printf ("Null pointer\n");
    if (error & STACK_ERR)
        STACK_DUMP(sp->stk, stack_verify (sp->stk));
    if (error & RUBBISH_SPU)
        printf ("Rubbish in spu\n");
    if (error & MEM_ALLOC_ERR_SPU)
        printf ("Error of allocation memory\n");
    if (error & OPEN_FILE_ERR)
        printf ("Error is in opening file\n");
    if (error & INPUT_NUM_ERR)
        printf ("Incorrect input data\n");
    if (error & SYNTAX_ERR)
        printf ("Error is in syntax\n");
    if (error & VERSION_ERR)
        printf ("Error is in version of programm\n");
    if (error & SIGNATURE_ERR)
        printf ("Error is in signature of programm\n");
    if (error & VERIFY_ERR)
        printf ("Error is in verifying of spu\n");
    printf (OFF_COL);
}

int execute (Spu* sp)
{
    int error = spu_verify (sp);
    if (error != 0)
    {
        spu_dump (sp, error);
        return VERIFY_ERR;
    }

    int i = 0;
    int command = 0;
    int number = 0;
    int number1 = 0;
    double num_double = 0.0;
    while (i < sp->num_comm)
    {
        command = (sp->code)[i];
        if (command == HLT)
        {
            return error;
        }
        else if (command == OUT)
        {
            stack_pop (sp->stk, &number);
            printf ("%lf\n", (double) number / PRECISION);
        }
        else if (command == IN)
        {
            printf ("Enter the number: ");
            if (scanf ("%lf", &num_double) == 1)
                stack_push (sp->stk, (int) (num_double * PRECISION));
            else
                return INPUT_NUM_ERR;
        }
        else if (command == DIV)
        {
            if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, (int) (((double) number1 / (double) number) * PRECISION));
        }
        else if (command == SUB)
        {
            if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, number1 - number);
        }
        else if (command == MUL)
        {
            if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, number * number1 / PRECISION);
        }
        else if (command == ADD)
        {
            if (stack_pop (sp->stk, &number) + stack_pop (sp->stk, &number1) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, number + number1);
        }
        else if (command == SQRT)
        {
            if (stack_pop (sp->stk, &number) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, (int) (sqrt ((double) number / PRECISION) * PRECISION));
        }
        else if (command == SIN)
        {
            if (stack_pop (sp->stk, &number) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, (int) (sin ((double) number / PRECISION * PI / 180) * PRECISION));
        }
        else if (command == COS)
        {
            if (stack_pop (sp->stk, &number) != 0)
                return SYNTAX_ERR;
            else
                stack_push (sp->stk, (int) (cos ((double) number / PRECISION * PI / 180) * PRECISION));
        }
        else
        {
            i++;
            if ((command & CODE_COMMAND_MASK) == PUSH)
            {
                if (command & BIT_IMM_CONST)
                {
                    number = (sp->code)[i];
                    stack_push (sp->stk, number);
                }
                else if (command & BIT_REGISTER)
                {
                    number = (sp->code)[i];
                    if (number == 1)
                        number = sp->rax;
                    else if (number == 2)
                        number = sp->rbx;
                    else if (number == 3)
                        number = sp->rcx;
                    else if (number == 4)
                        number = sp->rdx;
                    else
                        return SYNTAX_ERR;

                    stack_push (sp->stk, number);
                }
                else
                {
                    return SYNTAX_ERR;
                }
            }
            else if ((command & CODE_COMMAND_MASK) == POP)
            {
                if (command & BIT_REGISTER)
                {
                    number = (sp->code)[i];
                    if (number == 1)
                        stack_pop (sp->stk, &(sp->rax));
                    else if (number == 2)
                        stack_pop (sp->stk, &(sp->rbx));
                    else if (number == 3)
                        stack_pop (sp->stk, &(sp->rcx));
                    else if (number == 4)
                        stack_pop (sp->stk, &(sp->rdx));
                    else
                        return SYNTAX_ERR;
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
        i++;
    }
    return error;
}

int spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line)
{
    if (!sp)
        return NULL_POINTER_SPU;

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
        error |= MEM_ALLOC_ERR_SPU;

    return error;
}

int spu_dtor (Spu** sp)
{
    if (!sp || !(*sp))
        return NULL_POINTER_SPU;
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
    return 0;
}

int spu_verify (Spu* sp)
{
    int error = 0;
    if (!sp)
        return NULL_POINTER_SPU;
    if (stack_verify (sp->stk) != 0)                                                                error |= STACK_ERR;
    if (sp->rax == INT_MAX || sp->rbx == INT_MAX || sp->rcx == INT_MAX || sp->rbx == INT_MAX)       error |= RUBBISH_SPU;
    if (!(sp->code) || !(sp->file) || !(sp->func) || !(sp->name))                                   error |= NULL_POINTER_SPU;
    return error;
}

void spu_dump (Spu* sp, int error)
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
