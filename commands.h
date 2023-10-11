#ifndef COMMANDS_HEADER
#define COMMANDS_HEADER

#define TXT_BYTE_CODE

enum Arg_Types
{
    NO_ARG =         0,
    NUM_ARG =        1,
    REG_ARG =        2,
    NUM_OR_REG_ARG = 3
};

struct Command
{
    int code;
    const char* name;
    int arg_type;
};

const Command COMMANDS_LIST[] = {
            {-1, "hlt", NO_ARG},
            {1, "push", NUM_OR_REG_ARG},
            {2, "div", NO_ARG},
            {3, "sub", NO_ARG},
            {4, "mul", NO_ARG},
            {5, "add", NO_ARG},
            {6, "sqrt", NO_ARG},
            {7, "sin", NO_ARG},
            {8, "cos", NO_ARG},
            {9, "out", NO_ARG},
            {10, "in", NO_ARG},
            {11, "pop", REG_ARG}
};

const int BIT_IMM_CONST = 1 << 4;
const int BIT_REGISTER = 1 << 5;
const char SIGNATURE[] = "MEGERA";
const int VERSION = 2;
const int MAX_NAME_LEN = 40;
const int PRECISION = 100;
const int MAX_COMMAND_LEN = 20;
const int CODE_COMMAND_MASK = 0x0F;
const int NUM_OF_COMMANDS = 12;

#endif //COMMANDS_HEADER
