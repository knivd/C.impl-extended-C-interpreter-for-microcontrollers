#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include "xmem.h"

/* memory block header structure */
typedef struct {
	byte *data;		/* pointer to the allocated data block */
	int32_t len;	/* length of the allocated block; negative means free with the absolute size */
} xhdr_t;

/* block headers */
static byte *dcur;		/* points to the first unallocated data byte */
static xhdr_t *hcur;	/* points to the current last header */

/* dynamic memory array */
byte *pMEMORY;
unsigned long xmem_bytes;

byte defrag_cnt = 0;


size_t x_meminit(void) {
    xmem_bytes = (1024ul * MEMORY_SIZE_KB);
    pMEMORY = MEMORY;
    memset((byte *) pMEMORY, 0, xmem_bytes);
	dcur = (byte *) pMEMORY;
	hcur = (xhdr_t *) (pMEMORY + xmem_bytes);
    return (size_t) xmem_bytes;
}


/* internal function */
/* find header based on data pointer; return NULL if the header can not be found */
xhdr_t *findxhdr(byte *addr) {
	if(addr) {
		xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
		while(--h >= hcur) {
			if(h->data == addr) return h;
		}
	}
	return NULL;   /* unknown header (memory leak?) */
}


size_t x_blksize(byte *v) {
	xhdr_t *h = findxhdr(v);
	return (h ? abs(h->len) : 0);
}


size_t x_avail(void) {
	size_t t = (byte *) hcur - dcur;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->len < 0 && (size_t) -h->len > t) t = -h->len;
	}
	return t;
}


size_t x_total(void) {
	size_t t = (byte *) hcur - dcur;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->len < 0) t += (-h->len);
	}
	return t;
}


void x_list_alloc(void) {
	size_t used = 0;
	xhdr_t *p, *t, *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->len > 0) used += h->len;
	}
	printf("\r\nalloc: %u,  unalloc: %u,  largavl: %u,  totavl: %u\r\n",
						(unsigned int) used, (unsigned int ) ((byte *) hcur - dcur),
						(unsigned int) x_avail(), (unsigned int) x_total());
	unsigned int c = 0;
	h = findxhdr(pMEMORY);
	if(!h) return;
	do {
		printf("%3u: h:%8p,  d:%8p..%8p,  l:%6i\r\n",
			  ++c,
			  (void *) h,
			  (void *) h->data,
			  (void *) (h->data + abs(h->len) - 1),
			  h->len
		);
		p = NULL;
		t = (xhdr_t *) (pMEMORY + xmem_bytes);
		while(--t >= hcur) {	/* find the next block immediately after (h->data) */
			if(t->data > h->data && (!p || t->data < p->data)) p = t;
		}
		if(p && (h->data + abs(h->len)) != p->data) {	/* memory leak! */
			printf("%3u: ..........,  d:%8p..%8p,  l:%6u\r\n",
				  ++c,
				  (void *) (h->data + abs(h->len)),
				  (void *) (p->data - 1),
				  (unsigned int) (p->data - (h->data + abs(h->len)))
			);
		}
		h = p;
	} while(h);
}


/* internal function */
/* optimise the memory and try to free up a block with size (sz) or more */
void x_defrag(size_t sz) {
	xhdr_t *t, *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->len < 0) {
			t = (xhdr_t *) (pMEMORY + xmem_bytes);
			while(--t >= hcur) {			/* search for the next block immediately after (h->data) */
				if(t->data == (h->data - h->len)) {	/* h->len is negative here */
					if(t->len >= 0) break;	/* used block */
					h->len += t->len; 		/* both h->len and t->len are negative */
					t->len = 0;
					if((size_t) -h->len >= sz) break;       /* achieved the goal */
					t = (xhdr_t *) (pMEMORY + xmem_bytes);	/* restart the inner loop */
				}
			}
			if((size_t) -h->len >= sz) break;	/* achieved the goal */
		}
	}
	if(h->len < 0 && (h->data - h->len) == dcur) { h->len = 0; dcur += h->len; }	/* unallocate the last block */
	h = t = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--t >= hcur) {	/* remove the marked blocks */
		if(t->len) memcpy(--h, t, sizeof(xhdr_t));
	}
	hcur = h;
}


int x_free(byte **var) {
    if(!var || (*var == NULL)) return 0;
	if((*var < pMEMORY) || (*var > (pMEMORY + xmem_bytes))) return -1;
	xhdr_t *h = findxhdr(*var);
	if(!h) return -1;
	h->len = -h->len;	/* mark the block as free */
	*var = NULL;
    if((++defrag_cnt % 50) == 0) x_defrag((size_t) -1); /* run memory defragmentation from time to time */
	return 0;
}


void *x_malloc(byte **var, size_t sz) {
	if(!var || !sz || sz > INT32_MAX) { x_free(var); return NULL; }	/* size zero is a request to release the block */
	if(*var && (*var < pMEMORY || *var > (pMEMORY + xmem_bytes))) return NULL;
	sz = ((sz >> 4) + !!(sz & 0x0F)) << 4;      /* round the size up to 16-byte boundary */
	byte attempts = 2;
	retry:;
	xhdr_t *z = findxhdr(*var);	/* possible outcomes:
									(!*var && !z) - a new block to be allocated
									(!*var && z)  - impossible to happen
									(*var && !z)  - error
									(*var && z)   - resize an existing data block */
	if(*var && !z) return NULL;	/* a data block is supplied but not recognised */
	if(z) {
		if(sz == (size_t) z->len) return z->data;	/* nothing is needed to do */
		if((z->data + z->len) == dcur) {	/* luckily it happens that (z) is the last allocated block */
			if(sz > (size_t) z->len) memset((z->data + z->len), 0, (sz - z->len));	/* clear the expansion part */
			dcur = z->data + sz;
			z->len = sz;
			return z->data;
		}
	}
	byte *dptr = dcur;
	xhdr_t *h = NULL;			/* this will be the header with which we work */
	xhdr_t *t = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--t >= hcur) {		/* find the smallest free block able accommodate (sz) bytes */
		if(t->len < 0 && (size_t) -t->len >= sz && (!h || t->len > h->len)) {
			h = t;
			if((size_t) -h->len == sz) break;
		}
	}
	if(h) {	/* a suitable free block with length >= sz is found */
		if((size_t) -h->len == sz) dptr = h->data;	/* an exact match only requires assigning the variable to the block */
		else {	/* not an exact match; try to split the block */
			if(dcur > (byte *) (hcur - 1)) x_defrag((size_t) -1);   /* no room for a new header... try optimising */
			dptr = h->data;
			if(dcur <= (byte *) (hcur - 1)) {	/* create a new header for the carved off part */
				h->data += sz;
				h->len += sz;	/* h->len is negative here */
				h = --hcur;
			}
			else sz = -h->len;	/* last chance - use the block as it is and without resizing */
		}
	}
	else {	/* a new block will have to be allocated */
        if(attempts == 2) { x_defrag(sz); attempts--; goto retry; }
		if((dcur + sz) > (byte *) (hcur - 1)) {
			if(attempts == 1) { x_defrag((size_t) -1); attempts--; goto retry; }
			return NULL;	/* ran out of options */
		}
		dcur += sz;
		h = --hcur;
	}
	h->len = sz;
	if(h->data != dptr) h->data = dptr;
	*var = dptr;
	if(z) {		/* transfer the old block to the new place */
		if(h->len > z->len) {
			memcpy(h->data, z->data, z->len);
			memset((h->data + z->len), 0, (h->len - z->len));	/* clear the rest of the block */
		}
		else memcpy(h->data, z->data, h->len);
		z->len = -z->len;	/* mark the old block as free */
	}
	else memset(h->data, 0, h->len);
	return h->data;
}
