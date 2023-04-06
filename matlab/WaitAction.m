% WaitAction.m
% encoding: utf-8
%
% Create a pause during a parametric sweep.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 12:16:43
% Modified: 2023-04-06 13:35:29
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


% Pause for a specified duration during a parametric sweep.
classdef WaitAction < Action

   properties (GetAccess = private)
      waitTime(1) double {mustBeNonnegative} = 0;
   end

   methods

      % Create a WaitAction.
      %
      % :parameter waitTime: (double) amount of time (in seconds) to pause
      function obj = WaitAction(waitTime)
         arguments
            waitTime double {mustBeNonnegative};
         end
         
         obj.waitTime = waitTime
         
         % TODO: different call signature to specify automatic wait time
      end

      function run(obj, lockin)
         arguments
            obj WaitAction;
            lockin LockinAmplifier;
         end
         
         % Pause for the requested number of seconds
         pause(obj.waitTime);
         
         % TODO: automatically calculate appropriate wait time for current lockin state
      end

   end

end
