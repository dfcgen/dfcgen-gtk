/******************************************************************************/
/**
 * \file
 *           Polynomial functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef MATHPOLY_H
#define MATHPOLY_H


/* INCLUDE FILES **************************************************************/

#include "base.h"               /* includes config.h (include before math.h)  */

#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_complex_math.h>



#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/

/** Polynomial representation.
 */
typedef struct
{
    int degree;                                     /**< Degree of polynomial */
    double *coeff;               /**< Pointer to real polynomial coefficients */
    gsl_complex *root; /**< Pointer to roots of polynomial (NULL if not evaluated) */
} MATHPOLY;



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Allocates memory space for the coefficients of a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
    int mathPolyMallocCoeffs (MATHPOLY *poly);


/* FUNCTION *******************************************************************/
/** Allocates memory space for the roots of a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
    int mathPolyMallocRoots (MATHPOLY *poly);


/* FUNCTION *******************************************************************/
/** Allocates memory space for a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 *  \return             0 on success, else an error number.
 ******************************************************************************/
    int mathPolyMalloc (MATHPOLY *poly);


/* FUNCTION *******************************************************************/
/** Frees memory space allocated for a polynomial.
 *
 *  \param poly         Pointer to polynomial.
 *
 ******************************************************************************/
    void mathPolyFree (MATHPOLY *poly);


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
 *  \return             GSL_SUCCESS on success, else an error number (see
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
    int mathPolyRoots2Coeffs (MATHPOLY *poly, double factor);


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
    int mathPolyCoeffs2Roots (MATHPOLY *poly);



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
    void mathPolyAdd (MATHPOLY *poly1, const MATHPOLY *poly2, double scale);


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
    void mathPolyMulBinomial (MATHPOLY *poly, int degn, double a, double b);


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
                           int degn, double c, double d);


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
    int mathPolyBessel (int degree, double coeff[]);


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
    double mathPolyCheby (int degree, double x);


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
    double mathPolyChebyInv (int degree, double x);



#ifdef DEBUG

/* FUNCTION *******************************************************************/
/** Logs polynomial coefficients for debug purposes.
 *
 *  \param poly         Pointer to polynomial that holds the coefficients.
 *
 ******************************************************************************/
    void mathPolyDebugLog (MATHPOLY *poly);


#endif /* DEBUG */



#ifdef  __cplusplus
}
#endif


#endif /* MATHPOLY_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
