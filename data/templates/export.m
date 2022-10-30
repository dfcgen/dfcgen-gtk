function [num,den] = export
                     % EXPORT  Exports generated digital filter coefficients from DFCGen (GTK+)
%
% Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
%

    num =
    [
        $PRJ:FILTER:NUM:COEFF$
    ];

    den =
    [
        $PRJ:FILTER:DEN:COEFF$
    ];

