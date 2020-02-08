function [num,den] = export
                     % EXPORT  Exports generated digital filter coefficients from DFCGen (GTK+)
%
% Copyright (C) 2006-2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
%

    num =
    [
        $PRJ:FILTER:NUM:COEFF$
    ];

    den =
    [
        $PRJ:FILTER:DEN:COEFF$
    ];

