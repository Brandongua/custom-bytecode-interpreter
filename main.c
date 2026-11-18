#include "src/stack.h"
#include "src/interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){

    if(argc < 2){
        printf("No file name given\n");
        return 1;
    }

    const char* memblock;
    int fd;
    struct stat sb;

    fd = open(argv[1], O_RDONLY);
    fstat(fd, &sb);

    memblock = mmap(NULL, sb.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);

    if(memblock == MAP_FAILED){
        printf("Could open code\n");
        return 1;
    }


    /* char bytes[] = "\x01\xff\x02\x04\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00"; */

    interpret_code((unsigned char*)memblock, sb.st_size);

    return 0;
}
