/******************************************************************************/
/**
 * \file
 *           Miscellaneous mathematical functions and macros.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef MATHMISC_H
#define MATHMISC_H


/* INCLUDE FILES **************************************************************/

#include "base.h"               /* includes config.h (include before math.h)  */

#include <math.h>
#include <float.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_errno.h>


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Tries division \f$y=num/den\f$ wrt. overflow.
 *
 *  \param num          Numerator.
 *  \param den          Denominator.
 *
 *  \return             \f$y=num/den\f$ or GSL_POSINF/GSL_NEGINF. Use the
 *                      functions gsl_isinf() or gsl_finite() for result
 *                      checking.
 ******************************************************************************/
    double mathTryDiv(double num, double den);


/* FUNCTION *******************************************************************/
/** Swaps two double values.
 *
 *  \param p1           Pointer to first value.
 *  \param p2           Pointer to second value.
 *
 *  \return             Nothing.
 ******************************************************************************/
    void mathDoubleSwap(double *p1, double *p2);



#ifdef  __cplusplus
}
#endif


#endif /* MATHMISC_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
