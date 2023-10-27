#ifndef EXECUTOR_HEADER
#define EXECUTOR_HEADER

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

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
    int         err_code;
    int         err_line;
    const char* err_file;
    const char* err_func;
    const char* err_message;
};

struct Spu
{
    Stack*      stk;
    Stack*      ret_adrs;
    int         rax;
    int         rbx;
    int         rcx;
    int         rdx;
    int         num_comm;
    int*        code;
    int         ip;
    int*        memory;

    const char* name;
    const char* file;
    const char* func;
    int         line;
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

const char* FILE_NAME_READ_DEF =        "byte_code.txt";
const char* BIN_FILE_NAME_READ_DEF =    "byte_code.bin";
const double PI =                       3.1415926;

#ifdef TXT_BYTE_CODE
Error read_txt_byte_code (char* file_name_read, Spu* sp);
#endif
Error read_bin_byte_code (char* file_name_read, Spu* sp);
Error execute (Spu* sp);
void print_error_spu (Spu* sp, Error error);
Error spu_ctor (Spu* sp, int num_commands, const char* name, const char* file, const char* func, int line);
Error spu_dtor (Spu* sp);
Error spu_verify (Spu* sp);
Error check_header (File_Header* header, Spu* sp);
void spu_dump (Spu* sp, Error error);
bool parse_cmd_args (char file_name_read[], int argc, char* argv[]);
bool check_type_file (char file_name_read[]);

#endif //EXECUTOR_HEADER
