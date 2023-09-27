#! python3
# -*- coding: utf-8 -*-
#
# demo_interactive.py
# encoding: utf-8
#
# Demonstration of command-line interaction with an SR860 lock-in amplifier.
#
# Author:   Connor D. Pierce
# Created:  2023-07-27 14:30:48
# Modified: 2023-07-27 14:34:52
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


# To run this demo interactively, make sure you have a Stanford Research Systems SR860
# lock-in amplifier connected to your computer, then enter the following commands one
# at a time into the Python interpreter.

from lockin import devices
sr860 = devices.SR860()
sr860.connect()
sr860.ref_freq = 100  # set reference frequency to 100 Hz
sr860.ref_ampl = 0.2  # set reference amplitude to 0.2 V
sr860.input_coupling = "ac"  # set input coupling to AC-coupled
sr860.disconnect()
