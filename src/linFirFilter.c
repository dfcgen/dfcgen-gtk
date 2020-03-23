/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Linear FIR filter coefficients generator.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathMisc.h"       /* includes config.h (include before GNU headers) */
#include "mathFuncs.h"      /* HYPOT() */
#include "linFirFilter.h"
#include "filterSupport.h"
#include "filterResponse.h"



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* FUNCTION *******************************************************************/
/** Coefficients generator function.
 *
 *  \param x            Argument \f$x\f$, typically the ratio \f$f_c/f_0\f$.
 *  \param poly         The polynomial for which the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 ******************************************************************************/
typedef int (*LINFIR_SYSGEN_FUNC)(double x, MATHPOLY *poly);


/* FUNCTION *****************************************************************/
/**
 *  \brief Mathematical function \f$y=f(x;a)\f$ with parameter \f$a\f$.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        additional window parameter \f$a\f$
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ****************************************************************************/
typedef double (*LINFIR_WINDOW_FUNC)(int step, int degree, double param);



/* LOCAL CONSTANT DEFINITIONS *************************************************/


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
#define LINFIR_ERROR_RET(pFilter, cond, string) \
    ERROR_RET_IF(cond, string, filterFree (pFilter); gsl_set_error_handler (oldHandler))


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int genRectangularSystem (double x, MATHPOLY *poly);
static int genGaussianSystem (double x, MATHPOLY *poly);
static int genCosineSystem (double x, MATHPOLY *poly);
static int genCosine2System (double x, MATHPOLY *poly);
static int genSquaredSystem (double x, MATHPOLY *poly);
static double firWinKaiser (int step, int degree, double param);
static double firWinRectangle (int step, int degree, double param);
static double firWinHamming (int step, int degree, double param);
static double firWinHanning (int step, int degree, double param);
static double firWinBlackman (int step, int degree, double param);
static double ftrHighpass (FLTCOEFF *pFilter);
static double ftrBandpass (FLTCOEFF *pFilter, double fc, double bw, BOOL geometric);



/* LOCAL FUNCTION DEFINITIONS *************************************************/



/* FUNCTION *******************************************************************/
/** Rectangular magnitude response system generator. Calculation of coefficient
 *  \f$c_i\f$ is based on the argument \f$x\f$ and polynomial degree \f$n\f$
 *  following the formula:
    \f[
        c_i=\frac{\sin\left[2\pi x\,(i-n/2)\right]}{2\pi x\,(i-n/2)}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param poly         The numerator polynomial of the linear FIR system (in
 *                      \e Z domain), where the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 ******************************************************************************/
static int genRectangularSystem (double x, MATHPOLY *poly)
{
    int i, err;
    gsl_sf_result result;

    double deg2 = poly->degree / 2.0;

    for (i = 0; i <= poly->degree; i++)
    {
        err = gsl_sf_sinc_e (2.0 * x * (i - deg2), &result);

        if (err != GSL_SUCCESS)
        {
            return err;
        } /* if */

        poly->coeff[i] = result.val;
    } /* for */

    return GSL_SUCCESS;
} /* genRectangularSystem() */



/* FUNCTION *******************************************************************/
/** Cosine magnitude response system generator. Calculation of coefficient
 *  \f$c_i\f$ is based on the argument \f$x\f$ and polynomial degree \f$n\f$
 *  following the formula:
    \f[
        c_i=\frac{\cos\left[4\pi x\,(i-n/2)\right]}{1-64\left[x\,(i-n/2)\right]^2}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param poly         The numerator polynomial of the linear FIR system (in
 *                      \e Z domain), where the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 ******************************************************************************/
static int genCosineSystem (double x, MATHPOLY *poly)
{
    int i;
    double tmp;

    double deg2 = poly->degree / 2.0;

    for (i = 0; i <= poly->degree; i++)
    {

        tmp = x * (i - deg2);
        poly->coeff[i] = mathTryDiv (cos (4.0 * M_PI * tmp),
                                     1.0 - 64 * tmp * tmp);
        if (gsl_isinf (poly->coeff[i]))
        {
            return GSL_EDOM;
        } /* if */
    } /* for */

    return GSL_SUCCESS;
} /* genCosineSystem() */



/* FUNCTION *******************************************************************/
/** Squared cosine magnitude response system generator. Calculation of
 *  coefficient \f$c_i\f$ is based on the argument \f$x\f$ and polynomial degree
 *  \f$n\f$ following the formula:
    \f[
        c_i=\frac{}{}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param poly         The numerator polynomial of the linear FIR system (in
 *                      \e Z domain), where the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 *  \todo               Give formula
 ******************************************************************************/
static int genCosine2System (double x, MATHPOLY *poly)
{
    int i, err;
    double tmp;
    gsl_sf_result result;

    double deg2 = poly->degree / 2.0;
    double constant = M_PI / acos (1.0 / sqrt (M_SQRT2));

    for (i = 0; i <= poly->degree; i++)
    {
        tmp = constant * x * (i - deg2);
        err = gsl_sf_sinc_e (tmp, &result);

        if (err != GSL_SUCCESS)
        {
            return err;
        } /* if */


        poly->coeff[i] = mathTryDiv (result.val, 1.0 - tmp * tmp);

        if (gsl_isinf (poly->coeff[i]))
        {
            return GSL_EDOM;
        } /* if */
    } /* for */

    return GSL_SUCCESS;
} /* genCosine2System() */



/* FUNCTION *******************************************************************/
/** Squared first order lowpass magnitude response system generator. Calculation
 *  of coefficient \f$c_i\f$ is based on the argument \f$x\f$ and polynomial
 *  degree \f$n\f$ following the formula:
    \f[
        c_i=\exp\left(-\,\frac{2\pi\,x\,|i-n/2|}{\sqrt{\sqrt{2}-1}}\right)
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param poly         The numerator polynomial of the linear FIR system (in
 *                      \e Z domain), where the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 ******************************************************************************/
static int genSquaredSystem (double x, MATHPOLY *poly)
{
    int i, err;
    gsl_sf_result result;

    double deg2 = poly->degree / 2.0;
    double constant = 2.0 * M_PI / sqrt (M_SQRT2 - 1.0);

    for (i = 0; i <= poly->degree; i++)
    {
        err = gsl_sf_exp_e (-constant * x * fabs ((i - deg2)), &result);

        if (err != GSL_SUCCESS)
        {
            return err;
        } /* if */

        poly->coeff[i] = result.val;
    } /* for */

    return GSL_SUCCESS;
} /* genSquaredSystem() */



/* FUNCTION *******************************************************************/
/** \e Gaussian magnitude response system generator. Calculation of coefficient
 *  \f$c_i\f$ is based on the argument \f$x\f$ and polynomial degree \f$n\f$
 *  following the formula:
    \f[
        c_i=\exp\left\{-\frac{2[\pi x\,(i-n/2)]^2}{\log 2}\right\}
    \f]
 *
 *  \param x            Argument \f$x\f$.
 *  \param poly         The numerator polynomial of the linear FIR system (in
 *                      \e Z domain), where the coefficients shall be set.
 *
 *  \return             Zero on success, else an error code (from errno.h or
 *                      gsl_errno.h).
 ******************************************************************************/
static int genGaussianSystem (double x, MATHPOLY *poly)
{
    int i, err;
    double tmp;
    gsl_sf_result result;

    double deg2 = poly->degree / 2.0;

    for (i = 0; i <= poly->degree; i++)               /* set all coefficients */
    {
        tmp = x * (i - deg2) * M_PI;
        err = gsl_sf_exp_e (-2.0 * tmp * tmp / M_LN2, &result);

        if (err != GSL_SUCCESS)
        {
            return err;
        } /* if */

        poly->coeff[i] = result.val;
    } /* for */

    return GSL_SUCCESS;
} /* genGaussianSystem() */



/* FUNCTION *******************************************************************/
/** Rectangular window function.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        parameter (unused)
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double firWinRectangle (int step, int degree, double param)
{
    (void) param;
    return mathFuncRectangle ((double) step / degree);
} /* firWinRectangle() */


/* FUNCTION *******************************************************************/
/** \e Hamming window function.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        parameter (unused)
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double firWinHamming (int step, int degree, double param)
{
    (void) param;
    return mathFuncHamming ((double) step / degree);
} /* firWinHamming() */


/* FUNCTION *******************************************************************/
/** \e Hanning window function.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        parameter (unused)
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double firWinHanning (int step, int degree, double param)
{
    (void) param;
    return mathFuncHanning ((double) (step + 1) / (degree + 2));
} /* firWinHanning() */


/* FUNCTION *******************************************************************/
/** \e Blackman window function.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        parameter (unused)
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double firWinBlackman (int step, int degree, double param)
{
    return mathFuncBlackman ((double) (step + 1) / (degree + 2));
} /* firWinBlackman() */


/* FUNCTION *******************************************************************/
/** \e Kaiser window function.
 *
 *  \param step         windowing step, running from 0 to \p degree
 *  \param degree       degree of filter
 *  \param param        parameter \f$\alpha\f$ of \e Kaiser window
 *
 *  \return             Result \f$y\f$ when successful evaluated, else
 *                      GSL_POSINF or GSL_NEGINF. Use the functions gsl_isinf()
 *                      or gsl_finite() for result checking.
 ******************************************************************************/
static double firWinKaiser (int step, int degree, double param)
{
    return mathFuncKaiser ((double) step / degree, param);
} /* firWinKaiser() */


/* FUNCTION *******************************************************************/
/** \e Z domain highpass transformation of a linear FIR lowpass. The function
 *  transforms lowpass coefficients into a highpass using the following formula:
    \f[
        c_{n/2} := c_{n/2} - H(0)
    \f]
 *  The resulting magnitude response is:
    \f[
        |H_{HP}(f)| = H(0) - |H_{LP}(f)|
    \f]
 *
 *  \param pFilter      Pointer to linear FIR lowpass that coefficients shall
 *                      transformed.
 *
 *  \return             The frequency where the magnitude response has it's
 *                      maximum.
 * \todo Handle odd degree
 ******************************************************************************/
static double ftrHighpass (FLTCOEFF *pFilter)
{
    MATHPOLY *poly = &pFilter->num;
    double magnitude = gsl_poly_eval (poly->coeff, poly->degree + 1, 1.0);

    if (GSL_IS_EVEN (poly->degree))
    {
        poly->coeff[poly->degree / 2] -= magnitude;
    } /* if */

    return pFilter->f0 / 2.0 - DBL_EPSILON;
} /* ftrHighpass() */



/* FUNCTION *******************************************************************/
/** \e Z domain bandpass transformation of a linear FIR lowpass. The function
 *  transforms lowpass coefficients into a bandpass using the following formula:
    \f[
        c_i := c_i \cos\left[2\pi\,(i-\frac{n}{2})\,\frac{f_c}{f_0}\right]
    \f]
 *  If \f$f_c\f$ shall be interpreted as a geometric center frequency, then its
 *  arithmetic counterpart is derived by:
    \f[
        f_c := \sqrt{\left(\frac{\Delta f}{2}\right)^2+f_c^2}
    \f]
 *  The resulting magnitude response is:
    \f[
        \frac{|H_{BP}(f)|}{2} = |H_{LP}(f-f_c)| + |H_{LP}(f+f_c)|
    \f]
 *  which may produce an aliasing effect at \f$f=0\f$.
 *
 *
 *  \param pFilter      Pointer to linear FIR lowpass that coefficients shall
 *                      transformed.
 *  \param fc           Arithmetic or geometric center frequency.
 *  \param bw           Bandwidth \f$\Delta f\f$.
 *  \param geometric    If TRUE then the center frequency is geometric, else
 *                      arithmetic.
 *
 *  \return             The frequency where the magnitude response has it's
 *                      maximum.
 ******************************************************************************/
static double ftrBandpass (FLTCOEFF *pFilter, double fc, double bw, BOOL geometric)
{
    int i;
    double factor, tmp;

    MATHPOLY *poly = &pFilter->num;
    double deg2 = poly->degree / 2.0;                       /* half of degree */

    if (geometric)
    {
        fc = HYPOT (fc, 0.5 * bw);
    } /* if */

    factor = 2 * M_PI * fc / pFilter->f0;

    for (i = 0; i <= poly->degree / 2; i++)             /* modify coefficients */
    {
        tmp = cos (factor * (i - deg2));
        poly->coeff[i] *= tmp;
        poly->coeff[poly->degree - i] *= tmp;
    } /* for */

    return fc;
} /* ftrBandpass() */



/* FUNCTION *******************************************************************/
/** \e Z domain bandstop transformation.
 *
 *  \param pFilter      Pointer to linear FIR lowpass that coefficients shall
 *                      transformed.
 *  \param fc           Arithmetic or geometric center frequency.
 *  \param bw           Bandwidth \f$\Delta f\f$.
 *  \param geometric    If TRUE then the center frequency is geometric, else
 *                      arithmetic.
 *
 *  \return             The frequency where the magnitude response has it's
 *                      maximum.
 *  \todo Handle GSL_POSINF for call to filterResponsePoly()
 ******************************************************************************/
static double ftrBandstop (FLTCOEFF *pFilter, double fc, double bw, BOOL geometric)
{
    double magnitude;

    MATHPOLY *poly = &pFilter->num;

    fc = ftrBandpass (pFilter, fc, bw, geometric);
    magnitude = filterResponsePoly (2 * M_PI * fc / pFilter->f0, poly);

    if (GSL_IS_EVEN (poly->degree))
    {
        poly->coeff[poly->degree / 2] -= magnitude;
    } /* if */

    return pFilter->f0 / 2.0 - DBL_EPSILON;
} /* ftrBandstop() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Generates a linear FIR filter. The cutoff frequency always is assumed to be
 *  the 3dB point of magnitude response.
 *
 *  \param pDesign      Pointer to linear FIR filter design data.
 *  \param pFilter      Pointer to buffer which gets the generated filter.
 *                      Notice, that memory space for polynomials is allocated
 *                      in the function linFirFilterGen().
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 *  \todo               Implement semantic checks on bandwidth, center and cutoff
 *                      frequency wrt. sample frequency.
 *  \todo               Correct the lowpass design cutoff frequency (see Bessel
 *                      lowpass) when performing frequency transformations
 *                      (depends on characteristic too, significant on perfect
 *                      lowpass).
 *  \todo               Use fpsetround()?
 ******************************************************************************/
int linFirFilterGen (LINFIR_DESIGN *pDesign, FLTCOEFF *pFilter)
{
    static const LINFIR_SYSGEN_FUNC genFuncs[LINFIR_TYPE_SIZE] =
    {
        genRectangularSystem,                             /* LINFIR_TYPE_RECT */
        genCosineSystem,                                   /* LINFIR_TYPE_COS */
        genCosine2System,                                 /* LINFIR_TYPE_COS2 */
        genSquaredSystem,                                /* LINFIR_TYPE_GAUSS */
        genGaussianSystem                                  /* LINFIR_TYPE_SQR */
    };

    static const LINFIR_WINDOW_FUNC winFuncs[LINFIR_DSPWIN_SIZE] =
    {
        firWinRectangle,                                /* LINFIR_DSPWIN_RECT */
        firWinHamming,                               /* LINFIR_DSPWIN_HAMMING */
        firWinHanning,                               /* LINFIR_DSPWIN_HANNING */
        firWinBlackman,                             /* LINFIR_DSPWIN_BLACKMAN */
        firWinKaiser                                  /* LINFIR_DSPWIN_KAISER */
    };


    int i, err;
    gsl_error_handler_t *oldHandler;
    double fnorm;                                /* unity normation frequency */

    pFilter->factor = 0.0;                 /* roots are invalid (unused here) */

    /* Memory allocation for coefficients and roots space in numerator and
     * denominator polynomial.
     */
    pFilter->den.degree = 0;                           /* this is a FIR filter*/
    pFilter->num.degree = pDesign->order;
    ERROR_RET_IF (filterMalloc(pFilter),
                  "Linear FIR filter memory allocation");
    pFilter->den.coeff[0] = 1.0;

    /* All GSL errors are handled by the caller (starting from here), therefore
     * disable the abort behaviour of the GSL library.
     */
    oldHandler = gsl_set_error_handler_off ();

    pFilter->factor = 0.0;            /* no valid roots representation so far */

    switch (pDesign->ftr.type)                    /* frequency transformation */
    {
        case FTR_BANDPASS:
        case FTR_BANDSTOP:
            pDesign->cutoff = 0.5 * pDesign->ftr.bw;
            break; /* FTR_BANDPASS, FTR_BANDSTOP */


        case FTR_HIGHPASS :
            pDesign->cutoff = pDesign->ftr.fc;
            break; /* FTR_HIGHPASS */


        case FTR_NON: /* LOWPASS */
            break; /* FTR_NON */


        default:
            ASSERT(0);
    } /* switch */


    ASSERT (pDesign->type < LINFIR_TYPE_SIZE);
    err = genFuncs[pDesign->type] (pDesign->cutoff / pFilter->f0, &pFilter->num);
    LINFIR_ERROR_RET (pFilter, err, "Linear FIR filter generation has failed");

    for (i = 0; i <= pFilter->num.degree; i++)     /* apply window function */
    {
        fnorm = winFuncs[pDesign->dspwin] (i, pFilter->num.degree, pDesign->winparm);

        if (gsl_finite (fnorm))
        {
            pFilter->num.coeff[i] *= fnorm;
        } /* if */
    } /* for */


    /* Frequency transformation
     */
    switch (pDesign->ftr.type)
    {
        case FTR_BANDSTOP:                /* lowpass->bandstop transformation */
            fnorm = ftrBandstop (pFilter, pDesign->ftr.fc, pDesign->ftr.bw,
                                 pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC);
            break;


        case FTR_BANDPASS:
            fnorm = ftrBandpass (pFilter, pDesign->ftr.fc, pDesign->ftr.bw,
                                 pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC);
            break;


        case FTR_HIGHPASS:                /* lowpass->highpass transformation */
            fnorm = ftrHighpass (pFilter);
            break;


        case FTR_NON:                                              /* lowpass */
            fnorm = 0.0;
            break;


        default:
            ASSERT(0);
            break;
    } /* switch */


    i = normFilterMagnitude (pFilter, fnorm, 1.0);

    if (FLTERR_CRITICAL (i))
    {
        filterFree (pFilter);
    } /* if */

    gsl_set_error_handler (oldHandler);

    return i;
} /* linFirFilterGen() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
