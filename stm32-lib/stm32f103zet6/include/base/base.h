#ifndef BASE_H
#define BASE_H

#define null ((void*) 0)
#define null_handle 0
#define ARRAY_SIZE(arr) sizeof(arr) / sizeof(arr[0])
#define STR(x) #x
#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a > b) ? b : a

typedef unsigned int bsp_handle_t;

typedef enum {
    NO_ERROR = 0,
    EUNKNOW = -1,
    EBADPARAM = -2,
    ENOMEM = -3,
    EALREADY = -4,
    EHW = -5,
    ESTATE = -6,
} status_t;

typedef enum {
    true = 1,
    false = 0
} bool;

#endif