#pragma once

#include <stdlib.h>

typedef enum { OK, ERR } ResultVariant;

/// WARNING: 
/// T ok_value Can be NULL
/// char *err_msg Can be NULL, Beware.
#define RESULT(T, Name)                                                        \
    typedef struct {                                                           \
        ResultVariant variant;                                                 \
        char *err_msg;                                                         \
        T ok_value;                                                            \
    } Result##Name;

RESULT(void *, Void)
RESULT(int, Int)
RESULT(char *, String)
