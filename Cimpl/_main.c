#include <stdlib.h>
#include <stdio.h>
#include "cimpl.h"

/*
main entry point
loads a program source and executes the Cimpl() interpreter
used mainly for testing as it has no relevance to the actual interpreter
*/
int main(int argc, char *argv[]) {
    int r = 0;
    if(argc) {
        FILE *f = fopen(argv[1], "rb");
        if(f) {
            long int flen;
            fseek(f, 0, SEEK_END);
            flen = ftell(f);
            if(flen > 0) {
                char *p = malloc(flen+1);
                if(p) {
                    rewind(f);
                    fread(p, 1, flen, f);
                    fclose(f);
                    *(p + flen) = '\0'; /* ensure proper source termination */
                    r = Cimpl(p);
                }
                else printf("Memory allocation failure\r\n");
            }
            fclose(f);
        }
        else printf("Unable to open the input file\r\n");
    }
    return r;
}
