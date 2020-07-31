/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Miscellaneous FIR/IIR filter design functions.
 *
 * \note     Includes raw filters (filters without a design, except \f$f_{Sample}\f$).
 *
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef MISC_FILTER_H
#define MISC_FILTER_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"  /* includes config.h (include before GNU system headers) */


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Miscellaneous filters.
 *
 *  \attention Don't change the enums, because order must match list element
 *             index in dialog.
 */
typedef enum
{
    MISCFLT_UNKNOWN = -1,    /**< Unknown filter (no design data, raw filter) */
    MISCFLT_HILBERT = 0, /**< Hilbert transformer (\e Fourier series expansion) */
    MISCFLT_INT = 1,            /**< Integrator (\e Fourier series expansion) */
    MISCFLT_DIFF = 2,        /**< Differentiator (\e Fourier series expansion)*/
    MISCFLT_COMB = 3,                                        /**< Comb filter */
    MISCFLT_AVGFIR = 4,       /**< Moving average filter (non-recursive type) */
    MISCFLT_AVGIIR = 5,           /**< Moving average filter (recursive type) */
    MISCFLT_AVGEXP = 6,   /**< Exponential average filter (1st order lowpass) */

    MISCFLT_SIZE                                    /**< Size of MISCFLT enum */
} MISCFLT;


/** Misc filter design constraints.
 *  \see DESIGNDLG_COMMON
 */
typedef struct
{
    MISCFLT type;     /**< Type of filter. \attention Must be the 1st element */
    int order;       /**< Order of filter. \attention Must be the 2nd element */
} MISCFLT_DESIGN;



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


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
int miscFilterGen (const MISCFLT_DESIGN *pDesign, FLTCOEFF *pFilter);


#ifdef  __cplusplus
}
#endif


#endif /* MISC_FILTER_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

