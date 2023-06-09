% Communicator.m
% encoding: utf-8
%
% Abstract base class for communicating with SRS lock-in amplifiers.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 13:02:37
% Modified: 2023-04-06 16:13:06
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


% Abstract class for communicating with SRS lock-in amplifiers.
%
% Defines the main types of messages that can be sent to lock-in amplifiers (and the
% expected type of the response). Subclasses should implement these methods for various
% different types of communication interfaces, e.g. GPIB or USB.
classdef Communicator < handle

   methods

      % Send command to lock-in and read the integer response.
      %
      % :parameter obj: (Communicator)
      % :parameter cmd_str: (string) command string
      %
      % :return val: (int) integer response received from lock-in
      function val = command_int(obj, cmd_str)
         arguments
            obj Communicator;
            cmd_str string;
         end

         arguments (Output)
            val(1, 1) int;
         end

         throw(MException('Must be implemented by subclass!');
      end

      % Send command to lock-in and read the real-valued response.
      %
      % :parameter obj: (Communicator)
      % :parameter cmd_str: (string) command string
      %
      % :return val: (double) real-valued response received from lock-in
      function val = command_real(obj, cmd_str)
         arguments
            obj Communicator;
            cmd_str string;
         end

         arguments (Output)
            val double(1, 1);
         end

         throw(MException('Must be implemented by subclass!');
      end

      % Send command to lock-in that requires no response.
      %
      % :parameter obj: (Communicator)
      % :parameter cmd_str: (string) command string
      function command(obj, cmd_str)
         arguments
            obj Communicator;
            cmd_str string;
         end

         throw(MException('Must be implemented by subclass!');
      end

      % Send command to lock-in and read two-integer response.
      %
      % :parameter obj: (Communicator)
      % :parameter cmd_str: (string) command string
      %
      % :return val1: (int) first response received from lock-in
      % :return val2: (int) second response received from lock-in
      function val1, val2 = command_intint(obj, cmd_str)
         arguments
            obj Communicator;
            cmd_str string;
         end

         arguments (Output)
            val1(1, 1) int;
            val2(1, 1) int;
         end

         throw(MException('Must be implemented by subclass!');
      end

   end

end
