/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Interface to Linear IIR Filter interpolation functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/linFirFilter.h,v 1.2 2006-11-04 18:28:27 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/09/11 15:52:21  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/


#ifndef LINFIR_FILTER_H
#define LINFIR_FILTER_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/

/** Typical smoothing windows used in digital signal processing.
 *
 *  \attention Do not change enumeration values, because used as index.
 */
typedef enum
{
    LINFIR_DSPWIN_RECT = 0,
    LINFIR_DSPWIN_HAMMING = 1,
    LINFIR_DSPWIN_HANNING = 2,
    LINFIR_DSPWIN_BLACKMAN = 3,
    LINFIR_DSPWIN_KAISER = 4,

    LINFIR_DSPWIN_SIZE

} LINFIR_DSPWIN;



/** Linear FIR filter types.
 *
 *  \attention Do not change enumeration values, because used as index.
 */
typedef enum
{
    LINFIR_TYPE_RECT = 0,                            /**< Rectangular lowpass */
    LINFIR_TYPE_COS = 1,                                 /**< Cosinus lowpass */
    LINFIR_TYPE_COS2 = 2,                         /**< Square cosinus lowpass */
    LINFIR_TYPE_GAUSS = 3,                              /**< Gaussian lowpass */
    LINFIR_TYPE_SQR = 4,                     /**< Squared first order lowpass */

    LINFIR_TYPE_SIZE
} LINFIR_TYPE;


/** Linear FIR filter design data (from dialog).
 */
typedef struct
{
    LINFIR_TYPE type;    /**< Filter type. \attention Must be the 1st element */
    int order;       /**< Order of filter. \attention Must be the 2nd element */
    double cutoff;  /**< Cutoff frequency. \attention Must be the 3rd element */
    FTRDESIGN ftr; /**< Frequency transformation data. \attention Must be the 4th element */
    LINFIR_DSPWIN dspwin;                       /**< Type of smoothing window */
    double winparm;               /**< Parameter of window, e.g. Kaiser alpha */
} LINFIR_DESIGN;




/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Generates a linear FIR filter. The cutoff frequency always is assumed to be
 *  the 3dB point of amplitude response.
 *
 *  \param pDesign      Pointer to linear FIR filter design data.
 *  \param pFilter      Pointer to buffer which gets the generated filter.
 *                      Notice, that memory space for polynomials will be
 *                      allocated.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int linFirFilterGen (LINFIR_DESIGN *pDesign, FLTCOEFF *pFilter);


#ifdef  __cplusplus
}
#endif


#endif /* LINFIR_FILTER_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
