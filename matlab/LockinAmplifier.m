% LockinAmplifier.m
% encoding: utf-8
%
% Interface to a Stanford Research Systems lock-in amplifier.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 12:33:05
% Modified: 2023-04-06 12:59:59
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


% Interface to a Stanford Research Systems lock-in amplifier.
%
%
classdef LockinAmplifier < handle

   % Private properties
   properties (GetAccess = private, SetAccess = private)
      feature_names(:) string;
      features(:);
   end

   % Read-only properties
   properties (SetAccess = private)
      comm LockinCommunicator;
      vers_str string;
   end

   % Calculate-on-demand properties
   properties (Dependent)
      vers string;
   end

   % Class methods
   methods

      % Create a LockinAmplifier communicating on `comm`.
      %
      % :parameter comm: LockinCommunicator interface for communications
      %
      % :return obj: LockinAmplifier object
      function obj = LockinAmplifier(comm)
         obj.comm = comm;
         comm.open;
         obj.vers_str = comm.getVersionStr();
         % obj.features = %%TODO: get available features
      end

      % Get the availability of the given feature.
      %
      % Certain features/settings are available only on a subset of SRS lock-in
      % amplifiers. For example, phase measurements are available on SR830 amplifiers
      % but not on SR810 amplifiers. This method returns whether a given feature is
      % available on the represented lock-in amplifier.
      %
      % :parameter obj: LockinAmplifier
      % :parameter featureName: (string) name of the requested feature
      %
      % :return flag: (int) 1 if the requested feature is available, 0 otherwise
      function flag = hasFeature(obj, featureName)

         arguments
            obj LockinAmplifier;
            featureName string;
         end

         arguments (Output)
            flag int {mustbemember(flag, {0, 1})};
         end

         % Loop through available features and see if the requested feature exists
         for feature = obj.feature_names
            if feature.name == featureName
               % Feature exists, set flag to 1 and return
               flag = 1;
               return;
            end
         end

         % Feature not found; set flag to zero and return
         flag = 0;
         return;

      end

      % Get the lock-in amplifier version, e.g. "SR830".
      function vers = get.vers

         arguments
            obj LockinAmplifier;
         end

         arguments (Output)
            vers string;
         end

         vers = ""; %%TODO: extract version from version string
      end

   end

end
