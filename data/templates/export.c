/********************************* -*- C -*- *********************************
 * Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
 *
 * DFCGen (GTK+) exported coefficients
 *
 *****************************************************************************/

#ifndef DFCGEN
#define DFCGEN

dfcoeff_t numerator[$PRJ:FILTER:NUM:DEGREE$ + 1] =
{
    $PRJ:FILTER:NUM:COEFF$, /* z^{$PRJ:FILTER:NUM:EXPONENT$} */
};


dfcoeff_t denominator[$PRJ:FILTER:DEN:DEGREE$ + 1] =
{
    $PRJ:FILTER:DEN:COEFF$, /* z^{$PRJ:FILTER:DEN:EXPONENT$} */
};


#endif /* DFCGEN */
