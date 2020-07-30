/******************************************************************************/
/**
 * \file     filterResponse.c
 * \brief    Filter response functions.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de> 
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathMisc.h"
#include "filterResponse.h"

#include <string.h> /* memset() */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define FLTRESP_TIME_SAMPLES_LIMIT      2048   /**< Maximum number of samples */


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* MACRO **********************************************************************/
/** Increments a pointer on a circular buffer (for time responses).
 *
 *  \param ptr          Pointer to current value.
 *  \param buf          Buffer from type of *ptr.
 *  \param degree       Size of buffer expressed by polynomial degree.
 *
 ******************************************************************************/
#define FLTRESP_INC(ptr, buf, degree)                           \
    if ((ptr) >= (buf) + (degree)) (ptr) = (buf);               \
    else ++(ptr);


/* MACRO **********************************************************************/
/** Decrements a pointer on a circular buffer (for time responses).
 *
 *  \param ptr          Pointer to current value.
 *  \param buf          Buffer from type of *ptr.
 *  \param degree       Size of buffer expressed by polynomial degree.
 *
 ******************************************************************************/
#define FLTRESP_DEC(ptr, buf, degree)                           \
    if ((ptr) <= (buf)) (ptr) = (buf) + (degree);               \
    else --(ptr);


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static gsl_complex evalPolyZ(double omega, const MATHPOLY *poly);
static double evalPolyAngleZ(double omega, const MATHPOLY *poly);
static double evalPolyGroupZ(double omega, const MATHPOLY *poly);
static double timeResponseGetNext (double time, FLTSIGNAL sig);
static double timeResponseProcNext (FLTRESP_TIME_WORKSPACE *pWorkspace);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Evaluates complex magnitude associated with a polynomial in \e Z domain.
 *  The function returns the complex magnitude of the numerator or denominator
 *  polynomial of a time-discrete system at the circular frequency
 *  \f$\omega=2\pi f/f_0\f$. The calculation is performed by the following
 *  algorithm:
    \f{eqnarray*}
        H(z) &=& a_0+a_1 z^{-1}+a_2 z^{-2}+\cdots a_n z^{-n} \\ 
             &=& (\cdots((a_n z^{-1}+a_{n-1}) z^{-1}+a_{n-2})z^{-1}+\cdots+a_0
    \f}
 *  with \f$z^{-1}=\exp(-j\omega)\f$.
 *
 *  \param omega        Angular frequency in rad/s.
 *  \param poly         Pointer to polynomial coefficients in \e Z domain.
 *
 *  \return
 ******************************************************************************/
static gsl_complex evalPolyZ(double omega, const MATHPOLY *poly)
{
    gsl_complex result;
    double *pCoeff;

    int i = poly->degree;
    double reOld = 0.0;               /* real part of last value in iteration */
    double sinOmega = sin(omega);
    double cosOmega = cos(omega);

    GSL_SET_COMPLEX(&result, 0.0, 0.0);

    for (pCoeff = &poly->coeff[poly->degree]; i >= 0; i--, pCoeff--)
    {
        GSL_SET_COMPLEX(&result,
                        reOld * cosOmega + GSL_IMAG(result) * sinOmega + *pCoeff,
                        GSL_IMAG(result) * cosOmega - reOld * sinOmega);
        reOld = GSL_REAL(result);
    } /* for */

    return result;
} /* evalPolyZ() */



/* FUNCTION *******************************************************************/
/** Evaluates the angle (in rad) associated with a polynomial in \e Z domain.
 *  The function returns the angle of the following complex polynomial:
    \f[
    H(z)=a_0+a_1 z^{-1}+a_2 z^{-2}+\cdots a_n z^{-n}
    \f]
 *  with \f$z^{-1}=\exp(-j\omega)\f$ at circular frequency
 *  \f$\omega=2\pi f/f_0\f$, given in rad/s.
 *
 *  \param omega        Frequency ratio \f$2\pi f/f_0\f$.
 *  \param poly         Pointer to polynomial coefficients in \e Z domain.
 *
 *  \return             Angle \f$\phi\in[-\pi,+\pi]\f$ on success, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double evalPolyAngleZ(double omega, const MATHPOLY *poly)
{
    return gsl_complex_arg (evalPolyZ(omega, poly));
} /* evalPolyAngleZ() */


/* FUNCTION *******************************************************************/
/** Evaluates the group delay (in sec) associated with a polynomial in \e Z
 *  domain. The function returns the group delay of the following complex
 *  polynomial:
    \f[
    H(z)=a_0+a_1 z^{-1}+a_2 z^{-2}+\cdots a_n z^{-n}
    \f]
 *  with \f$z^{-1}=\exp(-j\omega)\f$ at circular frequency
 *  \f$\omega=2\pi f/f_0\f$, given in rad/s.
 *
 *  The group delay \f$T_g\f$ is calculated from phase \f$B(\omega)\f$ and
 *  magnitude \f$H(\omega)\f$ by:
    \f[
    T_g(\omega)=\frac{\imop'[B(\omega)]\reop[B(\omega)]-\reop'[B(\omega)]\imop[B(\omega)]}{H^2(\omega)}
    \f]
 *
 *  \param omega        Frequency ratio \f$2\pi f/f_0\f$.
 *  \param poly         Pointer to polynomial coefficients in \e Z domain.
 *
 *  \return             Angle \f$\phi\in[-\pi,+\pi]\f$ on success, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double evalPolyGroupZ(double omega, const MATHPOLY *poly)
{
    int i;
    double cosOmega, sinOmega;
    double *pCoeff;

    double rpart = 0.0, ipart = 0.0;
    double rdiff = 0.0, idiff = 0.0;

    for (i = 0, pCoeff = poly->coeff; i <= poly->degree; i++, pCoeff++)
    {
        cosOmega = cos(i * omega);
        sinOmega = sin(i * omega);
        rpart += *pCoeff * cosOmega;
        ipart += *pCoeff * sinOmega;
        rdiff += *pCoeff * i * sinOmega;
        idiff += *pCoeff * i * cosOmega;
    } /* for */

    return mathTryDiv (rpart * idiff + ipart * rdiff, rpart * rpart + ipart * ipart);
} /* evalPolyGroupZ() */



/* FUNCTION *******************************************************************/
/** Returns the next input sample for a time response.
 *
 *  \param xtime        Time in seconds.
 *  \param sig          The signal for which the next sample shall be returned.
 *
 *  \return             Sample value associated with passed signal and time.
 ******************************************************************************/
static double timeResponseGetNext (double xtime, FLTSIGNAL sig)
{
    switch (sig)
    {
        case FLTSIGNAL_HEAVISIDE:
            return 1.0;

        case FLTSIGNAL_DIRAC:
            return (xtime == 0.0) ? 1.0 : 0.0;

        case FLTSIGNAL_USER:
        default:
            ASSERT (0);
    } /* switch */

    return 0.0;
} /* timeResponseGetNext() */


/* FUNCTION *******************************************************************/
/** Processes the next input value on a time response.
 *
 *  \param pWorkspace   Pointer to time response workspace, formerly created
 *                      by filterResponseTimeNew().
 *
 *  \return             The currently calculated output sample value. If that
 *                      value is GSL_POSINF or GSL_NEGINF further calls to
 *                      function timeResponseProcNext() shall not occur.
 *                      Check this situation for example by the help of
 *                      function gsl_isinf() or gsl_finite().
  ******************************************************************************/
static double timeResponseProcNext (FLTRESP_TIME_WORKSPACE *pWorkspace)
{
    int i;
    double *p;                                   /* pointer to in/out samples */
    const MATHPOLY *poly;

    double osample = 0.0;                                    /* output sample */
    double isample = timeResponseGetNext (pWorkspace->curTime,
                                          pWorkspace->sig);
    *pWorkspace->pCurIn = isample;         /* put new value into input buffer */
    p = pWorkspace->pCurIn;                             /* hold pointer to it */
    poly = &pWorkspace->pFilter->num;                    /* for faster access */

    FLTRESP_DEC(pWorkspace->pCurIn, pWorkspace->pInBuf, poly->degree);

    for (i = 0; i <= poly->degree; i++)
    {
        osample += *p * poly->coeff[i];
        FLTRESP_INC(p, pWorkspace->pInBuf, poly->degree);
    } /* for */

    p = pWorkspace->pLastOut;
    poly = &pWorkspace->pFilter->den;                    /* for faster access */

    for (i = 1; i <= poly->degree; i++)
    {
        osample -= *p * poly->coeff[i];
        FLTRESP_INC(p, pWorkspace->pOutBuf, poly->degree);
    } /* for */

    osample = mathTryDiv (osample, poly->coeff[0]);
    FLTRESP_DEC(pWorkspace->pLastOut, pWorkspace->pOutBuf, poly->degree);
    *pWorkspace->pLastOut = osample;

    return osample;
} /* timeResponseProcNext() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


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
double filterResponsePoly (double omega, const MATHPOLY *poly)
{
    return gsl_complex_abs (evalPolyZ(omega, poly));
} /* filterResponsePoly() */


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
double filterResponseMagnitude (double f, const FLTCOEFF* pFilter)
{
    double omega = 2.0 * M_PI * f / pFilter->f0;

    return mathTryDiv (filterResponsePoly (omega, &pFilter->num),
                       filterResponsePoly (omega, &pFilter->den));

} /* filterResponseMagnitude() */


/* FUNCTION *******************************************************************/
/** Computes the attenuation of a time-discrete system at a given frequency in
 *  \e Z domain.
    \f[
       A(f)=-20\log[H(f)]
    \f]
 *
 *  \param f            Frequency point in Hz.
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Magnitude value when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
double filterResponseAttenuation (double f, FLTCOEFF* pFilter)
{
    double magnitude = filterResponseMagnitude (f, pFilter);

    if (gsl_finite (magnitude))
    {
        return -20.0 * log10 (magnitude);
    } /* if */

    return magnitude;
} /* filterResponseAttenuation() */


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
double filterResponsePhase (double f, FLTCOEFF* pFilter)
{
    double omega = 2.0 * M_PI * f / pFilter->f0;
    double angle_num = evalPolyAngleZ(omega, &pFilter->num);
    double angle_den = evalPolyAngleZ(omega, &pFilter->den);

    if (gsl_finite (angle_num) && gsl_finite (angle_den))
    {
        return (angle_den - angle_num);
    } /* if */

    return GSL_POSINF;
} /* filterResponsePhase() */


/* FUNCTION *******************************************************************/
/** Computes the phase delay \f$B(\omega)/\omega\f$ of a time-discrete system
 *  at a given frequency in \e Z domain.
 *
 *  \param f            Frequency point in Hz (with \f$\omega=2\pi f\f$).
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Phase delay in rad s.
 ******************************************************************************/
double filterResponsePhaseDelay (double f, FLTCOEFF* pFilter)
{
    double result = filterResponsePhase (f, pFilter);

    if (gsl_finite (result))
    {
        result = mathTryDiv (result, 2.0 * M_PI * f);
    } /* if */

    return result;
} /* filterResponsePhaseDelay() */



/* FUNCTION *******************************************************************/
/** Computes the group delay \f$\textup{d}B(\omega)/\textup{d}\omega\f$ of a
 *  time-discrete system at a given frequency in \e Z domain.
 *
 *  \param f            Frequency point in Hz (with \f$\omega=2\pi f\f$).
 *  \param pFilter      Representation of time-discrete system.
 *
 *  \return             Group delay in sec.
 ******************************************************************************/
double filterResponseGroupDelay (double f, FLTCOEFF* pFilter)
{
    double omega = 2.0 * M_PI * f / pFilter->f0;
    double tg1 = evalPolyGroupZ(omega, &pFilter->num);           /* numerator */
    double tg2 = evalPolyGroupZ(omega, &pFilter->den);         /* denominator */

    if (gsl_finite (tg1) && gsl_finite (tg2))
    {
        return (tg1 - tg2) / pFilter->f0;
    } /* if */

    return GSL_POSINF;

} /* filterResponseGroupDelay() */


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
double filterResponseChar (double f, FLTCOEFF* pFilter)
{
    double result;

    double magnitude = filterResponseMagnitude (f, pFilter);

    if (gsl_finite (magnitude))
    {
        result = mathTryDiv(1.0, magnitude * magnitude);

        if (gsl_finite (result))
        {
            result = result - 1.0;

            if (result >= 0.0)
            {
                return sqrt (result);
            } /* if */
        } /* if */
    } /* if */

    return GSL_POSINF;

} /* filterResponseChar() */



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
 *  \todo               Allow user break
 ******************************************************************************/
FLTRESP_TIME_WORKSPACE* filterResponseTimeNew (double start, double stop,
                                               FLTSIGNAL type, const FLTCOEFF* pFilter)
{
    FLTRESP_TIME_WORKSPACE *pWorkspace;
    double xtime, t0;

    if ((start * pFilter->f0 > FLTRESP_TIME_SAMPLES_LIMIT) ||
        ((stop - start) * pFilter->f0 > FLTRESP_TIME_SAMPLES_LIMIT))
    {
        DEBUG_LOG ("Too many samples for time response calculation (%G, %G)",
                   start * pFilter->f0, (stop - start) * pFilter->f0);
        return NULL;
    } /* if */

    pWorkspace = g_malloc (sizeof (FLTRESP_TIME_WORKSPACE));

    if (pWorkspace == NULL)
    {
        return NULL;
    } /* if */

    pWorkspace->pInBuf =
        g_malloc ((1 + pFilter->num.degree) * sizeof(pFilter->num.coeff[0]));

    if (pWorkspace->pInBuf == NULL)
    {
        g_free (pWorkspace);
        return NULL;
    } /* if */

    pWorkspace->pOutBuf =
        g_malloc ((1 + pFilter->den.degree) * sizeof(pFilter->den.coeff[0]));

    if (pWorkspace->pOutBuf == NULL)
    {
        g_free (pWorkspace->pInBuf);
        g_free (pWorkspace);
        return NULL;
    } /* if */

    pWorkspace->pLastOut = pWorkspace->pOutBuf;
    pWorkspace->pCurIn = pWorkspace->pInBuf;
    pWorkspace->sig = type;                         /* set passed signal type */
    pWorkspace->pFilter = pFilter;

    memset (pWorkspace->pInBuf, 0,
            (1 + pFilter->num.degree) * sizeof(pFilter->num.coeff[0]));
    memset (pWorkspace->pOutBuf, 0,
            (1 + pFilter->den.degree) * sizeof(pFilter->den.coeff[0]));

    pWorkspace->curTime = 0.0;
    t0 = 1.0 / pFilter->f0;

    while (pWorkspace->curTime < start)   /* process all samples up to start */
    {
        timeResponseProcNext (pWorkspace);
        pWorkspace->curTime += t0;
    } /* while */


    xtime = pWorkspace->curTime;         /* hold current time as start value */
    pWorkspace->samples = 0;

    while (xtime <= stop)           /* count number of samples up to the end */
    {
        ++pWorkspace->samples;
        xtime += t0;
    } /* while */

    return pWorkspace;
} /* filterResponseTimeNew() */



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
double filterResponseTimeNext (FLTRESP_TIME_WORKSPACE *pWorkspace, double *pTime)
{
    double sample;

    *pTime = pWorkspace->curTime;       /* give current sample time to caller */
    sample = timeResponseProcNext (pWorkspace);
    pWorkspace->curTime += 1.0 / pWorkspace->pFilter->f0;
    return sample;
} /* filterResponseTimeNext() */



/* FUNCTION *******************************************************************/
/** Free's a time response workspace.
 *
 *  \param pWorkspace   Pointer to time response workspace, formerly created
 *                      by filterResponseTimeNew().
 *
 ******************************************************************************/
void filterResponseTimeFree (FLTRESP_TIME_WORKSPACE *pWorkspace)
{
    g_free(pWorkspace->pInBuf);
    g_free(pWorkspace->pOutBuf);
    g_free(pWorkspace);

} /* filterResponseTimeFree() */




/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
