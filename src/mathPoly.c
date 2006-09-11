/******************************************************************************/
/**
 * \file
 *           Polynomial functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/src/mathPoly.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathPoly.h"
#include "mathMisc.h"

#include <string.h>


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static double chebyT(double degree, double x);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Evaluates \e Chebyshev function (polynomial) of first kind.
 * if |x| < 1 use : T(x) = cos(n*arcos(x))
 * if  x >= 1 use : T(x) = cosh(n*arcosh(x))    because cosh(jx) = cos(x)
 * if x < -1 you get a little problem, because cosh(x+j*Pi) = - cosh(x)
 * and therfore arcosh(-x) = arcosh(|x|) + j*Pi ,
 * then use T(x) = (-1)^n * cosh(n*arcosh(|x|))
 *
 *  \param degree       Degree \f$n\f$ of \e Chebyshev polynomial.
 *  \param x            Argument.
 *
 *  \return             Value of \e Chebyshev polynomial \f$\chebyT_n(x)\f$.
 *  \todo               Improve description by the help of LaTeX
 ******************************************************************************/
static double chebyT(double degree, double x)
{
    double result;

    if (fabs(x) < 1.0)
    {
        return cos(degree * acos(x));
    } /* if */

    result = cosh(degree * gsl_acosh(fabs(x)));

    if ((x >= 1.0) || (!GSL_IS_ODD((int)degree)))
    {
        return result;
    } /* if */

    return -result;
} /* chebyT() */




/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Allocates memory space for the coefficients of a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
int mathPolyMallocCoeffs (MATHPOLY *poly)
{
    poly->coeff = MALLOC ((1 + poly->degree) * sizeof(poly->coeff[0]));

    if (poly->coeff == NULL)
    {
        return ENOMEM;
    } /* if */

    return 0;
} /* mathPolyMallocCoeffs() */


/* FUNCTION *******************************************************************/
/** Allocates memory space for the roots of a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
int mathPolyMallocRoots (MATHPOLY *poly)
{
    poly->root = MALLOC (GSL_MAX_INT (1, poly->degree) * sizeof(poly->root[0]));

    if (poly->root == NULL)
    {
        return ENOMEM;
    } /* if */

    return 0;
} /* mathPolyMallocRoots() */


/* FUNCTION *******************************************************************/
/** Allocates memory space for a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
int mathPolyMalloc (MATHPOLY *poly)
{
    int ret = mathPolyMallocCoeffs (poly);

    if (ret != 0)
    {
        poly->root = NULL;
        return ret;
    } /* if */

    ret = mathPolyMallocRoots (poly);

    if (ret != 0)
    {
        FREE (poly->coeff);
        poly->coeff = NULL;
    } /* if */

    return ret;
} /* mathPolyMalloc() */



/* FUNCTION *******************************************************************/
/** Frees memory space allocated for a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 ******************************************************************************/
void mathPolyFree(MATHPOLY *poly)
{
    if (poly->coeff != NULL)
    {
        FREE (poly->coeff);
        poly->coeff = NULL;
    } /* if */

    if (poly->root != NULL)
    {
        FREE (poly->root);
        poly->root = NULL;
    } /* if */
} /* mathPolyFree() */



/* FUNCTION *******************************************************************/
/** Calculates \e Bessel polynomial of n'th order in \p coeff.
    \f{eqnarray*}
        B_n &=& (2 n - 1) * B_{n-1} + s^2 B_{n-2} \\ 
        B_0 &=& 1 \\ 
        B_1 &=& 1+s
    \f}
 *
 *
 *  \param degree       Degree of polynomial.
 *  \param coeff        Array to be used to fill-in the calculated coefficients.
 *
 *  \return             0 on success, else an error number.
 ******************************************************************************/
int mathPolyBessel (int degree, double coeff[])
{
    int ord, i;
    double coeffOld;

    double *polyOld_2 = MALLOC ((1 + degree) * sizeof(coeff[0]));

    coeff[0] = 1.0;                        /* B[0] = 1 */

    if (degree > 0)
    {
        coeff[1] = 1.0;                    /* B[1] = 1+s */
    } /* if */

    if (polyOld_2 == NULL)
    {
        return ENOMEM;
    } /* if */

    polyOld_2[0] = coeff[0];                                  /* B[n-2] (n=2) */

    for (ord = 2; ord <= degree; ord++)
    {
        coeff[ord] = 0.0;

        for (i = ord; i >= 0; i--)       /* from top to bottom for all coeffs */
        {
            coeffOld = coeff[i];                                      /* save */
            coeff[i] *= 2 * ord - 1;

            if (i > 1)
            {
                coeff[i] += polyOld_2[i-2];             /* new coeff complete */
            } /* if */

            polyOld_2[i] = coeffOld;                     /* save B[n-2] coeff */
        } /* for */
    } /* for */

    FREE (polyOld_2);

    return 0;
} /* mathPolyBessel() */


/* FUNCTION *******************************************************************/
/** Multiplies polynomial \f$p(z)\f$ by \f$z^n\f$, means shifts all
 *  coefficients left.
 *
 *  \param poly         Pointer to polynomial that coefficients \p poly->coeff
 *                      shall be shifted.
 *  \param n            Number of coefficients shift to left.
 *
 ******************************************************************************/
void mathPolyShiftLeft(MATHPOLY *poly, int n)
{
    int idxSrc = poly->degree;
    int idxDest = idxSrc + n;

    poly->degree = idxDest;                   /* correct degree of polynomial */

    while (idxSrc >= 0)
    {
        poly->coeff[idxDest--] = poly->coeff[idxSrc--];
    } /* while */


    while (idxDest >= 0)
    {
        poly->coeff[idxDest--] = 0.0;            /* clear lowest coefficients */
    } /* while */
} /* mathPolyShiftLeft() */



/* FUNCTION *******************************************************************/
/** Transforms polynomial coefficients for fractional variable substitution.
 *  The function replaces the variable \f$z\f$ in (source domain)
    \f{eqnarray*}
    p(z) &=& c_r z^r + c_{r-1} z^{r-1}+ \cdots + c_2 z^2 + c_1 z + c_0 \\
         &=& (\cdots(((c_r z + c_{r-1})z + c_{r-2})z + c_{r-3})z+\cdots+c_1)z+c_0
    \f}
 *  by
    \f{eqnarray*}
           z &=& a z^m + \frac{b}{z^n}\qquad a,b\in R;\quad m,n\in N \\
        p(z) &=& z^{r n} p(a z^m + b z^{-n})
    \f}
 *  Set \f$p_i(z)=u_i(z)/v_i(z)\f$ the transformation algorithm for
 *  coefficients \f$c_i\f$ is as follows:
    \f{eqnarray*}
    p_{i+1}(z) &=& p_{i}(z) (a z^m + b z^{-n}) + c_{r-i} \\
               &=& a z^m\, p_{i}(z) + b z^{-n} \, p_{i}(z) + c_{r-i} \\
               &=& p_{i}(z)\frac{a z^{n+m}+b}{z^n} + c_{r-i} \\
    \frac{u_{i+1}(z)}{v_{i+1}(z)} &=& 
    \frac{a z^{n+m} u_i(z)+b u_i(z)+z^n c_{r-i}v_i(z)}{z^n v_i(z)} \\
    v_{i+1}(z) &=& z^n v_i(z) \\
    u_{i+1}(z) &=& a z^{n+m}u_i(z)+b u_i(z)+c_{r-i}v_{i+1}(z)
    \f}
 *  with \f$p_{0}(z)=0\f$.
 *  So the following special transformation cases can be simply calculated:
 *  - linear (\f$n=0,m=1\f$): \f$p(a z+b)\f$
 *  - square (\f$n=0,m=2\f$): \f$p(a z^2+b)\f$
 *  - inverse (\f$n=1,a=0\f$): \f$z^r p(b/z)\f$
 *
 *  \param poly         Pointer to polynomial that coefficients \p poly->coeff
 *                      shall be transformed. The allocated memory space must
 *                      be enough to hold a polynomial with degree \f$r(n+m)\f$.
 *  \param degm         Numerator degree of transformation, means \p m.
 *  \param degn         Denominator degree of transformation, means \p n.
 *  \param a            Transform parameter \p a.
 *  \param b            Transform parameter \p b.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int mathPolyTransform(MATHPOLY *poly, int degm, int degn, double a, double b)
{
    int i, k;
    double *vecu;                             /* Polynomial u(z) coefficients */

    int expv = 0;                              /* Exponent of polynomial v(z) */

    degm += degn;                                /* use variable degm = n + m */
    vecu = MALLOC ((1 + (degm * poly->degree)) * sizeof(poly->coeff[0]));

    if (vecu == NULL)
    {
        DEBUG_LOG ("Polynomial memory allocation");
        return ENOMEM;
    } /* if */


    vecu[0] = 0.0;

    for (i = 0; i <= poly->degree; i++)            /* perform Horner's scheme */
    {
        for (k = i; k >= 0; k--)                     /* with all coefficients */
        {
            vecu[k + degm] = vecu[k];   /* u_i(z) * z^{n+m} (shift left degm) */
        } /* for */

        for (k = 0; k < degm; k++)          /* clear lower order coefficients */
        {
            vecu[k] = 0.0;
        } /* if */

        for (k = 0; k < i; k++)
        {                                /* a * z^{n+m} * u_i(z) + b * u_i(z) */
            vecu[k] = a * vecu[k] + b * vecu[k + degm];
        } /* for */

        for (k = 0; k < degm; k++) /* multiply higher order coefficients by a */
        {
            vecu[i + k] *= a;
        } /* if */

        expv += degn;                                 /* v_{i+1}(z)=z^n v_i(z)*/
        ASSERT(expv < (1 + (degm * poly->degree)));
        vecu[expv] += poly->coeff[poly->degree - i];  /* + c_{n-i} v_{i+1}(z) */
    } /* for */


    poly->degree *= degm;
    memcpy (poly->coeff, vecu, (1 + poly->degree) * sizeof(poly->coeff[0]));
    FREE (vecu);

    return 0;
} /* mathPolyTransform() */


/* FUNCTION *******************************************************************/
/** Calculates real polynomial coefficients from roots. The calculation of
 *  polynomial coefficients \f$c_i\f$ of
    \f[
    p(z)=c_n z^n + c_{n-1} z^{n-1}+ \cdots + c_2 z^2 + c_1 z + c_0
    \f]
    is performed using the following algorithm:
    \f[
    p_{i+1}(z)=p_{i}(z) (z-z_i)=z p_{i}(z)-z_i p_{i}(z)
    \f]
 *  with \f$p_{0}(z)=1\f$.
 *
 *  \param poly         Pointer to polynomial that holds the roots in \p
 *                      poly->root and gets the coefficients in \p poly->coeff.
 *  \param factor       Factor to be applied to all coefficients. To match a
 *                      roots representation to a polynomial polynomial the
 *                      coefficient \f$p_n\f$ must be multiplied as \p factor.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int mathPolyRoots2Coeffs(MATHPOLY *poly, double factor)
{
    int i, k;
    gsl_complex result;

    gsl_complex *cplxPoly = MALLOC ((1 + poly->degree) * sizeof(poly->root[0]));

    if (cplxPoly == NULL)
    {
        DEBUG_LOG ("Roots to coefficients conversion");
        return ENOMEM;
    } /* if */


    GSL_SET_COMPLEX(cplxPoly, 1.0, 0.0);    /* special case if degree is zero */

    for (i = 0; i < poly->degree; i++)                   /* process all roots */
    {
        for (k = i + 1; k > 0; k--)                  /* with all coefficients */
        {
            cplxPoly[k] = cplxPoly[k-1];                          /* P(i) * z */
        } /* for */

        GSL_SET_COMPLEX(cplxPoly, 0.0, 0.0);

        for (k = 0; k <= i; k++)                     /* with all coefficients */
        {
            result = gsl_complex_mul (cplxPoly[k+1], poly->root[i]);
            cplxPoly[k] = gsl_complex_sub (cplxPoly[k], result); /* -P(i)*z[i] */
        } /* for */
    } /* for */

    for (i = 0; i <= poly->degree; i++)
    {
        poly->coeff[i] = factor * GSL_REAL(cplxPoly[i]);
    } /* for */

    free(cplxPoly);

    return 0;
} /* mathPolyRoots2Coeffs() */



/* FUNCTION *******************************************************************/
/** \e Chebyshev function (polynomial) of first kind.
    \f{eqnarray*}
        y &=& \cos(n\arccos x)\quad(x\leq 1) \\
        y &=& \cosh(n\arcosh x)\quad(x>1)
    \f}
 *
 *  \param degree       Polynomial degree.
 *  \param x            Argument.
 *
 *  \return             \f$y \cos(n\arccos x)\f$.
 ******************************************************************************/
double mathPolyCheby(int degree, double x)
{
    return chebyT(degree, x);
} /* mathPolyCheby() */


/* FUNCTION *******************************************************************/
/** Inverse \e Chebyshev function (polynomial) of first kind.
    \f{eqnarray*}
        y &=& \cos(1/n \arccos x)\quad(x\leq 1) \\
        y &=& \cosh(1/n \arcosh x)\quad(x>1)
    \f}
 *
 *  \param degree       Polynomial degree.
 *  \param x            Argument.
 *
 *  \return             \f$y \cos(1/n \arccos x)\f$.
 ******************************************************************************/
double mathPolyChebyInv(int degree, double x)
{
    return chebyT(1.0 / degree, x);
} /* mathPolyChebyInv */




/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
