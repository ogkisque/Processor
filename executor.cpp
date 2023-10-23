#include "executor.h"

int main (int argc, char* argv[])
{
    char file_name_read[MAX_NAME_LEN] = "";
    bool is_txt_file = parse_cmd_args (file_name_read, argc, argv);

    Error error = {};
    Spu sp = {};
    if (is_txt_file)
    {
        printf ("With .txt file\n");
        error = read_txt_byte_code (file_name_read, &sp);
    }
    else
    {
        printf ("With .bin file\n");
        error = read_bin_byte_code (file_name_read, &sp);
    }
    PARSE_ERROR(&sp, error);

    error = execute (&sp);
    PARSE_ERROR(&sp, error);

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

    Error error = check_header (&header, sp);
    if (error.err_code != CORRECT)
        return error;

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

    File_Header header = {};
    fscanf (file_read, "%s", header.signature);
    fscanf (file_read, "%d", &(header.version));
    fscanf (file_read, "%d", &(header.num_commands));
    Error error = check_header (&header, sp);
    if (error.err_code != CORRECT)
        return error;

    int number = 0;
    int i = 0;
    while (fscanf (file_read, "%d", &number) == 1)
    {
        sp->code[i] = number;
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
        STACK_DUMP(sp->ret_adrs, stack_verify (sp->ret_adrs));
    }
    else
    {
        printf ("%s\n"
                "In file: %s, function: %s, line: %d\n",
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

    #define DEF_CMD(name, num, args, ...)   \
            case CMD_##name:                \
                __VA_ARGS__;                \
                break;

    while (sp->ip < sp->num_comm)
    {
        command = (sp->code)[sp->ip];
        switch (command & CODE_COMMAND_MASK)
        {
            #include "code_generate.h"
            default:
                RETURN_ERROR(SYNTAX_ERR, "Incorrect code of command");
                break;
        }
        (sp->ip)++;
    }

    #undef DEF_CMD

    RETURN_ERROR(CORRECT, "");
}

Error spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line)
{
    if (!sp)
        RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of spu");

    int error = 0;
    if (MAKE_STACK(&(sp->stk)) != 0)
        error |= STACK_ERR;
    if (MAKE_STACK(&(sp->ret_adrs)) != 0)
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
    sp->memory[SIZE_MEMORY] = {0};

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
    delete_stack (&((*sp)->ret_adrs));
    (*sp)->rax = INT_MAX;
    (*sp)->rbx = INT_MAX;
    (*sp)->rcx = INT_MAX;
    (*sp)->rdx = INT_MAX;
    (*sp)->ip = -1;
    (*sp)->num_comm = -1;
    (*sp)->memory[SIZE_MEMORY] = {0};
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
    if (stack_verify (sp->ret_adrs) != 0)                                                       RETURN_ERROR(STACK_ERR, "Error with stack of return adresses");
    if (sp->rax == INT_MAX || sp->rbx == INT_MAX || sp->rcx == INT_MAX || sp->rbx == INT_MAX)   RETURN_ERROR(RUBBISH_SPU, "Rubbish in registers");
    if (!(sp->code) || !(sp->file) || !(sp->func) || !(sp->name))                               RETURN_ERROR(NULL_POINTER_SPU, "Null pointer of elements in spu");
    RETURN_ERROR(CORRECT, "");
}

Error check_header (File_Header* header, Spu* sp)
{
    if (strcmp (header->signature, SIGNATURE) != 0)
        RETURN_ERROR(SIGNATURE_ERR, "Incorrect signature");
    if (header->version != VERSION)
        RETURN_ERROR(VERSION_ERR, "Incorrect version");

    Error error = SPU_CTOR(sp, header->num_commands);
    if (error.err_code != CORRECT)
        spu_dump (sp, error);

    RETURN_ERROR(CORRECT, "");
}

void spu_dump (Spu* sp, Error error)
{
    printf (RED_COL);
    printf ("-----------------------------------------\n");
    if (!sp)
    {
        printf ("Null pointer of spu\n");
        return;
    }

    printf ("Error is in spu: name - %s, file - %s, function - %s, line - %d\n",
            sp->name, sp->file, sp->func, sp->line);
    print_error_spu (sp, error);

    printf (RED_COL);
    printf ("Stack of numbers:\n");
    print_stack (sp->stk);

    printf ("Stack of return adresses:\n");
    print_stack (sp->ret_adrs);

    printf ("Registers:\n");
    printf ("rax = %d, rbx = %d, rcx = %d, rdx = %d\n",
            sp->rax, sp->rbx, sp->rcx, sp->rdx);

    printf ("Commands list:\n");
    for (int i = 0; i < sp->num_comm; i++)
    {
        if (i < 10)
            printf ("000%d ", i);
        else if (i < 100)
            printf ("00%d ", i);
        else if (i < 1000)
            printf ("0%d ", i);
        else
            printf ("%d ", i);
    }
    printf ("\n");
    for (int i = 0; i < sp->num_comm; i++)
    {
        if ((sp->code)[i] < 10)
            printf ("%d    ", (sp->code)[i]);
        else if ((sp->code)[i] < 100)
            printf ("%d   ", (sp->code)[i]);
        else if ((sp->code)[i] < 1000)
            printf ("%d  ", (sp->code)[i]);
        else
            printf ("%d ", (sp->code)[i]);
    }
    printf ("\n");
    for (int i = 0; i < sp->ip; i++)
        printf ("     ");
    printf ("^\n");

    printf ("Memory:\n");
    for (int i = 0; i < SIZE_MEMORY; i++)
        printf ("[%d] %d ", i, sp->memory[i]);
    printf ("\n");
    printf ("-----------------------------------------\n");
    printf (OFF_COL);
}

bool parse_cmd_args (char file_name_read[], int argc, char* argv[])
{
    if (argc < 2)
    {
        strcpy (file_name_read, BIN_FILE_NAME_READ_DEF);
        return false;
    }
    else
    {
        strcpy (file_name_read, argv[1]);
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
