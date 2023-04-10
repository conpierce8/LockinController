% connectToSR860.m
% encoding: utf-8
%
% Establish a connection to an SR860 lock-in amplifier.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:38:07
% Modified: 2023-04-10 16:25:12
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


function [sr860, name] = connectToSR860

   arguments (Output)
      sr860 visalib.USB;
      name string;
   end

   % Get list of all connected VISA devices
   vdl = visadevlist();

   % Search the list of devices for a connected SR860
   for row = 1:size(vdl, 1)
      if strcmp(vdl{row, 'Model'}, 'SR860')
         sr860 = visadev(vdl{row, 'ResourceName'});

         % Verify connection by asking SR860 for identification string
         name = writeread(sr860, "*IDN?");

         return
      end
   end
   
   % None found, throw an error
   throw(MException('No SR860 lock-in found.'));

end
