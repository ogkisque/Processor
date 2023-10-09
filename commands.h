#ifndef COMMANDS_HEADER
#define COMMANDS_HEADER

enum Commands
{
    HLT =   -1,
    PUSH =   1,
    DIV =    2,
    SUB =    3,
    MUL =    4,
    ADD =    5,
    SQRT =   6,
    SIN =    7,
    COS =    8,
    OUT =    9,
    IN =     10,
    POP =    11
};

const int BIT_IMM_CONST = 1 << 4;
const int BIT_REGISTER = 1 << 5;
const char* SIGNATURE = "MEGERA";
const int VERSION = 1;
const int MAX_NAME_LEN = 40;
const int PRECISION = 100;
const int MAX_COMMAND_LEN = 20;
const int CODE_COMMAND_MASK = 0x0F;

#endif //COMMANDS_HEADER
