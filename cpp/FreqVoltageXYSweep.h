// FreqVoltageXYSweep.h
// encoding: utf-8
//
// Stanford Research Systems lock-in amplifier control application.
//
// Author:   Connor D. Pierce
// Created:  2018-02-02
// Modified: 2023-04-05 21:30:11
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


#ifndef MAIN_H_
#define MAIN_H_

#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <sstream>
#include <limits.h>
#include <cmath>
#include <ctime>
#include <fstream>

#include "resource.h"
#include "SR830.h"


///////////////////////////////////// DEFINES //////////////////////////////////////////

#define ATTEN_2F -80
#define SIGNAL_TO_DC -40
#define FIRST_STEP_WAIT 2000
#define LOG10_3 0.477121
#define STEPS_PER_RAMP_DECADE 25


///////////////////////////////////// CONSTANTS ////////////////////////////////////////

const char g_szClassName[] = "myWindowClass";


/* Version string */
const char FVXY_version[] = SOFTWARE_NAME_VERSION;


//////////////////////////////////// STRUCTS ///////////////////////////////////////////

/*
 * struct SweepParameters
 *
 * This data structure defines the data that is needed to perform a sweep
 * across N variables. This structure describes the dimensionality, nesting
 * order, repetition, wait times, and ranges for a multi-parameter sweep.
 *
 * Fields:
 *   parameters - int array of size N. This parameter describes the nesting of
 *       the sweeps. parameters[0] contains an identifier for the parameter to
 *       be controlled in the top-level sweep. parameters[1] contains an
 *       identifier for the parameter to be controlled in the second-level
 *       sweep; that is, a full sweep of parameters[1] will be made for each
 *       value of parameters[0] in the top level of the sweep
 *   maxRecursionLevel - integer describing the size of parameters, identifying
 *       the number of parameters to be swept, and therefore the number of
 *       levels of recursion of the sweep
 *   starts - array with size equal to size(parameters). starts[i] gives the
 *       starting value for the i^th level of the sweep
 *   ends - array with size equal to size(parameters). ends[i] gives the final
 *       value for the i^th level of the sweep
 *   steps - array with size equal to size(parameters). steps[i] gives the
 *       number of steps between starts[i] and ends[i]. Note that a sweep with
 *       N steps will include N+1 points, in order to include the endpoints
 *   repeats - array with size equal to size(parameters). repeats[i] gives the
 *       number of times to repeat the i^th level of the sweep
 *   waits - array with size equal to size(parameters). waits[i] gives the
 *       number of milliseconds to wait after setting the value for the i^th
 *       parameter in the sweep. If i denotes the innermost level of the sweep,
 *       the program will wait for a period of waits[i] before measuring the
 *       response of the vibrometer. If i denotes a level which is not the
 *       innermost level of the sweep, the program will wait for a period of
 *       waits[i] before entering the (i+1)^th level of the sweep
 *   autoTimeConst - if true, allow the program to automatically adjust the time
 *       constant to be appropriate for the frequency being studied. This value
 *       will have no effect unless parameters[i] = SWEEP_F for some i.
 *   autoSens - allow the program to automatically adjust the lockin sensitivity
 *       to adjust to signal magnitudes which have a functional dependence on
 *       the sweep parameters.
 *   detHarm - detection harmonic, for example, if detHarm=2, the lockin will
 *       excite a frequency f and measure the signal component at 2f
 *   logSpacing - true to specify logarithmicly-spaced sweep values, false to
 *       specify linearly-spaced sweep values. ADDED IN V1.0.9. As of V1.0.9,
 *       only amplitude and frequency can be log-spaced.
 */
struct SweepParameters {
  bool ac_couple;
  bool autoSens;
  bool autoTimeConst;
  bool bidirectional[NUM_AVAIL_PARAMS];
  int detHarm;
  double ends[NUM_AVAIL_PARAMS];
  bool logSpacing[NUM_AVAIL_PARAMS];
  int maxRecursionLevel;
  int parameters[NUM_AVAIL_PARAMS];
  int repeats[NUM_AVAIL_PARAMS];
  double signalToDC;
  double starts[NUM_AVAIL_PARAMS];
  int steps[NUM_AVAIL_PARAMS];
  long waits[NUM_AVAIL_PARAMS];
};


enum STRCONV_ERROR {
  CONV_SUCCESS,
  CONV_OVERFLOW,
  CONV_UNDERFLOW,
  CONV_INCONVERTIBLE
};


/////////////////////////////////// VARIABLES //////////////////////////////////////////

bool acceptTextInput = false;

bool activeParams[] = {false, false, true, true, false};

bool averaging = false;

HANDLE bgThreadHandle = NULL, gpibCheckerHandle = NULL;

volatile bool cancelSweep = true;

const HANDLE cancelSweepEvent = CreateEvent(NULL, TRUE, FALSE, "CancelSweepEvent");

volatile bool connReady = false;

int count = 0;

int currCustomStep = 0;

double* customX;

double* customY;

int filterType = 3;

GPIBInterface *gpibInterface = NULL;

HWND hwnd;

SR830 *lockin = NULL; 

std::ofstream logger("log.txt");

Addr4882_t nstrmnts[31], rslt[31];

int numActiveParams = 2;

int numAvgPts = 10;

int numCustom;

OPENFILENAME ofn;

const HANDLE quitGpibCheckerEvent = CreateEvent(NULL, TRUE, FALSE,
    "QuitGpibCheckerEvent");

int rampType = 1;

bool settingCustomSweep = FALSE;

SweepParameters sweepSetup;

char szFileName[MAX_PATH] = "";


/////////////////////////////////// FUNCTIONS //////////////////////////////////////////

STRCONV_ERROR str2int (int &i, char const *s) {
  char *end;
  long  l;
  errno = 0;
  l = strtol(s, &end, 10);
  if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
    std::cout << "overflow" << std::endl;
    return CONV_OVERFLOW;
  }
  if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
    return CONV_UNDERFLOW;
  }
  if (*s == '\0' || *end != '\0') {
    return CONV_INCONVERTIBLE;
  }
  i = l;
  return CONV_SUCCESS;
}


STRCONV_ERROR str2long (long &l, char const *s) {
  char *end;
  long i;
  errno = 0;
  i = strtol(s, &end, 10);
  if (errno == ERANGE && i == LONG_MAX) {
    return CONV_OVERFLOW;
  }
  if (errno == ERANGE && i == LONG_MIN) {
    return CONV_UNDERFLOW;
  }
  if (*s == '\0' || *end != '\0') {
    return CONV_INCONVERTIBLE;
  }
  l = i;
  return CONV_SUCCESS;
}


STRCONV_ERROR str2dbl (double &d, char const *s) {
  char *end;
  double i;
  errno = 0;
  i = strtod(s, &end);
  if (errno == ERANGE && (i == HUGE_VAL || i == -HUGE_VAL)) {
    return CONV_OVERFLOW;
  }
  if (errno == ERANGE) {
    return CONV_UNDERFLOW;
  }
  if (*s == '\0' || *end != '\0') {
    return CONV_INCONVERTIBLE;
  }
  d = i;
  return CONV_SUCCESS;
}


LRESULT CALLBACK AveragingDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


long calculateSweepDuration();


DWORD WINAPI connectToAmp(LPVOID lpParam);


void createControls(HINSTANCE hInstance);


void createCustomXYControls(HINSTANCE hInstance, int yPos, BOOL active);


void createParamControls(HINSTANCE hInstance, int yPos, int param, BOOL active);


LRESULT CALLBACK CustomXYDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


void deleteCustomXYPoint();


LRESULT CALLBACK DetHarmDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


bool enableCustomSweep(bool useXY);


/*
 * Get the most appropriate lockin sensitivity for the given voltage.
 */
int getBestSens(double ampl, int currSens);


/*
 * Gets the Win32 identifiers for the controls corresponding to the parameter
 * identified by param. The values are stored in the array IDs, which should be
 * a 1XCONTROLS_PER_PARAM array. (See resource.h for a description of the meaning of
 * CONTROLS_PER_PARAM). The last value in the IDs array will always be the EDIT
 * control for the order value.
 *
 * IDs[0] = ID for the LTEXT heading
 * IDs[1] = ID for the EDIT control for the start value
 * IDs[2] = ID for the EDIT control for the end value
 * IDs[3] = ID for the EDIT control for the # of steps value
 * IDs[4] = ID for the EDIT control for the # of repeats value
 * IDs[5] = ID for the EDIT control for the wait time value
 * IDs[CONTROLS_PER_PARAM-1] = ID for the EDIT control for the order value
 */
void getControlIDs(uintptr_t* IDs, int param);


void getCurrentValues(int currParam, double * currValues);


bool getCustomXYValues(double &x, double &y);


/*
 * Get the value (in volts) of the sensitivity given by the index i.
 *
 * See LockinSettings.cpp for valid indices and the values of the sensitivity to which
 * they correspond.
 */
double getSensValue(int i);


/*
 * Get the minimum required time constant.
 *
 * The time constant is computed for the given frequency and filter slope, assuming the
 * lockin input coupling is set by sweepSetup.ac_couple.
 */
double getTauReq(double freq, int filtSlope);

/*
 * Get the smallest available time constant setting that is larger than reqTau.
 */
int getTimeConst(double reqTau);


/*
 * Returns the double-precision value representing the value of the time 
 * constant given by the index i. See LockinSettings.cpp for valid indices and
 * the values of the time constant to which they correspond. Returned value is
 * in seconds.
 */
double getTimeConstValue(int i);


/*
 * Returns the double-precision value representing the slope of the low-pass
 * filter given by the index i. See LockinSettings.cpp for valid indices and the
 * values of the filter slope to which they correspond. Returned value is in
 * dB/octet and is signed, i.e. a filter with roll-off of 18 dB/octet will be
 * represented by -18.0.
 */
double getFiltSlopeValue(int i);


/*
 * Returns the multiplier which relates the wait time to the time constant
 * that is appropriate for the indicated slope of the low-pass filter.
 */
double getWaitFactor(int filtSlope);


void logCanceledSweep();


void populateTree(HWND tree);


/*
 * Determines whether the given voltage `ampl` is out of range for the lockin
 * sensitivity `sens`.
 */
bool outOfRange(double ampl, int sens);


/*
 * Ramp the current parameter down from its current value to its initial value.
 *
 * @param recursionLevel - the level of the parametric sweep
 * @param currParam - the parameter at the current sweep level
 * @param allSteps - the array of values for the current sweep level
 * @param i - the currently-active value in the sweep
 */
void rampDown(int recursionLevel, int currParam, double * allSteps, int& i);


void sendCommandToLockin(int currParam, double currVal);


/*
 * Callback function for the signal-to-dc dialog box.
 */
LRESULT CALLBACK SignalDCDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


/*
 * Run a parametric sweep
 */
int sweep();


/*
 * Get the average value and standard deviation over multiple repeated measurements.
 */
void sweepDoAveraging(
  double ampl0, double phs0, double *ampl, double *phs, double *stdDev
);


/*
 * Record measured value to file.
 */
void sweepWriteMeasurement(
  std::ofstream& outps, std::string prefix, double ampl, double phs, double stdDev
);


/*
 * Take a measurement with the lock-in amplifier and verify the sensitivity setting.
 */
int sweepDoMeasurement(
  double *ampl,
  double *phs,
  int waitTime,
  int r,
  int i,
  int &initialSens,
  std::string prefix,
  double currVal
);


/*
 * Initialize output file and logging at the beginning of a sweep.
 */
void sweepInitOutput(std::ofstream& outps);


/*
 * Log the end time of the sweep and flush the output and settings files.
 */
void sweepFinalizeOutput(std::ofstream& outps);


/*
 * Parameter values loop for a single level of the multi-level sweep.
 */
int sweepParameterLoop(
  int recursionLevel,
  double * currValues,
  int nValues,
  int currWait,
  bool reversed,
  int repeatNum,
  int &i,
  int &initialSens,
  std::string prefix,
  std::ofstream& outps
);


/*
 * "Repeats" loop for a single level of the multi-level sweep
 */
int sweepRepeatLoop(int recursionLevel, std::string prefix, std::ofstream& outps);


/*
 * Calculate and set the minimum time constant for the given frequency.
 */
int sweepSetAutoTimeConst(double currFreq, int *waitTime);


DWORD WINAPI SweepThreadFunction( LPVOID lpParam );


bool storeCustomXYPoint();


/*
 * Toggle bidirectionality for the given parameter.
 */
void toggleBidirectional(int ltext_id);


void updateActiveParams(int changedParam);


void updateCustomXYText();


bool validateSweepParams(bool runSweep);


#endif /* MAIN_H_ */


