% updateView.m
% encoding: utf-8
%
% Simple script for performing parametric sweeps with SR860 lock-in amplifier.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:18:03
% Modified: 2023-04-10 17:10:33
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


function [outp, phas] = getAmplPhas(sr860, autoSensitivity)

   arguments (Input)
      sr860 visalib.USB;
      autoSensitivity(1, 1) int8 { ...
         mustBeGreaterThanOrEqual(autoSensitivity, 0), ...
         mustBeLessThanOrEqual(autoSensitivity, 1) ...
      };
   end

   arguments (Output)
      outp double;
      phas double;
   end

   % Simultaneously acquire amplitude and phase from the SR860 using the SNAP command
   val_str = writeread(sr860, 'SNAP? 2, 3');

   % Returned value is a comma-separated string. Parse the individual values:
   substrs = split(val_str, ',');
   outp = str2double(substrs(1));
   phas = str2double(substrs(2));

   if autoSensitivity
      %%TODO: check outp and adjust sensitivity if needed
   end

end
