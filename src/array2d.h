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

#define PRECEDE(code) for(int __exit_var__=1; __exit_var__;) for(code;__exit_var__;__exit_var__=0)
#define FOLLOW(code) for(int __exit_var__=1; __exit_var__;code, __exit_var__=0)
 
#define FILTER(a, na, b, nb, i) PRECEDE((b=malloc(sizeof(a[0])*n), nb=0))\
     for(int i=0, __accept__=0;i<na; i++, __accept__=0)\
      FOLLOW( __accept__?(b[nb]=a[i],(++nb)):(nb=nb))
      
#define ACCEPT __accept__=1


#endif
