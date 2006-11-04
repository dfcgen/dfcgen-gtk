/******************************************************************************/
/**
 * \file
 *           Mathematical functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/mathFuncs.c,v 1.2 2006-11-04 18:26:27 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/09/11 15:52:19  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathFuncs.h"
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


/* FUNCTION *******************************************************************/
/** Rectangle function
    \f[
       y = \begin{cases} 0, & \mbox{if} \;\; x<0 \\
                         0, & \mbox{if} \;\; x>1 \\
                         1, & \mbox{else} \end{cases}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *
 *  \return             Result \f$y\f$.
 ******************************************************************************/
double mathFuncRectangle (double x)
{
    if ((x < 0) || (x > 1))
    {
        return 0;
    } /* if */

    return 1;
} /* mathFuncRectangle() */



/* FUNCTION *******************************************************************/
/** \e Hamming window function
    \f[
       y = \begin{cases} 0, & \mbox{if} \;\; x<0 \\
                         0, & \mbox{if} \;\; x>1 \\
                         0.54-0.46 \cos(2\pi x), & \mbox{else} \end{cases}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *
 *  \return             Result \f$y\f$.
 ******************************************************************************/
double mathFuncHamming (double x)
{
    if ((x < 0) || (x > 1))
    {
        return 0;
    } /* if */

    return 0.53836 - 0.46164 * cos (2 * M_PI * x);
} /* mathFuncHamming() */


/* FUNCTION *******************************************************************/
/** \e Hanning window function
    \f[
       y = \begin{cases} 0, & \mbox{if} \;\; x<0 \\
                         0, & \mbox{if} \;\; x>1 \\
                         \frac{1}{2}\,[1-\cos(2\pi x)], & \mbox{else} \end{cases}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *
 *  \return             Result \f$y\f$.
 ******************************************************************************/
double mathFuncHanning (double x)
{
    if ((x < 0) || (x > 1))
    {
        return 0;
    } /* if */

    return 0.5 - 0.5 * cos (2 * M_PI * x);
} /* mathFuncHanning() */


/* FUNCTION *******************************************************************/
/** \e Blackman window function
    \f[
       y = \begin{cases} 0, & \mbox{if} \;\; x<0 \\
                         0, & \mbox{if} \;\; x>1 \\
                         0.42-0.5\cos(2\pi x)+0.08\cos(4\pi x), & \mbox{else}
           \end{cases}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *
 *  \return             Result \f$y\f$.
 ******************************************************************************/
double mathFuncBlackman (double x)
{
    if ((x < 0) || (x > 1))
    {
        return 0;
    } /* if */

    return 0.42 - 0.5 * cos (2 * M_PI * x) + 0.08 * cos(4 * M_PI * x);
} /* mathFuncBlackman() */



/* FUNCTION *******************************************************************/
/** \e Kaiser window function
    \f[
       y = \begin{cases} 0, & \mbox{if} \;\; x<0 \\
                         0, & \mbox{if} \;\; x>1 \\
                         \frac{I_0\left(\alpha\sqrt{1-(2 x-1)^2}\right)}
                              {I_0(\alpha)}, & \mbox{else}
           \end{cases}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param alpha        Parameter \f$\alpha\f$.
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
double mathFuncKaiser (double x, double alpha)
{
    gsl_sf_result result;
    double tmp;

    if ((x < 0) || (x > 1))
    {
        return 0;
    } /* if */

    tmp = 1.0 - 2.0 * x;

    if (gsl_sf_bessel_I0_e (alpha * sqrt(1.0 - tmp * tmp), &result) != GSL_SUCCESS)
    {
        return GSL_POSINF;
    } /* if */

    return mathTryDiv (result.val, gsl_sf_bessel_I0 (alpha));
} /* mathFuncKaiser() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

