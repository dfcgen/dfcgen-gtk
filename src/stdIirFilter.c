/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Standard IIR filter coefficients generator.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathFuncs.h"      /* HYPOT(), POW10() */
#include "mathMisc.h"       /* mathTryDiv(), mathDoubleSwap() */
#include "stdIirFilter.h"
#include "filterSupport.h"

#include <gsl/gsl_roots.h>                          /* roots solver functions */
#include <gsl/gsl_sf.h>                              /* all special functions */



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

/** Maximum number of iterations for finding \e Bessel lowpass cut-off
    frequency.
*/
#define STDIIR_BESSEL_MAXITER   1000


/** Maximum relative error \f$\varepsilon\f$ while finding \e Bessel lowpass
    cutoff frequency. The value is used to check the current range [a,b] wrt.
    to \f$|a-b|<\varepsilon \min(|a|,|b|)\f$.
 */
#define STDIIR_BESSEL_EPSREL    1.0E-9



/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* MACRO **********************************************************************/
/** Error code check and conditional return. The macro checks for an error code
 *  unequal to zero in \p cond. On that condition it calls filterFree(), then
 *  returns.
 *
 *  \param pFilter      Pointer to filter.
 *  \param cond         Condition to be checked (e.g. may be a function call).
 *  \param string       Text (string), which describes the error/cause.
 *
 ******************************************************************************/
#define STDIIR_ERROR_RET(pFilter, cond, string) \
    ERROR_RET_IF(cond, string, filterFree (pFilter); gsl_set_error_handler (oldHandler))


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static double bilinearInv (double fz, double f0);
static double drosselung(double att);
static int ftrHighpass(FLTCOEFF *pFilter, double omega);
static int ftrBandpass(FLTCOEFF *pFilter, double omega, double quality);
static double evalPolyAbsLaplace(double omega, MATHPOLY *poly);
static double magnitudeLaplace(double omega, FLTCOEFF *pFilter);
static double cutoffMagnitude(double omega, void *pFilter);
static double approxButterworth (FLTCOEFF *pFilter);
static double approxChebyPassband (double maxAtt, FLTCOEFF *pFilter);
static double approxChebyStopband (double minAtt, FLTCOEFF *pFilter);
static double approxCauer (STDIIR_TYPE type, double angle, double dr, FLTCOEFF *pFilter);
static double approxBessel (FLTCOEFF *pFilter);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


#ifdef DEBUG

/* FUNCTION *******************************************************************/
/** Logs filter coefficients.
 *
 *  \param pFilter      Representation of system in \e Laplace domain.
 *
 ******************************************************************************/
static void debugLogCoeffs (FLTCOEFF *pFilter)
{
    DEBUG_LOG ("L-domain numerator coefficients");
    mathPolyDebugLog (&pFilter->num);
    DEBUG_LOG ("L-domain denominator coefficients");
    mathPolyDebugLog (&pFilter->den);
} /* debugLogCoeffs() */

#endif



/* FUNCTION *******************************************************************/
/** Backward bilinear (frequency) transformation.
 *  The function transforms a frequency \f$f_z\f$ from Z domain into \e Laplace
 *  domain (\f$f_l\f$) using bilinear transformation:
 *  \f[
    f_l(s)=\frac{f_0}{\pi}\cdot\frac{\sin(2\pi f_z/f_0)}{1+\cos(2\pi f_z/f_0)}
    \f]
 *  That calculation is based on:
    \f[
    s=2 f_0\frac{z-1}{z+1}=2 f_0\frac{\exp(s/f_0) - 1}{\exp(s/f_0) + 1}
    \f]
 *
 *  \param fz           Frequency in Z domain.
 *  \param f0           Sampling frequency.
 *
 *  \return             Frequency in \e Laplace domain.
 ******************************************************************************/
static double bilinearInv(double fz, double f0)
{
    fz = 2.0 * M_PI * fz / f0;
    return f0 * M_1_PI * sin(fz) / (1.0 + cos(fz));
} /* bilinearInv() */



/* FUNCTION *******************************************************************/
/** Calculates discrimination (in German \e Drosselung) from attenuation. The
 *  calculation is based on \f$10^{A / 10} - 1\f$, which is the reverse of:
    \f[
        A(\omega)=-20\log H(\omega)=10\log[1+D^2(\omega)]
    \f]
 *
 *  \param att          Attenuation \f$A\f$.
 *
 *  \return             Discrimination.
 ******************************************************************************/
static double drosselung(double att)
{
    return sqrt (POW10 (0.1 * att) - 1.0);
} /* drosselung() */



/* FUNCTION *******************************************************************/
/** \e Laplace domain highpass transformation. The function transforms
 *  lowpass coefficients into a highpass by substitution:
    \f[
        s:=\frac{\omega_c^2}{s}
    \f]
 *  and corrects the numerator filter degree.
 *
 *  \param pFilter      Representation of system in \e Laplace domain.
 *  \param omega        Angular frequency of mirror point \f$\omega_c\f$,
 *                      the denormalized lowpass cut-off frequency.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
static int ftrHighpass (FLTCOEFF *pFilter, double omega)
{
    int deg = pFilter->den.degree - pFilter->num.degree; /* degree difference */

    ASSERT(deg >= 0);
    omega *= omega;

    ERROR_RET_IF (mathPolyTransform (&pFilter->den, 0, 0.0, omega, 1, 1.0, 0.0),
                  "Highpass transformation (denominator)");
    ERROR_RET_IF (mathPolyTransform (&pFilter->num, 0, 0.0, omega, 1, 1.0, 0.0),
                  "Highpass transformation (numerator)");

    mathPolyMulBinomial (&pFilter->num, deg, 1.0, 0);           /* correction */
    return 0;
} /* ftrHighpass() */


/* FUNCTION *******************************************************************/
/** \e Laplace domain bandpass transformation. The function transforms
 *  denormalized lowpass coefficients into a bandpass:
    \f{eqnarray*}
        s &:=& Q\left(s+\frac{\omega_m^2}{s}\right) \\
        s &:=& Q\frac{s^2+\omega_m^2}{s}
    \f}
 *  with
    \f{eqnarray*}
        \omega_m^2 &=& \omega_{c_1}\omega_{c_2} \\
                 Q &=& \frac{\omega_m}{\omega_{c_2} - \omega_{c_1}}
    \f}
 *  and corrects the numerator filter degree.
 *
 *  \param pFilter      Representation of system in \e Laplace domain.
 *  \param omega        Denormalized angular center (mid) frequency \f$\omega_m\f$.
 *  \param quality      Bandpass quality \f$Q\f$.

 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
static int ftrBandpass (FLTCOEFF *pFilter, double omega, double quality)
{
    int deg = pFilter->den.degree - pFilter->num.degree; /* degree difference */

    ASSERT (deg >= 0);

    ERROR_RET_IF (mathPolyTransform (&pFilter->den, 2, quality,
                                     omega * omega * quality, 1, 1.0, 0.0),
                  "Bandpass transformation (denominator)");
    ERROR_RET_IF (mathPolyTransform (&pFilter->num, 2, quality,
                                     omega * omega * quality, 1, 1.0, 0.0),
                  "Bandpass transformation (numerator)");

    /* Compensate the pre-factor z^{n-m} generated by function
       mathPolyTransform(), means correct the number of zeros.
     */
    mathPolyMulBinomial (&pFilter->num, deg, 1.0, 0);


    return 0;
} /* ftrBandpass() */


/* FUNCTION *******************************************************************/
/** Evaluates absolute magnitude associated with a polynomial in \e Laplace
 *  domain. The function returns the absolute value of polynomial \f$P(s)\f$
 *  for \f$s\Rightarrow j\omega\f$:
    \f{eqnarray*}
        P(s) &=& a_0+a_1 j\omega+a_2 (j\omega)^2+\cdots a_n (j\omega)^n \\
             &=& (\cdots(a_n j\omega + a_{n-1}) j\omega + a_{n-2}) j\omega
                 +\cdots+a_0
    \f}
 *
 *  \param omega        Angular frequency in rad/s.
 *  \param poly         Pointer to polynomial coefficients in \e Laplace domain.
 *
 *  \return             Absolut value in \e Laplace domain.
 ******************************************************************************/
static double evalPolyAbsLaplace(double omega, MATHPOLY *poly)
{
    gsl_complex result;

    int i = poly->degree;
    double *pCoeff = &poly->coeff[i];

    GSL_SET_COMPLEX(&result, 0.0, 0.0);

    while (i >= 0)
    {
        result = gsl_complex_add_real (gsl_complex_mul_imag (result, omega), *pCoeff);
        --i; --pCoeff;
    } /* for */

    return gsl_complex_abs(result);
} /* evalPolyAbsLaplace() */



/* FUNCTION *******************************************************************/
/** \e Laplace domain magnitude evaluation of a LTI-system by their
 *  coefficients.
 *
 *  \param omega        Angular frequency in rad/s.
 *  \param pFilter      Representation of system in \e Laplace domain.
 *
 *  \return             Magnitude value when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double magnitudeLaplace (double omega, FLTCOEFF *pFilter)
{
    return mathTryDiv(evalPolyAbsLaplace(omega, &pFilter->num),
                      evalPolyAbsLaplace(omega, &pFilter->den));

} /* magnitudeLaplace() */



/* FUNCTION *******************************************************************/
/** GSL wrapper/callback function for 3dB angular cut-off frequency finding in
 *  \e Laplace domain.
 *
 *  \param omega        Current angular frequency in rad/s.
 *  \param pFilter      Representation of LTI system in \e Laplace domain (FLTCOEFF *).
 *
 *  \return             Magnitude - \f$1/\sqrt{2}\f$value when successful
 *                      evaluated, else GSL_POSINF or GSL_NEGINF. Normally GSL
 *                      stops root finding under the  condition INF is returned
 *                      by this function.
 ******************************************************************************/
static double cutoffMagnitude (double omega, void *pFilter)
{
    double magnitude = magnitudeLaplace (omega, (FLTCOEFF *)pFilter);

    if (gsl_isinf (magnitude))
    {
        return magnitude;                       /* stop iteration immediately */
    } /* if */

    return magnitude - M_SQRT1_2;

} /* cutoffMagnitude() */



/* FUNCTION *******************************************************************/
/** Normalized \e Butterworth lowpass approximation. The function calculates
 *  poles and zeros of a \e Butterworth lowpass in \e Laplace domain with
 *  angular cut-off frequency \f$\omega=1\f$.
    \f[
        H(\omega)=\frac{1}{\sqrt{1+\omega^{2 n}}}
    \f]
 *
 *  \param pFilter      Pointer to L-domain filter coefficients/roots. Notice
 *                      that \p pFilter->den.degree defines the filter degree.
 *
 *  \return             Normalized angular cut-off frequency \f$\omega_c\f$.
 *                      In case of an error the value 0.0 will be returned (and
 *                      \a errno set).
 ******************************************************************************/
static double approxButterworth (FLTCOEFF *pFilter)
{
    int i;
    gsl_complex *pRoots;                                     /* roots pointer */

    double deltaPi = M_PI_2 / pFilter->den.degree;

    pFilter->num.degree = 0;                          /* set numerator degree */
    pFilter->factor = 1.0;

    for (i = 0, pRoots = pFilter->den.root; i < pFilter->den.degree; i++, pRoots++)
    {                                                      /* circle of roots */
        GSL_SET_COMPLEX (pRoots,
                         -sin((2 * i + 1) * deltaPi),
                         -cos((2 * i + 1) * deltaPi));
    } /* for */

    return 1.0;
} /* approxButterworth() */



/* FUNCTION *******************************************************************/
/** \e Chebyshev passband approximation. Best approximation of characteristic
 *  function by \e Chebyshev polynomials of first kind.
    \f{eqnarray*}
        H(\omega)       &=& \frac{1}{\sqrt{1+\sigma^2 \chebyT_n^2(\omega)}} \\
        \chebyT(\omega) &=& \cos(n\arccos\omega)\quad(\omega\leq 1) \\
        \chebyT(\omega) &=& \cosh(n\arcosh\omega)\quad(\omega>1)
    \f}
 *
 *  \param maxAtt       Maximum passband (ripple) attenuation in dB.
 *  \param pFilter      Pointer to L-domain filter coefficients. Notice that 
 *                      pFilter->den.degree defines the filter degree.
 *
 *  \return             Normalized 3dB angular cut-off frequency
 *                      \f$\omega_c=\cos(n^{-1}\arccos \sigma^{-1})\f$.
 *                      In case of an error the value 0.0 will be returned.
 ******************************************************************************/
static double approxChebyPassband (double maxAtt, FLTCOEFF *pFilter)
{
    int i;
    gsl_complex *pRoots;                                     /* roots pointer */

    int degree = pFilter->den.degree;
    double deltaPi = M_PI_2 / degree;
    double sigmaInv = 1.0 / drosselung (maxAtt);               /* 1/sigma > 1 */
    double reFactor = - sinh (asinh (sigmaInv) / degree);
    double imFactor = cosh (asinh (sigmaInv) / degree);

    pFilter->num.degree = 0;                          /* set numerator degree */
    pFilter->factor = sigmaInv * 2;

    for (i = 0, pRoots = pFilter->den.root; i < degree; i++, pRoots++)
    {
        GSL_SET_COMPLEX(pRoots,
                        reFactor * sin ((2 * i + 1) * deltaPi),
                        imFactor * cos ((2 * i + 1) * deltaPi));

        /* Coefficient of highest degree power of Chebyshev polynomial is
           2^{n - 1}. Because T(s)T(-s)=1/(1+sigma^2 T^2(s/j)) the inverse of
           2^{n-1}*sigma must be multiplied with the linear factor representation
           of polynomial.
        */
        pFilter->factor *= 0.5;
    } /* for */

    return mathPolyChebyInv (degree, sigmaInv);      /* 3dB cut-off frequency */
} /* approxChebyPassband() */



/* FUNCTION *******************************************************************/
/** \e Chebyshev stopband approximation (\e Chebyshev inverse lowpass). Best
 *  approximation of inverse characteristic function by \e Chebyshev polynomials
 *  of first kind.
    \f{eqnarray*}
        D(\omega) &=& \sigma\frac{\chebyT_n(\omega_s)}{\chebyT_n(\omega_s/\omega)} \\
        H(\omega) &=& \frac{\chebyT_n(\omega_s/\omega)}
                           {\sqrt{\chebyT_n^2(\omega_s/\omega)+\sigma^2\chebyT_n^2(\omega_s)}}
    \f}
 *  Because the cut-off frequency is defined in this implementation to be always
 *  the 3dB-point: \f$A_{max}=10\log(1+\sigma^2)=3dB\f$, the scaling factor
 *  \f$\sigma\f$ must be 1. The stopband frequency \f$\omega_s\f$ then can be
 *  evaluated by:
    \f[
    \chebyT_n(\omega_s)=\sqrt{10^{A_{min}/10}-1}
    \f]
 *
 *  \param minAtt       Minimum stopband attenuation in dB.
 *  \param pFilter      Pointer to L-domain filter coefficients. Notice that 
 *                      pFilter->den.degree defines the filter degree.
 *
 *  \return             Normalized 3dB angular cut-off frequency \f$\omega_c\f$,
 *                      which is 1 for this lowpass type. In case of an error
 *                      the value 0.0 will be returned.
 ******************************************************************************/
static double approxChebyStopband (double minAtt, FLTCOEFF *pFilter)
{
    gsl_complex *pRoot, omega_s;
    double arg;
    int i;

    int degree = pFilter->den.degree;
    double deltaPi = M_PI_2 / degree;
    double maxAmpl = drosselung(minAtt); /* max. stopband magnitude T_n(omega_s) */
    double reFactor = sinh(asinh(maxAmpl) / degree);
    double imFactor = cosh(asinh(maxAmpl) / degree);

    GSL_SET_COMPLEX (&omega_s, mathPolyChebyInv (degree, maxAmpl), 0.0);

    /* First calculate all denominator roots.
     */
    for (i = 0, pRoot = pFilter->den.root; i < degree; i++, pRoot++)
    {
        arg = (2 * i + 1) * deltaPi;
        GSL_SET_COMPLEX (pRoot, -reFactor * sin(arg), imFactor * cos(arg));
        *pRoot = gsl_complex_div (omega_s, *pRoot);
    } /* for */

    /* Now determine the numerator roots (all located at imag. axis). But notice
     * that for odd degree one zero is located at infinity, means the numerator
     * degree is less than the denominator degree. Determination of the
     * pre-factor is a little bit tricky and requires analysis of numerator and
     * denominator polynomial for both cases: the odd and the even degree (also
     * of Chebyshev polynomials).
     */
    if (GSL_IS_ODD (degree))
    {
        pFilter->factor = degree-- * GSL_REAL (omega_s) / maxAmpl;
    } /* if */
    else                                                       /* even degree */
    {
        pFilter->factor = 1.0 / HYPOT (1.0, maxAmpl);
    } /* else */

    for (i = 0; i < degree / 2; i++)
    {
        arg = GSL_REAL (omega_s) / cos ((2 * i + 1) * deltaPi);
        GSL_SET_COMPLEX (&pFilter->num.root[i], 0.0, arg);
        GSL_SET_COMPLEX (&pFilter->num.root[degree - 1 - i], 0.0, -arg);
    } /* for */

    pFilter->num.degree = degree;

    return 1.0;
} /* approxChebyStopband() */



/* FUNCTION *******************************************************************/
/** \e Cauer lowpass approximation based on first principal elliptic
 *  transformation. The function calculates numerator and denominator
 *  polynomial roots of \f$H(s)\f$ for \e Cauer type lowpass approximation.
 *  This type of approximation deals with the characteristic functions:
    \f{eqnarray*}
        D(s) &=& \frac{k}{\lambda} M\,s \prod_{\eta=2,4,\ldots}^{n-1}
                 \frac{\jsn^2(\eta K/n)+s^2}{\frac{1}{k^2 \jsn^2(\eta K/n)}+s^2}
                 \quad (n=\textup{ungerade}) \\
        D(s) &=& \frac{1}{\lambda} \prod_{\eta=1,3,\ldots}^{n-1}
                 \frac{\jsn^2(\eta K/n)+s^2}{\frac{1}{k^2 \jsn^2(\eta K/n)}+s^2}
                 \quad (n=\textup{gerade})
    \f}

 *  which are best approximations in \f$0<\omega<1\f$ and
 *  \f$1/k<\omega<\infty\f$ respectively. Each characteristic function may be
 *  written in fractional terms \f$D(s)=P(s)/Q(s)\f$ too:
    \f{eqnarray*}
        |H(s)|^2  &=& \frac{1}{1+\sigma^2 D(s)D(-s)} \\ 
        H(s)H(-s) &=& \frac{Q(s)Q(-s)}{Q(s)Q(-s)+\sigma^2 P(s)P(-s)}
    \f}

 *  If we take the special property of first principal elliptic transformation
 *  into account, namely the \e Laplace variable always is used squared (except
 *  one root at the origin in the odd case), which means \f$P(s)=\pm P(-s)\f$
 *  (neg. sign only for odd case) and \f$Q(s)=Q(-s)\f$, then:
    \f{eqnarray*}
        P(s)P(-s) &=& \pm P^2(s)\\
        Q(s)Q(-s) &=& \hphantom{\pm} Q^2(s)
    \f}
 *  With this in mind the substitution \f$s^2=x\f$ is possible and roots
 *  finding can be performed in variable x domain.
 *
 *  \param type         Type of \e Cauer lowpass (STDIIR_TYPE_CAUER1,
 *                      STDIIR_TYPE_CAUER2).
 *  \param angle        Elliptic module angle \f$\phi,\;k=\sin\phi\f$.
 *  \param dr           Characteristic function value in dB (min. stopband
 *                      attenuation or ripple).
 *  \param pFilter      Pointer to \e Laplace domain filter coefficients/roots.
 *                      The member \p pFilter->den.degree defines the filter
 *                      degree. The \e Laplace domain roots representation will
 *                      be returned in \p pFilter->num.root, \p pFilter->den.root
 *                      and \p pFilter->factor.
 *
 *  \return             Normalized angular cut-off frequency, for which the
 *                      equation \f$H(\omega)=1/\sqrt{2}\f$ holds.
 *                      This type of approximation is described by:
                        \f{eqnarray*}
                            H(\omega) &=& \frac{1}{\sqrt{1+\sigma^2 \jsn^2(u/M;\lambda)}}\\
                            \omega    &=& \jsn(u;k)
                        \f}
                        which in range \f$u=K+j \Im(u)=nM\Lambda+j \Im(u)\f$ can
                        be evaluated as:
                        \f{eqnarray*}
                            H(\omega) &=& \frac{1}{\sqrt{1+\sigma^2 \jnd^2(\Im u/M;\lambda')}}\\
                            \omega    &=& \jnd(\Im u;k')
                        \f}
                        Then \f$1\leq |D(\omega)|\leq 1/\lambda\f$ and
                        \f$1\leq\omega_c\leq 1/k\f$ is true and the 3dB point
                        condition is
                        \f[
                            \jsn(u/M;\lambda') = \frac{1-\sigma^2}{1-\lambda^2}
                        \f]
 *                      In case of an error the value 0.0 will be returned.
 *
 ******************************************************************************/
static double approxCauer (STDIIR_TYPE type, double angle, double dr, FLTCOEFF *pFilter)
{
    int i;
    gsl_sf_result result;    /* special GSL result structure (etmporary used) */
    gsl_complex *pRootNumD;         /* pointer to P(x) resp. P(s), with x=s^2 */
    gsl_complex *pRootDenD;         /* pointer to Q(x) resp. Q(s), with x=s^2 */
    gsl_complex *pRootNumH;             /* Pointer to numerator roots of H(s) */
    MATHPOLY denPoly2;     /* Q(x)^2, means squared denominator poly in x=s^2 */
    double snOdd, snEven, cn, dn;       /* Jacobian elliptic function results */
    double deltaK;             /* partial complete elliptic integral K(k) / n */
    double sigma;       /* scaling factor of characteristc function D(\omega) */
    double *pZero;           /* pointer to numerator root (zero) of polynomial*/
    double factor; /* multiplier of characteristic function (\f$k/\lambda M\f$ */
                             /* in odd case, \f$1/\lambda\f$ for even degree) */
    double multiplier = 1.0;                /* multiplier M of transformation */
    double lambda = 1.0;                 /* elliptic module of transformation */

    /* The module k^2 corresponds to kappa from Cauers original book or m in
       Abramowitz/Stegun.
    */
    double module = sin (angle / 180.0 * M_PI);
    double kappa = module * module;
    int degree = pFilter->den.degree;


    /* Allocate denominator polynomial of squared characteristic function D(s).
     */
    denPoly2.degree = degree;

    if (mathPolyMalloc(&denPoly2) != 0)
    {
        return 0.0;                                    /* out of memory space */
    } /* if */


    pFilter->num.degree = denPoly2.degree = (degree / 2) * 2;
    deltaK = gsl_sf_ellint_Kcomp (module, GSL_PREC_DOUBLE) / degree;

    /* Set start index for running variable during for-loop. For odd degree the
       running index must take 1,3,5,... and for even degree 2,4,6,.. The
       variable \a pZero ensures this, while \c i is counting from zero.
    */
    pZero = GSL_IS_ODD (degree) ? &snEven : &snOdd;

    /* Now prepare for-loop
     */
    pRootNumH = pFilter->num.root;                           /* zeros of H(s) */
    pRootNumD = pFilter->den.root;  /* P(x), means zeros of D^2(x) with x=s^2 */
    pRootDenD = denPoly2.root;                 /* Q(x), means poles of D^2(x) */

    for (i = 0; i < denPoly2.degree; i += 2)
    {
        if ((gsl_sf_elljac_e ((i + 1) * deltaK, kappa, &snOdd, &cn, &dn) != 0)
            || (gsl_sf_elljac_e ((i + 2) * deltaK, kappa, &snEven, &cn, &dn) != 0))
        {                                                             /* EDOM */
            DEBUG_LOG ("Jacobian elliptic function calculation has failed");
            mathPolyFree(&denPoly2);
            return 0.0;
        } /* if */


        /* Roots of numerator polynomial of transfer function are determined by
           denominator roots of characteristic function D(s).
        */
        GSL_SET_COMPLEX(pRootNumH, 0.0, 1.0 / (module * *pZero));
        GSL_SET_COMPLEX(pRootNumH + 1, 0.0, - GSL_IMAG(*pRootNumH));

        snOdd *= snOdd;                                     /* square to sn^2 */
        snEven *= snEven;

        multiplier *= snOdd / snEven;                   /* update M's product */
        lambda *= snOdd * snOdd;            /* update \lambda product by sn^4 */

        /* With x=s^2 the associated roots of D(s)D(-s) are located at
         * x = -sn^2(i K/n) respectively x = -1/(k sn(i K/n))^2 in x-domain.
         */
        GSL_SET_COMPLEX(pRootNumD, - *pZero, 0.0);          /* real root in x */
        *(pRootNumD + 1) = *pRootNumD;       /* double root of P(s)P(-s) in x */

        GSL_SET_COMPLEX(pRootDenD, -1.0 / (*pZero * kappa), 0.0);
        *(pRootDenD + 1) = *pRootDenD;       /* double root of Q(s)Q(-s) in x */

        pRootNumH += 2;
        pRootNumD += 2;
        pRootDenD += 2;
    } /* for */


    lambda *= gsl_pow_int (module, degree);              /* multiply with k^n */
    factor = 1.0 / lambda;     /* final result for factor at even degree poly */

    if (GSL_IS_ODD (degree))                                      /* odd case */
    {
        GSL_SET_COMPLEX (pRootNumD, 0.0, 0.0); /* root of P(s)P(-s) at origin */
        factor *= -module * multiplier;  /* odd case factor = - k / \lambda M */
    } /* if */


    /* For the following checks notice:
       - the ripple of first principal elliptic transformation is +/-1;
       - \sigma provides the scaling wrt. the desired ripple or minimum
         stopband attenuation
       - the minimum stopband value is 1 / \lambda.
    */
    switch (type)
    {
        case STDIIR_TYPE_CAUER1:              /* design by ripple attenuation */

            /* Check that the (resulting) min. stopband attenuation is greater
               than 3dB. Because the minimum value of D(omega) in stopband is
               1/lambda, the minimum attenuation is 10log(1+sigma^2/lambda^2).
            */
            sigma = dr;                                   /* sigma = dr / 1.0 */

            if (sigma < lambda * drosselung (STDIIR_STOPATT_MIN))
            {
                DEBUG_LOG ("Resulting stopband attenuation is less than 3dB");
                mathPolyFree (&denPoly2);

                return 0.0;        /* min. stopband attenuation less than 3dB */
            } /* if */

            break;


        case STDIIR_TYPE_CAUER2:       /* design by min. stopband attenuation */

            /* Check that the (resulting) passband ripple 10log(1+sigma^2) is
               less than 3dB.  Because the minimum value of D(omega) in stopband
               is 1/lambda, sigma is determined by 10log(1+sigma^2/lambda^2)=A_min.
            */
            sigma = lambda * dr;         /* scaling of approximation function */

            if (sigma > drosselung (STDIIR_RIPPLE_MAX))
            {
                DEBUG_LOG ("Resulting passband ripple is greater than 3dB");
                mathPolyFree(&denPoly2);

                return 0.0;                        /* ripple greater than 3dB */
            } /* if */

            break;


        default:
            ASSERT(0);
            break;

    } /* switch */


    /* Calculate denominator polynomial Q(x)^2 of squared transfer function
     * H(s)H(-s).
     */
    factor *= sigma;           /* factor now holds the product factor * sigma */
    pFilter->factor = 1.0 / fabs (factor); /* set scaling factor of H(s) roots */
    factor *= fabs (factor); /* square \sigma wrt. calculations of H(s)H(-s), */
                           /* but preserves the negative sign in the odd case */

    if ((mathPolyRoots2Coeffs (&pFilter->den, factor) != 0) /* coeffs of \sigma^2 P(x)^2 */
        || (mathPolyRoots2Coeffs (&denPoly2, 1.0) != 0))  /* coeffs of Q(x)^2 */
    {
        DEBUG_LOG ("Denominator polynomial coefficients calculation of squared"
                   " transfer function failed for Cauer filter");
        mathPolyFree (&denPoly2);
        return 0.0;
    } /* if */


    /* Calculate Q(s)Q(-s) + \sigma^2 P(s)P(-s) for x=s^2.
     */
    mathPolyAdd (&pFilter->den, &denPoly2, 1.0);
    mathPolyFree(&denPoly2);                              /* no longer needed */


    /* Now calculate the denominator roots (poles) of transfer function in x
     * domain.
     */
    if (mathPolyCoeffs2Roots (&pFilter->den) != GSL_SUCCESS)
    {
        DEBUG_LOG ("Roots finding for Cauer filter has failed");
        return 0.0;
    } /* if */


    /* Because roots finding is performed on H(s)H(-s) in x=s^2, the roots in
     * \e Laplace domain are located at s = +/- \sqrt{x}. To define stabil LTI
     * systems make the real part of all roots negative.
     */
    for (i = 0, pRootNumH = pFilter->den.root; i < degree; i++, pRootNumH++)
    {
        *pRootNumH = gsl_complex_negative (gsl_complex_sqrt (*pRootNumH));
    } /* for */


    /* Last calculate 3dB cut-off frequency (associated with nd(u/M; lambda')=
       1/sigma, omega=nd(u;k')). So the condition is:
       sn^2(u/M;lambda') = (1 - sigma^2) / (1 - lambda^2)
     */
    lambda = sqrt (1.0 - lambda * lambda);                        /* \lambda' */
    sigma = asin (sqrt (1.0 - sigma * sigma) / lambda);  /* argument to funcs */

    if (lambda == 1)                /* gsl_sf_ellint_F_e() cannot handle this */
    {
        result.val = log (fabs (tan (sigma / 2 + M_PI_4)));
    } /* if */
    else
    {
        if (gsl_sf_ellint_F_e (sigma, lambda, GSL_PREC_DOUBLE, &result) != GSL_SUCCESS)
        {                               /* may give EDOM if \lambda is near 1 */
            return 0.0;
        } /* if */
    } /* else */


    if ((gsl_sf_elljac_e (multiplier * result.val, 1.0 - kappa,
                          &snOdd, &cn, &dn) != GSL_SUCCESS))
    {
        return 0.0;
    } /* if */

    dn = mathTryDiv (1.0, dn);                                   /* nd = 1/dn */

    if (gsl_isinf (dn))
    {
        return 0.0;
    } /* if */

    return dn;
} /* approxCauer() */



/* FUNCTION *******************************************************************/
/** \e Bessel group delay approximation. The \e Bessel type lowpass is
 *  associated with the following transfer function:
    \f{eqnarray*}
        H(s)   &=& \frac{b_0}{B_n(s)} \\
        B_n(s) &=& (2 n - 1)B_{n-1}(s)+s^2 B_{n-2}(s) \\
        B_0(s) &=& 1 \\
        B_{-1}(s) &=& \frac{1}{s}
    \f}
 *
 *  \param pFilter      Pointer to \e Laplace domain filter coefficients. Notice
 *                      that \p pFilter->den.degree defines the filter degree.
 *                      The function returns only the polynomial (not roots)
 *                      representation of the \e Bessel lowpass.
 *
 *  \return             Normalized 3dB angular cut-off frequency \f$\omega_c\f$.
 *                      This value is calculated using \e Brent's algorithm.
 *                      In case of an error the value 0.0 will be returned.
 ******************************************************************************/
static double approxBessel (FLTCOEFF *pFilter)
{
    int status;
    gsl_function cutoffFunc;
    gsl_root_fsolver *pSolverSpace;

    const gsl_root_fsolver_type *solverAlgo = gsl_root_fsolver_brent;
    int iter = 0;                                        /* iteration counter */

    double omega = 0.0;                                  /* cut-off frequency */
    double omega1 = 0.0;           /* initial and current range value (lower) */
    double omega2 = 10.0 * pFilter->den.degree;          /* upper range value */

    cutoffFunc.function = cutoffMagnitude; /* set function for which a root to be find */
    cutoffFunc.params = pFilter;

    pFilter->num.degree = 0;

    if (mathPolyBessel (pFilter->den.degree, pFilter->den.coeff) == 0)
    {
        pFilter->num.coeff[0] = pFilter->den.coeff[0];      /* ensures H(0)=1 */
        pSolverSpace = gsl_root_fsolver_alloc (solverAlgo);

        if (pSolverSpace != NULL)
        {
            gsl_root_fsolver_set (pSolverSpace, &cutoffFunc, omega1, omega2);

            do
            {
                iter++;
                status = gsl_root_fsolver_iterate (pSolverSpace);

                if (status == GSL_SUCCESS)       /* next iteration succesful? */
                {
                    omega = gsl_root_fsolver_root (pSolverSpace);
                    omega1 = gsl_root_fsolver_x_lower (pSolverSpace);
                    omega2 = gsl_root_fsolver_x_upper (pSolverSpace);
                    status = gsl_root_test_interval (omega1, omega2,
                                                     0, STDIIR_BESSEL_EPSREL);
                } /* if */
            } /* do */
            while ((status == GSL_CONTINUE) && (iter < STDIIR_BESSEL_MAXITER));

            gsl_root_fsolver_free (pSolverSpace);

            if ((status == GSL_SUCCESS) && (iter < STDIIR_BESSEL_MAXITER))
            {
                DEBUG_LOG ("Bessel cut-off finding finished (%d iterations), omega = %G", iter, omega);
                return omega;                      /* cut-off frequency found */
            } /* if */
        } /* if */
    } /* if */

    return 0.0;
} /* approxBessel() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/**
 *  \brief Generates an IIR filter from standard approximations.
 *
 *  \attention          For bandpass/bandstop frequency transformation the
 *                      system order (degree) must be even.
 *  \note               The cutoff frequency is assumed to be the 3dB point
 *                      of magnitude response.
 *
 *  \param pDesign      Pointer to standard IIR design data.
 *  \param pFilter      Pointer to buffer which gets the generated filter.
 *                      Notice, that memory space for polynomials will be
 *                      allocated.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int stdIirFilterGen (STDIIR_DESIGN *pDesign, FLTCOEFF *pFilter)
{
    int i, degree;
    gsl_error_handler_t *oldHandler;
    double fc;                   /* Real-world (design) 3dB cut-off frequency */
    double normOmega; /* Effective (normalized) 3dB cut-off angular frequency */
    double bpQuality;                                        /* Q of BP or BS */

    /* Memory allocation for coefficients and roots space in numerator and
     * denominator polynomial.
     */
    pFilter->num.degree = pFilter->den.degree = pDesign->order;
    ERROR_RET_IF (filterMalloc(pFilter),
                  "Standard IIR filter memory allocation");

    /* All GSL errors are handled by the caller (starting from here), therefore
     * disable the abort behaviour of the GSL library.
     */
    oldHandler = gsl_set_error_handler_off ();

    pFilter->factor = 0.0;            /* no valid roots representation so far */

    switch (pDesign->ftr.type)                    /* frequency transformation */
    {
        case FTR_BANDPASS:
        case FTR_BANDSTOP:
        {
            double f1, f2;

            ASSERT(GSL_IS_EVEN(pDesign->order));
            pFilter->den.degree /= 2;    /* design lowpass with half degree */

            /* If center frequency \f$f_c\f$ is geometric, the bandwidth \f$B\f$
               may exceed half of center frequency, because \f$f_c^2 = f_1^2
               * f_2^2\f$ and \f$f_1 = sqrt{(B/2)^2 + f_c^2} - B/2\f$ and \f$f_2
               = sqrt{(B/2)^2 + f_c^2} + B/2, means the virtual linear center
               frequency is sqrt{(B/2)^2 + f_c^2}.
             * But when linear, the expression \f$f_c = 1/2 (f_1 + f_2)\f$ is valid,
               and this means \f$B = f_2 - f_1\f$.
             */
            if (pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC)
            {
                fc = HYPOT (pDesign->ftr.fc, 0.5 * pDesign->ftr.bw);
            } /* if */
            else
            {
                fc = pDesign->ftr.fc;
            } /* else */

            pDesign->cutoff = fc;            /* lowpass cutoff (return value) */

            f1 = bilinearInv (fc - 0.5 * pDesign->ftr.bw, pFilter->f0);
            STDIIR_ERROR_RET (pFilter, (f1 <= FLT_SAMPLE_MIN / 2) ? GSL_EFAILED : 0,
                              "Bandwidth vs. cutoff frequency mismatch");
            f2 = bilinearInv (fc + 0.5 * pDesign->ftr.bw, pFilter->f0);
            fc = sqrt (f1 * f2);                /* geometric center frequency */
            bpQuality = fc / (f1 - f2);                     /* quality = fc/B */

            break;
        } /* FTR_BANDPASS, FTR_BANDSTOP */


        case FTR_HIGHPASS :
            pDesign->cutoff = pDesign->ftr.fc;                /* return value */
            fc = bilinearInv (pDesign->ftr.fc, pFilter->f0);
            break; /* FTR_HIGHPASS */


        case FTR_NON: /* LOWPASS */
            fc = bilinearInv(pDesign->cutoff, pFilter->f0);
            break; /* FTR_NON */


        default:
            ASSERT(0);
    } /* switch */



    /* Now perform the desired approximation.
     */
    switch (pDesign->type)                          /* which type of lowpass? */
    {
        case STDIIR_TYPE_BUTTERWORTH:      /* flat magnitude response filters */
            normOmega = approxButterworth (pFilter);
            break;


        case STDIIR_TYPE_CHEBY: /* best approximation (in passband) ripple filter */
            normOmega = approxChebyPassband (pDesign->ripple, pFilter);
            break;


        case STDIIR_TYPE_CHEBYINV:
            normOmega = approxChebyStopband (pDesign->minatt, pFilter);
            break;


        case STDIIR_TYPE_CAUER1: /* input is max. ripple att. and module angle */
            normOmega = approxCauer (STDIIR_TYPE_CAUER1, pDesign->angle,
                                     drosselung(pDesign->ripple), pFilter);
            break;


        case STDIIR_TYPE_CAUER2: /* input is min. stopband att. and module angle */
            normOmega = approxCauer (STDIIR_TYPE_CAUER2, pDesign->angle,
                                     drosselung(pDesign->minatt), pFilter);
            break;


        case STDIIR_TYPE_BESSEL :
            normOmega = approxBessel (pFilter);
            break;

        default:
            ASSERT(0);
    } /* switch */


    STDIIR_ERROR_RET (pFilter, (normOmega == 0.0) ? GSL_EFAILED : 0,
                      "Standard IIR filter approximation has failed");

    if (pFilter->factor != 0.0)            /* lowpass specification by roots? */
    {                                          /* transform into coefficients */
        STDIIR_ERROR_RET (pFilter, mathPolyRoots2Coeffs (&pFilter->den, 1.0),
                          "Conversion of poles into coefficients has failed");
        STDIIR_ERROR_RET (pFilter, mathPolyRoots2Coeffs (&pFilter->num, pFilter->factor),
                          "Conversion of zeros into coefficients has failed");
    } /* if */


    pFilter->factor = 0.0;                /* roots are valid only in L-domain */

    switch (pDesign->ftr.type)
    {
        case FTR_HIGHPASS:                /* lowpass->highpass transformation */
            STDIIR_ERROR_RET (pFilter, ftrHighpass (pFilter, normOmega),
                              "Lowpass->Highpass transformation");
            break;


        case FTR_BANDSTOP:
            STDIIR_ERROR_RET (pFilter, ftrHighpass (pFilter, normOmega),
                              "Lowpass->Bandstop transformation");
                                                               /* fall trough */

        case FTR_BANDPASS:                           /* LP->BP transformation */
            STDIIR_ERROR_RET (pFilter, ftrBandpass (pFilter, normOmega, bpQuality),
                              "Lowpass->Bandpass transformation");
            break;  /* FTR_BANDPASS, FTR_BANDSTOP */


        case FTR_NON:
            break;

        default:
            ASSERT(0);
            break;
    } /* switch */


    /* Scale standard approximation interval to normOmega / fc and
       multiply with f0, which is the pre-factor of bilinear transformation.
     */
    normOmega = normOmega * pFilter->f0 / fc / M_PI;

    /* Denominator polynomial degree is greater/equal numerator polynomial
     * degree for standard approximations. Compensate later the pre-factor
     * (1+z)^{n-m} inserted by function mathPolyTransform().
     */
    degree = pFilter->den.degree - pFilter->num.degree;
    ASSERT (degree >= 0);

    STDIIR_ERROR_RET (pFilter, mathPolyTransform (&pFilter->den, 1, normOmega, -normOmega, 1, 1.0, 1.0),
                      "Bilinear transformation and denormalization (denominator)");
    STDIIR_ERROR_RET (pFilter, mathPolyTransform (&pFilter->num, 1, normOmega, -normOmega, 1, 1.0, 1.0),
                      "Bilinear transformation and denormalization (numerator)");

    for (i = 0; i < degree; i++)
    {
        mathPolyMulBinomial (&pFilter->num, 1, 1.0, 1.0);
    } /* for */


    for (i = 0; i <= pFilter->num.degree / 2; i++)       /* re-sort wrt. z^-1 */
    {
        mathDoubleSwap (&pFilter->num.coeff[i],
                        &pFilter->num.coeff[pFilter->num.degree - i]);

        mathDoubleSwap (&pFilter->den.coeff[i],
                        &pFilter->den.coeff[pFilter->den.degree - i]);
    } /* for */


    i = normFilterCoeffs (pFilter);

    if (FLTERR_CRITICAL (i))
    {
        filterFree (pFilter);
    } /* if */

    gsl_set_error_handler (oldHandler);

    return i;
} /* stdIirFilterGen() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
