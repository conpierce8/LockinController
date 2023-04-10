% main.m
% encoding: utf-8
%
% Simple script for performing parametric sweeps with SR860 lock-in amplifier.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 12:56:55
% Modified: 2023-04-10 17:26:36
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


% Perform parametric sweep using SR860 lock-in amplifier.
function data = main
   persistent outputfilename outputpathname;


   % Sweep parameters
   all_ampl = logspace(-1, 0, 2);     % EDIT THIS LINE TO CONTROL AMPLITUDE
   all_freq = linspace(6500, 6700, 2);  % EDIT THIS LINE TO CONTROL FREQUENCY
   repeats_ampl = 1;                      % EDIT THIS LINE TO CONTROL MULTIPLICITY
   repeats_freq = 1;                      % EDIT THIS LINE TO CONTROL MULTIPLICITY
   n_ampl = size(all_ampl, 2);
   n_freq = size(all_freq, 2);


   % Establish connection to the SR860 using VISA
   sr860 = connectToSR860();


   % Select an output file
   [outputfilename, outputpathname] = uiputfile( ...
      '*.txt', 'Save as...', strcat(outputfilename, outputpathname) ...
   );
   if outputfilename == 0
      return;
   end


   % Initialize SR860 settings
   %%TODO % Input coupling: AC
   %%TODO % Detection harmonic: 1

    
   % Create figure for visualization
   fig = figure();


   % Automatic time constant and wait time
   autoTimeConst = 0;   % Set this variable to 1 to enable auto time-constant
   atten_2f = -80;      % Desired attenuation of the 2f component [dB]
   filterType = 3;      % Filter type (used to calculate time constant)
   waitTime = 1;        % Default wait time [sec]


   % Automatic sensitivity control
   autoSensitivity = 0;   %  Set this variable to 1 to enable auto sensitivity


   % Main execution loop
   data = zeros(repeats_ampl * n_ampl * repeats_freq * n_freq, 4);
   for r_A = 1:repeats_ampl
      for i_A = 1:n_ampl

         % Set the current amplitude
         ampl = all_ampl(i_A);
         ampl_actual = setAmplitude(sr860, ampl);

         % Loop over frequency
         for r_f = 1:repeats_freq
            for i_f = 1:n_freq

               % Set the current frequency
               freq = all_freq(i_f);
               freq_actual = setFrequency(sr860, freq);

               % Pause and wait for lockin output to settle
               if autoTimeConst
                  waitTime = autoTimeConst(sr860, freq, atten_2f, filterType);
                  pause(waitTime);  % Comment this line to use auto wait-time
               else
                  pause(waitTime);
               end

               % Measure amplitude and phase
               [outp, phas] = getAmplPhas(sr860, autoSensitivity);

               % Store in the appropriate row of the array
               row = ( ...
                  (r_A - 1) * (n_ampl * repeats_freq * n_freq) ...
                  + (i_A - 1) * (repeats_freq * n_freq) ...
                  + (r_f - 1) * n_freq ...
                  + i_f ...
               );
               data(row, 1) = ampl_actual;
               data(row, 2) = freq_actual;
               data(row, 3) = outp;
               data(row, 4) = phas;

               % Update visualization
               updateView( ...
                  fig, ...
                  data, ...
                  repeats_ampl, ...
                  n_ampl, ...
                  repeats_freq, ...
                  n_freq, ...
                  r_A, ...
                  i_A, ...
                  r_f, ...
                  i_f ...
               );
            end
         end
      end
   end

   % Save the recorded data as text file
   outputfile = fopen(strcat(outputpathname, outputfilename), 'w');
   fprintf(outputfile, '%.4e\t%.4e\t%.4e\t%.4e\n', data');
   fclose(outputfile);

   % Also save as .mat file
   dot_index = strfind(outputfilename, '.');
   if length(dot_index) == 1
      matfilename = strcat([outputpathname, outputfilename(1:dot_index), 'mat']);
   else
      matfilename = strcat([outputpathname, outputfilename(1:dot_index(end)), 'mat']);
   end
   save(matfilename, 'data');

end
