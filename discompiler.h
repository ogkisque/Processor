#ifndef DISCOMPILER_HEADER
#define DISCOMPILER_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"

struct Commands_Arr
{
    int* commands;
    int num_commands;
    int capacity;
    int str_asm;
};

const char* FILE_NAME_READ_DEF =        "byte_code.txt";
const char* BIN_FILE_NAME_READ_DEF =    "byte_code.bin";
const char* FILE_NAME_PRINT_DEF =       "asm_discomp.txt";

#ifdef TXT_BYTE_CODE
void read_txt_byte_code (char* file_name_read, Commands_Arr* commands);
#endif
void read_bin_byte_code (char* file_name_read, Commands_Arr* commands);
void discompile (char* file_name_print, Commands_Arr* commands);
bool parse_cmd_args (char file_name_read[], char file_name_print[], int argc, char* argv[]);
bool check_type_file (char file_name_read[]);

#endif //DISCOMPILER_HEADER
