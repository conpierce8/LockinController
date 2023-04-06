% Action.m
% encoding: utf-8
%
% Abstract base class for an action performed in a parametric sweep.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 12:05:50
% Modified: 2023-04-06 16:11:21
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


% Abstract base class of all actions that can be performed in a parametric sweep.
classdef Action < handle

   properties (Static, GetAccess = private, SetAccess = private)
      version = '0.1';
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

         throw(MException('Must be implemented by subclass!');
      end

      % Obtain JSON representation.
      %
      % :parameter obj: (Action)
      function jsondata = dump(obj)
         arguments
            obj Action;
         end

         throw(MException('Must be implemented by subclass!');
      end

   end

   methods (Static)

      % Load Action from JSON representation.
      %
      % :parameter data: (string) JSON representation of the Action
      function obj = load(data)
         arguments(Output)
            obj: Action
         end

         throw(MException('Must be implemented by subclass!');
      end

   end

end
