section .data
    text BYTE 33,32,33,33,10,0
section .code
_start:
    ;move the address of text into A
    MOV A,[text]
    ;put a on the stack (for later)
    PUSHR A
    ;Load 1 byte from the address in A into B
    REA 1,B,A
PCHAR:
    PSTR B ;print char in B register
    ;these 3 essentially adds 1 to the A register because it assumes A is already in the stack
    PUSH 1
    ADDS
    POP
    ;Load 1 byte from the location in a into b
    REA 1,B,A
    ;push A back onto the stack
    PUSHR A
    ;these 2 instructions check if b is 0
    MOV A,0
    SUB B,A
    ;jump if b is B - 0 is zero (meaning b is 0)
    JNZ PCHAR
    ;otherwise halt
    HALT

    ;TODO
    ;MALLOC REG
        ;takes the amount of bytes to malloc from the REG register
        ;puts the address of the newly allocated memory in the REG register

    ;FREE REG
        ;frees the memory at the address stored in REG

    ;TODO
    ;in the ADD instruction, if there is a carry, set FLAG_CARRY
                            ;if there is an overflow, set FLAG_OVERFLOW

    ;TODO
    ;SUM BYTE
        ;pops and sums the entire stack
        ;reads the next byte, if 0, put result into A register
        ;otherwise put result on stack

    ;TODO
    ;COMPL REG
        ;find the 2s compliment of a register, put the result back into the same register
