#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "stack.h"

#include "interpreter.h"

#include "instructions.h"
#include "registers.h"

/*
There is a hack used throughout this file that looks something like
```c
*(uint64_t*)&char[location]
```
what this does is force it to read 8 bytes starting at char[location]
as opposed to reading the byte specifically at char[location]
*/

uint64_t PROGRAM_MEM_START;
uint64_t PROGRAM_MEM_END;

Stack callStack;

// given a pointer,
// this pointer could either be an offset from the start of the code
// of an actual pointer to an arbitrary location in memory
// we dont know which
// this function first assumes that it is an offset and converts it to an actual
// pointer by adding PROGRAM_MEM_START to it next it checks if the address is
// outside the bounds of the program if it is subtract PROGRAM_MEM_START to get
// back the original address This is done with a multiplication of the result of
// the comparison to avoid an if statement

// there may be a better way to handle this but i kind of like this approach
// despite it's complexity it ensures that an offset and a real memory address
// work the same way
uint64_t getMemoryLocation(uint64_t offset) {
  uint64_t real_addr = offset + PROGRAM_MEM_START;
  if (real_addr >= PROGRAM_MEM_END) {
    real_addr -= PROGRAM_MEM_START;
  }
  return real_addr;
}

void setFlagsRegister(uint64_t n){
    uint64_t flags = 0;
    flags |= (n == 0) * FLAG_ZERO;
    REGISTERS[rFLAGS] = flags;
}

void handle_call(char code[], Stack* stack){
    //dont store the next location because after we return the location increases
    stack_push(&callStack, REGISTERS[IP]);
    uint64_t jump;
    stack_pop(stack, &jump);
    REGISTERS[IP] = jump;
}

void handle_revs(char code[], Stack* stack){
    stack_reverse(stack);
}

void handle_ret(char code[], Stack* stack){
    uint64_t jump;
    stack_pop(&callStack, &jump);
    REGISTERS[IP] = jump;
}

void handle_retz(char code[], Stack* stack){
    if((REGISTERS[rFLAGS] & FLAG_ZERO) != 1){
        return;
    }
    return handle_ret(code, stack);
}

void handle_push(char code[], Stack *stack) {
  stack_push(stack, *(uint64_t *)&code[REGISTERS[IP]]);
  REGISTERS[IP] += 8;
}

void handle_pushp(char code[], Stack *stack) {
  uint64_t data = *(uint64_t *)&code[REGISTERS[IP]];
  uint64_t pointer = getMemoryLocation(data);
  stack_push(stack, *(uint64_t *)pointer);
  REGISTERS[IP] += 8;
}

void handle_pushr(char code[], Stack *stack) {
  char reg = code[REGISTERS[IP]++];
  stack_push(stack, REGISTERS[reg]);
}

void handle_rea(char code[], Stack *stack) {
  char byte_count = code[REGISTERS[IP]++];
  char reg1 = code[REGISTERS[IP]++];
  char reg2 = code[REGISTERS[IP]++];
  uint64_t addr = REGISTERS[reg2];
  uint64_t real_addr = getMemoryLocation(addr);
  memcpy(&REGISTERS[reg1], (void *)real_addr, byte_count);
}

void handle_pop(char code[], Stack *stack) { stack_pop(stack, &REGISTERS[A]); }

void handle_sub(char code[], Stack *stack) {
  char reg1 = code[REGISTERS[IP]++];
  char reg2 = code[REGISTERS[IP]++];
  REGISTERS[reg1] -= REGISTERS[reg2];
  setFlagsRegister(REGISTERS[reg1]);
}

void handle_cmp(char code[], Stack* stack){
  char reg1 = code[REGISTERS[IP]++];
  char reg2 = code[REGISTERS[IP]++];
  setFlagsRegister(REGISTERS[reg1] - REGISTERS[reg2]);
}

void handle_mov(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  // hack to move 8 bytes from code[REGISTERS[IP]] to the reg_byte register
  // which is a 8byte int
  uint8_t *reg_start = (uint8_t *)&(REGISTERS[reg_byte]);
  memcpy(reg_start, &code[REGISTERS[IP]], 8);
  REGISTERS[IP] += 8;
}

void handle_movp(char code[], Stack *stack) {
  char size = code[REGISTERS[IP]++];
  char addr_reg = code[REGISTERS[IP]++];
  uint64_t addr = getMemoryLocation(REGISTERS[addr_reg]);
  char reg_byte = code[REGISTERS[IP]++];
  memcpy((void *)addr, &REGISTERS[reg_byte], size);
}

void handle_movr(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  char from_reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  REGISTERS[reg_byte] = REGISTERS[from_reg_byte];
}

void handle_add(char code[], Stack *stack) {
  char result_reg = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  char value_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  switch (value_byte) {
  // read the next byte, get the value at in register, add it to result_reg
  case 1: {
    char from_reg = code[REGISTERS[IP]];
    REGISTERS[result_reg] += REGISTERS[from_reg];
    break;
  }
  // read the next 8 bytes and add it to result_reg
  case 2: {
    REGISTERS[result_reg] = *(uint64_t *)&code[REGISTERS[IP]];
    REGISTERS[IP] += 8;
    break;
  }
  }
  setFlagsRegister(result_reg);
  REGISTERS[IP]++;
}

void handle_adds(char code[], Stack *stack) {
  uint64_t a;
  stack_pop(stack, &a);
  uint64_t b;
  stack_pop(stack, &b);
  stack_push(stack, a + b);
}

void handle_preg(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  printf("%ld\n", REGISTERS[reg_byte]);
}

void handle_pstr(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  char *text = (char *)&(REGISTERS[reg_byte]);
  if (text[3] != '\0') {
    text[3] = '\0';
  }
  printf("%s", text);
}

void handle_goto(char code[], Stack *stack) {
  char location = code[REGISTERS[IP]];
  uint64_t full_location = 0;
  memcpy(&full_location, &location, 8);
  REGISTERS[IP] = full_location;
}

void handle_jz(char code[], Stack * stack) {
  uint64_t location = *(uint64_t *)&code[REGISTERS[IP]];
  if ((REGISTERS[rFLAGS] & FLAG_ZERO) != 0) {
    REGISTERS[IP] = location;
  } else {
    REGISTERS[IP] += 8;
  }
}

void handle_jnz(char code[], Stack * stack) {
  uint64_t location = *(uint64_t *)&code[REGISTERS[IP]];
  if ((REGISTERS[rFLAGS] & FLAG_ZERO) == 0) {
    REGISTERS[IP] = location;
  } else {
    REGISTERS[IP] += 8;
  }
}

void handle_inc(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  REGISTERS[reg_byte]++;
}

void handle_dec(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  REGISTERS[reg_byte]--;
  if (REGISTERS[reg_byte] == 0) {
    REGISTERS[rFLAGS] |= FLAG_ZERO;
  } else if((REGISTERS[rFLAGS] & FLAG_ZERO) != 0){
    REGISTERS[rFLAGS] -= FLAG_ZERO;
  }
}

void handle_malloc(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]++];
  void *ptr = malloc(REGISTERS[reg_byte]);
  REGISTERS[reg_byte] = (uint64_t)ptr;
}

void handle_free(char code[], Stack *stack) {
  char reg_byte = code[REGISTERS[IP]++];
  free((void *)REGISTERS[reg_byte]);
}

void interpret_instruction(char code[], Stack *stack) {
  char instruction = code[REGISTERS[IP]];
  REGISTERS[IP]++;
  switch (instruction) {
  case HALT:
    REGISTERS[rSTATUS] = 1;
    return;
  case PUSH:
    handle_push(code, stack);
    break;
  case PUSHP:
    handle_pushp(code, stack);
    break;
  case PUSHR:
    handle_pushr(code, stack);
    break;
  case POP:
    handle_pop(code, stack);
    break;
  case MOV:
    handle_mov(code, stack);
    break;
  case PREG:
    handle_preg(code, stack);
    break;
  case PSTR:
    handle_pstr(code, stack);
    break;
  case GOTO:
    handle_goto(code, stack);
    break;
  case JZ:
    handle_jz(code, stack);
    break;
  case JNZ:
    handle_jnz(code, stack);
    break;
  case NOP:
  case NOP2:
    break;
  case INC:
    handle_inc(code, stack);
    break;
  case DEC:
    handle_dec(code, stack);
    break;
  case MOVR:
    handle_movr(code, stack);
    break;
  case MOVP:
    handle_movp(code, stack);
    break;
  case ADD:
    handle_add(code, stack);
    break;
  case ADDS:
    handle_adds(code, stack);
    break;
  case SUB:
    handle_sub(code, stack);
    break;
  case REA:
    handle_rea(code, stack);
    break;
  case MALLOC:
    handle_malloc(code, stack);
    break;
  case FREE:
    handle_free(code, stack);
    break;
  case CALL:
    handle_call(code, stack);
    break;
  case RET:
    handle_ret(code, stack);
    break;
  case RETZ:
    handle_retz(code, stack);
    break;
  case REVS:
    handle_revs(code, stack);
    break;
  case CMP:
    handle_cmp(code, stack);
    break;
  default:
    printf("Invalid instruction: %u\n", instruction);
  }
}

void interpret_code(char code[], uint64_t length) {
  Stack stack;

  stack_create(&stack);
  stack_create(&callStack);

  // The first 8 bytes of the code should be a memory address where the entry
  // point is.
  uint64_t *addr = (uint64_t *)&code[0];
  REGISTERS[IP] = *addr;

  PROGRAM_MEM_START = (uint64_t)code;
  PROGRAM_MEM_END = (uint64_t)&code[length];

  do {
    interpret_instruction(code, &stack);
  } while (REGISTERS[rSTATUS] != 1);
}
