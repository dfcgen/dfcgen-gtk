function [num,den] = export
                     % EXPORT  Exports generated digital filter coefficients from DFCGen (GTK+)
%
% Copyright (C) 2012 Ralf Hoppe <ralf.hoppe@ieee.org>
% Version: $Id$
%

    num =
    [
        $PRJ:FILTER:NUM:COEFF$
    ];

    den =
    [
        $PRJ:FILTER:DEN:COEFF$
    ];

