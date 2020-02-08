/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Basic types, constants and macros.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 *
 ******************************************************************************/


#ifndef BASE_H
#define BASE_H


/* INCLUDE FILES **************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


typedef int BOOL;          /**< Boolean data type (TRUE, FALSE are from gLib) */



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


#ifdef __GNUC__
#define ATTRIBUTE(attr)   __attribute__ ((attr))
#else
#define ATTRIBUTE(attr)
#endif


#define N_ELEMENTS(array)       G_N_ELEMENTS (array) /**< Number of elements in array */
#define ASSERT(cond)            g_assert (cond) /**< assert() macro (currently from gLib) */


#if defined (DEBUG) && defined (__GNUC__)
#define DEBUG_LOG(format, ...)  g_printerr ("\n%s, %d: " format, __FILE__ , __LINE__, ## __VA_ARGS__)
#else  /* !__GNUC__ */
#define DEBUG_LOG(format, ...)
#endif /* __GNUC__ */



/* MACRO **********************************************************************/
/** Error code check and conditional return. The macro checks for an error code
 *  unequal to GSL_SUCCESS in \p cond. On that condition it executes the code
 *  in \p post, then returns.
 *
 *  \param cond         Condition to be checked (e.g. may be a function call).
 *  \param string       Text (string), which describes the error/cause.
 *
 ******************************************************************************/
#define ERROR_RET_IF(cond, string, ...)                         \
{                                                               \
    int __err = (cond);                                         \
    if (__err != 0)                                             \
    {                                                           \
        __VA_ARGS__;                                            \
        DEBUG_LOG (string);                                     \
        return __err;                                           \
    }                                                           \
}



/* EXPORTED FUNCTIONS *********************************************************/



#ifdef  __cplusplus
}
#endif


#endif /* BASE_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
