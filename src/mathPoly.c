/******************************************************************************/
/**
 * \file     mathPoly.c
 * \brief    Polynomial functions.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "mathPoly.h"
#include "mathMisc.h"

#include <string.h> /* memcpy() */


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
 * - If |x| < 1 use : \f$ \chebyT(x) = \cos(n \arccos x) \f$
 * - If x >= 1 use : \f$ \chebyT(x) = \cosh(n \arcosh x) \f$ because \f$ \cosh(j x) = \cos x \f$ .
 * - If x < -1 you get a problem, because \f$ \cosh(x + j \pi) = - \cosh x \f$
 *   and therefore \f$ \arcosh(-x) = \arcosh(|x|) + j \pi\f$ , then use
 *   \f$ \chebyT(x) = (-1)^n \cosh(n \arcosh |x|) \f$ .
 *
 *  \param degree       Degree \f$n\f$ of \e Chebyshev polynomial.
 *  \param x            Argument.
 *
 *  \return             Value of \e Chebyshev polynomial \f$\chebyT_n(x)\f$.
 ******************************************************************************/
static double chebyT(double degree, double x)
{
    double result;

    if (fabs(x) < 1.0)
    {
        return cos (degree * acos(x));
    } /* if */

    result = cosh (degree * gsl_acosh (fabs (x)));

    if ((x >= 1.0) || GSL_IS_EVEN ((int)degree))
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
    poly->coeff = g_malloc ((1 + poly->degree) * sizeof(poly->coeff[0]));

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
    poly->root = g_malloc (GSL_MAX_INT (1, poly->degree) * sizeof(poly->root[0]));

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
        g_free (poly->coeff);
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
        g_free (poly->coeff);
        poly->coeff = NULL;
    } /* if */

    if (poly->root != NULL)
    {
        g_free (poly->root);
        poly->root = NULL;
    } /* if */
} /* mathPolyFree() */



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
double mathPolyCheby (int degree, double x)
{
    return chebyT (degree, x);
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
double mathPolyChebyInv (int degree, double x)
{
    return chebyT (1.0 / degree, x);
} /* mathPolyChebyInv */



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

    double *polyOld_2 = g_malloc ((1 + degree) * sizeof(coeff[0]));

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

    g_free (polyOld_2);

    return 0;
} /* mathPolyBessel() */


/* FUNCTION *******************************************************************/
/** Adds two polynomials with scaling.
 *
 *  \param poly1        Pointer to first polynomial and result. The coefficients
 *                      vector memory space must be large enough to get all
 *                      coefficients. The degree is increased (without malloc
 *                      of new memory), if \p poly2->degree is greater than
 *                      poly1->degree;
 *  \param poly2        Pointer to second polynomial.
 *  \param scale        Factor which is applied to each coefficient of \p poly2.
 *
 ******************************************************************************/
void mathPolyAdd (MATHPOLY *poly1, const MATHPOLY *poly2, double scale)
{
    int i;

    int mindeg = MIN (poly1->degree, poly2->degree);

    poly1->degree = MAX (poly1->degree, poly2->degree);

    for (i = 0; i <= mindeg; i++)
    {
        poly1->coeff[i] += scale * poly2->coeff[i];
    } /* for */

    for (i = mindeg + 1; i <= poly2->degree; i++)
    {
        poly1->coeff[i] = scale * poly2->coeff[i];
    } /* for */

} /* mathPolyAdd() */



/* FUNCTION *******************************************************************/
/** Multiplies a polynomial with the binomial \f$a z^n+b\f$.
 *  The function multiplies the polynomial
    \f{eqnarray*}
    p(z) &=& c_r z^r + c_{r-1} z^{r-1}+ \cdots + c_2 z^2 + c_1 z + c_0 \\
         &=& (\cdots(((c_r z + c_{r-1})z + c_{r-2})z + c_{r-3})z+\cdots+c_1)z+c_0
    \f}
 *  with \f$az^n+b\f$. The degree of new polynomial is \f$rn\f$, which must
 *  be available in \p poly.
    \f{eqnarray*}
    p(z) &=& (az^n + b)p(z) \\
         &=& a z^n p(z) + b p(z)
    \f}
 *
 *  \param poly         Pointer to polynomial which shalle be multiplied (in place).
 *  \param degn         Degree of polynomial \f$az^n+b\f$.
 *  \param a            Parameter \p a in polynomial \f$az^n+b\f$.
 *  \param b            Parameter \p b in polynomial \f$az^n+b\f$.
 *
 ******************************************************************************/
void mathPolyMulBinomial (MATHPOLY *poly, int degn, double a, double b)
{
    int i;

    ASSERT (degn >= 0);

    for (i = poly->degree; i >= 0; i--)                           /* z^n p(z) */
    {
        poly->coeff[i + degn] = poly->coeff[i];
    } /* for */

    for (i = 0; i < degn; i++)                    /* clear lower coefficients */
    {
        poly->coeff[i] = 0.0;
    } /* for */


    for (i = 0; i <= poly->degree; i++)          /* a * z^n * p(z) + b * p(z) */
    {
        poly->coeff[i] = a * poly->coeff[i] + b * poly->coeff[i + degn];
    } /* for */

    for (i = poly->degree + 1; i <= poly->degree + degn; i++)
    {                                                   /* upper coefficients */
        poly->coeff[i] *= a;
    } /* if */

    poly->degree += degn;
} /* mathPolyMulBinomial() */



/* FUNCTION *******************************************************************/
/** Transforms polynomial coefficients for fractional variable substitution.
 *  The function transforms the polynomial
    \f{eqnarray*}
    p(z) &=& c_r z^r + c_{r-1} z^{r-1}+ \cdots + c_2 z^2 + c_1 z + c_0 \\
         &=& (\cdots(((c_r z + c_{r-1})z + c_{r-2})z + c_{r-3})z+\cdots+c_1)z+c_0
    \f}
 *  by replacing
    \f[
      z := \frac{\alpha z^m+\beta}{\gamma z^n+\delta}\qquad\alpha,\beta,\gamma,\delta\in R;\quad m,n\in N
    \f]
 *  into polynomial
    \f[
        p(z) := (\gamma z^n+\delta)^r p\left(\frac{\alpha z^m+\beta}{\gamma z^n+\delta}\right)
    \f]
 *  The new degree is \f$r\max(n,m)\f$.
 *  Set \f$p_i(z)=u_i(z)/v_i(z)\f$ the transformation algorithm is based
 *  on \e Horners scheme:
    \f{eqnarray*}
    p_{i}(z) &=& z\, p_{i-1}(z) + c_{r-i} \\
             &=& \frac{\alpha z^m+\beta}{\gamma z^n+\delta}\, p_{i-1}(z) + c_{r-i} \\
             &=& \frac{(\alpha z^m+\beta)\,p_{i-1}(z)+(\gamma z^n+\delta)\,c_{r-i}}{\gamma z^n+\delta} \\
    \frac{u_{i}(z)}{v_{i}(z)} &=& 
                 \frac{(\alpha z^m+\beta)\,\frac{u_{i-1}(z)}{v_{i-1}(z)}+(\gamma z^n+\delta)\,c_{r-i}}{\gamma z^n+\delta} \\
    v_{i}(z) &=& (\gamma z^n+\delta) v_{i-1}(z),\quad v_0=1 \\
    u_{i}(z) &=& (\alpha z^m+\beta)u_{i-1}(z)+c_{r-i}v_{i}(z),\quad u_0=c_r
    \f}
 *  with \f$i=1\ldots n\f$ and \f$p_{0}(z)=c_r\f$.
 *  So the following special transformation cases can be simply calculated:
 *  - linear (\f$n=0,\gamma=1,\delta=0,m=1\f$): \f$p(\alpha z+\beta)\f$
 *  - square (\f$n=0,\gamma=1,\delta=0,m=2\f$): \f$p(\alpha z^2+\beta)\f$
 *  - inverse (\f$n=1,\gamma=1,\delta=0,\alpha=0\f$): \f$z^r p(\beta/z)\f$
 *  - bilinear (\f$n=1,m=1,\alpha=1,\gamma=1,\beta=-1,\delta=1\f$): \f$(z+1)^r p\left(\frac{z-1}{z+1}\right)\f$
 *
 *  \param poly         Pointer to polynomial that coefficients \p poly->coeff
 *                      shall be transformed. The allocated memory space must
 *                      be enough to hold a polynomial of degree \f$r\max(n,m)\f$.
 *  \param degm         Numerator degree of transformation, means \f$m\f$.
 *  \param a            Transform parameter \f$\alpha\f$.
 *  \param b            Transform parameter \f$\beta\f$.
 *  \param degn         Denominator degree of transformation, means \f$n\f$.
 *  \param c            Transform parameter \f$\gamma\f$.
 *  \param d            Transform parameter \f$\delta\f$.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int mathPolyTransform (MATHPOLY *poly,
                       int degm, double a, double b,
                       int degn, double c, double d)
{
    int i;
    MATHPOLY vecu, vecv;                           /* polynomial coefficients */

    vecv.degree = degn * poly->degree;
    vecu.degree = MAX (degn, degm) * poly->degree;

    if (mathPolyMalloc (&vecu) != 0)
    {
        DEBUG_LOG ("Polynomial memory allocation");
        return ENOMEM;
    } /* if */

    if (mathPolyMalloc (&vecv) != 0)
    {
        mathPolyFree (&vecu);
        DEBUG_LOG ("Polynomial memory allocation");
        return ENOMEM;
    } /* if */

    vecu.degree = vecv.degree = 0;      /* start with a constant (degree = 0) */
    vecu.coeff[0] = poly->coeff[poly->degree];
    vecv.coeff[0] = 1.0;

    for (i = 1; i <= poly->degree; i++)            /* perform Horner's scheme */
    {
        mathPolyMulBinomial (&vecv, degn, c, d);                    /* v_i(z) */
        mathPolyMulBinomial (&vecu, degm, a, b);
        mathPolyAdd (&vecu, &vecv, poly->coeff[poly->degree - i]);  /* u_i(z) */
    } /* for */


    poly->degree = vecu.degree;
    memcpy (poly->coeff, vecu.coeff, (1 + vecu.degree) * sizeof(vecu.coeff[0]));
    mathPolyFree (&vecu);
    mathPolyFree (&vecv);

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

    gsl_complex *cplxPoly = g_malloc ((1 + poly->degree) * sizeof(poly->root[0]));

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
            cplxPoly[k] = cplxPoly[k - 1];                        /* P(i) * z */
        } /* for */

        GSL_SET_COMPLEX(cplxPoly, 0.0, 0.0);

        for (k = 0; k <= i; k++)                     /* with all coefficients */
        {
            result = gsl_complex_mul (cplxPoly[k + 1], poly->root[i]);
            cplxPoly[k] = gsl_complex_sub (cplxPoly[k], result); /* -P(i)*z[i] */
        } /* for */
    } /* for */

    for (i = 0; i <= poly->degree; i++)       /* copy into real result vector */
    {
        poly->coeff[i] = factor * GSL_REAL (cplxPoly[i]);
    } /* for */

    free(cplxPoly);

    return 0;
} /* mathPolyRoots2Coeffs() */



/* FUNCTION *******************************************************************/
/** Computes the complex roots \f$z_i\f$ associated with the polynomial
    \f[
    p(z)=c_n z^n + c_{n-1} z^{n-1}+ \cdots + c_2 z^2 + c_1 z + c_0
    \f]
 *
 *  \param poly         Pointer to polynomial that holds the coefficients in
 *                      \p poly->coeff and gets the roots in \p poly->roots.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 *  \todo               Try to avoid mixing error codes from gsl_errno.h
 *                      and errno.h
 ******************************************************************************/
int mathPolyCoeffs2Roots (MATHPOLY *poly)
{
    int err = 0;

    if (poly->degree > 0)
    {
        gsl_poly_complex_workspace *polyWsp = /* polynomial roots finder workspace */
            gsl_poly_complex_workspace_alloc (poly->degree + 1);

        if (polyWsp == NULL)
        {
            return GSL_ENOMEM;                           /* not enough memory */
        } /* if */


        err = gsl_poly_complex_solve (poly->coeff, poly->degree + 1, polyWsp,
                                      (gsl_complex_packed_ptr) poly->root);
        gsl_poly_complex_workspace_free (polyWsp);
    } /* if */

    return err;
} /* mathPolyRoots2Coeffs() */



#ifdef DEBUG

/* FUNCTION *******************************************************************/
/** Logs polynomial coefficients for debug purposes.
 *
 *  \param poly         Pointer to polynomial that holds the coefficients.
 *
 ******************************************************************************/
void mathPolyDebugLog (MATHPOLY *poly)
{
    int i;

    for (i = 0; i <= poly->degree; i++)
    {
        DEBUG_LOG ("coeff[%d] = %G", i, poly->coeff[i]);
    } /* for */

    for (i = 0; i < poly->degree; i++)
    {
        DEBUG_LOG (" root[%d] = %G +j %G", i,
                   GSL_REAL (poly->root[i]), GSL_IMAG (poly->root[i]));
    } /* for */

    DEBUG_LOG ("");
} /* mathPolyDebugLog() */

#endif /* DEBUG */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
