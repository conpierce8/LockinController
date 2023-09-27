#! python3
# -*- coding: utf-8 -*-
#
# utils.py
# encoding: utf-8
#
# Utility functions for performing frequency sweeps.
#
# Author:   Connor D. Pierce
# Created:  2023-05-01 15:42:15
# Modified: 2023-07-27 14:30:08
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
import lockin
import numpy as np
import pyvisa
import time
import typing

from lockin import devices
from matplotlib import pyplot as plt


# Functions
def auto_time_const(sr860, freq, atten_2f, filterType):
    ##TODO: implement this correctly
    waitTime = 1;


def dmma_time_const(
    lockins: tuple[lockin.devices.Lockin],
    min_ampl: float,
    ampl: float,
    freq: float,
    atten_2f: tuple[float, float],
    filterType: float,
):
    """Set time constant for DMMA experiments."""

    atten = min(atten_2f[1], atten_2f[0] + 20*np.log10(ampl / min_ampl))
    octets = atten / filterType
    tau_p = 2 ** octets / (4 * np.pi * freq)
    tc_vals = []
    for lockin in lockins:
        tc = lockin.time_const_idx(tau_p)
        lockin.time_const = tc
        tc_vals.append(lockin.time_const_value(tc))
    # print(atten, tc_vals)
    return max(tc_vals)


def connect_to_lockin(model):
    """Get a list of all connected VISA devices."""

    rm = pyvisa.ResourceManager()

    # Search the list of devices for a connected SR860
    for row, addr in enumerate(rm.list_resources()):
        device = rm.open_resource(addr)
        idn = device.query("*IDN?").split()
        if idn[0] == "Stanford_Research_Systems" and model.lower() == idn[1].lower():
            return device
        else:
            device.close()

    # None found, throw an error
    raise ValueError(f"No {model} lock-in found.")


def get_ampl_phas(lockins, auto_sens, wait_time):
    """Read amplitude and phase from lockin."""

    def _sens_val(lockin, sens):
        """Get min and max amplitude for a single lockin."""
        val = lockin.sens_get_value(sens)
        return 100 * val / 65536, 0.99 * val

    def _sens_vals(lockins, sens):
        """Get min and max amplitude for multiple lockins."""
        val = [lockin.sens_get_value(s) for lockin, s in zip(lockins, sens)]
        return [(100 * x / 65536, 0.99 * x) for x in val]

    # Simultaneously acquire amplitude and phase from the SR860 using the SNAP command
    if isinstance(lockins, devices.Lockin):
        sens = lockins.sens
        ampl_min, ampl_max = _sens_val(lockins, sens)

        ampl, phas = lockins.get_ampl_phas()
        # Adjust sensitivity and repeat measurement until the value is within range
        while auto_sens and (ampl < ampl_min or ampl > ampl_max):
            if ampl < ampl_min:
                lockins.sens = sens - lockins.sens_inc
            elif ampl > ampl_max:
                lockins.sens = sens + lockins.sens_inc
            sens = lockins.sens
            ampl_min, ampl_max = _sens_val(lockins, sens)
            time.sleep(wait_time)
            ampl, phas = lockins.get_ampl_phas()
        return ampl, phas
    elif isinstance(lockins, typing.Iterable):
        sens = [lockin.sens for lockin in lockins]
        minmax = _sens_vals(lockins, sens)

        ampl_phas = [lockin.get_ampl_phas() for lockin in lockins]
        while auto_sens and any(
            [x[0] < lim[0] or x[0] > lim[1] for x, lim in zip(ampl_phas, minmax)]
        ):
            for i, (lockin, data, limits) in enumerate(zip(lockins, ampl_phas, minmax)):
                if data[0] < limits[0]:
                    lockin.sens = sens[i] - lockin.sens_inc
                    sens[i] = lockin.sens
                    minmax[i] = _sens_val(lockin, sens[i])
                elif data[0] > limits[1]:
                    lockin.sens = sens[i] + lockin.sens_inc
                    sens[i] = lockin.sens
                    minmax[i] = _sens_val(lockin, sens[i])
            time.sleep(wait_time)
            ampl_phas = [lockin.get_ampl_phas() for lockin in lockins]
        return ampl_phas
    else:
        raise TypeError("lockins must be Lockin or iterable of Lockins")


def set_amplitude(lockin, ampl):
    """Set the lockin reference amplitude and get the actual amplitude that was set."""

    # Send command to lockin to set the amplitude
    decade = 3 * int(np.floor(np.log10(ampl) / 3));
    val = ampl / (10 ** decade);
    if decade == -9:
        lockin.write("SLVL {0:.4f} NV".format(val))
    elif decade == -6:
        lockin.write("SLVL {0:.4f} UV".format(val))
    elif decade == -3:
        lockin.write("SLVL {0:.4f} MV".format(val))
    elif decade == 0:
        lockin.write("SLVL {0:.4f} V".format(val))

    # Get the actual amplitude that was set
    a_str = sr860.query('SLVL?')
    return float(a_str)


def set_frequency(lockin, freq):
    """Set the lockin frequency and return the actual frequency that was set."""

    # Send command to lockin to set the frequency
    lockin.write("FREQ {0:.6f}".format(freq))

    # Get the actual frequency that was set
    f_str = lockin.query("FREQ?")
    return float(f_str)


def updateView(
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
):
    """Update the data view."""

    # Get the existing plot elements so they can be updated with the current data. Check
    # that number of plot elements is the same for both subplots.
    chil1 = ax1.get_children()
    chil2 = ax2.get_children()

    N_lines1 = len(chil1)
    N_lines2 = len(chil2)

    if N_lines1 != N_lines2:
        # For now, just print a warning if the numbers don't match. Maybe change this in
        # the future
        print("Warning: number of lines does not match between subplots.\n")
    N_lines = N_lines1

    # Calculate the number of sweeps that have been completed
    N_complete = r_A * n_ampl * repeats_freq + i_A * repeats_freq + r_f

    # Update the existing curves or create a new curve, depending on the number of
    # completed sweeps.
    rowA = (
        r_A * n_ampl * repeats_freq * n_freq + i_A * repeats_freq * n_freq
        + r_f * n_freq + 1
    )
    rowB = (
        r_A * n_ampl * repeats_freq * n_freq + i_A * repeats_freq * n_freq
        + r_f * n_freq + i_f
    )
    if N_lines == N_complete:
        # Need to add a new curve
        colors = parula(n_ampl);
        plot(ax1, data[rowA:rowB, 2], data[rowA:rowB, 3], '.-')#, 'Color', colors(i_A, :))
        plot(ax2, data[rowA:rowB, 2], data[rowA:rowB, 4], '.-')#, 'Color', colors(i_A, :))
    elif N_lines == N_complete + 1:
        # Update the curve for the current sweep
        plt.set(chil1[0], 'XData', data[rowA:rowB, 1])
        plt.set(chil1[0], 'YData', data[rowA:rowB, 2])
        plt.set(chil2[0], 'XData', data[rowA:rowB, 1])
        plt.set(chil2[0], 'YData', data[rowA:rowB, 3])
    else:
        # This case should not occur; print a warning
        print("Number of plot curves does not match number of sweeps.\n")


def update_view_dmma(h, data_d, data_f, row):
    """Update the data view."""

    color = data_f[:, 2] / (data_d[:, 2] + 1e-9)
    color[np.isclose(data_d[:, 2], 0)] = data_f[row, 2] / data_d[row, 2]
    offsets = data_d[:, (1, 0)]
    offsets[np.isclose(data_d[:, 2], 0), :] = 1E-9
    h.set_offsets(offsets)
    h.set_array(color)
    plt.pause(0.01)
