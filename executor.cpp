#include <stdio.h>
#include <math.h>

#include "stack.h"
#include "commands.h"

enum Errors_exec
{
    CORRECT =      -1,
    OPEN_FILE_ERR = 1,
    INPUT_NUM_ERR = 2,
    SYNTAX_ERR =    3
};

const char* FILE_NAME_READ = "byte_code.txt";
const int PRECISION = 100;
const int PI = 3.1415926;

Errors_exec execute (const char* name_file_read);
void print_error (Errors_exec error);

int main ()
{
    Errors_exec error = execute (FILE_NAME_READ);
    print_error (error);
    if (error != CORRECT)
        return 1;
    return 0;
}

void print_error (Errors_exec error)
{
    switch (error)
    {
    case OPEN_FILE_ERR:
        printf ("Error is in opening file\n");
        break;
    case INPUT_NUM_ERR:
        printf ("Incorrect input data\n");
        break;
    case SYNTAX_ERR:
        printf ("Error is in syntax\n");
        break;
    case CORRECT:
        printf ("Correct execution\n");
        break;
    default:
        break;
    }
}

Errors_exec execute (const char* name_file_read)
{
    Stack* stk;
    MAKE_STACK(&stk);

    FILE* file_read = fopen (name_file_read, "r");
    if (!file_read)
        return OPEN_FILE_ERR;

    int command = 0;
    int number = 0;
    int number1 = 0;
    double num_double = 0;
    while (true)
    {
        if (fscanf (file_read, "%d", &command) != 1)
        {
            return CORRECT;
            break;
        }
        else
        {
            switch (command)
            {
            case HLT:
                return CORRECT;
                break;
            case OUT:
                stack_pop (stk, &number);
                printf ("%lf\n", (double) number / PRECISION);
                break;
            case IN:
                if (scanf ("%lf", &num_double) == 1)
                    stack_push (stk, (int) (num_double * PRECISION));
                else
                    return INPUT_NUM_ERR;
                break;
            case DIV:
                if (stack_pop (stk, &number) + stack_pop (stk, &number1) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, (int) (((double) number1 / (double) number) * PRECISION));
                break;
            case SUB:
                if (stack_pop (stk, &number) + stack_pop (stk, &number1) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, number1 - number);
                break;
            case MUL:
                if (stack_pop (stk, &number) + stack_pop (stk, &number1) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, number * number1 / PRECISION);
                break;
            case ADD:
                if (stack_pop (stk, &number) + stack_pop (stk, &number1) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, number + number1);
                break;
            case SQRT:
                if (stack_pop (stk, &number) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, (int) (sqrt ((double) number / PRECISION) * PRECISION));
                break;
            case SIN:
                if (stack_pop (stk, &number) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, (int) (sin ((double) number / PRECISION * PI / 180) * PRECISION));
                break;
            case COS:
                if (stack_pop (stk, &number) != 0)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, (int) (cos ((double) number / PRECISION * PI / 180) * PRECISION));
                break;
            case PUSH:
                if (fscanf (file_read, "%d", &number) != 1)
                    return SYNTAX_ERR;
                else
                    stack_push (stk, number * PRECISION);
                break;
            default:
                return SYNTAX_ERR;
                break;
            }
        }
    }
    fclose (file_read);
    return CORRECT;
}
