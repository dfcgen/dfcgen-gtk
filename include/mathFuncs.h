/******************************************************************************/
/**
 * \file
 *           Mathematical functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/mathFuncs.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef MATHFUNCS_H
#define MATHFUNCS_H


/* INCLUDE FILES **************************************************************/

#include "base.h"               /* includes config.h (include before math.h)  */

#include <math.h>
#include <gsl/gsl_math.h>


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Decimal normalized double value. The original value is determined by
 *  \f$m 10^e\f$.
 */
typedef struct
{
    double mantissa;                                          /**< mantissa m */
    double exponent;                                          /**< exponent e */
} MATH_NORMDBL;




/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/** This macro swaps two integers (inline).
 */
#define MATH_SWAP_INT(int1, int2)                               \
{                                                               \
    (int1) ^= (int2);                                           \
    (int2) ^= (int1);                                           \
    (int1) ^= (int2);                                           \
}




#ifndef HAVE_HYPOT
#define hypot(arg) gsl_hypot(arg)
#endif

#ifndef HAVE_POW10                                     /* pow10() not exists? */
#ifdef HAVE_EXP10
#define pow10(x) exp10(x)        /**< 10 raised to \p x, means \f$y=10^{x}\f$ */
#else
#define pow10(x) pow(10, (x))          /* fallback to (slow) generic function */
#endif
#endif


#ifndef HAVE_TRUNC
#define trunc(x)           ((GSL_SIGN(x) > 0) ? floor(x) : ceil(x))
#endif


#ifndef HAVE_ROUND
#define round(x)           floor((x) + 0.5)     /* fallback to (slow) generic */
#endif




/* EXPORTED FUNCTIONS *********************************************************/



/* FUNCTION *******************************************************************/
/** Calculates a normalized value consisting of (decimal) mantissa and exponent.
 *
 *  \param val          Value to be converted.
 *
 *  \return             Normalized value.
 ******************************************************************************/
    MATH_NORMDBL mathNorm10(double val);



/* FUNCTION *******************************************************************/
/** Denormalizes a decimal value from its mantissa and exponent.
 *
 *  \param val          Normalized value to be converted.
 *
 *  \return             Denormalized value.
 ******************************************************************************/
    double mathDenorm10(MATH_NORMDBL val);


#ifdef  __cplusplus
}
#endif


#endif /* MATHFUNCS_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

