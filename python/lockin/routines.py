#! python3
# -*- coding: utf-8 -*-
#
# routines.py
# encoding: utf-8
#
# Collection of routines for various of parametric sweeps with SRS lock-in amplifiers.
#
# Author:   Connor D. Pierce
# Created:  2023-05-01 15:14:58
# Modified: 2023-07-27 14:30:11
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

from lockin.devices import Lockin, SR830, SR860
from lockin.utils import *
from matplotlib import pyplot as plt


# Functions
def ampl_freq_sweep(
    all_ampl: np.ndarray,
    all_freq: np.ndarray,
    repeats_ampl: int,
    repeats_freq: int,
    filename: str,
    model: str = "SR860",
):
    """Sweep over amplitude and frequency, setting amplitude first, then frequency.
    
    Parameters
    ----------
    all_ampl
        Array of amplitude values to sweep over (N, )
    all_freq
        Array of frequency values to sweep over (M, )
    repeats_ampl
        Number of times to repeat the amplitude sweep
    repeats_freq
        Number of times to repeat the frequency sweep
    filename
        File in which to save the data
    model
        SRS lock-in model to use
    """

    n_ampl = len(all_ampl)
    n_freq = len(all_freq)

    # Establish connection to the lock-in using VISA
    lockin = connect_to_lockin(model)

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
            ampl_actual = set_amplitude(lockin, ampl)

            # Loop over frequency
            for r_f in range(repeats_freq):
                for i_f in range(n_freq):

                    # Set the current frequency
                    freq = all_freq[i_f]
                    freq_actual = set_frequency(lockin, freq)

                    # Pause and wait for lockin output to settle
                    if autoTimeConst:
                        waitTime = auto_time_const(lockin, freq, atten_2f, filterType)
                        time.sleep(waitTime)  # Comment this line to use auto wait-time
                    else:
                        time.sleep(waitTime)

                    # Measure amplitude and phase
                    outp, phas = get_ampl_phas(lockin, autoSensitivity)

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
    if filename.endswith(".txt"):
        outputfilename = filename
    else:
        outputfilename = filename + ".txt"
    np.savetxt(
        outputfilename,
        data,
        header="Amplitude\tFrequency\tR\tTheta\n",
        fmt="%.4e",
        delimiter="\t",
    )
    np.save(outputfilename.replace(".txt", ".npy"), data)


def dmma_sweep(
    limits: tuple[tuple[float, float], tuple[float, float]] = ((0.02, 2.0), (10, 1000)),
    n: tuple[tuple[int, int], tuple[int, int]] = ((3, 201), (31, 9)),
    filename: str = "Test",
    models: tuple[str, str] = ("SR830", "SR860"),
    atten_2f: tuple[float, float] = (-160, -80),
):
    """Measure displacement and force for DMMA using two lock-in amplifiers.
    
    Parameters
    ----------
    limits
        Amplitude and frequency limits
    n
        Number of values for amplitude and frequency sweeps
    filename
        File in which to save the data
    models
        SRS lock-in models to use for displacement and force measurements
    atten_2f
        Desired attenuation of the 2f component [dB]
    """

    # Establish connection to the lock-in using VISA
    lockins = []

    # Details about filter slopes
    filt_slopes = [(-6, 5), (-12, 7), (-18, 9), (-24, 10)]
    filt_idx = 3

    for model in models:
        if model == "SR830":
            lockins.append(SR830())
        elif model == "SR860":
            lockins.append(SR860())
    for lockin in lockins:
        lockin.connect()
        lockin.det_harm = 1
        lockin.filt_slope = filt_idx
        lockin.input_config = "voltage - A"
        lockin.input_coupling = "ac"
        lockin.ref_phase = 0
        lockin.sync_filt = True
    lockins[0].ref_source = "int"
    lockins[1].ref_source = "ext"
    lockins[1].ref_trig = "POS TTL"

    # Create figure for visualization
    plt.ion()
    plt.figure()
    plt.subplot(1, 1, 1)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude [V]')
    min_x, max_x = np.log10(limits[1][0]), np.log10(limits[1][1])
    min_y, max_y = np.log10(limits[0][0]), np.log10(limits[0][1])
    dec_x = max_x - min_x
    dec_y = max_y - min_y
    plt.xlim([10**(min_x - 0.05*dec_x), 10**(max_x + 0.05*dec_x)])
    plt.ylim([10**(min_y - 0.05*dec_y), 10**(max_y + 0.05*dec_y)])
    plt.xscale("log")
    plt.yscale("log")
    # plt.show(block=False)

    # Automatic time constant and wait time
    autoTimeConst = True     # Set this variable to False to disable auto time-constant
    filterType = 3           # Filter type (used to calculate time constant)
    waitTime = 1             # Default wait time [sec]
    slope, wait_factor = filt_slopes[filt_idx]

    # Automatic sensitivity control
    autoSensitivity = True   #  Set this variable to 1 to enable auto sensitivity

    # Detecting frequency lock
    n_lock = 5

    # Main execution loop
    n_all = n[0][0] * n[0][1]
    data_d = np.zeros((n_all, 4))
    data_f = np.zeros((n_all, 4))
    row = 0

    all_ampl = np.logspace(np.log10(limits[0][0]), np.log10(limits[0][1]), n[0][0])
    all_freq = np.logspace(np.log10(limits[1][0]), np.log10(limits[1][1]), n[0][1])
    h = plt.scatter(data_d[:, 0], data_d[:, 1], c=data_d[:, 2])
    for i_A in range(n[0][0]):
        # Set the current amplitude
        lockins[0].ref_ampl = all_ampl[i_A]
        ampl_actual = lockins[0].ref_ampl

        # Loop over frequency
        for i_f in range(n[0][1]):
            # Set the current frequency
            lockins[0].ref_freq = all_freq[i_f]
            freq_actual = lockins[0].ref_freq

            # Pause and wait for lockin output to settle
            if autoTimeConst:
                waitTime = wait_factor * dmma_time_const(
                    lockins, limits[0][0], ampl_actual, freq_actual, atten_2f, slope
                )

            # Ensure that lockins[1] is phase-locked to lockins[0]
            if lockins[1].status_unlock:
                status = np.full((n_lock, ), True, dtype=bool)
                while np.any(status):
                    time.sleep(0.03)
                    status[:-1] = status[1:]
                    status[-1] = lockins[1].status_unlock

            # Wait for output to settle
            time.sleep(waitTime)

            # Measure amplitude and phase
            displ, force = get_ampl_phas(lockins, autoSensitivity, waitTime)

            # Store in the appropriate row of the array
            data_d[row, 0] = ampl_actual
            data_d[row, 1] = freq_actual
            data_d[row, 2] = displ[0]
            data_d[row, 3] = displ[1]
            data_f[row, 0] = ampl_actual
            data_f[row, 1] = freq_actual
            data_f[row, 2] = force[0]
            data_f[row, 3] = force[1]

            # Update visualization
            update_view_dmma(h, data_d, data_f, row)
            row += 1
    if filename.endswith(".txt"):
        outputfilename_d = filename[:-4] + "a_Displ.txt"
        outputfilename_f = filename[:-4] + "a_Force.txt"
    else:
        outputfilename_d = filename + "a_Displ.txt"
        outputfilename_f = filename + "a_Force.txt"
    np.savetxt(
        outputfilename_d,
        data_d,
        header="Amplitude\tFrequency\tR\tTheta\n",
        fmt="%.4e",
        delimiter="\t",
    )
    np.savetxt(
        outputfilename_f,
        data_f,
        header="Amplitude\tFrequency\tR\tTheta\n",
        fmt="%.4e",
        delimiter="\t",
    )

    # Second execution loop
    n_all = n[1][0] * n[1][1]
    data_d = np.zeros((n_all, 4))
    data_f = np.zeros((n_all, 4))
    row = 0

    all_ampl = np.logspace(np.log10(limits[0][0]), np.log10(limits[0][1]), n[1][0])
    all_freq = np.logspace(np.log10(limits[1][0]), np.log10(limits[1][1]), n[1][1])
    h = plt.scatter(data_d[:, 0], data_d[:, 1], c=data_d[:, 2])
    for i_f in range(n[1][1]):
        # Set the current frequency
        lockins[0].ref_freq = all_freq[i_f]
        freq_actual = lockins[0].ref_freq

        # Loop over frequency
        for i_A in range(n[1][0]):
            # Set the current amplitude
            lockins[0].ref_ampl = all_ampl[i_A]
            ampl_actual = lockins[0].ref_ampl

            # Pause and wait for lockin output to settle
            if autoTimeConst:
                waitTime = wait_factor * dmma_time_const(
                    lockins, limits[0][0], ampl_actual, freq_actual, atten_2f, slope
                )

            # Ensure that lockins[1] is phase-locked to lockins[0]
            if lockins[1].status_unlock:
                status = np.full((n_lock, ), True, dtype=bool)
                while np.any(status):
                    time.sleep(0.03)
                    status[:-1] = status[1:]
                    status[-1] = lockins[1].status_unlock

            # Wait for output to settle
            time.sleep(waitTime)
            
            # Measure amplitude and phase
            displ, force = get_ampl_phas(lockins, autoSensitivity, waitTime)

            # Store in the appropriate row of the array
            data_d[row, 0] = ampl_actual
            data_d[row, 1] = freq_actual
            data_d[row, 2] = displ[0]
            data_d[row, 3] = displ[1]
            data_f[row, 0] = ampl_actual
            data_f[row, 1] = freq_actual
            data_f[row, 2] = force[0]
            data_f[row, 3] = force[1]

            # Update visualization
            update_view_dmma(h, data_d, data_f, row)
            row += 1

    # Save the recorded data as text file
    if filename.endswith(".txt"):
        outputfilename_d = filename[:-4] + "b_Displ.txt"
        outputfilename_f = filename[:-4] + "b_Force.txt"
    else:
        outputfilename_d = filename + "b_Displ.txt"
        outputfilename_f = filename + "b_Force.txt"
    np.savetxt(
        outputfilename_d,
        data_d,
        header="Amplitude\tFrequency\tR\tTheta\n",
        fmt="%.4e",
        delimiter="\t",
    )
    np.savetxt(
        outputfilename_f,
        data_f,
        header="Amplitude\tFrequency\tR\tTheta\n",
        fmt="%.4e",
        delimiter="\t",
    )

    lockins[0].ref_ampl = limits[0][0]
    lockins[0].ref_freq = limits[1][0]
    for lockin in lockins:
        lockin.disconnect()
