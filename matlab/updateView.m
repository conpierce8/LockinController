% updateView.m
% encoding: utf-8
%
% Simple script for performing parametric sweeps with SR860 lock-in amplifier.
%
% Author:   Connor D. Pierce
% Created:  2023-04-10 15:13:57
% Modified: 2023-04-10 15:16:41
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


% Update plot of parametric data in real-time.
%
% Plots the amplitude on `ax1` and the phase on `ax2`.
function updateView( ...
   ax1, ...
   ax2, ...
   data, ...
   repeats_ampl, ...
   n_ampl, ...
   repeats_freq, ...
   n_freq, ...
   r_A, ...
   i_A, ...
   r_f, ...
   i_f ...
)
   arguments (Input)
      %%TODO: implement argument checking
   end


   % Get the existing plot elements so they can be updated with the current data. Check
   % that number of plot elements is the same for both subplots.
   chil1 = get(ax1, 'Children');
   chil2 = get(ax2, 'Children');

   N_lines1 = length(chil1, 1);
   N_lines2 = length(chil2, 2);

   if N_lines1 ~= N_lines2
      % For now, just print a warning if the numbers don't match. Maybe change this in
      % the future
      fprintf('Warning: number of lines does not match between subplots.');
   end
   N_lines = N_lines1;


   % Calculate the number of sweeps that have been completed
   N_complete = ( ...
      (r_A - 1) * (n_ampl * repeats_freq) ...
      + (i_A - 1) * (repeats_freq) ...
      + (r_f - 1) ...
   );


   % Take one of two actions: either update the 
   if N_lines ~= (N_complete + 1) && N_lines 
end
