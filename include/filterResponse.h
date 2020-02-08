/******************************************************************************/
/**
 * \file
 *           Filter response functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef FILTER_RESPONSE_H
#define FILTER_RESPONSE_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/

typedef struct
{
    /* public */
    int samples;                           /**< Number of samples in interval */

    /* private */
    double curTime;                            /**< Current time (in seconds) */
    double *pLastOut;                       /**< Pointer to last output value */
    double *pCurIn;          /**< Pointer where to store the next input value */
    double *pInBuf;                    /**< Input buffer pointer (per malloc) */
    double *pOutBuf;                  /**< Output buffer pointer (per malloc) */
    FLTSIGNAL sig;            /**< Signal type (\e Dirac, \e Heaviside, etc.) */
    const FLTCOEFF *pFilter;                           /**< Pointer to filter */

} FLTRESP_TIME_WORKSPACE;


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Evaluates absolute magnitude associated with a polynomial in \e Z domain.
 *  The function returns the absolute value of the following polynomial:
    \f[
    H(z)=a_0+a_1 z^{-1}+a_2 z^{-2}+\cdots a_n z^{-n}
    \f]
 *  with \f$z^{-1}=\exp(-j\omega)\f$ at circular frequency
 *  \f$\omega=2\pi f/f_0\f$, given in rad/s.
 *
 *  \param omega        Frequency ratio \f$2\pi f/f_0\f$.
 *  \param poly         Pointer to polynomial coefficients in \e Z domain.
 *
 *  \return             Magnitude value associated with the polynomial when
 *                      evaluated successful, else GSL_POSINF or GSL_NEGINF.
 *                      Use the functions gsl_isinf() or gsl_finite() for
 *                      result checking.
 ******************************************************************************/
    double filterResponsePoly (double omega, const MATHPOLY *poly);


/* FUNCTION *******************************************************************/
/** Computes the magnitude of a time-discrete system at a given frequency in
 *  \e Z domain.
 *
 *  \param f            Frequency point in Hz.
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Magnitude value when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
    double filterResponseMagnitude (double f, const FLTCOEFF *pFilter);



/* FUNCTION *******************************************************************/
/** Computes the attenuation of a time-discrete system at a given frequency in
 *  \e Z domain.
    \f[
       A(f) = -20\log[H(f)]
    \f]
 *
 *  \param f            Frequency point in Hz.
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Magnitude value when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
    double filterResponseAttenuation (double f, FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** Computes the phase of a time-discrete system at a given frequency in
 *  \e Z domain.
 *
 *  \param f            Frequency point in Hz.
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Phase in rad when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
    double filterResponsePhase (double f, FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** Computes the phase delay \f$B(\omega)/\omega\f$ of a time-discrete system
 *  at a given frequency in \e Z domain.
 *
 *  \param f            Frequency point in Hz (with \f$\omega=2\pi f\f$).
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Phase delay in rad s.
 ******************************************************************************/
    double filterResponsePhaseDelay (double f, FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** Computes the group delay \f$\textup{d}B(\omega)/\textup{d}\omega\f$ of a
 *  time-discrete system at a given frequency in \e Z domain.
 *
 *  \param f            Frequency point in Hz (with \f$\omega=2\pi f\f$).
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Group delay in sec.
 ******************************************************************************/
    double filterResponseGroupDelay (double f, FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** Computes the characteristic function \f$D(f)\f$ of a time-discrete system
 *  in \e Z domain.
    \f{eqnarray*}
        H^2(f) &=& \frac{1}{1+D^2(f)} \\ 
        D(f)   &=& \sqrt{\frac{1}{H^2(f)}-1}
    \f}
 *
 *  \param f            Frequency point in Hz.
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Characteristic function value on success, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
    double filterResponseChar (double f, FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** This function creates an workspace for time response calculation.
 *
 *  \param start        Time to start (must be positive).
 *  \param stop         Time to stop (must be positive).
 *  \param type         Signal type.
 *  \param pFilter      Pointer to filter coefficients.
 *
 *  \return             Pointer to an workspace for time response calculation
 *                      via function filterResponseTimeNext(). All public
 *                      members are up to data after return (never use privates).
 *                      On error (includes the case of too much samples in
 *                      range) NULL is returned.
 ******************************************************************************/
FLTRESP_TIME_WORKSPACE* filterResponseTimeNew (double start, double stop,
                                               FLTSIGNAL type, const FLTCOEFF* pFilter);


/* FUNCTION *******************************************************************/
/** Returns the next output sample for a time response.
 *
 *  \param pWorkspace   Pointer to time response workspace, formerly created
 *                      by filterResponseTimeNew().
 *  \param pTime        Pointer to a variable which gets the next time value.
 *
 *  \return             Next output sample value on success, else GSL_POSINF
 *                      or GSL_NEGINF. Use the functions gsl_isinf() or
 *                      gsl_finite() for result checking. Avoid calling
 *                      this function again under the mentioned error condition.
 ******************************************************************************/
    double filterResponseTimeNext (FLTRESP_TIME_WORKSPACE *pWorkspace, double *pTime);


/* FUNCTION *******************************************************************/
/** Free's a time response workspace.
 *
 *  \param pWorkspace   Pointer to time response workspace, formerly created
 *                      by filterResponseTimeNew().
 *
 ******************************************************************************/
    void filterResponseTimeFree (FLTRESP_TIME_WORKSPACE *pWorkspace);



#ifdef  __cplusplus
}
#endif


#endif /* FILTER_RESPONSE_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

