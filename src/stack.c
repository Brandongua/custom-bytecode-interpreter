#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void stack_push(Stack *stack, uint64_t item){
    if(stack->len >= stack->maxLen){
        stack->items = realloc(stack->items, sizeof(uint64_t) * (stack->maxLen * 2));
        stack->maxLen *= 2;
    }

    stack->items[stack->len] = item;
    stack->len++;
}

bool stack_pop(Stack *stack, uint64_t *out){
    //this puts 0 in out for us
    if(!stack_peek(stack, out)){
        return false;
    }
    stack->len--;
    return true;
}

bool stack_peek(Stack *stack, uint64_t *out){
    if(stack->len == 0){
        *out = 0;
        return false;
    }

    *out = stack->items[stack->len - 1];
    return true;
}

void stack_create(Stack *out){
    out->len = 0;
    out->maxLen = 4;
    out->items = malloc(sizeof(uint64_t) * out->maxLen);
}

void stack_delete(Stack* s){
    free(s->items);
}

int stack_len(Stack * stack){
    return stack->len;
}

//O(n)
void stack_reverse(Stack * stack){
    int stackLen = stack->len;
    int lastItem = stackLen - 1;
    for(int i = 0; i < stack->len / 2; i++){
        uint64_t temp = stack->items[i];
        stack->items[i] = stack->items[lastItem - i];
        stack->items[lastItem - i] = temp;
    }
}
