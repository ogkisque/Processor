#ifndef COMPILER_HEADER
#define COMPILER_HEADER

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

const int REALLOC_STEP =                2;
const char* FILE_NAME_READ_DEF =        "asm.txt";
const char* FILE_NAME_PRINT_DEF =       "byte_code.txt";
const char* BIN_FILE_NAME_PRINT_DEF =   "byte_code.bin";

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
Error parse_mem_oper_arg (char arg[],
                          Commands_Arr* commands_struct,
                          Arg_Types* arg_type_real);
bool parse_register_arg (char arg[],
                         Commands_Arr* commands_struct,
                         Arg_Types* arg_type_real);
bool parse_num_arg (char arg[],
                    Commands_Arr* commands_struct,
                    Arg_Types* arg_type_real);
void parse_label_arg (char arg[],
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
void del_bracket (char* str);

#endif //COMPILER_HEADER
