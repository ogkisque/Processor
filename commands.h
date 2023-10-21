#ifndef COMMANDS_HEADER
#define COMMANDS_HEADER

#define TXT_BYTE_CODE

const int BIT_IMM_CONST = 1 << 8;
const int BIT_REGISTER = 1 << 9;
const char SIGNATURE[] = "MEGERA";
const int VERSION = 3;
const int MAX_NAME_LEN = 40;
const int PRECISION = 100;
const int MAX_COMMAND_LEN = 20;
const int CODE_COMMAND_MASK = 0x00FF;
const int NUM_OF_COMMANDS = 21;
const int NUM_OF_LABELS = 20;

enum Arg_Types
{
    NO_ARG =         1 << 0,
    NUM_ARG =        1 << 1,
    REG_ARG =        1 << 2,
    NUM_OR_REG_ARG = (1 << 1 | 1 << 2),
    LABEL =          1 << 3
};

struct File_Header
{
    char signature[sizeof (SIGNATURE)];
    int version;
    int num_commands;
};

struct Command
{
    int code;
    const char* name;
    Arg_Types arg_type;
};

#define DEF_CMD(name, num, args, ...) \
        {num, #name, args},

const Command COMMANDS_LIST[] = {
    #include "code_generate.h"
};

#undef DEF_CMD

#define DEF_CMD(name, num, ...) \
        CMD_##name = num,

enum Commands
{
    #include "code_generate.h"
};

#undef DEF_CMD

#endif //COMMANDS_HEADER
