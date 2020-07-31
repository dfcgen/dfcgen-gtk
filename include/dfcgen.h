/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *              Digital Filter Coefficients Generator common types.
  *
 * \author      Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/

#ifndef DFCGEN_H
#define DFCGEN_H


/* INCLUDE FILES **************************************************************/

#include "base.h"                                /* basic types and constants */
#include "mathPoly.h"                                 /* for polynomial types */


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Frequency transformations applied to lowpass filters.
 *
 *  \attention Don't change the enums, because order must match list element
 *             index in dialog.
 */
typedef enum
{
    FTR_NON = 0,
    FTR_HIGHPASS,
    FTR_BANDPASS,
    FTR_BANDSTOP,

    FTR_SIZE
} FTR;


/** Special signals.
 */
typedef enum
{
    FLTSIGNAL_DIRAC,
    FLTSIGNAL_HEAVISIDE,
    FLTSIGNAL_USER
} FLTSIGNAL;


/** Basic filter classes (from design viewpoint).
 *
 *  \attention Don't change the enums, because order must match list element
 *             index in dialog.
 */
typedef enum
{
    FLTCLASS_NOTDEF = -1,                       /**< No filter defined/loaded */
    FLTCLASS_MISC = 0,                     /**< Miscellaneous FIR/IIR filters */
    FLTCLASS_LINFIR = 1,                               /**< Linear FIR filter */
    FLTCLASS_STDIIR = 2,                                      /**< IIR filter */

    FLTCLASS_SIZE                                           /**< Size of enum */
} FLTCLASS;



/** Frequency transformation data.
 */
typedef struct
{
    FTR type;                              /**< Frequency transformation type */
    unsigned flags;        /**< Special flags for bandpass/bandstop transform */
    double fc;                  /**< Center (BP, BS) or cutoff (HP) frequency */
    double bw;                                /**< Bandwidth (only BP or BS)  */
} FTRDESIGN;


/* Digital filter.
 */
typedef struct
{
    double f0;                                         /**< Sample frequency */
    MATHPOLY num;            /**< Numerator polynomial coefficients \& roots */
    MATHPOLY den;           /**< Denominator polynomial coefficients & roots */
    double factor;      /**< Transfer function factor applied to roots product.
                             If this member has value 0.0 then no valid roots
                             representation is available. */
} FLTCOEFF;



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


#define FLT_DEGREE_MAX          1024          /**< Maximum degree of a filter */
#define FLT_DEGREE_MIN          1             /**< Minimum degree of a filter */
#define FLT_SAMPLE_MAX          (4.0 / DBL_EPSILON) /**< Maximum sampling frequency */
#define FLT_SAMPLE_MIN          (1E-6)        /**< Minimum sampling frequency */

#define FLTCLASS_DEFAULT        FLTCLASS_MISC /**< default filter class (raw) */


/** Define to be used in element \em flags of structure FTRDESIGN to indicate
    that center frequency (BP, BS) is geometric mean of cutoff frequencies
    \f$f_c = \sqrt{f_1 f_2}\f$. Normally (arithmetic mean) it is calculated by
    \f$f_c = (f_1 + f_2) / 2\f$.
*/
#define FTRDESIGN_FLAG_CENTER_GEOMETRIC 1



/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/



#ifdef  __cplusplus
}
#endif


#endif /* DFCGEN_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
