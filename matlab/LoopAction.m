% LoopAction.m
% encoding: utf-8
%
% Loop over a parameter in a parametric sweep.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 15:26:02
% Modified: 2023-04-06 15:50:55
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


% Loop over a parameter.
classdef LoopAction < SequenceAction

   % Private properties
   properties (GetAccess = private, SetAccess = private)
      subactions(1, :) Action;
   end

   methods

      % Execute the action on the provided lock-in amplifier.
      %
      % :parameter obj: (Action)
      % :parameter lockin: (LockinAmplifier) interface to a lock-in amplifier
      function run(obj, lockin)
         arguments
            obj Action;
            lockin LockinAmplifier;
         end
         
         % Do nothing since this is a generic base class.
         pass
      end

   end

end
