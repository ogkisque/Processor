#include "stack.h"

#define STACK_DUMP(stk, error) \
        stack_dump (stk, error, __FILE__, __func__, __LINE__);

struct Stack
{
#ifdef CANARY
    Canaryt l_canary;
#endif
    int size_st;
    int capacity;
    Elemt* data;

    const char* name;
    const char* file;
    const char* func;
    int line;
#ifdef HASH
    Hasht hash_st;
#endif
#ifdef CANARY
    Canaryt r_canary;
#endif
};

int stack_verify (Stack* stk);
int stack_ctor (Stack* stk, const char* name, const char* file, const char* func, int line);
int stack_dtor (Stack* stk);
void print_error (int error);
void stack_dump (const Stack* stk, int error, const char* file, const char* func, int line);
bool need_realloc (const Stack* stk);
int stack_realloc (Stack* stk, Actions action);
Elemt* get_elem_point (const Stack* stk, int num);
#ifdef HASH
Hasht get_hash (const Stack* stk);
#endif

int make_stack (Stack** stk, const char* name, const char* file, const char* func, int line)
{
    int error = 0;
    if (stk == NULL)
    {
        error |= MEM_ALLOC_ERR;
        return error;
    }

    *stk = (Stack*) calloc (1, sizeof (Stack));

    if (*stk == NULL)
    {
        error |= MEM_ALLOC_ERR;
        return error;
    }

    error = stack_ctor (*stk, name, file, func, line);
    return error;
}

int delete_stack (Stack** stk)
{
    int error = stack_dtor (*stk);
    free (*stk);
    *stk = NULL;
    return error;
}

int stack_ctor (Stack* stk, const char* name, const char* file, const char* func, int line)
{
    int error = 0;
    if (!(stk && name && file && func))
    {
        error |= NULL_POINTER;
        return error;
    }

    stk->capacity = CAPACITY_START;
    stk->size_st = 0;

#ifndef CANARY
    stk->data = (Elemt*) calloc (CAPACITY_START, sizeof (Elemt));
#else
    stk->l_canary = REF_VAL_CAN;
    stk->r_canary = REF_VAL_CAN;
    stk->data = (Elemt*) calloc (sizeof (Canaryt) * 2 + CAPACITY_START * sizeof (Elemt), 1);
    ((Canaryt*) (stk->data))[0] = REF_VAL_CAN;
    ((Canaryt*) (get_elem_point (stk, CAPACITY_START)))[0] = REF_VAL_CAN;
#endif
    stk->name = name;
    stk->file = file;
    stk->func = func;
    stk->line = line;
    for (int i = 0; i < stk->capacity; i++)
        (get_elem_point (stk, i))[0] = INT_MAX;
#ifdef HASH
    stk->hash_st = get_hash (stk);
#endif
    error = stack_verify (stk);
    if (error != 0)
    {
        STACK_DUMP(stk, error);
        return error;
    }

    return error;
}

int stack_dtor (Stack* stk)
{
    int error = 0;
    if (!stk)
    {
        error |= NULL_POINTER;
        return error;
    }

#ifdef CANARY
    stk->l_canary = 0;
    stk->r_canary = 0;
    ((Canaryt*)stk->data)[0] = 0;
    ((Canaryt*) get_elem_point (stk, stk->capacity))[0] = 0;
#endif

    stk->size_st = -1;
    stk->capacity = -1;
    stk->name = NULL;
    stk->func = NULL;
    stk->file = NULL;
    stk->line = -1;
    for (int i = 0; i < (stk->size_st); i++)
            *get_elem_point (stk, i) = INT_MAX;
    free (stk->data);
    stk->data = NULL;

    return error;
}

int stack_push (Stack* stk, Elemt value)
{
    int error = stack_verify (stk);
    if (error != 0)
    {
        STACK_DUMP(stk, error);
        return error;
    }

    if (need_realloc (stk))
    {
        error = stack_realloc (stk, EXPAND);
        if (error != 0)
        {
            STACK_DUMP(stk, error);
            return error;
        }
    }

    (get_elem_point (stk, stk->size_st))[0] = value;
    (stk->size_st)++;
#ifdef HASH
    stk->hash_st = get_hash (stk);
#endif
    return error;
}

int stack_pop (Stack* stk, Elemt* value)
{
    int error = stack_verify (stk);
    if (error != 0)
    {
        STACK_DUMP(stk, error);
        return error;
    }

    if (!value)
    {
        error |= MEM_ALLOC_ERR;
        return error;
    }

    if ((stk->size_st) == 0)
    {
        printf ("Stack is empty. Unable to perform pop\n");
        *value = INT_MAX;
        error |= EMPTY_STACK;
        return error;
    }

    *value = (get_elem_point (stk, stk->size_st - 1))[0];
    (get_elem_point (stk, stk->size_st - 1))[0] = INT_MAX;
    (stk->size_st)--;

    if (need_realloc (stk))
    {
        error = stack_realloc (stk, REDUCE);
        if (error != 0)
        {
            STACK_DUMP(stk, error);
            return error;
        }
    }
#ifdef HASH
    stk->hash_st = get_hash (stk);
#endif
    return error;
}

int stack_verify (Stack* stk)
{
    int error = 0;
    if (!(stk && (stk->data) && (stk->name) && (stk->file) && (stk->func)))
    {
        error |= NULL_POINTER;
        return error;
    }
    if ((stk->size_st) < 0)                                                   error |= NEGATIVE_SIZE;
    if ((stk->capacity) < 0)                                                  error |= NEGATIVE_CAPACITY;
    if ((stk->capacity) < (stk->size_st))                                     error |= SIZE_MORE_CAPACITY;
    for (int i = 0; i < (stk->size_st); i++)
            if ((*get_elem_point (stk, i)) == INT_MAX)                        error |= RUBBISH;
#ifdef CANARY
    if ((stk->l_canary) != REF_VAL_CAN)                                       error |= L_CANARY_ERR;
    if ((stk->r_canary) != REF_VAL_CAN)                                       error |= R_CANARY_ERR;
    if (((Canaryt*) (stk->data))[0] != REF_VAL_CAN)                           error |= L_CANARY_DATA_ERR;
    if (((Canaryt*) (get_elem_point (stk, stk->capacity)))[0] != REF_VAL_CAN) error |= R_CANARY_DATA_ERR;
#endif

#ifdef HASH
    Hasht prev_hash = stk->hash_st;
    Hasht new_hash = get_hash (stk);
    stk->hash_st = new_hash;
    if (prev_hash != new_hash)                                                error |= HASH_ERR;
#endif

    return error;
}

void print_error (int error)
{
    printf (BOLD_RED_COL);
    if (error & NULL_POINTER)
        printf ("Null pointer\n");
    if (error & MEM_ALLOC_ERR)
        printf ("Error of memory allocation\n");
    if (error & NEGATIVE_SIZE)
        printf ("Size of stack is negative\n");
    if (error & NEGATIVE_CAPACITY)
        printf ("Size of capacity is negative\n");
    if (error & SIZE_MORE_CAPACITY)
        printf ("Size is more than capacity\n");
    if (error & RUBBISH)
        printf ("This stack contains rubbish values\n");
    if (error & EMPTY_STACK)
        printf ("This stack is empty\n");

#ifdef CANARY
    if (error & L_CANARY_ERR)
        printf ("Left canary is incorrect\n");
    if (error & R_CANARY_ERR)
        printf ("Right canary is incorrect\n");
    if (error & L_CANARY_DATA_ERR)
        printf ("Left canary in data is incorrect\n");
    if (error & R_CANARY_DATA_ERR)
            printf ("Right canary in data is incorrect\n");
#endif

#ifdef HASH
    if (error & HASH_ERR)
        printf ("Hash is incorrect\n");
#endif

    printf (OFF_COL);
}

void stack_dump (const Stack* stk, int error, const char* file, const char* func, int line)
{
    if (!stk)
    {
        printf ("Null pointer of stack\n");
        return;
    }

    printf (RED_COL "Error is in file %s, function %s, line %d in stack %s.\n", stk->file, stk->func, stk->line, stk->name);
    printf ("Called from file %s, function %s, line %d.\n", file, func, line);
    print_error (error);
    printf (RED_COL);

#ifdef CANARY
    printf ("Left canary if 0x%llX\n", stk->l_canary);
    printf ("Right canary if 0x%llX\n", stk->r_canary);
#endif

#ifdef HASH
   printf ("Hash is %llu\n", stk->hash_st);
#endif

    printf ("Size     of stack is %d\n"
            "Capacity of stack is %d\n",
            stk->size_st,
            stk->capacity);

    if (!(stk->data))
    {
        printf ("Null pointer of data in stack\n" OFF_COL);
        return;
    }

#ifdef CANARY
    printf ("Left canary in data is 0x%llX\n", ((Canaryt*) stk->data)[0]);
    printf ("Right canary in data is 0x%llX\n", ((Canaryt*) (get_elem_point (stk, stk->capacity)))[0]);
#endif

    for (int i = 0; i < stk->capacity; i++)
    {
        if (i < stk->size_st)
            printf ("*");
        else
            printf (" ");
        printf ("[%d] = %d\n", i, (get_elem_point (stk, i))[0]);
    }
    printf (OFF_COL);
}

bool need_realloc (const Stack* stk)
{
    return (((stk->size_st) < ((stk->capacity) / COEFF_ALLOC / COEFF_ALLOC)) && (stk->capacity > 8)) || (stk->size_st == stk->capacity);
}

int stack_realloc (Stack* stk, Actions action)
{
    int error = 0;
    int capacity_start = stk->capacity;
    Elemt* data_start = stk->data;

    if (action == EXPAND)
        stk->capacity = (stk->capacity) * COEFF_ALLOC;
    else
        stk->capacity = (stk->capacity) / COEFF_ALLOC;

    int new_size = 0;
#ifdef CANARY
    new_size = 2 * sizeof (Canaryt) + stk->capacity * sizeof (Elemt);
#else
    new_size = stk->capacity * sizeof (Elemt);
#endif

    stk->data = (Elemt*) realloc (stk->data, new_size);
    if (!(stk->data))
    {
        stk->data = data_start;
        stk->capacity = capacity_start;
        error |= MEM_ALLOC_ERR;
            return error;
    }

#ifdef CANARY
    ((Canaryt*) (get_elem_point (stk, stk->capacity)))[0] = REF_VAL_CAN;
#endif

    for (int i = stk->size_st; i < stk->capacity; i++)
        (get_elem_point (stk, i))[0] = INT_MAX;
#ifdef HASH
    stk->hash_st = get_hash (stk);
#endif

    return error;
}

int print_stack (Stack* stk)
{
    int error = stack_verify (stk);
    if (error != 0)
    {
        STACK_DUMP(stk, error);
        return error;
    }

    printf ("Size - %d, capacity - %d\n", stk->size_st, stk->capacity);

    if (stk->size_st > 0)
    {
        for (int i = 0; i < stk->size_st; i++)
            printf ("%d ", (get_elem_point (stk, i))[0]);
        printf ("\n");
    }

    return error;
}

Elemt* get_elem_point (const Stack* stk, int num)
{
#ifdef CANARY
    return (Elemt*) ((char*) (stk->data) + sizeof (Canaryt) + num * sizeof (Elemt));
#else
    return &(stk->data[num]);
#endif
}

#ifdef HASH
Hasht get_hash (const Stack* stk)
{
    Hasht hash_st = 5381;
    for (char* i = (char*) &(stk->size_st); i < ((char*) &(stk->size_st) + sizeof (stk->size_st)); i++)
        hash_st = hash_st * 33 + *i;
    for (char* i = (char*) &(stk->capacity); i < ((char*) &(stk->capacity) + sizeof (stk->capacity)); i++)
        hash_st = hash_st * 33 + *i;
#ifndef CANARY
    for (int i = 0; i < stk->size_st; i++)
        for (char* j = (char*) &(stk->data[i]); j < (char*) (&(stk->data[i]) + sizeof (Elemt)); j++)
            hash_st = hash_st * 33 + *j;
#else
    for (char* i = (char*) ((char*) stk->data + sizeof (Canaryt)); i < ((char*) get_elem_point (stk, stk->size_st)); i++)
        hash_st = hash_st * 33 + *i;
#endif
    return hash_st;
}
#endif
