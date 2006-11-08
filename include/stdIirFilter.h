/******************************************************************************/
/**
 * \file
 *           Interface to Standard IIR Filter approximation functions.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/include/stdIirFilter.h,v 1.3 2006-11-08 17:59:50 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/11/04 18:28:27  ralf
 * Further work (near 0.1 now)
 *
 * Revision 1.1.1.1  2006/09/11 15:52:21  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/


#ifndef STDIIR_FILTER_H
#define STDIIR_FILTER_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"  /* includes config.h (include before GNU system headers) */

#include <float.h>



#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Standard lowpass filter approximations (results in IIR filter).
 *
 *  \attention Don't change the enums, because order must match list element
 *             index in dialog.
 */
typedef enum
{
    STDIIR_TYPE_BUTTERWORTH = 0, /**< Power filters (max. flat magnitude response) */
    STDIIR_TYPE_CHEBY = 1,                /**< Chebyshev with passband ripple */
    STDIIR_TYPE_CHEBYINV = 2,        /**< Chebyshev inverse (stopband ripple) */
    STDIIR_TYPE_CAUER1 = 3,     /**< Cauer filter (design by passband ripple) */
    STDIIR_TYPE_CAUER2 = 4, /**< Cauer filter (design by stopband attenuation) */
    STDIIR_TYPE_BESSEL = 5,        /**< Bessel filter (max. flat group delay) */

    STDIIR_TYPE_SIZE                     /**< Size of STDIIR_TYPE enumeration */
} STDIIR_TYPE;


/** Laplace to Z-domain transformation algorithms.
 */
typedef enum
{
    ZTR_BILINEAR,
    ZTR_EULER_FORWARD,
    ZTR_EULER_BACKWARD,

    ZTR_SIZE
} STDIIR_ZTR;



/** Standard IIR filter design data (from dialog).
 */
typedef struct
{
    STDIIR_TYPE type;   /**< Filter type. \attention Must be the 1st element. */
    int order;       /**< Order of filter. \attention Must be the 2nd element */
    double cutoff;  /**< Cutoff frequency. \attention Must be the 3rd element */
    FTRDESIGN ftr; /**< Frequency transformation data. \attention Must be the 4th element */
    STDIIR_ZTR zAlgo; /**< Laplace to Z transform algorithm (not implemented yet) */
    double ripple; /**< Maximum passband (ripple) attenuation in dB (elliptic filter) */
    double minatt;  /**< Minimum stopband attenuation in dB (elliptic filter) */
    double angle;                         /**< Module angle (\e Cauer filter) */
} STDIIR_DESIGN;




/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


#define STDIIR_ATT_MAX          (20.0 * FLT_MAX_10_EXP) /**< Maximum attenuation input */
#define STDIIR_RIPPLE_MIN       (1.0 / STDIIR_ATT_MAX) /**< Minimum ripple attenuation (0dB) */
#define STDIIR_RIPPLE_MAX       (10*log10(2.0)) /**< Maximum ripple attenuation (3dB) */
#define STDIIR_STOPATT_MIN      (10*log10(2.0)) /**< Minimum stopband attenuation (3dB) */
#define STDIIR_STOPATT_MAX      STDIIR_ATT_MAX /**< Maximum stopband attenuation (\f$\infty\f$) */
#define STDIIR_ANGLE_MIN        (0.001) /**< Minimum modular angle of elliptic filters */
#define STDIIR_ANGLE_MAX        (89.999) /**< Maximum modular angle of elliptic filters */


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Generates an IIR filter from standard approximations. The cutoff frequency
 *  always is assumed to be the 3dB point of magnitude response.
 *
 *  Detailed description.
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
int stdIirFilterGen (STDIIR_DESIGN *pDesign, FLTCOEFF *pFilter);



#ifdef  __cplusplus
}
#endif


#endif /* STDIIR_FILTER_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

