% setAmplitude.m
% encoding: utf-8
%
% Set SR860 reference amplitude.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:35:45
% Modified: 2023-04-10 16:24:47
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


% Set the SR860 reference amplitude and get the actual amplitude that was set.
function a = setAmplitude(sr860, ampl)

   arguments (Input)
      sr860 visalib.USB;
      ampl(1, 1) double {mustBeInRange(ampl, 1E-9, 2.0)};
   end

   arguments (Output)
      a(1, 1) double {mustBeInRange(a, 1E-9, 2.0)};
   end


   % Send command to SR860 to set the amplitude
   decade = 3 * floor(log10(ampl) / 3);
   val = ampl / (10 ^ decade);
   switch decade
      case -9
         write(sr860, sprintf('SLVL %.4f NV', val));
      case -6
         write(sr860, sprintf('SLVL %.4f UV', val));
      case -3
         write(sr860, sprintf('SLVL %.4f MV', val));
      case 0
         write(sr860, sprintf('SLVL %.4f V', val));
   end

   % Get the actual amplitude that was set
   a_str = writeread(sr860, 'SLVL?');
   a = str2double(a_str);

end
