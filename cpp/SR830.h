// SR830.h
// encoding: utf-8
//
// Interface to SR830 lock-in amplifier.
//
// Author:   Connor D. Pierce
// Created:  2018-02-01
// Modified: 2023-04-05 21:45:37
//
// Copyright (C) 2018-2023 Connor D. Pierce
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// SPDX-License-Identifier: MIT


#ifndef SR830_H
#define SR830_H

#include "GPIB.h"
#include "LockinSettings.h"
#include <string>


class SR830 {

public:
  GPIBInterface *gInterface;
  Addr4882_t address;
  LockinSettings settings;
  bool phaseAccessible;

  SR830(GPIBInterface *interface1, int address1):
      settings(interface1)
  {
    address = interface1->DeviceAddress(address1);
    gInterface = interface1;
    settings.queryAllOptions(address);
    int bufLen = 60;
    char buf[bufLen];
    gInterface->getDeviceDesc(buf, bufLen);
    if(strstr(buf, "SR830")) {
      phaseAccessible = true;
    }
    else {
      phaseAccessible = false;
    }
  }

  std::string get_device_description()
  {
    char desc[BUF_SIZE];
    gInterface->getDeviceDesc(desc, BUF_SIZE);
    return std::string(desc);
  }

  double get_reference_amplitude()
  {
    return settings.get("Sine Output Amplitude").dblVal1;
  }

  void set_reference_amplitude(double ampl)
  {
    settings.set(address, "Sine Output Amplitude", ampl);
  }

  int get_harmonic()
  {
    return settings.get("Detection Harmonic").intVal1;
  }

  void set_harmonic(int harmonic)
  {
    settings.set(address, "Detection Harmonic", harmonic);
  }

  double get_reference_phase()
  {
    return settings.get("Reference Phase Shift").dblVal1;
  }

  void set_reference_phase(double phas)
  {
    if(phas < 180 || phas > 180) {
      std::cout << "Note: phase " << phas << " will be wrapped to the range"
          << "-180 to +180";
    }
    settings.set(address, "Reference Phase Shift", phas);
  }

  double get_frequency()
  {
    return settings.get("Reference Frequency").dblVal1;
  }

  void set_frequency(double freq)
  {
    settings.set(address, "Reference Frequency", freq);
  }

  void ground_shield()
  {
    settings.set(address, "Input Shield Grounding", 1);
  }

  void float_shield()
  {
    settings.set(address, "Input Shield Grounding", 0);
  }

  void internal_reference()
  {
    settings.set(address, "Reference Source", 1);
  }

  void AC_couple()
  {
    settings.set(address, "Input Coupling", 0);
  }

  void DC_couple()
  {
    settings.set(address, "Input Coupling", 1);
  }

  int get_time_constant()
  {
    return settings.get("Time Constant").intVal1;
  }

  void set_time_constant(int tc)
  {
    if(tc >= 0 && tc <= 19) {
      settings.set(address, "Time Constant", tc);
    }
    else {
      std::cout << "SR830: Invalid lowpass filter time constant.\n";
    }
  }

  int get_sensitivity()
  {
    return settings.get("Sensitivity").intVal1;
  }

  void set_sensitivity(int sens)
  {
    if(sens >= 0 && sens <= 26) {
        settings.set(address, "Sensitivity", sens);
    }
    else {
        std::cout << "SR830: Invalid lowpass filter sensitivity.\n";
    }
  }

  int get_order()
  {
    return settings.get("Low Pass Filter Slope").intVal1;
  }

  int set_order(int order)
  {
    if(order >= 1 && order <= 4) {
      settings.set(address, "Low Pass Filter Slope", order-1);
      return 1;
    }
    else {
      std::cout << "SR830: Invalid lowpass filter order.\n";
      return 0;
    }
  }

  double get_amplitude()
  {
    char command[8];
    strcpy(command, "OUTP?3\0");
    return gInterface->numerical_response_command(address, command);
  }

  void get_AmplPhase(double &ampl, double &phs)
  {
    if(phaseAccessible) {
      char command[9];
      strcpy(command, "SNAP?3,4\0");
      char result[80];
      gInterface->string_response_command(address, command, result, 80);
      std::string response(result);
      int commaLoc = response.find(',');
      ampl = atof(response.substr(0,commaLoc).c_str());
      phs  = atof(response.substr(commaLoc+1).c_str());
    }
    else {
      ampl = get_amplitude();
      phs  = 0;
    }
  }

  int getIndex(char *string, int len, char c)
  {
    for(int i = 0; i<len; i++) {
      if(string[i] == c) {
        return i;
      }
    }
    return -1;
  }

  bool isPhaseAccessible()
  {
    return phaseAccessible;
  }

  double get_phase()
  {
    if(phaseAccessible) {
      char command[8];
      strcpy(command, "OUTP?4\0");
      return gInterface->numerical_response_command(address, command);
    }
    else {
      std::cout << "SR830: phase is not accessible" << std::endl;
      return 0;
    }
  }

  double get_X()
  {
    char command[8];
    strcpy(command, "OUTP?1\0");
    return gInterface->numerical_response_command(address, command);
  }

  double get_Y()
  {
    char command[8];
    strcpy(command, "OUTP?2\0");
    return gInterface->numerical_response_command(address, command);
  }

  void set_auxout1(double xvol)
  {
    settings.set(address, "Aux Out 1", xvol);
  }

  void set_auxout2(double yvol)
  {
      settings.set(address, "Aux Out 2", yvol);
  }

  double get_auxin1()
  {
    char command[8];
    strcpy(command, "OAUX?1\0");
    return gInterface->numerical_response_command(address, command);
  }

  double get_auxin2()
  {
    char command[8];
    strcpy(command, "OAUX?2\0");
    return gInterface->numerical_response_command(address, command);
  }

};

#endif
