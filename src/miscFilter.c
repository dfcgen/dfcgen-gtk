/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Misc filter design.
 *
 * \note     Includes raw filters (filters without a design, except \f$f_{Sample}\f$).
 *
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "miscFilter.h"
#include "filterSupport.h"

#include <gsl/gsl_math.h>                              /* includes math.h too */
#include <gsl/gsl_sf.h>                              /* all special functions */
#include <gsl/gsl_errno.h>


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
/** Generates a miscellaneous FIR/IIR filter. The following digital filters are
 *  supported:
 *  - \e Hilbert transformer (linear FIR): \f$\displaystyle{c_{n/2\pm k}=-\frac{2}{k\pi}}\f$ for odd k, else 0
 *  - Perfect integrator (FIR): \f$\displaystyle{c_{n/2\pm k}=\frac{1}{2}-\frac{1}{\pi}\Si(k\pi)}\f$
 *  - Perfect differentiator (linear FIR): \f$\displaystyle{c_{n/2\pm k}=\frac{(-1)^{k+1}}{k\pi}}\f$
 *  - Moving average (FIR): \f$H(z)=1+z^{-1}+z^{-2}+\cdots+z^{-n}\f$
 *  - Moving average (IIR): \f$\displaystyle{H(z)=\frac{1-z^{-n}}{1-z^{-1}}}\f$
 *  - Comb filter (FIR):  \f$H(z)=1-z^{-n}\f$
 *  - Exponential average (IIR): \f$\displaystyle{H(z)=\frac{1}{n-(n-1)z^{-1}}}\f$
 *
 *  \param pDesign      Pointer to misc filter design data.
 *  \param pFilter      Pointer to buffer which gets the generated filter.
 *                      Notice, that memory space for polynomials will be
 *                      allocated.
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
int miscFilterGen (const MISCFLT_DESIGN *pDesign, FLTCOEFF *pFilter)
{
    int err;
    int i, ic;                                       /* index, centered index */
    double *vec;                                /* ptr to coefficients vector */

    pFilter->factor = 0.0;                 /* roots are invalid (unused here) */

    /* First set default numerator an denominator polynomial degree for memory
     * allocation.
     */
    pFilter->num.degree = pFilter->den.degree = pDesign->order;

    switch (pDesign->type)                   /* check which filter type is it */
    {
        case MISCFLT_AVGEXP:
            pFilter->num.degree = 0;
                                                          /* and fall through */
        case MISCFLT_AVGIIR:        /* moving average filter (recursive type) */
            pFilter->den.degree = 1;
            break; /* MISCFLT_AVGIIR, MISCFLT_AVGEXP */

        case MISCFLT_HILBERT:                          /* hilbert transformer */
        case MISCFLT_INT:            /* integrator (Fourier series expansion) */
        case MISCFLT_DIFF:       /* differantiator (Fourier series expansion) */
        case MISCFLT_COMB:                                     /* comb filter */
        case MISCFLT_AVGFIR:    /* moving average filter (non-recursive type) */
            pFilter->den.degree = 0; /* default denominator polynomial degree */
            break;

        default:
            ASSERT (0);
    } /* switch */


    err = filterMalloc(pFilter);

    if (err != 0)
    {
        return err;
    } /* if */

    pFilter->factor = 0;                                     /* roots unknown */
    pFilter->den.coeff[0] = pFilter->num.coeff[0] = 1.0; /* default a[0]=b[0]=1 */
    ic = pDesign->order / 2;                                  /* center index */
    vec = pFilter->num.coeff;                                     /* shortcut */

    switch (pDesign->type)
    {
        case MISCFLT_HILBERT:                          /* hilbert transformer */
            vec[ic] = 0.0;                                    /* num[n/2] = 0 */

            for (i = 1; i <= ic; i++)      /* i means index of Fourier coeffs */
            {
                if (GSL_IS_ODD(i))
                {
                    vec[ic - i] = -M_2_PI / i;
                    vec[ic + i] = -vec[ic - i];        /* antimetric lin. FIR */
                } /* if */
                else
                {
                    vec[ic - i] = vec[ic + i] = 0.0;
                } /* else */
            } /* for */

            break; /* MISCFLT_HILBERT */


        case MISCFLT_INT:            /* integrator (Fourier series expansion) */
            vec[ic] = 0.5;                              /* center coefficient */

            for (i = 1; i <= ic; i++)
            {
                vec[ic - i] = 0.5 - M_1_PI * gsl_sf_Si (M_PI * i);
                vec[ic + i] = 1.0 - vec[ic - i];
            } /* for */                  /* not antimetric -> non lin. FIR */

            err = normFilterMagnitude (pFilter, 0.0, 1.0);

            if (FLTERR_CRITICAL(err))
            {
                filterFree (pFilter);
                DEBUG_LOG ("Generation of perfect integrator has failed");

                return err;
            } /* if */

            break; /* MISCFLT_INT */


        case MISCFLT_DIFF:       /* differantiator (Fourier series expansion) */
            vec[ic] = 0.0;                             /* c[0] = num[n/2] = 0 */

            for (i = 1; i <= ic; i++)
            {
                vec[ic - i] = -M_1_PI / i;

                if (GSL_IS_EVEN (i))
                {
                    vec[ic - i] *= -1;                         /* change sign */
                } /* if */

                vec[ic + i] = -vec[ic - i];            /* antimetric lin. FIR */
            } /* for */

            break; /* MISCFLT_DIFF */


        case MISCFLT_AVGFIR:    /* moving average filter (non-recursive type) */
            for (i = 0; i <= pFilter->num.degree; i++)
            {
                vec[i] = 1.0 / (pFilter->num.degree + 1);
            } /* for */

            break; /* MISCFLT_AVGFIR */


        case MISCFLT_AVGIIR:        /* moving average filter (recursive type) */
            pFilter->den.coeff[1] = -1.0;

            for (i = 1; i < pFilter->num.degree; i++)
            {
                vec[i] = 0.0;
            } /* for */

            vec[0] = 1.0 / pFilter->num.degree;
            vec[pFilter->num.degree] = -vec[0];
            break; /* MISCFLT_AVGIIR */


        case MISCFLT_COMB:                                     /* comb filter */
            for (i = 1; i < pFilter->num.degree; i++)
            {
                vec[i] = 0.0;
            } /* for */

            vec[0] = 0.5;
            vec[pFilter->num.degree] = -0.5;
            break; /* MISCFLT_COMB */


        case MISCFLT_AVGEXP:                    /* exponential average filter */
            pFilter->den.coeff[0] = pDesign->order;
            pFilter->den.coeff[1] = 1 - pDesign->order;
            break; /* MISCFLT_AVGEXP */


        default:
            ASSERT (0);

    } /* switch */


    err = filterCheck(pFilter);

    if (FLTERR_CRITICAL(err))
    {
        filterFree (pFilter);
        DEBUG_LOG ("Implementation of filter impossible");

        return GSL_EFAILED;
    } /* if */

    return err;
} /* miscFilterGen() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

