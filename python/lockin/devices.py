#! python3
# -*- coding: utf-8 -*-
#
# devices.py
#
# Representation of SRS lock-in amplifier devices.
#
# Author:   Connor D. Pierce
# Created:  2023-05-02 10:49:18
# Modified: 2023-05-03 14:36:47
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
import pyvisa


# Classes
class Lockin:
    _ofsl = [-6, -12, -18, -24]

    def __init__(self, visa_dev):
        self._visa_dev = visa_dev

    @property
    def visa_dev(self):
        return self._visa_dev

    @property
    def filt_slope(self):
        resp = self._visa_dev.query("OFSL?")
        return int(resp)

    @filt_slope.setter
    def filt_slope(self, slope):
        if slope < 0 or slope > 3:
            raise ValueError("slope must be >= 0 or <= 3")
        else:
            self._visa_dev.write("OFSL {0:d}".format(slope))

    def filt_slope_value(self, slope):
        if slope < 0 or slope > 3:
            raise ValueError("slope must be >= 0 or <= 3")
        else:
            return self._ofsl[slope]

    @property
    def input_coupling(self):
        resp = self._visa_dev.query("ICPL?")
        if resp == "0":
            return "ac"
        elif resp == "1":
            return "dc"
        else:
            raise RuntimeError("Unexpected response from lockin: {0:s}".format(resp))

    @input_coupling.setter
    def input_coupling(self, coupling):
        if coupling == "ac":
            self._visa_dev.write("ICPL 0")
        elif coupling.lower() == "dc":
            self._visa_dev.write("ICPL 1")
        else:
            raise ValueError("coupling must be ac or dc, not {0:s}".format(coupling))

    @property
    def ref_ampl(self):
        resp = self._visa_dev.query("SLVL?")
        return float(resp)

    @ref_ampl.setter
    def ref_ampl(self, ampl):
        self._visa_dev.write("SLVL {0:.4f}".format(ampl))
        ampl_str = self._visa_dev.query("SLVL?")
        return float(ampl_str)

    @property
    def ref_freq(self):
        resp = self._visa_dev.query("FREQ?")
        return float(resp)

    @ref_freq.setter
    def ref_freq(self, freq):
        self._visa_dev.write("FREQ {0:.6f}".format(freq))
        freq_str = self._visa_dev.query("FREQ?")
        return float(freq_str)

    @property
    def sync_filt(self):
        resp = self._visa_dev.query("SYNC?")
        if resp == "0":
            return False
        elif resp == "1":
            return True
        else:
            raise self._unexpected(resp)

    @sync_filt.setter
    def sync_filt(self, flag):
        if flag:
            self._visa_dev.write("SYNC 1")
        else:
            self._visa_dev.write("SYNC 0")

    def time_const_idx(self, value):
        if np.all(self._time_const > value):
            raise ValueError("Requested tc is too small.")
        else:
            return np.amin(np.arange(self._time_const.size)[self._time_const > value])

    def _connect(self, model):
        rm = pyvisa.ResourceManager()
        res = rm.list_resources()
        candidates = []
        for i, desc in enumerate(res):
            dev = rm.open_resource(desc)
            dev.clear()
            idn = dev.query("*IDN?").split(",")
            if idn[0] == "Stanford_Research_Systems" and idn[1] == model:
                if self.sn is None:
                    candidates.append(i)
                else:
                    if self.sn == idn[2][3:]:
                        candidates.append(i)
            dev.close()
        if len(candidates) == 0:
            raise RuntimeError("No SR830s found.")
        elif len(candidates) > 1:
            raise RuntimeError("Multiple SR830s found; please specify s/n")
        else:
            self._visa_dev = rm.open_resource(res[candidates[0]])

    def disconnect(self):
        if self._visa_dev is not None:
            self._visa_dev.close()
            self._visa_dev = None

    def _unexpected(self, resp):
        return RuntimeError("Unexpected lockin response: '{0:s}'".format(resp))

class SR830(Lockin):
    _sens_arr = np.array(
        [
            2E-9, 5E-9,
            1E-8, 2E-8, 5E-8,
            1E-7, 2E-7, 5E-7,
            1E-6, 2E-6, 5E-6,
            1E-5, 2E-5, 5E-5,
            1E-4, 2E-4, 5E-4,
            1E-3, 2E-3, 5E-3,
            1E-2, 2E-2, 5E-2,
            1E-1, 2E-1, 5E-1,
            1E-0,
        ]
    )
    _time_const = np.array(
        [
            1E-5, 3E-5,
            1E-4, 3E-4,
            1E-3, 3E-3,
            1E-2, 3E-2,
            1E-1, 3E-1,
            1E0, 3E0,
            1E1, 3E1,
            1E2, 3E2,
            1E3, 3E3,
            1E4, 3E4,
        ]
    )
    sens_inc = 1

    def __init__(self, sn=None):
        super().__init__(None)
        self._sn = sn

    @property
    def sn(self):
        return self._sn

    def connect(self):
        self._connect("SR830")

    @property
    def det_harm(self):
        resp = self._visa_dev.query("HARM?")
        return int(resp)

    @det_harm.setter
    def det_harm(self, harm):
        if harm < 1 or harm > 19999:
            raise ValueError("Harmonic must be >= 1 and <= 19999")
        else:
            self._visa_dev.write("HARM {0:d}".format(harm))

    def get_ampl_phas(self):
        resp = self._visa_dev.query("SNAP? 3,4")
        vals = resp.split(",")
        return float(vals[0]), float(vals[1])

    @property
    def input_config(self):
        resp = self._visa_dev.query("ISRC?")
        if resp == "0":
            return "voltage - A"
        elif resp == "1":
            return "voltage - A-B"
        elif resp == "2":
            return "current - 1Meg"
        elif resp == "3":
            return "current - 100Meg"
        else:
            raise _unexpected(resp)

    @input_config.setter
    def input_config(self, config):
        if config.lower() == "voltage - a":
            value = 0
        elif config.lower() == "voltage - a-b":
            value = 1
        elif config.lower() == "current - 1meg":
            value = 2
        elif config.lower() == "current - 100meg":
            value = 3
        else:
            raise ValueError("Invalid input config: '{0:s}'".format(config))
        self._visa_dev.write("ISRC {0:d}".format(value))

    @property
    def ref_phase(self):
        resp = self._visa_dev.query("PHAS?")
        return float(resp)

    @ref_phase.setter
    def ref_phase(self, p):
        if p < -360:
            raise ValueError("p must be >= -360")
        elif p > 729.99:
            raise ValueError("p must be <= 729.99")
        else:
            self._visa_dev.write("PHAS {0:.2f}".format(p))

    @property
    def ref_source(self):
        resp = self._visa_dev.query("FMOD?")
        if resp == "0":
            return "ext"
        elif resp == "1":
            return "int"
        else:
            raise RuntimeError("Unexpected lockin response: '{0:s}'".format(resp))

    @ref_source.setter
    def ref_source(self, src):
        if src in ("internal", "int"):
            value = 1
        elif src in ("external", "ext"):
            value = 0
        else:
            raise ValueError("Invalid reference source: {0:s}".format(src))
        self._visa_dev.write("FMOD {0:d}".format(value))

    @property
    def ref_trig(self):
        resp = self._visa_dev.query("RSLP?")
        val = int(resp)
        if val == 0:
            return "SINE"
        elif val == 1:
            return "POS TTL"
        elif val == 2:
            return "NEG TTL"
        else:
            raise self._unexpected(resp)

    @ref_trig.setter
    def ref_trig(self, trig):
        _t = trig.lower()
        if _t in ("sine", "sin"):
            self._visa_dev.write("RSLP 0")
        elif _t in ("pos ttl", "pos", "default"):
            self._visa_dev.write("RSLP 1")
        elif _t in ("neg ttl", "neg"):
            self._visa_dev.write("RSLP 2")
        else:
            raise ValueError("trig must be SINE, POS TTL, or NEG TTL")


    @property
    def sens(self):
        resp = self._visa_dev.query("SENS?")
        val = int(resp)
        return val

    @sens.setter
    def sens(self, s):
        if s < 0 or s > 26:
            raise ValueError("s must be >= 0 or <= 26")
        else:
            self._visa_dev.write("SENS {0:d}".format(s))

    def sens_get_value(self, idx):
        if idx < 0:
            raise IndexError("idx must be >= 0")
        elif idx > 26:
            raise IndexError("idx must be <= 26")
        else:
            return self._sens_arr[idx]

    @property
    def status_unlock(self):
        resp = self._visa_dev.query("LIAS? 3")
        val = int(resp)
        if val == 0:
            return False
        elif val == 1:
            return True
        else:
            raise self._unexpected(resp)

    @property
    def time_const(self):
        resp = self._visa_dev.query("OFLT?")
        value = int(resp)
        if value < 0 or value > 19:
            raise self._unexpected(resp)
        else:
            return value

    @time_const.setter
    def time_const(self, tc):
        if tc < 0 or tc > 19:
            raise ValueError("tc must be >= 0 or <= 19")
        else:
            self._visa_dev.write("OFLT {0:d}".format(tc))

    def time_const_value(self, idx):
        if idx < 0 or idx > 19:
            raise ValueError("tc must be >= 0 or <= 19")
        else:
            return self._time_const[idx]


class SR860(Lockin):
    _sens_arr = np.array(
        [
            1E0,
            5E-1, 2E-1, 1E-1,
            5E-2, 2E-2, 1E-2,
            5E-3, 2E-3, 1E-3,
            5E-4, 2E-4, 1E-4,
            5E-5, 2E-5, 1E-5,
            5E-6, 2E-6, 1E-6,
            5E-7, 2E-7, 1E-7,
            5E-8, 2E-8, 1E-8,
            5E-9, 2E-9, 1E-9,
        ]
    )
    _time_const = np.array(
        [
            1E-6, 3E-6,
            1E-5, 3E-5,
            1E-4, 3E-4,
            1E-3, 3E-3,
            1E-2, 3E-2,
            1E-1, 3E-1,
            1E0, 3E0,
            1E1, 3E1,
            1E2, 3E2,
            1E3, 3E3,
            1E4, 3E4,
        ]
    )
    sens_inc = -1

    def __init__(self, sn=None):
        super().__init__(None)
        self._sn = sn

    @property
    def sn(self):
        return self._sn

    def connect(self):
        self._connect("SR860")

    @property
    def adv_filt(self):
        resp = self._visa_dev.query("ADVFILT?")
        return int(resp)

    @adv_filt.setter
    def adv_filt(self, flag):
        if flag:
            self._visa_dev.write("ADVFILT 1")
        else:
            self._visa_dev.write("ADVFILT 0")

    @property
    def det_harm(self):
        resp = self._visa_dev.query("HARM?")
        return int(resp)

    @det_harm.setter
    def det_harm(self, harm):
        if harm < 1 or harm > 99:
            raise ValueError("Harmonic must be >= 1 and <= 99")
        else:
            self._visa_dev.write("HARM {0:d}".format(harm))

    @property
    def filter(self, filt):
        """Filter time constant."""
        resp = self._visa_dev.query("OFLT?")
        
        pass

    @filter.setter
    def filter(self, filt):
        if filt < 0 or filt > 21:
            raise ValueError("filt must be >= 0 and <= 21")
        else:
            self._visa_dev.write("OFLT {0:d}".format(filt))

    def get_ampl_phas(self):
        resp = self._visa_dev.query("SNAP? 2,3")
        vals = resp.split(",")
        return float(vals[0]), float(vals[1])

    @property
    def input_config(self):
        resp1 = self._visa_dev.query("IVMD?")
        if resp1 == "0":
            # Voltage mode
            resp2 = self._visa_dev.query("ISRC?")
            if resp2 == "0":
                return "voltage - A"
            elif resp2 == "1":
                return "voltage - A-B"
            else:
                raise self._unexpected(resp2)
        elif resp1 == "1":
            # Current mode
            resp2 = self._visa_dev.query("ICUR?")
            if resp2 == "0":
                return "current - 1Meg"
            elif resp2 == "1":
                return "current - 100Meg"
            else:
                raise self._unexpected(resp2)
        else:
            raise self._unexpected(resp)

    @input_config.setter
    def input_config(self, config):
        if config.lower() == "voltage - a":
            self._visa_dev.write("IVMD 0")
            self._visa_dev.write("ISRC 0")
        elif config.lower() == "voltage - a-b":
            self._visa_dev.write("IVMD 0")
            self._visa_dev.write("ISRC 1")
        elif config.lower() == "current - 1meg":
            self._visa_dev.write("IVMD 1")
            self._visa_dev.write("ICUR 0")
        elif config.lower() == "current - 100meg":
            self._visa_dev.write("IVMD 1")
            self._visa_dev.write("ICUR 1")
        else:
            raise ValueError("Invalid input config: '{0:s}'".format(config))

    @property
    def ref_phase(self):
        resp = self._visa_dev.query("PHAS?")
        return float(resp)

    @ref_phase.setter
    def ref_phase(self, p):
        if p < -360E3:
            raise ValueError("p must be >= -360,000")
        elif p > 360E3:
            raise ValueError("p must be <= 360,000")
        else:
            self._visa_dev.write("PHAS {0:.7f}".format(p))

    @property
    def ref_source(self):
        resp = self._visa_dev.query("RSRC?")
        return int(resp)

    @ref_source.setter
    def ref_source(self, src):
        if src in ("internal", "int"):
            value = 0
        elif src in ("external", "ext"):
            value = 1
        elif src in ("dual", ):
            value = 2
        elif src in ("chop", ):
            value = 3
        else:
            raise ValueError("Invalid reference source: {0:s}".format(src))
        self._visa_dev.write("RSRC {0:d}".format(value))

    @property
    def ref_trig(self):
        resp = self._visa_dev.query("RTRG?")
        val = int(resp)
        if val == 0:
            return "SINE"
        elif val == 1:
            return "POS TTL"
        elif val == 2:
            return "NEG TTL"
        else:
            raise self._unexpected(resp)

    @ref_trig.setter
    def ref_trig(self, trig):
        _t = trig.lower()
        if _t in ("sine", "sin"):
            self._visa_dev.write("RTRG 0")
        elif _t in ("pos ttl", "pos", "default"):
            self._visa_dev.write("RTRG 1")
        elif _t in ("neg ttl", "neg"):
            self._visa_dev.write("RTRG 2")
        else:
            raise ValueError("trig must be SINE, POS TTL, or NEG TTL")

    @property
    def ref_input_impedance(self):
        resp = self._visa_dev.query("REFZ?")
        return int(resp)

    @ref_input_impedance.setter
    def ref_input_impedance(self, z):
        if z.lower() in ("50", "50 ohm"):
            self._visa_dev.write("REFZ 0")
        elif z.lower() in ("1m", "1 meg", "1 megaohm"):
            self._visa_dev.write("REFZ 1")
        else:
            raise ValueError("Invalid reference input impedance: {0:s}".format(z))

    @property
    def sens(self):
        resp = self._visa_dev.query("SCAL?")
        val = int(resp)
        return val

    @sens.setter
    def sens(self, s):
        if s < 0 or s > 27:
            raise ValueError("s must be >= 0 or <= 27")
        else:
            self._visa_dev.write("SCAL {0:d}".format(s))

    def sens_get_value(self, idx):
        if idx < 0:
            raise IndexError("idx must be >= 0")
        elif idx > 27:
            raise IndexError("idx must be <= 27")
        else:
            return self._sens_arr[idx]

    @property
    def status_unlock(self):
        resp = self._visa_dev.query("LIAS? 3")
        val = int(resp)
        if val == 0:
            return False
        elif val == 1:
            return True
        else:
            raise self._unexpected(resp)

    @property
    def time_const(self):
        resp = self._visa_dev.query("OFLT?")
        value = int(resp)
        if value < 0 or value > 21:
            raise self._unexpected(resp)
        else:
            return value

    @time_const.setter
    def time_const(self, tc):
        if tc < 0 or tc > 21:
            raise ValueError("tc must be >= 0 or <= 21")
        else:
            self._visa_dev.write("OFLT {0:d}".format(tc))

    def time_const_value(self, idx):
        if idx < 0 or idx > 21:
            raise ValueError("tc must be >= 0 or <= 21")
        else:
            return self._time_const[idx]
