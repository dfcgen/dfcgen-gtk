/******************************************************************************/
/**
 * \file
 *           Miscellaneous mathematical functions and macros.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 *  \todo               Implement a generic error handler
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathMisc.h"



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* EXPORTED FUNCTION DEFINITIONS **********************************************/


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
double mathTryDiv(double num, double den)
{
    if (fabs(den) <= 1.0)             /* no overflow on fabs(den) * DBL_MAX ? */
    {
        if (fabs(num) >= fabs(den) * GSL_DBL_MAX)      /* overflow expected ? */
        {                                            /* path handles also 0/0 */
            if (((num < 0.0) && (den > 0.0)) || ((num > 0.0) && (den < 0.0)))
            {
                return GSL_NEGINF;
            } /* if */

            return GSL_POSINF;
        } /* if */
    } /* if */

    return num / den;
} /* mathTryDiv() */



/* FUNCTION *******************************************************************/
/** Swaps two double values.
 *
 *  \param p1           Pointer to first value.
 *  \param p2           Pointer to second value.
 *
 *  \return             Nothing.
 ******************************************************************************/
void mathDoubleSwap(double *p1, double *p2)
{
    double tmp = *p1;

    *p1 = *p2;
    *p2 = tmp;
} /* mathDoubleSwap() */




/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
