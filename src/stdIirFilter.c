/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Standard IIR filter coefficients generator.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/src/stdIirFilter.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * \see
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathMisc.h"       /* includes config.h (include before GNU headers) */
#include "stdIirFilter.h"
#include "filterSupport.h"

#include <gsl/gsl_math.h>                              /* includes math.h too */
#include <gsl/gsl_poly.h>                               /* polynomial support */
#include <gsl/gsl_roots.h>                          /* roots solver functions */
#include <gsl/gsl_sf.h>                              /* all special functions */



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

/** Maximum number of iterations for finding \e Bessel lowpass cut-off
    frequency.
*/
#define STDIIR_BESSEL_MAXITER   10000


/** Maximum relative error \f$\varepsilon\f$ while finding \e Bessel lowpass
    cutoff frequency. The value is used to check the current range [a,b] wrt.
    to \f$|a-b|<\varepsilon \min(|a|,|b|)\f$.
 */
#define STDIIR_BESSEL_EPSREL    1.0E-6



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
    ERROR_RET_IF(cond, string, filterFree (pFilter))


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static double bilinearInv (double fz, double f0);
static double drosselung(double att);
static int ftrHighpass(FLTCOEFF *pFilter, double omega);
static int ftrBandpass(FLTCOEFF *pFilter, double omega, double quality);
static double evalPolyAbsLaplace(double omega, MATHPOLY *poly);
static double amplitudeLaplace(double omega, FLTCOEFF *pFilter);
static double cutoffAmplitude(double omega, void *pFilter);
static double approxButterworth (FLTCOEFF *pFilter);
static double approxChebyPassband (double maxAtt, FLTCOEFF *pFilter);
static double approxChebyStopband (double minAtt, FLTCOEFF *pFilter);
static double approxCauer (STDIIR_TYPE type, double angle, double dr, FLTCOEFF *pFilter);
static double approxBessel (FLTCOEFF *pFilter);


/* LOCAL FUNCTION DEFINITIONS *************************************************/



/* FUNCTION *******************************************************************/
/** Backward bilinear (frequency) transformation.
 *  The function corrects a frequency in Z domain to \e Laplace domain
 *  wrt. bilinear transformation:
 *  \f[
    f(s)=\frac{f_0}{\pi}\cdot\frac{\sin(2\pi f/f_0)}{1+\cos(2\pi f/f_0)}
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
    return sqrt(pow10(0.1 * att) - 1.0);
} /* drosselung() */



/* FUNCTION *******************************************************************/
/** \e Laplace domain highpass transformation. The function transforms
 *  denormalized lowpass coefficients into a highpass:
    \f[
        s:=\frac{\omega_c^2}{s}
    \f]
 *  and corrects the numerator filter degree.
 *
 *  \param pFilter      Representation of system in \e Laplace domain.
 *  \param omega        Angular frequency of mirror point \f$\omega_c\f$,
 *                      normally the lowpass cut-off frequiency.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
static int ftrHighpass(FLTCOEFF *pFilter, double omega)
{
    omega *= omega;                    /* mirror at omega_c^2 */

    ERROR_RET_IF(mathPolyTransform(&pFilter->den, 0, 1, 0.0, omega),
                 "Highpass transformation (denominator)");
    ERROR_RET_IF(mathPolyTransform(&pFilter->num, 0, 1, 0.0, omega),
                 "Highpass transformation (numerator)");

    ASSERT(pFilter->den.degree >= pFilter->num.degree);
    mathPolyShiftLeft(&pFilter->num, pFilter->den.degree - pFilter->num.degree);

    return 0;
} /* ftrHighpass() */


/* FUNCTION *******************************************************************/
/** \e Laplace domain bandpass transformation. The function transforms
 *  denormalized lowpass coefficients into a bandpass:
    \f[
        s:=Q\frac{s^2+\omega_m^2}{s}=Qs+\frac{Q\,\omega_m^2}{s}
    \f]
 *  with
    \f{eqnarray*}
        \omega_m^2 &=& \omega_{c_1}\omega_{c_2} \\
                 Q &=& \frac{\omega_m}{\omega_{c_2} - \omega_{c_1}}
    \f}
 *  and corrects the numerator filter degree.
 *
 *  \param pFilter      Representation of system in \e Laplace domain.
 *  \param omega        Angular center (mid) frequency \f$\omega_m\f$ in rad/s.
 *  \param quality      Bandpass quality \f$Q\f$.

 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
static int ftrBandpass(FLTCOEFF *pFilter, double omega, double quality)
{
    omega = omega * omega * quality;

    /* With H(s) = U(s) / V(s) the following calculations are performed here:
     * U(z) := s^m U(Q s + Q\omega_m^2 s^{-1})
     * V(z) := s^n V(Q s + Q\omega_m^2 s^{-1})
     */

    ERROR_RET_IF (mathPolyTransform(&pFilter->den, 1, 1, quality, omega),
                  "Bandpass transformation (denominator)");
    ERROR_RET_IF (mathPolyTransform(&pFilter->num, 1, 1, quality, omega),
                  "Bandpass transformation (numerator)");

    /* Correct the number of zeros
     */
    ASSERT(pFilter->den.degree >= pFilter->num.degree);
    mathPolyShiftLeft(&pFilter->num, pFilter->den.degree - pFilter->num.degree);

    return 0;
} /* ftrBandpass() */


/* FUNCTION *******************************************************************/
/** Evaluates absolute amplitude associated with a polynomial in \e Laplace
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
/** \e Laplace domain amplitude evaluation of a LTI-system by their coefficients.
 *
 *  \param omega        Angular frequency in rad/s.
 *  \param pFilter      Representation of system in \e Laplace domain.
 *
 *  \return             Amplitude value when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double amplitudeLaplace(double omega, FLTCOEFF *pFilter)
{
    return mathTryDiv(evalPolyAbsLaplace(omega, &pFilter->num),
                      evalPolyAbsLaplace(omega, &pFilter->den));

} /* amplitudeLaplace() */



/* FUNCTION *******************************************************************/
/** GSL wrapper/callback function for 3dB angular cut-off frequency finding in
 *  \e Laplace domain.
 *
 *  \param omega        Current angular frequency in rad/s.
 *  \param pFilter      Representation of LTI system in \e Laplace domain (FLTCOEFF *).
 *
 *  \return             Amplitude - \f$1/\sqrt{2}\f$value when successful
 *                      evaluated, else GSL_POSINF or GSL_NEGINF. Normally GSL
 *                      stops root finding under the  condition INF is returned
 *                      by this function.
 ******************************************************************************/
static double cutoffAmplitude(double omega, void *pFilter)
{
    double amplitude = amplitudeLaplace(omega, (FLTCOEFF *)pFilter);

    if (gsl_isinf(amplitude))
    {
        return amplitude;                       /* stop iteration immediately */
    } /* if */

    return amplitude - M_SQRT1_2;

} /* cutoffAmplitude() */



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
        GSL_SET_COMPLEX(pRoots,
                        -sin((2.0 * i + 1) * deltaPi),
                        -cos((2.0 * i + 1) * deltaPi));
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
    double sigmaInv = 1.0 / drosselung(maxAtt);                /* 1/sigma > 1 */

    double reFactor = - sinh(asinh(sigmaInv) / degree)
                      / mathPolyChebyInv(degree, sigmaInv);

    double imFactor = cosh(asinh(sigmaInv) / degree)
                      / mathPolyChebyInv(degree, sigmaInv);

    pFilter->num.degree = 0;                          /* set numerator degree */
    pFilter->factor = sigmaInv * 2;

    for (i = 0, pRoots = pFilter->den.root; i < degree; i++, pRoots++)
    {
        GSL_SET_COMPLEX(pRoots,
                        reFactor * sin((2.0 * i + 1) * deltaPi),
                        imFactor * cos((2.0 * i + 1) * deltaPi));

        /* Coefficient of highest degree power of Chebyshev polynomial is
           2^{n - 1}.  Because T(s)T(-s)=1/(1+sigma^2 T^2(s/j)) the inverse of
           2^{n-1}*sigma must be multiplied with the linearfactor representation
           of polynomial.
        */
        pFilter->factor *= 0.5;
    } /* for */

    return mathPolyChebyInv(degree, sigmaInv);
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
    double maxAmpl = drosselung(minAtt); /* max. stopband amplitude T_n(omega_s) */
    double reFactor = sinh(asinh(maxAmpl) / degree);
    double imFactor = cosh(asinh(maxAmpl) / degree);

    GSL_SET_COMPLEX (&omega_s, mathPolyChebyInv(degree, maxAmpl), 0.0);

    /* First calculate all denominator roots.
     */
    for (i = 0, pRoot = pFilter->den.root; i < degree; i++, pRoot++)
    {
        arg = (2.0 * i + 1) * deltaPi;
        GSL_SET_COMPLEX(pRoot, -reFactor * sin(arg), imFactor * cos(arg));
        *pRoot = gsl_complex_div(omega_s, *pRoot);
    } /* for */


    pFilter->num.degree = degree;

    /* Now determine the numerator roots (all located at j*omega).
     */
    for (i = 0, pRoot = pFilter->num.root; i < degree; i++, pRoot++)
    {
        GSL_SET_COMPLEX (pRoot, 0.0,
                         GSL_REAL(omega_s) / cos((2.0 * i + 1) * deltaPi));
    } /* for */

    /* Because the highest power coefficient in numerator and denominator
       polynomial of H(s) are equal, there is no need for a correcting factor in
       roots represantation.
    */
    pFilter->factor = 1.0;

    return 1.0;
} /* approxChebyStopband() */



/* FUNCTION *******************************************************************/
/** \e Cauer lowpass approximation based on first principal elliptic
 *  transformation. The function calculates numerator and denominator
 *  polynomial roots of \f$H(s)\f$ for \e Cauer type lowpass approximation.
 *  This type of approximation deals with the characteristic functions:
    \f{eqnarray*}
        D(s) &=& -j\,s\,\frac{k}{\lambda} M \prod_{\eta=2,4,\ldots}^{n-1}
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
 *  into acount, namely the \e Laplace variable always is used squared (except
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
                        which in range \f$u=K+j Im(u)=nM\Lambda+j \Im(u)\f$ can
                        be evaluated as:
                        \f{eqnarray*}
                            H(\omega) &=& \frac{1}{\sqrt{1+\sigma^2 \jnd^2(\Im u/M;\lambda')}}\\
                            \omega    &=& \jnd(\Im u;k')
                        \f}
                        Then \f$1\leq |D(\omega)|\leq 1/\lambda\f$ and
                        \f$1\leq\omega_c\leq 1/k\f$ is true.
 *                      In case of an error the value 0.0 will be returned (and
 *                      \a errno set);
 *
 ******************************************************************************/
static double approxCauer (STDIIR_TYPE type, double angle, double dr, FLTCOEFF *pFilter)
{
    int i;
    int err;
    gsl_poly_complex_workspace *pPolyWsp; /* polynomial roots finder workspace */
    gsl_complex *pRootNumD;         /* pointer to P(x) resp. P(s), with x=s^2 */
    gsl_complex *pRootDenD;         /* pointer to Q(x) resp. Q(s), with x=s^2 */
    gsl_complex *pRootNumH;             /* Pointer to numerator roots of H(s) */
    MATHPOLY denPoly2;     /* Q(x)^2, means squared denominator poly in x=s^2 */
    double snOdd, snEven, cn, dn;       /* Jacobian elliptic function results */
    double deltaK;             /* partial complete elliptic integral K(k) / n */
    double sigma;       /* scaling factor of characteristc function D(\omega) */
    double *pZero;          /* pointer too numerator root (zero) of polynomial*/
    double factor;       /* \f$k/\lambda M\f$ (odd) or \f$1/\lambda\f$ (even) */

    double multiplier = 1.0;                /* multiplier M of transformation */
    double lambda = 1.0;                 /* elliptic module of transformation */

    /* The module k^2 corresponds to kappa from Cauers original book or m in
       Abramowitz/Stegun.
    */
    double module = sin(angle / 180.0 * M_PI);
    double kappa = module * module;
    int degree = pFilter->den.degree;


    /* Allocate denominator polynomial of squared characteristic function D(s).
     */
    denPoly2.degree = degree;

    if (mathPolyMalloc(&denPoly2) != 0)
    {
        return 0.0;                                    /* out of memory space */
    } /* if */


    pFilter->num.degree = degree;
    denPoly2.degree = (degree / 2) * 2;

    deltaK = gsl_sf_ellint_Kcomp (module, GSL_PREC_DOUBLE) / degree;

    /* Set start index for running variable during for-loop. For odd degree the
       running index must take 1,3,5,... and for even degree 2,4,6,.. The
       variable \a pZero ensures this, while \c i is counting from zero.
    */
    pZero = GSL_IS_ODD(degree) ? &snEven : &snOdd;

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

    if (GSL_IS_ODD(degree))                                       /* odd case */
    {
        GSL_SET_COMPLEX(pRootNumD, 0.0, 0.0);  /* root of P(s)P(-s) at origin */
        denPoly2.coeff[degree] = 0.0;                   /* see for-loop later */
        factor *= module * multiplier;     /* odd case factor = k / \lambda M */
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

            if (sigma < lambda * drosselung(STDIIR_STOPATT_MIN))
            {
                mathPolyFree(&denPoly2);
                return 0.0;        /* min. stopband attenuation less than 3dB */
            } /* if */

            break;


        case STDIIR_TYPE_CAUER2:       /* design by min. stopband attenuation */

            /* Check that the (resulting) passband ripple 10log(1+sigma^2) is
               less than 3dB.  Because the minimum value of D(omega) in stopband
               is 1/lambda, sigma is determined by 10log(1+sigma^2/lambda^2)=A_min.
            */
            sigma = lambda * dr;         /* scaling of approximation function */

            if (sigma > drosselung(STDIIR_RIPPLE_MAX))
            {
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
    pFilter->factor = 1.0 / factor;       /* set scaling factor of H(s) roots */
    factor *= factor;                /* square wrt. calculations of H(s)H(-s) */

    if ((mathPolyRoots2Coeffs(&pFilter->den, 1.0) != 0)   /* coeffs of P(x)^2 */
        || (mathPolyRoots2Coeffs(&denPoly2, 1.0) != 0));  /* coeffs of Q(x)^2 */
    {
        mathPolyFree(&denPoly2);
        return 0.0;
    } /* if */


    /* Calculate Q(s)Q(-s) + \sigma^2 P(s)P(-s) in x=s^2.
     */
    for (i = 0; i <= degree; i++)
    {
        pFilter->den.coeff[i] = denPoly2.coeff[i]
                                + factor * pFilter->den.coeff[i];
    } /* for */


    mathPolyFree(&denPoly2);                              /* no longer needed */


    /* Now calculate the denominator roots (poles) of transfer function in x
     * domain.
     */
    pPolyWsp = gsl_poly_complex_workspace_alloc (degree + 1);

    if (pPolyWsp == NULL)
    {
        return 0.0;                                      /* not enough memory */
    } /* if */


    err = gsl_poly_complex_solve (pFilter->den.coeff, degree + 1, pPolyWsp,
                                  (gsl_complex_packed_ptr) pFilter->den.root);

    gsl_poly_complex_workspace_free (pPolyWsp);

    if (err != GSL_SUCCESS)
    {
        return 0.0;
    } /* if */


    /* Because roots finding is performed on H(s)H(-s) in x=s^2, the roots in
     * \e Laplace domain are located at s = +/- \sqrt{x}. To define stabil LTI
     * systems make the real part of all roots negative.
     */
    for (i = 0, pRootNumH = pFilter->den.root; i < degree; i++, pRootNumH++)
    {
        *pRootNumH = gsl_complex_negative(gsl_complex_sqrt (*pRootNumH));
    } /* for */


    /* Last calculate 3dB cut-off frequency (associated with nd(u/M; lambda')=
       1/sigma, omega=nd(u;k')). So the condition is:
       sn^2(u/M;lambda') = (1 - sigma^2) / (1 - lambda^2)
     */
    sigma = 1.0 - sigma * sigma;
    lambda = 1.0 - lambda * lambda;                      /* square of lambda' */

    if ((gsl_sf_elljac_e (multiplier * gsl_sf_ellint_F (asin(sqrt(sigma / lambda)),
                                                        sqrt(lambda), GSL_PREC_DOUBLE),
                          sqrt(1.0 - kappa), &snOdd, &cn, &dn) != 0))
    {
        return 0.0;
    } /* if */

    return 1.0 / sqrt(1.0 - kappa * snOdd * snOdd);
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

    cutoffFunc.function = cutoffAmplitude; /* set function for which a root to be find */
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
                return omega;                      /* cut-off frequency found */
            } /* if */
        } /* if */
    } /* if */

    return 0.0;
} /* approxBessel() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Generates an IIR filter from standard approximations. The cutoff frequency
 *  always is assumed to be the 3dB point of amplitude response.
 *
 *  \note               gsl_error_handler_t * gsl_set_error_handler (gsl_error_handler_t new_handler)
 *                      fpsetround()
 *
 *  \param pDesign      Pointer to standard IIR design data.
 *  \param pFilter      Pointer to buffer which gets the generated filter.
 *                      Notice, that memory space for polynomials will be
 *                      allocated.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int stdIirFilterGen (const STDIIR_DESIGN *pDesign, FLTCOEFF *pFilter)
{
    int i;
    double realOmega;    /* Real-world (design) 3dB cut-off frequency [rad/s] */
    double normOmega; /* Effective (normalized) 3dB cut-off angular frequency */
    double bpQuality;                                        /* Q of BP or BS */

    /* Memory allocation for coefficients and roots space in numerator and
     * denominator polynomial.
     */
    pFilter->num.degree = pFilter->den.degree = pDesign->order;
    ERROR_RET_IF (filterMalloc(pFilter),
                  "Standard IIR filter memory allocation");
    pFilter->factor = 0.0;            /* no valid roots representation so far */

    switch (pDesign->ftr.type)                    /* frequency transformation */
    {
        case FTR_BANDPASS:
        case FTR_BANDSTOP:
        {
            double fu, fo;

            pFilter->den.degree /= 2;      /* design lowpass with half degree */

            /* If center frequency \f$f_c\f$ is geometric, the bandwidth \f$B\f$
               may exceed half of center frequency, because \f$f_c^2 = f_o^2
               * f_u^2\f$ and \f$f_u = sqrt{(B/2)^2 + f_c^2} - B/2\f$ and \f$f_o
               = sqrt{(B/2)^2 + f_c^2} + B/2, means the virtual linear center
               frequency is sqrt{(B/2)^2 + f_c^2}.
             * But when linear the expression \f$f_c = 1/2 (f_o + f_u)\f$ is valid,
               and this means \f$B = f_o - f_u\f$.
             */
            if (pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC)
            {
                realOmega = hypot(pDesign->ftr.center, 0.5 * pDesign->ftr.bandwidth);
            } /* if */
            else
            {
                realOmega = pDesign->ftr.center;
            } /* else */

            fu = bilinearInv(realOmega - 0.5 * pDesign->ftr.bandwidth, pFilter->f0);
            fo = bilinearInv(realOmega + 0.5 * pDesign->ftr.bandwidth, pFilter->f0);

            realOmega = sqrt(fu * fo);           /* geometric center frequency */
            bpQuality = realOmega / (fo - fu);               /* quality = fc/B */

            break;
        } /* FTR_BANDPASS, FTR_BANDSTOP */


        case FTR_HIGHPASS :
            realOmega = bilinearInv(pDesign->ftr.bandwidth, pFilter->f0);
            break; /* FTR_HIGHPASS */


        case FTR_NON: /* LOWPASS */
            realOmega = bilinearInv(pDesign->cutoff, pFilter->f0);
            break; /* FTR_NON */


        default:
            ASSERT(0);
    } /* switch */


    realOmega *= 2.0 * M_PI;      /* transform into angular frequency [rad/s] */


    /* Now perform the desired approximation. All approximation functions must
     * calculate the (L-domain) roots. Coefficients will be re-calculated later.
     */
    switch (pDesign->type)                          /* Which type of lowpass? */
    {
        case STDIIR_TYPE_BUTTERWORTH:      /* flat magnitude response filters */
            normOmega = approxButterworth (pFilter);
            break;


        case STDIIR_TYPE_CHEBY: /* best approximation (in passband) ripple filter */
            normOmega = approxChebyPassband (pDesign->rippleAtt, pFilter);
            break;


        case STDIIR_TYPE_CHEBYINV:
            normOmega = approxChebyStopband (pDesign->minAtt, pFilter);
            break;


        case STDIIR_TYPE_CAUER1: /* input is max. ripple att. and module angle */
            normOmega = approxCauer (STDIIR_TYPE_CAUER1, pDesign->modAngle,
                                     drosselung(pDesign->rippleAtt), pFilter);
            break;


        case STDIIR_TYPE_CAUER2: /* input is min. stopband att. and module angle */
            normOmega = approxCauer (STDIIR_TYPE_CAUER1, pDesign->modAngle,
                                    drosselung(pDesign->minAtt), pFilter);
            break;


        case STDIIR_TYPE_BESSEL :
            normOmega = approxBessel (pFilter);
            break;

        default:
            ASSERT(0);
    } /* switch */


    STDIIR_ERROR_RET (pFilter, (normOmega == 0.0) ? GSL_EFAILED : 0,
                      "Standard IIR filter approximation");

    if (pFilter->factor != 0.0)              /* lowpass description by roots? */
    {                                          /* transform into coefficients */
        STDIIR_ERROR_RET (pFilter, mathPolyRoots2Coeffs(&pFilter->den, 1.0),
                          "Standard IIR filter approximation");
        STDIIR_ERROR_RET (pFilter, mathPolyRoots2Coeffs(&pFilter->num, pFilter->factor),
                          "Standard IIR filter approximation");
    } /* if */


    pFilter->factor = 0.0;       /* invalidate roots, because not in Z-domain */


    /* Denormalize filter coefficients to fit target cut-off frequency
     */
    STDIIR_ERROR_RET (pFilter, mathPolyTransform(&pFilter->den, 1, 0,
                                                 normOmega / realOmega, 0.0),
                      "Standard IIR filter frequency denormalization (denominator)");
    STDIIR_ERROR_RET (pFilter, mathPolyTransform(&pFilter->num, 1, 0,
                                                 normOmega / realOmega, 0.0),
                      "Standard IIR filter frequency denormalization (numerator)");

    switch (pDesign->ftr.type)
    {
        case FTR_HIGHPASS:                /* lowpass->highpass transformation */
            STDIIR_ERROR_RET (pFilter, ftrHighpass(pFilter, realOmega),
                              "Lowpass->Highpass transformation");
            break;


        case FTR_BANDSTOP:
            STDIIR_ERROR_RET (pFilter, ftrHighpass(pFilter, realOmega),
                              "Lowpass->Bandstop transformation");
                                                               /* fall trough */

        case FTR_BANDPASS:                           /* LP->BP transformation */
            STDIIR_ERROR_RET (pFilter, ftrBandpass(pFilter, realOmega, bpQuality),
                              "Lowpass->Bandpass transformation");
            break;  /* FTR_BANDPASS, FTR_BANDSTOP */


        case FTR_NON:
            break;

        default:
            ASSERT(0);
            break;
    } /* switch */


    for (i = 0; i <= pFilter->num.degree/2; i++)         /* re-sort wrt. z^-1 */
    {
        mathDoubleSwap(&pFilter->num.coeff[i],
                       &pFilter->num.coeff[pFilter->num.degree - i]);

        mathDoubleSwap(&pFilter->den.coeff[i],
                       &pFilter->den.coeff[pFilter->den.degree - i]);
    } /* for */

    return 0;
} /* stdIirFilterGen() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
