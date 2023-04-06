% EmulatedCommunicator.m
% encoding: utf-8
%
% Emulated lock-in communicator (for testing).
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 16:08:12
% Modified: 2023-04-06 16:09:33
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


% Emulated communication interface.
classdef EnumlatedCommunicator < Communicator

   properties (SetAccess = private)
      
   end

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
            val int;
         end
         
         val = 0;
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
            val double;
         end
         
         val = 0.0;
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

         pass
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
            val1 int;
            val2 int;
         end
         
         val1 = 0;
         val2 = 0;
      end

   end

end
