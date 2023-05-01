# __main__.py
# encoding: utf-8
#
# Simple script for performing parametric sweeps with SR860 lock-in amplifier.
#
# Author:   Connor D. Pierce
# Created:  2023-05-01 15:14:58
# Modified: 2023-05-01 16:17:04
#
# Copyright (c) 2023 Connor D. Pierce
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# SPDX-License-Identifier: MIT


# Imports
import numpy as np
import time

from lockin.utils import *
from matplotlib import pyplot as plt


# Sweep parameters
all_ampl = np.logspace(-1, 0, 8);         # EDIT THIS LINE TO CONTROL AMPLITUDE
all_freq = np.linspace(6500, 6700, 15);   # EDIT THIS LINE TO CONTROL FREQUENCY
repeats_ampl = 1;                         # EDIT THIS LINE TO CONTROL MULTIPLICITY
repeats_freq = 1;                         # EDIT THIS LINE TO CONTROL MULTIPLICITY
n_ampl = len(all_ampl)
n_freq = len(all_freq)


# Establish connection to the SR860 using VISA
sr860 = connect_to_lockin("SR860")


# Select an output file
outputfilename = input("Enter output filename ['../data/data.txt']: ")
if len(outputfilename) == 0:
   outputfilename = "../data/data.txt"
if not outputfilename.endswith(".txt")
    outputfilename += ".txt"


# Initialize SR860 settings
##TODO # Input coupling: AC
##TODO # Detection harmonic: 1


# Create figure for visualization
fig = plt.figure();
ax1 = fig.add_subplot(2, 1, 1)
ax2 = fig.add_subplot(2, 1, 2)
ax1.set_xlabel('Frequency [Hz]')
ax1.set_ylabel('Amplitude [V]')
ax2.set_xlabel('Frequency [Hz]')
ax2.set_ylabel('Phase [deg]')


# Automatic time constant and wait time
autoTimeConst = False   # Set this variable to 1 to enable auto time-constant
atten_2f = -80          # Desired attenuation of the 2f component [dB]
filterType = 3          # Filter type (used to calculate time constant)
waitTime = 1            # Default wait time [sec]


# Automatic sensitivity control
autoSensitivity = False   #  Set this variable to 1 to enable auto sensitivity


# Main execution loop
data = np.zeros((repeats_ampl * n_ampl * repeats_freq * n_freq, 4))
for r_A in range(repeats_ampl):
    for i_A in range(n_ampl):

        # Set the current amplitude
        ampl = all_ampl[i_A]
        ampl_actual = set_amplitude(sr860, ampl)

        # Loop over frequency
        for r_f in range(repeats_freq):
            for i_f in range(n_freq):

                # Set the current frequency
                freq = all_freq[i_f]
                freq_actual = set_frequency(sr860, freq)

                # Pause and wait for lockin output to settle
                if autoTimeConst:
                    waitTime = auto_time_const(sr860, freq, atten_2f, filterType)
                    time.sleep(waitTime)  # Comment this line to use auto wait-time
                else:
                    time.sleep(waitTime)

                # Measure amplitude and phase
                outp, phas = get_ampl_phas(sr860, autoSensitivity)

                # Store in the appropriate row of the array
                row = (
                    r_A * n_ampl * repeats_freq * n_freq
                    + i_A * repeats_freq * n_freq
                    + r_f * n_freq
                    + i_f
                )
                data[row, 0] = ampl_actual
                data[row, 1] = freq_actual
                data[row, 2] = outp
                data[row, 3] = phas

                # Update visualization
                update_view(
                    ax1,
                    ax2,
                    data,
                    repeats_ampl,
                    n_ampl,
                    repeats_freq,
                    n_freq,
                    r_A,
                    i_A,
                    r_f,
                    i_f,
                )


# Save the recorded data as text file
np.savetxt(
    outputfilename,
    data,
    header="Amplitude\tFrequency\tR\tTheta\n",
    fmt="%.4e",
    delimiter="\t",
)
np.save(outputfilename.replace(".txt", ".npy"), data)
