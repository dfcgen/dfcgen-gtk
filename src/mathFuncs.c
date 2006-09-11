/******************************************************************************/
/**
 * \file
 *           Mathematical functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/mathFuncs.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathFuncs.h"


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
/** Calculates a normalized value consisting of (decimal) mantissa and exponent.
 *
 *  \param val          Value to be converted.
 *
 *  \return             Normalized value.
 ******************************************************************************/
MATH_NORMDBL mathNorm10(double val)
{
    MATH_NORMDBL norm;

    norm.exponent = trunc(log10(fabs(val)));
    norm.mantissa = val / pow10(norm.exponent);

    return norm;
} /* mathNorm10() */


/* FUNCTION *******************************************************************/
/** Denormalizes a decimal value from its mantissa and exponent.
 *
 *  \param val          Normalized value to be converted.
 *
 *  \return             Denormalized value.
 ******************************************************************************/
double mathDenorm10(MATH_NORMDBL val)
{
    return val.mantissa * pow10(val.exponent);
} /* mathDenorm10() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

