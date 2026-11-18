#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int maxLen;
    int len;
    uint64_t* items;
} Stack;

void stack_push(Stack* stack, uint64_t item);
//returns false if the stack was empty
//0 will be put in out
bool stack_pop(Stack* stack, uint64_t* out);
//returns false if the stack was empty
bool stack_peek(Stack* stack, uint64_t* out);

void stack_create(Stack* out);
void stack_delete(Stack*);

int stack_len(Stack*);

void stack_reverse(Stack*);
