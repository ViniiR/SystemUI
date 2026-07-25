#pragma once

typedef enum { OK, ERR } ResultVariant;

/// INFO:
/// T ok_value Can be NULLable.
/// char *err_msg Can be NULLable.
///
/// How to use them properly:
/// When 'variant' is ERR:
/// | define 'err_msg' to a string,
/// | define 'ok_value' to an empty value to satisfy the compiler.
/// When 'variant' is OK:
/// | define 'err_msg' to "",
/// | define 'ok_value' to the intended result.
#define RESULT(T, Name)                                                        \
    typedef struct {                                                           \
        ResultVariant variant;                                                 \
        char *err_msg;                                                         \
        T ok_value;                                                            \
    } Result##Name;

RESULT(void *, Void)
RESULT(int, Int)
RESULT(char *, String)

#define RESULT_ERR_MSG_UNKNOWN "Unknown error"

#define RESULT_VOID_DEFAULT                                                    \
    {.variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL};
