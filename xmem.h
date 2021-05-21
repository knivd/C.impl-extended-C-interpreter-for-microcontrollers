#ifndef XMEM_H
#define XMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MEMORY_SIZE_KB
#error MEMORY_SIZE_KB definition is required with total dynamic memory size in kB
#endif

typedef unsigned char byte;

/* global MEMORY[] array */
__attribute__ ((aligned(8))) byte MEMORY[1024ul * MEMORY_SIZE_KB];

#include <stdlib.h> /* needed for the size_t definition */

/* initialise or reinitialise the entire memory */
/* needs to be called initially before referring to any other xmem function */
/* will return the size of the memory */
size_t x_meminit(void);

/* allocate block or change size of already allocated one */
/*
    when calling for initial allocation the variable must be previously initialised with NULL
    the new memory block is cleared
    if the block is being extended, the extension area is cleared
    will update the supplied variable with the pointer, or NULL in case of unsuccessful allocation
	The function usually returns pointer to the memory block with the exception of cases when a
	block is already allocated and needs changing its size. If unsuccessful, the function will
	return NULL but the original block will not be affected
*/
void *x_malloc(byte **var, size_t sz);

/* free allocated block */
/*
    will do nothing if the parameter doesn't point to a valid allocated block
    will update the variable with NULL
	return 0 if successful, or -1 for error
*/
int x_free(byte **var);

/* return the actual size of an allocated block */
size_t x_blksize(byte *v);

/* return the size of the largest continuous currently available block */
size_t x_avail(void);

/* return the total size of the currently available memory (could be fragmented in many separate blocks) */
size_t x_total(void);

/* list all currently allocated blocks (FOR DEBUG) */
void x_list_alloc(void);

#ifdef __cplusplus
}
#endif

#endif /* XMEM_H */

