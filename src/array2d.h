#ifndef __ARRAY2D_H_
#define __ARRAY2D_H_ 

#define ALLOC(t) (t*)malloc(sizeof(t))
#define ALLOC_N(t, n) (t*)malloc(sizeof(t)*n)

#define DEF_ALLOC(v, t) t* v = ALLOC(t)
#define DEF_ALLOC_N(v, t, n) t* v = ALLOC_N(t, n)

#define ALLOC_2DARRAY(a, n, m, t) do { \
    a = (t**)malloc(sizeof(t*)*(n)); \
    int i; \
    for(i=0; i<n; i++) \
        a[i] = (t*)malloc(sizeof(t)*(m)); \
    } while(0)

#define FREE_2DARRAY(a, n, m) do { \
    int i; \
    for(i=0; i<n; i++) \
        free(a[i]); \
    free(a); \
    } while(0)

#endif
