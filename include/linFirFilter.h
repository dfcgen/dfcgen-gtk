/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Interface to Linear IIR Filter interpolation functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/linFirFilter.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
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

/** Typical smoothing windows used in digital signal processing
 */
typedef enum
{
    LINFIR_DSPWIN_RECT,
    LINFIR_DSPWIN_HAMMING,
    LINFIR_DSPWIN_HANNING,
    LINFIR_DSPWIN_BLACKMAN,
    LINFIR_DSPWIN_KAISER
} LINFIR_DSPWIN;



/** Linear FIR filter types.
 */
typedef enum
{
    LINFIR_TYPE_RECT,                                /**< Rectangular lowpass */
    LINFIR_TYPE_COS,                                     /**< Cosinus lowpass */
    LINFIR_TYPE_COS2,                             /**< Square cosinus lowpass */
    LINFIR_TYPE_GAUSS,                                  /**< Gaussian lowpass */
    LINFIR_TYPE_SQR                                                 /* Square */
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
    double alpha;                           /**< Parameter of Kaiser window */
} LINFIR_DESIGN;




/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


#ifdef  __cplusplus
}
#endif


#endif /* LINFIR_FILTER_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

