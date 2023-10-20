#define DO_PUSH(num) stack_push (sp->stk, num)
#define DO_POP(num_p) stack_pop (sp->stk, num_p)
#define MAKE_CMD_JUMP(name, num, type_cmp)                                  \
        DEF_CMD(name, num, LABEL,                                           \
                {                                                           \
                    int err1 = DO_POP(&number);                             \
                    int err2 = DO_POP(&number1);                            \
                    if (err1 + err2 != 0)                                   \
                        RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");   \
                    else                                                    \
                        if (number1 type_cmp number)                        \
                            sp->ip = (sp->code)[sp->ip + 1] - 1;            \
                        else                                                \
                            (sp->ip)++;                                     \
                })

DEF_CMD(hlt,  0,  NO_ARG,
        {
            RETURN_ERROR(CORRECT, "");
        })

DEF_CMD(push, 1,  NUM_OR_REG_ARG,
        {
            (sp->ip)++;
            if (command & BIT_IMM_CONST)
            {
                number = (sp->code)[sp->ip];
            }
            else if (command & BIT_REGISTER)
            {
                number = (sp->code)[sp->ip];
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
            }
            else
            {
                RETURN_ERROR(SYNTAX_ERR, "Error in syntax");
            }
            DO_PUSH(number);
        })

DEF_CMD(div,  2,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            int err2 = DO_POP(&number1);
            if (err1 + err2 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH((int) (((double) number1 / (double) number) * PRECISION));
        })

DEF_CMD(sub,  3,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            int err2 = DO_POP(&number1);
            if (err1 + err2 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH(number1 - number);
        })

DEF_CMD(mul,  4,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            int err2 = DO_POP(&number1);
            if (err1 + err2 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH(number * number1 / PRECISION);
        })

DEF_CMD(add,  5,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            int err2 = DO_POP(&number1);
            if (err1 + err2 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH(number + number1);
        })

DEF_CMD(sqrt, 6,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            if (err1 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH((int) (sqrt ((double) number / PRECISION) * PRECISION));
        })

DEF_CMD(sin,  7,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            if (err1 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH((int) (sin ((double) number / PRECISION * PI / 180) * PRECISION));
        })

DEF_CMD(cos,  8,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            if (err1 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                DO_PUSH((int) (cos ((double) number / PRECISION * PI / 180) * PRECISION));
        })

DEF_CMD(out,  9,  NO_ARG,
        {
            int err1 = DO_POP(&number);
            if (err1 != 0)
                RETURN_ERROR(SYNTAX_ERR, "Pop from empty stack");
            else
                printf ("%.2lf\n", (double) number / PRECISION);
        })

DEF_CMD(in,   10, NO_ARG,
        {
            printf ("Enter the number: ");
            if (scanf ("%lf", &num_double) == 1)
                DO_PUSH((int) (num_double * PRECISION));
            else
                RETURN_ERROR(INPUT_NUM_ERR, "Error with input data");
        })

DEF_CMD(pop,  11, REG_ARG,
        {
            (sp->ip)++;
            if (command & BIT_REGISTER)
            {
                number = (sp->code)[sp->ip];
                int tmp = 0;
                if (DO_POP(&tmp) != 0)
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
        })

DEF_CMD(jmp, 12, LABEL,
        {
            sp->ip = (sp->code)[sp->ip + 1] - 1;
        })

MAKE_CMD_JUMP(ja,  13, >)
MAKE_CMD_JUMP(jae, 14, >=)
MAKE_CMD_JUMP(jb,  15, <)
MAKE_CMD_JUMP(jbe, 16, <=)
MAKE_CMD_JUMP(je,  17, ==)
MAKE_CMD_JUMP(jne, 18, !=)
