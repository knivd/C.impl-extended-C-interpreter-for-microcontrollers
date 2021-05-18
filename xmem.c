#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "xmem.h"

/* memory block header structure */
typedef struct {
    byte **cv;     /* C variable (NULL if the block is free) */
	size_t len;    /* length of the allocated block */
	size_t need;   /* actually needed size for the block (smaller or equal to .len) */
	byte *data;    /* pointer to the allocated data block */
} xhdr_t;

/* block headers */
static byte *dcur;
static xhdr_t *hcur;

/* dynamic memory array */
byte *pMEMORY;
unsigned long xmem_bytes;


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
xhdr_t *findxhdr(byte *da) {
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(da == h->data) return h;
	}
	return NULL;   /* unknown header (memory leak?) */
}


/* internal function */
/* try to find a free data block with suitable size among the already allocated headers */
/* return NULL if no such block exist */
xhdr_t *findxavl(size_t sz) {
	xhdr_t *hbest = NULL;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {   /* search among the currently allocated but unused blocks */
		if(h->cv == NULL && h->len >= sz) {
			if(hbest) {
				if((sz - h->len) < (sz - hbest->len)) hbest = h;   /* find smallest free block with size bigger than (sz) */
				if(sz == hbest->len) break;     /* exact size found */
			}
			else hbest = h;
		}
	}
	return hbest;
}


size_t x_blksize(byte *v) {
	xhdr_t *h = findxhdr(v);
	return (h ? h->len : 0);
}


size_t x_avail(void) {
	x_defrag();
	size_t t = (byte *) hcur - dcur;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->cv == NULL && h->len > t) t = h->len;
	}
	return t;
}


size_t x_total(void) {
	x_defrag();
	size_t t = 0;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		if(h->cv == NULL) t += h->len; else t += (h->len - h->need);
	}
	return (t + ((byte *) hcur - dcur) + 1);
}


void x_defrag(void) {
    xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {   /* combine neighbouring unused blocks */
		if(h->cv == NULL) {

			byte flag = 0;
			void *p = h->data + h->len;
			xhdr_t *ht = (xhdr_t *) (pMEMORY + xmem_bytes);

			while(--ht >= hcur) {
				if(ht->data == p && ht->cv == NULL) {   /* this one is suitable for combining with (h) */
					h->len += ht->len;
					memmove((byte *) ((xhdr_t *) (hcur + 1)),
                            (byte *) hcur, ((ht - hcur) * sizeof(xhdr_t)));
					hcur++;
					ht = (xhdr_t *) (pMEMORY + xmem_bytes);  /* restart the inner loop */
					flag = 1;  /* a change has been made */
				}
			}

			if(flag) h = (xhdr_t *) (pMEMORY + xmem_bytes);  /* restart the outer loop */

		}
	}
}


int x_free(byte **v) {
    if(!v || (*v == NULL)) return 0;
	if((*v < pMEMORY) || (*v > (pMEMORY + xmem_bytes))) return -1;
	xhdr_t *h = findxhdr(*v);
	if(h) {
		h->need = 0;
		h->cv = NULL;
	}
	else return -1;
	*v = NULL;
	return 0;
}


void *x_malloc(byte **v, size_t sz) {
	if(!v || !sz) {    /* size zero is equivalent to free the memory */
		x_free(v);
		return NULL;
	}
	if(*v && (*v < pMEMORY || *v > (pMEMORY + xmem_bytes))) return NULL;
	sz = ((sz >> 2) + !!(sz & 3)) << 2;	/* round the size up to 32-bit words */
    byte retryf = 0;
    retry:

    if(*v) {    /* re-allocation of an existing block */
        xhdr_t *z = findxhdr(*v);   /* find the current header */
		if(z) {

			if(z->len < sz) {		/* data relocation will be needed */
				xhdr_t *h = findxavl(sz);

				if(!h) {			/* new block will be needed for this size */
					*v = NULL;
					if(x_malloc(v, sz)) {	/* allocate a new block with the needed size */
						memcpy(*v, z->data, z->len);  /* move the existing data to new location */
						memset((*v + z->len), 0, (sz - z->len));	/* clear the expansion area */
						z->cv = NULL;	/* free up the old block */
					}
					else {			/* if unsuccessful will return NULL but the old block will be retained */
						*v = z->data;
						return NULL;	
					}
				}

				else {      /* reusing already allocated block and relocating the data */
					memcpy(h->data, z->data, z->len);
					memset((h->data + z->len), 0, (sz - z->len));  /* clear the expansion area */
					*v = h->data;
					h->need = sz;
					h->cv = z->cv;
					z->cv = NULL;  /* free up the old block */
				}

			}

			else {       /* nothing is needed since the current block can be expanded to the needed size */
				memset((z->data + sz), 0, (z->len - sz));   /* clear the expansion area */
				z->need = sz;
			}

		}

		else *v = NULL;   /* the supplied (*v) was not currently allocated known block */
    }

    else {          /* new allocation */

		xhdr_t *h = findxavl(sz);
		if(!h) {    /* a new block will have to be allocated */
			size_t htf, hf = -1;
			xhdr_t *ht = (xhdr_t *) (pMEMORY + xmem_bytes);

			while(--ht >= hcur) {    /* check to see if some of the currently allocated blocks have enough extra space */
				htf = ht->len - ht->need;
				if(htf >= sz && htf < hf) {
					h = ht;
					if(hf == sz) break; else hf = htf;	/* immediately break on the first exact fit */
				}
			}

			if(h && (size_t) ((byte *) hcur - dcur) >= sizeof(xhdr_t)) {         /* reusing an existing block */
				hcur--;
				hcur->len = sz;
				hcur->need = sz;
				hcur->cv = v;
				hcur->data = h->data + (h->len - sz);
				h->len -= sz;   /* trim the existing old block down by (sz) bytes */
				memset(hcur->data, 0, sz);  /* clear the data area */
				*v = hcur->data;
			}

			else if((size_t) ((byte *) hcur - dcur) >= (sz + sizeof(xhdr_t))) {  /* completely new block */
				hcur--;
				hcur->len = sz;
				hcur->need = sz;
				hcur->cv = v;
				hcur->data = dcur;
				dcur += sz;
				memset(hcur->data, 0, sz);	/* clear the newly allocated block */
				*v = hcur->data;
			}

			else {   /* can't find enough memory - try defragmenting first */
                x_defrag();
                retryf = !retryf;
                if(retryf) goto retry;
                *v = NULL;  /* unable to allocate block with this size */
            }

		}

		else {       /* reusing already allocated block */
			memset(h->data, 0, h->len);		/* clear the block */
			h->cv = v;
			h->need = sz;
			*v = h->data;
		}

    }

	return *v;
}


int x_reclaim(byte **v_new, byte **v_old) {
	if(!v_new || !v_old || (*v_old == NULL) || (*v_old < pMEMORY) || (*v_old > (pMEMORY + xmem_bytes))) return 0;
	xhdr_t *h = findxhdr(*v_old);
	if(h) {
		h->cv = v_new;
		*v_new = *v_old;
		*v_old = NULL;
		return 0;
	}
	else return -1;
}


void x_list_alloc(void) {
	printf("\r\ntotal: %u,  largest: %u\r\n", x_total(), x_avail());
	unsigned int c = 0;
	xhdr_t *h = (xhdr_t *) (pMEMORY + xmem_bytes);
	while(--h >= hcur) {
		printf("%3u: v:%8p,  *v:%8p,  h:%8p,  d:%8p,  l:%4u,  l:%4u\r\n",
			  c++,
			  (h->cv ? (void *) h->cv : NULL),
			  (h->cv ? (void *) *(h->cv) : NULL),
			  (void *) h,
			  (void *) h->data,
			  h->len,
			  h->need
		);
	}
}

