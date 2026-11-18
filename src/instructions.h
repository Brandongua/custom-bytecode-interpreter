#define HALT 0
//takes the next 8 bytes and pushes it on the stack
#define PUSH 1
//takes the next 8 bytes as a memory address, pushes the value at that address
#define PUSHP 12
//pops the top item on the stack and put it into A
#define POP  2
//Reverses the order of the stack
#define REVS 25
//takes the next byte as a register byte, then the next 8 bytes as the data and puts it into the specified register
#define MOV  3
//move a register to another register
#define MOVR 8
//reads the first byte as the size of where the register points
//reads the next byte as a register with the memory address in it
//reads the next byte as a register with the data to put at the memory address
#define MOVP 21
//prints the data in the register specified by the next byte as a long int
#define PREG 4
//prints the data in the register specified by the next byte as a string
#define PSTR 5

//reads the next 8 bytes and jumps to that location in the code
#define GOTO 6

//reads the next byte as a register, increment by 1
#define INC 7
//reads the next byte as a register, decrement by 1, sets the 0 flag
#define DEC 13

//jumps to the specified address if FLAGS_ZERO is set
#define JZ 14
//jumps to the specified address if FLAGS_ZERO is not set
#define JNZ 18

//subtracts the 2nd register from the first, sets the 0 flag
#define SUB 15

//pushes the contents of a register on the stack
#define PUSHR 16

//Reads N bytes from the address from a register and puts it into another register
//eg: REA 8,A,B ;read 8 bytes starting at address B and put it into A
#define REA 17

//alloc the amount of bytes specified in the provided in the given register
//then put the memory address back into that register
#define MALLOC 19
//free the memory at the location provided in the given register
#define FREE 20

//pushes the location onto the internal call stack
//pops from the stack, jumps to the location provided
//when the RET instruction is encountered
//  pop from the internal call stack
//  jump to that location
#define CALL 22
//pop from the internal call stack
//jump to that location
#define RET 23
//if the ZERO_FLAG is set acts the same as return
#define RETZ 24

//compares 2 registers
//sets the flags register
#define CMP 26

//TODO:
//CMPI compare a register to an immediate value

//reads the next byte
//  This is the register the result is put into
//reads the next byte
//  if the next byte is 0x01 it goes to the next byte and gets the contents of that register
//  if the next byte is 0x02 it treats the next 8 bytes as an integer
//adds this value to the result register
#define ADD 9

//Pops 2 items off the stack, adds them, then puts the result on the stack
#define ADDS 11

#define NOP 0x20
#define NOP2 '\n'

//to solve the problem where an instruction is 1 byte
//this instruction says to read the next byte as the instruction
//that way there can be an infinite set of instructions
//as chaining these together increases the "instruction class"
//This should probably not be used by the programmer directly although they could theoretically do it
#define NEXTCLASS 255
