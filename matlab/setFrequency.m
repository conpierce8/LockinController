% setFrequency.m
% encoding: utf-8
%
% Set SR860 reference frequency.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:29:37
% Modified: 2023-04-10 16:24:28
%
% Copyright (c) 2023 Connor D. Pierce
%
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included in all
% copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
% SOFTWARE.
%
% SPDX-License-Identifier: MIT


function f = setFrequency(sr860, freq)

   arguments (Input)
      sr860 visalib.USB;
      freq(1, 1) double {mustBeInRange(freq, 0.001, 500000)};
   end

   arguments (Output)
      f(1, 1) double {mustBeInRange(f, 0.001, 500000)};
   end


   % Send command to SR860 to set the frequency
   write(sr860, sprintf('FREQ %.6f', freq));

   % Get the actual frequency that was set
   f_str = writeread(sr860, 'FREQ?');
   f = str2double(f_str);

end
