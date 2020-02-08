/******************************************************************************/
/**
 * \file
 *           Filter support functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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
    int filterDuplicate (FLTCOEFF *dest, FLTCOEFF *src);




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
    int normFilterCoeffs (FLTCOEFF *pFilter);


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
    int normFilterMagnitude (FLTCOEFF *pFilter, double f, double refgain);


#ifdef  __cplusplus
}
#endif


#endif /* FILTERSUPPORT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

