/******************************************************************************/
/**
 * \file
 *           Polynomial functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/include/mathPoly.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * \see
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
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
 *  \return             GSL_SUCCESS on success, else an error number (see
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
    int mathPolyTransform (MATHPOLY *poly, int degm, int degn, double a, double b);


/* FUNCTION *******************************************************************/
/** Multiplies polynomial \f$p(z)\f$ by \f$z^n\f$, means shifts all
 *  coefficients left.
 *
 *  \param poly         Pointer to polynomial that coefficients \p poly->coeff
 *                      shall be shifted.
 *  \param n            Number of coefficients shift to left.
 *
 ******************************************************************************/
    void mathPolyShiftLeft (MATHPOLY *poly, int n);


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



#ifdef  __cplusplus
}
#endif


#endif /* MATHPOLY_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
