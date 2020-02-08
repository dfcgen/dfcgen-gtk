/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Filter support functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathMisc.h"
#include "filterSupport.h"
#include "filterResponse.h"

#include <string.h> /* memcpy() */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define FILTER_APPROX_ZERO    (DBL_EPSILON / 32.0) /**< Assume zero if less than value */


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int checkPolyZ(MATHPOLY *poly);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Checks a polynomial wrt. size of coefficients. If any coefficient is less
 *  than FILTER_APPROX_ZERO, the it will be set to zero (and on leading
 *  coefficients the degree of polynomial adopted).
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             - 0 if okay and nothing has changed
 *                      - a negative number if a coefficient or the degree has
 *                        changed, but the polynomial is valid
 *                      - a positive error number (from errno.h) that something
 *                        is wrong and the polynomial must be seen as invalid.
 ******************************************************************************/
static int checkPolyZ (MATHPOLY *poly)
{
    int i, k, ret = 0;
    int degree = poly->degree;

    for (i = degree; i >= 0; i--)
    {
        if (fabs(poly->coeff[i]) < FILTER_APPROX_ZERO)
        {
            --degree;
        } /* if */
        else
        {
            break;               /* break for loop if first valid coeff found */
        } /* else */
    } /* for */

    if (degree < 0)
    {
        DEBUG_LOG ("All coefficients of polynomial to small");
        return ERANGE;
    } /* if */


    i = 0;

    while ((i <= degree) &&
           (fabs(poly->coeff[i]) < FILTER_APPROX_ZERO))
    {                         /* search first valid coeff starting at index 0 */
        ++i;
    } /* while */


    if (i > 0)                           /* first coefficient(s) are invalid? */
    {
        if (i > degree)                                    /* that's too much */
        {
            DEBUG_LOG ("Coefficients check");
            return ERANGE;
        } /* if */

        degree -= i;                                            /* shift down */

        for (k = 0; k <= degree; k++)
        {
            poly->coeff[k] = poly->coeff[k + i];
        } /* for */
    } /* if */


    if (degree != poly->degree)           /* realloc memory if degree changed */
    {
        poly->coeff = realloc(poly->coeff, (1 + degree) * sizeof(poly->coeff[0]));
        poly->root  = realloc(poly->root, GSL_MAX (1, degree) * sizeof(poly->root[0]));
        ret = GSL_CONTINUE;
    } /* if */

    if ((poly->root == NULL) || (poly->coeff == NULL))
    {
        DEBUG_LOG ("Polynomial memory re-allocation has failed");
        return ENOMEM;
    } /* if */

    poly->degree = degree;

    return ret;
} /* checkPolyZ() */


/* EXPORTED FUNCTION DEFINITIONS **********************************************/



/* FUNCTION *******************************************************************/
/** Allocates memory space for a filter.
 *
 *  This function mallocs memory space for filter coefficients and roots based
 *  on the degree of numerator and denominator polynomial.
 *
 *  \param flt          Filter structure.
 *
 *  \return             Zero on success, else an error code (typically ENOMEM).
 ******************************************************************************/
int filterMalloc (FLTCOEFF *flt)
{
    int err = mathPolyMalloc (&flt->num);

    if (err != 0)
    {
        DEBUG_LOG ("Filter memory allocation (numerator)");
        return err;
    } /* if */

    err = mathPolyMalloc (&flt->den);

    if (err != 0)
    {
        mathPolyFree(&flt->num);
        DEBUG_LOG ("Filter memory allocation (denominator)");
        return err;
    } /* if */

    return 0;
} /* filterMalloc() */



/* FUNCTION *******************************************************************/
/** Duplicates a filter. This function mallocs memory space for filter
 *  coefficients and roots based on the degree of the source filter pointed by
 *  \p src. Then it copies the numerator and denominator polynomial to \p dest.
 *  Use function filterFree() to free all the new associated memory.
 *
 *  \param src          Source filter (input).
 *  \param dest         Destination filter (output).
 *
 *  \return             Zero on success, else an error code (typically ENOMEM).
 ******************************************************************************/
int filterDuplicate (FLTCOEFF *dest, FLTCOEFF *src)
{
    int err;

    dest->f0 = src->f0;
    dest->num.degree = src->num.degree;
    dest->den.degree = src->den.degree;

    err = filterMalloc (dest);

    if (err != 0)
    {
        return err;
    } /* if */

    memcpy (dest->num.coeff, src->num.coeff,
            (1 + src->num.degree) * sizeof(src->num.coeff[0]));
    memcpy (dest->den.coeff, src->den.coeff,
            (1 + src->den.degree) * sizeof(src->den.coeff[0]));
    memcpy (dest->num.root, src->num.root,
            src->num.degree * sizeof(src->num.root[0]));
    memcpy (dest->den.root, src->den.root,
            src->den.degree * sizeof(src->den.root[0]));

    return 0;
} /* filterDuplicate() */


/* FUNCTION *******************************************************************/
/** Free's all memory space allocated for a filter.
 *
 *  \param flt          Filter structure.
 *
 *  \return             Nothing.
 ******************************************************************************/
void filterFree(FLTCOEFF *flt)
{
    mathPolyFree(&flt->num);
    mathPolyFree(&flt->den);
} /* filterFree */



/* FUNCTION *******************************************************************/
/** Checks ability to implement a digital system/filter.
 *
 *  \param pFilter      Pointer to filter coefficients/roots.
 *
 *  \return             - 0 (or GSL_SUCCESS) if okay and nothing has changed.
 *                      - a negative number (typically GSL_CONTINUE) if a
 *                        coefficient or the degree has changed, but the filter
 *                        is valid. You can use the FLTERR_WARNING macro from
 *                        filterSupport.h to check this condition.
 *                      - a positive error number (typically from from errno.h
 *                        or gsl_errno.h) that something is wrong and the
 *                        filter must be seen as invalid. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition.
 ******************************************************************************/
int filterCheck (FLTCOEFF *pFilter)
{
    int err1 = checkPolyZ (&pFilter->num);
    int err2 = checkPolyZ (&pFilter->den);

    if (FLTERR_CRITICAL (err1))  /* anything wrong with numerator polynomial? */
    {
        return err1;
    } /* if */

    if (!FLTERR_SUCCESS (err2)) /* wrong denominator polynomial (or changed)? */
    {
        return err2;
    } /* if */

    return err1;
} /* filterCheck() */



/* FUNCTION *******************************************************************/
/** Normalizes the coefficients of a filter. To perform that, it
 *  first modifies the denominator coefficients such that \f$den_0=1\f$ is
 *  ensured. At the second step it re-calculates the numerator coefficients.
 *
 *  \param pFilter      Pointer to filter.
 *
 *  \return             - 0 (or GSL_SUCCESS) if okay and nothing has changed.
 *                      - a negative number (typically GSL_CONTINUE) if a
 *                        coefficient or the degree has changed, but the filter
 *                        is valid. You can use the FLTERR_WARNING macro from
 *                        filterSupport.h to check this condition.
 *                      - a positive error number (typically from from errno.h
 *                        or gsl_errno.h) that something is wrong and the
 *                        filter must be seen as invalid. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition.
 ******************************************************************************/
int normFilterCoeffs (FLTCOEFF *pFilter)
{
    int i;

    double *pCoeff, normCoeff = pFilter->den.coeff[0];

    for(i = 0, pCoeff = pFilter->den.coeff; i <= pFilter->den.degree; i++, pCoeff++)
    {
        *pCoeff = mathTryDiv (*pCoeff, normCoeff);

        if (gsl_isinf(*pCoeff))
        {
            return ERANGE;
        } /* if */
    } /* for */


    for (i = 0, pCoeff = pFilter->num.coeff; i <= pFilter->num.degree; i++, pCoeff++)
    {
        *pCoeff = mathTryDiv (*pCoeff, normCoeff);

        if (gsl_isinf(*pCoeff))
        {
            return ERANGE;
        } /* if */
    } /* for */

    return filterCheck (pFilter);
} /* normFilterCoeffs() */


/* FUNCTION *******************************************************************/
/** Normalizes the magnitude of a filter. The function trys to normalize the
 *  transfer ratio (filter magnitude) at a given frequency. To perform that, it
 *  first modifies the denominator coefficients such that \f$den_0=1\f$ is
 *  ensured. At the second step it re-calculates the numerator coefficients in
 *  a way, that the magnitude response at the reference frequency is unity (1).
 *
 *  \param pFilter      Pointer to filter.
 *  \param f            Frequency (normalization point).
 *  \param refgain      Reference transfer ratio \f$H(2\pi f)\f$.
 *
 *  \return             - 0 (or GSL_SUCCESS) if okay and nothing has changed.
 *                      - a negative number (typically GSL_CONTINUE) if a
 *                        coefficient or the degree has changed, but the filter
 *                        is valid. You can use the FLTERR_WARNING macro from
 *                        filterSupport.h to check this condition.
 *                      - a positive error number (typically from from errno.h
 *                        or gsl_errno.h) that something is wrong and the
 *                        filter must be seen as invalid. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition.
 ******************************************************************************/
int normFilterMagnitude (FLTCOEFF *pFilter, double f, double refgain)
{
    double magnitude;

    int i, err = normFilterCoeffs (pFilter);

    if (FLTERR_CRITICAL (err))
    {
        return err;
    } /* if */

    magnitude = filterResponseMagnitude (f, pFilter);

    if (gsl_isinf(magnitude))                       /* magnitude singularity? */
    {
        return ERANGE;
    } /* if */

    magnitude = mathTryDiv(refgain, magnitude);

    if (gsl_isinf(magnitude))
    {
        return (ERANGE);
    } /* if */


    for (i = 0; i <= pFilter->num.degree; i++)  /* scale all numerator coeffs */
    {
        pFilter->num.coeff[i] *= magnitude;
    } /* for */

    return filterCheck(pFilter);
} /* normFilterMagnitude() */




/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
