/******************************************************************************/
/**
 * \file
 *           Description.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/include/filterSupport.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * \see
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef FILTERSUPPORT_H
#define FILTERSUPPORT_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


#define FLTERR_CRITICAL(err)    ((err) > 0)
#define FLTERR_WARNING(err)     ((err) < 0)
#define FLTERR_SUCCESS(err)     ((err) == 0)


/* EXPORTED FUNCTIONS *********************************************************/



/* FUNCTION *******************************************************************/
/** Allocates memory space for a filter.
 *
 *  This function mallocs memory space for filter coefficients and roots based
 *  on the degree of numerator and denominator polynomial.
 *
 *  \param flt          Filter structure.
 *
 *  \return             0 on success, else an error code (typically ENOMEM).
 ******************************************************************************/
    int filterMalloc (FLTCOEFF *flt);


/* FUNCTION *******************************************************************/
/** Free's all memory space allocated for a filter.
 *
 *  \param flt          Filter structure.
 *
 ******************************************************************************/
    void filterFree(FLTCOEFF *flt);


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
    int filterCheck (FLTCOEFF *pFilter);


/* FUNCTION *******************************************************************/
/** Normalizes the amplitude of a filter. The function trys to normalize the
 *  transfer ratio (filter amplitude) at a given frequency. To perform that, it
 *  first modifies the denominator coefficients such that \f$den_0=1\f$ is
 *  ensured. At the second step it re-calculates the numerator coefficients in
 *  a way, that the amplitude response at the reference frequency is unity (1).
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
    int normFilterAmplitude (FLTCOEFF *pFilter, double f, double refgain);


#ifdef  __cplusplus
}
#endif


#endif /* FILTERSUPPORT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

