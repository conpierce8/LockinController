% autoTimeConst.m
% encoding: utf-8
%
% Automatically determine appropriate time constant and wait time.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:25:00
% Modified: 2023-04-10 16:24:13
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


function waitTime = autoTimeConst(sr860, freq, atten_2f, filterType)

   arguments (Input)
      sr860 visalib.USB;
      freq(1, 1) double {mustBeInRange(freq, 0.001, 500000)};
      atten_2f(1, 1) double {mustBeNonpositive};
      filterType(1, 1) int8 {mustBeInRange(filterType, 0, 3)};
   end

   arguments (Output)
      waitTime(1, 1) double;
   end

   %%TODO: implement this correctly
   waitTime = 1;

end
