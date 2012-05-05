/********************************* -*- C -*- *********************************
 * Copyright (C) 2012 Ralf Hoppe <ralf.hoppe@ieee.org>
 * Version: $Id$
 *
 * DFCGEN-GTK exported coefficients
 *
 *****************************************************************************/

#ifndef DFCGEN
#define DFCGEN

dfcoeff_t numerator[$PRJ:FILTER:NUM:DEGREE$ + 1] =
{
    $PRJ:FILTER:NUM:COEFF$, /* z^{$PRJ:FILTER:NUM:EXPONENT$} */
};


dfcoeff_t denominator[$PRJ:FILTER:DEN:DEGREE$] =
{
    $PRJ:FILTER:DEN:COEFF$, /* z^{$PRJ:FILTER:DEN:EXPONENT$} */
};


#endif /* DFCGEN */
