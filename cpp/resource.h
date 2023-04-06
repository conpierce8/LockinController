// resource.h
// encoding: utf-8
//
// Defined constants for Win32 GUI.
//
// Author:   Connor D. Pierce
// Created:  2018-02-01
// Modified: 2023-04-05 21:39:11
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


#ifndef RESOURCE_H_
#define RESOURCE_H_


#include "version.h"


/*
 * This section defines the icons that are used in the application, for use by
 * the Win32 GUI. Icons are assigned numbers in the 100s.
 */
#define SWEEP_ICON 101

/*
 * This section defines the generic menu controls: the menu, the exit
 * menu item, and a menu item separator, for use by the Win32 GUI. Generic
 * menu items are assigned numbers in the 200s.
 */
#define SWEEP_MENU 201
#define MI_EXIT 202
#define MI_SEPARATOR 203

/*
 * This section defines the number of parameters which can be controlled with
 * this application.
 *
 * Currently, the available parameters are two mirror axes (X and Y), the
 * amplitude of the piezo driving signal, the frequency of the piezo driving
 * signal, and a custom XY sweep.
 */
#define NUM_AVAIL_PARAMS 5

/*
 * This section defines identifiers for menu items, for use by the Win32 GUI.
 * Each of these identifiers is paired with a corresponding identifier, for use
 * by sweep() in FreqVoltageXYSweep.cpp. Menu items for controlling the sweep
 * settings are assigned numbers in the 300s.
 */
#define MI_SWEEP_OVER_X 300
#define SWEEP_X 0

#define MI_SWEEP_OVER_Y 301
#define SWEEP_Y 1

#define VOLTAGE_MAX 10
#define VOLTAGE_MIN -10

#define MI_SWEEP_OVER_F 302
#define SWEEP_F 2

#define MI_SWEEP_OVER_A 303
#define SWEEP_A 3

#define MI_CUSTOM_BLANK 304
#define SWEEP_CUSTOM 4

#define MI_CUSTOM_XY 305

#define MI_CUSTOM_FROMFILE 307

#define MI_CUSTOM_DISABLE 306

#define MI_AUTO_SENS 308

#define MI_DET_HARM 309

#define MI_AVERAGING 310

#define MI_RAMP_NONE 311
#define MI_RAMP_ALL 312
#define MI_RAMP_LOG 313

#define MI_SIGNAL_DC 314

const char PARAM_DESCRIPTIONS[][10] = {"X","Y","F","A","Custom XY"};

/*
 * This section defines controls (such as text fields, buttons, etc.), to be
 * used by the Win32 GUI. The run and cancel buttons are assigned numbers in
 * the 400s; text fields for entering sweep settings are assigned numbers in
 * the 500s.
 */
#define BTN_RUN_SWEEP 401
#define BTN_CANCEL_SWEEP 402

#define CONTROLS_PER_PARAM 8

#define LTEXT_X_PARAMS 500
#define LTEXT_X_START 501
#define LTEXT_X_END 502
#define LTEXT_X_STEPS 503
#define LTEXT_X_REPEAT 504
#define LTEXT_X_WAIT 505
#define LTEXT_X_ORDER 506
#define LTEXT_X_BIDIR 507

#define LTEXT_Y_PARAMS 510
#define LTEXT_Y_START 511
#define LTEXT_Y_END 512
#define LTEXT_Y_STEPS 513
#define LTEXT_Y_REPEAT 514
#define LTEXT_Y_WAIT 515
#define LTEXT_Y_ORDER 516
#define LTEXT_Y_BIDIR 517

#define LTEXT_F_PARAMS 520
#define LTEXT_F_START 521
#define LTEXT_F_END 522
#define LTEXT_F_STEPS 523
#define LTEXT_F_REPEAT 524
#define LTEXT_F_WAIT 525
#define LTEXT_F_ORDER 526
#define LTEXT_F_BIDIR 527
#define LTEXT_F_AUTOTC 528
#define LTEXT_F_LOG 529

#define LTEXT_A_PARAMS 530
#define LTEXT_A_START 531
#define LTEXT_A_END 532
#define LTEXT_A_STEPS 533
#define LTEXT_A_REPEAT 534
#define LTEXT_A_WAIT 535
#define LTEXT_A_ORDER 536
#define LTEXT_A_BIDIR 537
#define LTEXT_A_LOG 538

#define LTEXT_C_PARAMS 540
#define LTEXT_C_STEP 541
#define LTEXT_C_XVOLTS 542
#define LTEXT_C_YVOLTS 543
#define LTEXT_C_ORDER 544
#define LTEXT_C_WAIT 545
#define LTEXT_C_BIDIR 546
#define BTN_C_ADD 547
#define BTN_C_DELETE 548

#define TVIEW_SWEEP_ORDER 701
#define LTEXT_SUMMARY 702

/*
 * This section defines controls identifiers for the custom XY sweep dialog.
 */
#define CUSTOM_XY_DIALOG 801
#define LTEXT_DIALOG 802 
#define DET_HARM_DIALOG 803
#define LTEXT_DET_HARM 804
#define AVERAGING_DIALOG 805
#define LTEXT_AVG_PTS 806
#define SIGNAL_DC_DIALOG 807
#define LTEXT_SIGNAL_DC 808
#define BTN_DC_COUPLE 809
#define BTN_AC_COUPLE 810

#endif /* RESOURCE_H_ */

