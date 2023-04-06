// FreqVoltageXYSweep.cpp
// encoding: utf-8
//
// Stanford Research Systems lock-in amplifier control application.
//
// Author:   Connor D. Pierce
// Created:  2018-02-02
// Modified: 2023-04-05 21:30:24
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


#include "FreqVoltageXYSweep.h"


LRESULT CALLBACK WndProc(HWND hwndLcl, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static int count = 0;
  switch(msg) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case MI_SWEEP_OVER_X:
        case MI_SWEEP_OVER_Y:
        case MI_SWEEP_OVER_A:
        case MI_SWEEP_OVER_F:
        case MI_AUTO_SENS:
          updateActiveParams(LOWORD(wParam));
          validateSweepParams(false);
          break;
        case MI_CUSTOM_BLANK:
        case MI_CUSTOM_XY:
          if(LOWORD(wParam) == MI_CUSTOM_BLANK) {
            int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(CUSTOM_XY_DIALOG), hwnd, CustomXYDlgProc);
            if(ret == IDOK) {
              enableCustomSweep(false);
            } else {
              numCustom = 0;
              settingCustomSweep = false;
              currCustomStep = 0;
            }
          } else {
            logger << "WndProc: new blank sweep, numCustom=" << numCustom <<std::endl;
            enableCustomSweep(true);
          }
          break;
        case MI_CUSTOM_DISABLE:
          settingCustomSweep = false;
          updateActiveParams(MI_CUSTOM_BLANK);
          break;
        case MI_DET_HARM: {
          int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DET_HARM_DIALOG), hwnd, DetHarmDlgProc);
          if(ret == IDOK) {
            logger << "WndProc: user set the detection harmonic" << std::endl;
            validateSweepParams(false);
          }
          break;
        }
        case MI_SIGNAL_DC: {
          int ret = DialogBox(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(SIGNAL_DC_DIALOG),
            hwnd,
            SignalDCDlgProc
          );
          if(ret == IDOK) {
            logger << "WndProc: user set the detection harmonic" << std::endl;
            validateSweepParams(false);
          }
          break;
        }
        case MI_AVERAGING: {
          HMENU menu = GetMenu(hwnd);
          if(averaging) {
            averaging = false;
            CheckMenuItem(menu, MI_AVERAGING, MF_UNCHECKED);
          } else {
            int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(AVERAGING_DIALOG), hwnd, AveragingDlgProc);
            if(ret == IDOK) {
              logger << "WndProc: user enabled averaging; points: " << numAvgPts << std::endl;
              averaging = true;
              CheckMenuItem(menu, MI_AVERAGING, MF_CHECKED);
            }
          }
          break;
        }
        case MI_EXIT:
          PostMessage(hwndLcl, WM_CLOSE, 0, 0);
          break;
        case BTN_RUN_SWEEP:
          if(GetSaveFileName(&ofn)) {
            validateSweepParams(true);
          }
          logger << "Run Sweep!" << std::endl;
          break;
        case MI_RAMP_NONE: {
          rampType = 0;
          HMENU menu = GetMenu(hwnd);
          CheckMenuItem(menu, MI_RAMP_NONE, MF_CHECKED);
          CheckMenuItem(menu, MI_RAMP_ALL, MF_UNCHECKED);
          CheckMenuItem(menu, MI_RAMP_LOG, MF_UNCHECKED);
          break;
        }
        case MI_RAMP_ALL: {
          rampType = 1;
          HMENU menu = GetMenu(hwnd);
          CheckMenuItem(menu, MI_RAMP_NONE, MF_UNCHECKED);
          CheckMenuItem(menu, MI_RAMP_ALL, MF_CHECKED);
          CheckMenuItem(menu, MI_RAMP_LOG, MF_UNCHECKED);
          break;
        }
        case MI_RAMP_LOG: {
          rampType = 2;
          HMENU menu = GetMenu(hwnd);
          CheckMenuItem(menu, MI_RAMP_NONE, MF_UNCHECKED);
          CheckMenuItem(menu, MI_RAMP_ALL, MF_UNCHECKED);
          CheckMenuItem(menu, MI_RAMP_LOG, MF_CHECKED);
          break;
        }
        case BTN_CANCEL_SWEEP:
          cancelSweep = true;
          SetEvent(cancelSweepEvent);
//          logger << count << std::endl;
          break;
        case BTN_C_ADD:
          if(settingCustomSweep) {
            if(storeCustomXYPoint()) {
              ++currCustomStep;
              logger << "WndProc: currCustomStep incremented to: " << currCustomStep << std::endl;
              if(currCustomStep == numCustom){
                logger << "WndProc: currCustomStep equals numCustom" << std::endl;
                currCustomStep = 0;
                settingCustomSweep = false;
              }
              updateCustomXYText();
            }
          }
          validateSweepParams(false);
          break;
        case BTN_C_DELETE:
          if(settingCustomSweep) {
            deleteCustomXYPoint();
            if(currCustomStep == numCustom) {
              currCustomStep == 0;
              settingCustomSweep = false;
            }
            updateCustomXYText();
          }
          validateSweepParams(false);
          break;
        case LTEXT_C_XVOLTS: case LTEXT_C_YVOLTS: {
          double X, Y;
          bool flag = getCustomXYValues(X, Y);
          if(flag && connReady) {
            sendCommandToLockin(SWEEP_X, X);
            sendCommandToLockin(SWEEP_Y, Y);
          }
          break;
        }
        case LTEXT_SUMMARY:
          break;
        case LTEXT_F_AUTOTC: {
          sweepSetup.autoTimeConst = !sweepSetup.autoTimeConst;
          HWND chkBx = GetDlgItem(hwnd, LTEXT_F_AUTOTC);
          if(sweepSetup.autoTimeConst) {
            SendMessage(chkBx, BM_SETCHECK, BST_CHECKED, 0);
          } else {
            SendMessage(chkBx, BM_SETCHECK, BST_UNCHECKED, 0);
          }
          HWND waitText = GetDlgItem(hwnd, LTEXT_F_WAIT);
          EnableWindow(waitText, !sweepSetup.autoTimeConst);
          validateSweepParams(false);
          break;
        }
        case LTEXT_F_LOG: {
          sweepSetup.logSpacing[SWEEP_F] = !sweepSetup.logSpacing[SWEEP_F];
          HWND chkBx = GetDlgItem(hwnd, LTEXT_F_LOG);
          if(sweepSetup.logSpacing[SWEEP_F]) {
            SendMessage(chkBx, BM_SETCHECK, BST_CHECKED, 0);
          } else {
            SendMessage(chkBx, BM_SETCHECK, BST_UNCHECKED, 0);
          }
          validateSweepParams(false);
          break;
        }
        case LTEXT_A_LOG: {
          sweepSetup.logSpacing[SWEEP_A] = !sweepSetup.logSpacing[SWEEP_A];
          HWND chkBx = GetDlgItem(hwnd, LTEXT_A_LOG);
          if(sweepSetup.logSpacing[SWEEP_A]) {
            SendMessage(chkBx, BM_SETCHECK, BST_CHECKED, 0);
          } else {
            SendMessage(chkBx, BM_SETCHECK, BST_UNCHECKED, 0);
          }
          validateSweepParams(false);
          break;
        }
        case LTEXT_X_BIDIR:
        case LTEXT_Y_BIDIR:
        case LTEXT_A_BIDIR:
        case LTEXT_F_BIDIR:
        case LTEXT_C_BIDIR: {
          toggleBidirectional(LOWORD(wParam));
          validateSweepParams(false);
          break;
        }
        default:
          if(HIWORD(wParam) == EN_CHANGE) {
            count++;
            if(acceptTextInput) {
              validateSweepParams(false);
            }
            break;
          } else {
          logger << LOWORD(wParam) << ";" << HIWORD(wParam) << ";"
              << LOWORD(lParam) << ";" << HIWORD(lParam)
              << std::endl;
          }
      }
      break;
    case WM_CLOSE:
      SetEvent(cancelSweepEvent);
      SetEvent(quitGpibCheckerEvent);
      WaitForSingleObject(bgThreadHandle, 5000ul);
      WaitForSingleObject(gpibCheckerHandle, 1000);
      logger.flush();
      logger.close();
      DestroyWindow(hwndLcl);
    break;
    case WM_DESTROY:
      PostQuitMessage(0);
    break;
    default:
      return DefWindowProc(hwndLcl, msg, wParam, lParam);
  }
  return 0;
}


void toggleBidirectional(int ltext_id)
{
  int paramId = -1;
  switch(ltext_id) {
    case LTEXT_X_BIDIR: {
      paramId = SWEEP_X;
      break;
    }
    
    case LTEXT_Y_BIDIR: {
      paramId = SWEEP_Y;
      break;
    }
    
    case LTEXT_A_BIDIR: {
      paramId = SWEEP_A;
      break;
    }
    
    case LTEXT_F_BIDIR: {
      paramId = SWEEP_F;
      break;
    }
    
    case LTEXT_C_BIDIR: {
      paramId = SWEEP_CUSTOM;
      break;
    }
  }
  sweepSetup.bidirectional[paramId] = !sweepSetup.bidirectional[paramId];
  HWND chkBx = GetDlgItem(hwnd, ltext_id);
  if(sweepSetup.bidirectional[paramId]) {
    SendMessage(chkBx, BM_SETCHECK, BST_CHECKED, 0);
  } else {
    SendMessage(chkBx, BM_SETCHECK, BST_UNCHECKED, 0);
  }
}


bool getCustomXYValues(double &x, double &y)
{
  char content[80];  //Create a temp variable to store the text entered by user
  double tmpX, tmpY; //Create temp variables to store the values entered by user
  
  //Obtain a handle to the field for X value
  HWND ctrl = GetDlgItem(hwnd, LTEXT_C_XVOLTS);
  
  //Extract user input from text field and store in "content"
  GetWindowText(ctrl, content, 80);
  
  //Attempt to convert user input to numerical value. If conversion succeeds,
  //"res" will be equal to CONV_SUCCESS
  STRCONV_ERROR res = str2dbl(tmpX, content);
  if(res != CONV_SUCCESS) {
    //Conversion was not successful; we cannot assign a numerical value to "x"
    return false;
  }
  
  //Repeat above steps to obtain y value
  ctrl = GetDlgItem(hwnd, LTEXT_C_YVOLTS);
  GetWindowText(ctrl, content, 80);
  res = str2dbl(tmpY, content);
  if(res != CONV_SUCCESS) {
    return false;
  }
  
  //If we reach this point in function, both conversions succeeded. Store
  //the temporary numerical values and return true. The galvo system can
  //only handle voltages from -10V to +10V, so x and y are restricted to
  //these values.
  if(tmpX > VOLTAGE_MAX) {
    x = VOLTAGE_MAX;
  } else if(tmpX < VOLTAGE_MIN) {
    x = VOLTAGE_MIN;
  } else {
    x = tmpX;
  }
  if(tmpY > VOLTAGE_MAX) {
    y = VOLTAGE_MAX;
  } else if(tmpY < VOLTAGE_MIN) {
    y = VOLTAGE_MIN;
  } else {
    y = tmpY;
  }
  return true;
}


bool storeCustomXYPoint()
{
  return getCustomXYValues(customX[currCustomStep], customY[currCustomStep]);
}


void updateCustomXYText()
{
  std::ostringstream oss;
  logger << "updateCustomXYText: currCustomStep=" << currCustomStep << std::endl;
  logger << "updateCustomXYText: customX=" << customX[currCustomStep] << std::endl;
  logger << "updateCustomXYText: numCustom=" << numCustom << std::endl;
  oss << customX[currCustomStep];
  HWND ctrl = GetDlgItem(hwnd, LTEXT_C_XVOLTS);
  SetWindowText(ctrl, oss.str().c_str());
  std::ostringstream oss2;
  oss2 << customY[currCustomStep];
  ctrl = GetDlgItem(hwnd, LTEXT_C_YVOLTS);
  SetWindowText(ctrl, oss2.str().c_str());
  std::ostringstream oss3;
  oss3 << currCustomStep;
  ctrl = GetDlgItem(hwnd, LTEXT_C_STEP);
  SetWindowText(ctrl, oss3.str().c_str());
}


void deleteCustomXYPoint()
{
  for(int i = currCustomStep; i < numCustom-1; i++) {
    customX[i] = customX[i+1];
    customY[i] = customY[i+1];
  }
  numCustom--;
}


LRESULT CALLBACK CustomXYDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message) {
        case WM_INITDIALOG:
          return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDOK:
                  STRCONV_ERROR res;
          HWND ctrl;
          ctrl = GetDlgItem(hwnd, LTEXT_DIALOG);
          char content[80];
          GetWindowText(ctrl, content, 80);
          res = str2int(numCustom, content);
          if(res == CONV_SUCCESS){
            EndDialog(hwnd, IDOK);
          }
                  break;
                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                  break;
            }
          break;
        default:
            return FALSE;
    }
    return TRUE;
}


LRESULT CALLBACK DetHarmDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  HWND ctrl = GetDlgItem(hwnd, LTEXT_DET_HARM);
  char content[80];
    switch(Message) {
        case WM_INITDIALOG:
          snprintf(content, 80, "%d", sweepSetup.detHarm);
          SetWindowText(ctrl, content);
          return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDOK: {
          GetWindowText(ctrl, content, 80);
          STRCONV_ERROR res = str2int(sweepSetup.detHarm, content);
          if(res == CONV_SUCCESS){
            EndDialog(hwnd, IDOK);
          }
                  break;
        }
                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                  break;
            }
          break;
        default:
            return FALSE;
    }
    return TRUE;
}


LRESULT CALLBACK AveragingDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  HWND ctrl = GetDlgItem(hwnd, LTEXT_AVG_PTS);
  char content[80];
    switch(Message) {
        case WM_INITDIALOG:
          snprintf(content, 80, "%d", numAvgPts);
          SetWindowText(ctrl, content);
          return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDOK: {
          GetWindowText(ctrl, content, 80);
          STRCONV_ERROR res = str2int(numAvgPts, content);
          if(res == CONV_SUCCESS){
            EndDialog(hwnd, IDOK);
          }
                  break;
        }
                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                  break;
            }
          break;
        default:
            return FALSE;
    }
    return TRUE;
}


int WINAPI WinMain(
  HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
)
{
  WNDCLASSEX wc;
  MSG Msg;

  HICON hMyIcon   = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(SWEEP_ICON), IMAGE_ICON, 32, 32, 0);
  HICON hMyIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(SWEEP_ICON), IMAGE_ICON, 16, 16, 0);
  logger << "Loaded icons!" << std::endl;
  
  //Step 1: Registering the Window Class
  wc.cbSize     = sizeof(WNDCLASSEX);
  wc.style     = 0;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra   = 0;
  wc.cbWndExtra   = 0;
  wc.hInstance   = hInstance;
  wc.hIcon     = hMyIcon;
  wc.hIconSm       = hMyIconSm;
  wc.hCursor     = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
  wc.lpszMenuName  = MAKEINTRESOURCE(SWEEP_MENU);
  wc.lpszClassName = g_szClassName;

  if(!RegisterClassEx(&wc))
  {
    MessageBox(NULL, "Window Registration Failed!", "Error!",
      MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  logger << "Registered Window Class" << std::endl;

  // Step 2: Creating the Window
  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    g_szClassName,
    FVXY_version,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 760, 550,
    NULL, NULL, hInstance, NULL);

  if(hwnd == NULL) {
    MessageBox(NULL, "Window Creation Failed!", "Error!",
      MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  
  logger << "Created the window" << std::endl;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
      OFN_OVERWRITEPROMPT;
      
  logger << "Created filechooser struct" << std::endl;

  INITCOMMONCONTROLSEX initCtrls;
  initCtrls.dwICC = ICC_TREEVIEW_CLASSES;
  initCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);

  BOOL iccInitFinished = InitCommonControlsEx(&initCtrls);
  logger << "Common Control Initialization:" << iccInitFinished << std::endl;
  
  sweepSetup.detHarm = 1;
  sweepSetup.ac_couple = true;
  sweepSetup.signalToDC = -40;
  for(int i = 0; i<NUM_AVAIL_PARAMS; i++) {
    sweepSetup.logSpacing[i] = false;
  }
  createControls(hInstance);
  acceptTextInput = TRUE;
  LPDWORD thrdId = NULL;
  for(int i = 0; i<NUM_DEVICES - 1; i++) {
    nstrmnts[i] = i+1;
  }
  nstrmnts[NUM_DEVICES - 1] = NOADDR;
  gpibCheckerHandle = CreateThread(NULL, 0, connectToAmp, NULL, 0, thrdId);
  logger << "Started GPIB checking" << std::endl;

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);
  logger << "Showed window" << std::endl;

  // Step 3: The Message Loop
  while(GetMessage(&Msg, NULL, 0, 0) > 0)
  {
    if(!IsDialogMessage(hwnd,&Msg)) {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }
  return Msg.wParam;
}


DWORD WINAPI connectToAmp(LPVOID lpParam)
{
  while(1) {
    if(cancelSweep) {
      //Only perform this check if a sweep is not currently active
      if(gpibInterface == NULL) {
        //Currently not connected; try to connect by creating
        //new GPIBInterface and new SR830 objects
        try {
          logger << "connectToAmp: trying to create interface" << std::endl;
          gpibInterface = new GPIBInterface(0);
          logger << "  connectToAmp: created interface" << std::endl;
          lockin = new SR830(gpibInterface,8);
          lockin->settings.queryAllOptions(lockin->address);
          filterType = lockin->settings.get("Low Pass Filter Slope").intVal1;
          logger << "    connectToAmp: created SR830" << std::endl;
          connReady = true;
        } catch(DisconnectedException &ex) {
          logger << "  connectToAmp: disconnected exception:" << ex.what() << std::endl;
          delete gpibInterface;
          gpibInterface = NULL;
          connReady = false;
        } catch(std::exception &e) {
          logger << "Exception in GPIB checker: " << e.what();
          gpibInterface = NULL;
          connReady = false;
          break;
        }
      } else {
        //We were connected at last check; see if we're still connected
        logger << "connectToAmp: checking to see if still connected" << std::endl;
        FindLstn(0, nstrmnts, rslt, 31);
        if(ibsta & ERR) {
          logger << "  connectToAmp: error occured in FindLstn" << std::endl;
              //Connection to GPIB board had some kind of error; we're
              //probably not connected anymore. Free up the memory used
              //by the current GPIBInterface and SR830 objects.
              delete gpibInterface;
              delete lockin;
              gpibInterface = NULL;
              lockin = NULL;
              connReady = false;
            }
      }
    }
    logger << "  connectToAmp: validating sweep params" << std::endl;
    validateSweepParams(false);
    if(WaitForSingleObject(quitGpibCheckerEvent, 5000) == WAIT_OBJECT_0) {
      break;
    }
  }
  return 1;
}


void createControls(HINSTANCE hInstance)
{
  CreateWindow("BUTTON", "Run Sweep",
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
      220, 370, 100, 100, hwnd, (HMENU) BTN_RUN_SWEEP, hInstance, NULL);
  CreateWindow("BUTTON", "Cancel Sweep",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 370,
      100, 100, hwnd, (HMENU) BTN_CANCEL_SWEEP, hInstance, NULL);
  CreateWindowEx(WS_EX_LEFT, WC_TREEVIEW,
            TEXT("Tree View"), WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
            10, 370, 200, 100, hwnd, (HMENU)TVIEW_SWEEP_ORDER, hInstance,NULL);
  CreateWindow("EDIT", (connReady)?"GPIB Ready":"GPIB Disconnected!",
      WS_VISIBLE | WS_CHILD | ES_READONLY | ES_MULTILINE,
      450, 370, 290, 100, hwnd, (HMENU) LTEXT_SUMMARY, hInstance, NULL);

  int yPos = 10;
  for(int ctrl = SWEEP_X; ctrl <= SWEEP_A; ctrl++) {
    createParamControls(hInstance, yPos, ctrl, activeParams[ctrl]);
    yPos += 70;
  }
  createCustomXYControls(hInstance, yPos, activeParams[SWEEP_CUSTOM]);
}


void createParamControls(HINSTANCE hInstance, int yPos, int param, BOOL active)
{
  uintptr_t controlIDs[CONTROLS_PER_PARAM];
  getControlIDs(controlIDs, param);

  uintptr_t ltext_hdg    = controlIDs[0]; uintptr_t ltext_start = controlIDs[1];
  uintptr_t ltext_end    = controlIDs[2]; uintptr_t ltext_steps = controlIDs[3];
  uintptr_t ltext_repeat = controlIDs[4]; uintptr_t ltext_wait  = controlIDs[5];
  uintptr_t ltext_order  = controlIDs[6]; uintptr_t ltext_bidir = controlIDs[7];
  std::string paramStr;

  switch(param) {
    case SWEEP_X:
      paramStr   ="Parameters for X sweep:"; break;
    case SWEEP_Y:
      paramStr   ="Parameters for Y sweep:"; break;
    case SWEEP_F:
      paramStr   ="Parameters for F sweep:"; break;
    case SWEEP_A:
      paramStr   ="Parameters for A sweep:"; break;
    case SWEEP_CUSTOM:
      paramStr   ="Parameters for custom X,Y sweep:"; break;
  }

  HWND tmp;

  CreateWindow("STATIC", "A frame to surround the parameter controls",
      WS_VISIBLE | WS_CHILD | SS_ETCHEDFRAME, 5, yPos-5, 730, 55, hwnd,
      (HMENU) -1, hInstance, NULL);

  tmp = CreateWindow("STATIC", paramStr.data(), WS_VISIBLE | WS_CHILD,
      10, yPos, 150, 15, hwnd, (HMENU) ltext_hdg, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Order:", WS_VISIBLE | WS_CHILD,
      620, yPos, 50, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP ,
      675, yPos, 50, 15, hwnd, (HMENU) ltext_order, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Start at:", WS_VISIBLE | WS_CHILD,
      10, yPos + 25, 50, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP ,
      65, yPos + 25, 50, 15, hwnd, (HMENU) ltext_start, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Sweep to:",  WS_VISIBLE | WS_CHILD,
      135, yPos + 25,  65, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      205, yPos+25, 50, 15, hwnd, (HMENU) ltext_end, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "# of steps:", WS_VISIBLE | WS_CHILD,
      275, yPos + 25,  75, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      355, yPos+25, 50, 15, hwnd, (HMENU) ltext_steps, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "# of repeats:",  WS_VISIBLE | WS_CHILD,
      425, yPos+25, 85, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      515, yPos+25, 30, 15, hwnd, (HMENU) ltext_repeat, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Wait time (ms):", WS_VISIBLE | WS_CHILD,
      565, yPos+25, 105, 15, hwnd,  (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      675, yPos+25, 50, 15, hwnd, (HMENU) ltext_wait, hInstance, NULL);
  EnableWindow(tmp, active);
  
  tmp = CreateWindow("BUTTON", "Bi-dir", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            520, yPos, 70, 15,
      hwnd, (HMENU) ltext_bidir, hInstance, NULL);
  EnableWindow(tmp, active);
  
  if(param == SWEEP_F) {
    tmp = CreateWindow("BUTTON", "Auto Time Constant",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 235, yPos, 150, 15,
        hwnd, (HMENU) LTEXT_F_AUTOTC, hInstance, NULL);
    EnableWindow(tmp, active);
    tmp = CreateWindow("BUTTON", "Log Spacing",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 400, yPos, 100, 15,
        hwnd, (HMENU) LTEXT_F_LOG, hInstance, NULL);
    EnableWindow(tmp, active);
  } else if(param == SWEEP_A) {
    tmp = CreateWindow("BUTTON", "Log Spacing",
        WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 400, yPos, 100, 15,
        hwnd, (HMENU) LTEXT_A_LOG, hInstance, NULL);
    EnableWindow(tmp, active);
  }
}


void createCustomXYControls(HINSTANCE hInstance, int yPos, BOOL active)
{
  uintptr_t controlIDs[CONTROLS_PER_PARAM];
  getControlIDs(controlIDs, SWEEP_CUSTOM);

  uintptr_t ltext_hdg    = controlIDs[0]; uintptr_t ltext_wait   = controlIDs[1];
  uintptr_t ltext_xvolts = controlIDs[2]; uintptr_t ltext_yvolts = controlIDs[3];
  uintptr_t btn_confirm  = controlIDs[4]; uintptr_t btn_delete   = controlIDs[5];
  uintptr_t ltext_order  = controlIDs[6]; uintptr_t ltext_bidir  = controlIDs[7];
  std::string paramStr ="Parameters for custom X,Y sweep:";

  HWND tmp;
  HWND tmpEdit;

  CreateWindow("STATIC", "A frame to surround the parameter controls",
      WS_VISIBLE | WS_CHILD | SS_ETCHEDFRAME, 5, yPos-5, 730, 55, hwnd,
      (HMENU) -1, hInstance, NULL);

  tmp = CreateWindow("STATIC", paramStr.data(), WS_VISIBLE | WS_CHILD,
      10, yPos, 150, 15, hwnd, (HMENU) ltext_hdg, hInstance, NULL);
  EnableWindow(tmp, active);
  
  tmp = CreateWindow("BUTTON", "Bi-dir", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            520, yPos, 70, 15,
      hwnd, (HMENU) LTEXT_C_BIDIR, hInstance, NULL);
  EnableWindow(tmp, active);
  
  CreateWindow("STATIC", "Wait (ms):", WS_VISIBLE | WS_CHILD,
      400, yPos, 35, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP ,
      440, yPos, 50, 15, hwnd, (HMENU) ltext_wait, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Order:", WS_VISIBLE | WS_CHILD,
      620, yPos, 50, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP ,
      675, yPos, 50, 15, hwnd, (HMENU) ltext_order, hInstance, NULL);
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Step #:", WS_VISIBLE | WS_CHILD,
      10, yPos + 25, 50, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD,
      65, yPos+25, 50, 15, hwnd, (HMENU) LTEXT_C_STEP, hInstance, NULL);
  EnableWindow(tmp, false);

  CreateWindow("STATIC", "X voltage:", WS_VISIBLE | WS_CHILD,
      135, yPos + 25,  65, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      205, yPos+25, 50, 15, hwnd, (HMENU) ltext_xvolts, hInstance, NULL);
//  tmp = CreateWindowEX(WS_EX_CLIENTEDGE, "UPDOWN_CLASS", NULL,
//      WS_VISIBLE | WS_CHILD | WS_TABSTOP | UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
//      205, yPos+25, 50, 15, hwnd, (HMENU) ltext_xvolts, hInstance, NULL);
//  SendMessage(tmp, UDM_SETRANGE, 0, MAKELPARAM((WORD) -10, (WORD) 10));
  EnableWindow(tmp, active);

  CreateWindow("STATIC", "Y voltage:", WS_VISIBLE | WS_CHILD,
      275, yPos + 25,  75, 15, hwnd, (HMENU) -1, hInstance, NULL);
  tmp = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
      355, yPos+25, 50, 15, hwnd, (HMENU) ltext_yvolts, hInstance, NULL);
//  tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "UPDOWN_CLASS", NULL,
//      WS_VISIBLE | WS_CHILD | WS_TABSTOP| UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
//      355, yPos+25, 50, 15, hwnd, (HMENU) ltext_yvolts, hInstance, NULL);
  EnableWindow(tmp, active);

  tmp = CreateWindow("BUTTON", "Confirm Point", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
      425, yPos+25, 100, 20, hwnd, (HMENU) btn_confirm, hInstance, NULL);
  EnableWindow(tmp, active);

  tmp = CreateWindow("BUTTON", "Delete Point", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
      535, yPos+25, 100, 20, hwnd, (HMENU) btn_delete, hInstance, NULL);
  EnableWindow(tmp, active);
}


void getControlIDs(uintptr_t* IDs, int param)
{
  switch(param) {
    case SWEEP_X: case MI_SWEEP_OVER_X:
      IDs[0] = LTEXT_X_PARAMS; IDs[1] = LTEXT_X_START;
      IDs[2] = LTEXT_X_END   ; IDs[3] = LTEXT_X_STEPS;
      IDs[4] = LTEXT_X_REPEAT; IDs[5] = LTEXT_X_WAIT ;
      IDs[6] = LTEXT_X_ORDER ; IDs[7] = LTEXT_X_BIDIR;
      break;
    case SWEEP_Y: case MI_SWEEP_OVER_Y:
      IDs[0] = LTEXT_Y_PARAMS; IDs[1] = LTEXT_Y_START;
      IDs[2] = LTEXT_Y_END   ; IDs[3] = LTEXT_Y_STEPS;
      IDs[4] = LTEXT_Y_REPEAT; IDs[5] = LTEXT_Y_WAIT ;
      IDs[6] = LTEXT_Y_ORDER ; IDs[7] = LTEXT_Y_BIDIR;
      break;
    case SWEEP_F: case MI_SWEEP_OVER_F:
      IDs[0] = LTEXT_F_PARAMS; IDs[1] = LTEXT_F_START;
      IDs[2] = LTEXT_F_END   ; IDs[3] = LTEXT_F_STEPS;
      IDs[4] = LTEXT_F_REPEAT; IDs[5] = LTEXT_F_WAIT ;
      IDs[6] = LTEXT_F_ORDER ; IDs[7] = LTEXT_F_BIDIR;
      break;
    case SWEEP_A: case MI_SWEEP_OVER_A:
      IDs[0] = LTEXT_A_PARAMS; IDs[1] = LTEXT_A_START;
      IDs[2] = LTEXT_A_END   ; IDs[3] = LTEXT_A_STEPS;
      IDs[4] = LTEXT_A_REPEAT; IDs[5] = LTEXT_A_WAIT ;
      IDs[6] = LTEXT_A_ORDER ; IDs[7] = LTEXT_A_BIDIR;
      break;
    case SWEEP_CUSTOM: case MI_CUSTOM_BLANK: case MI_CUSTOM_XY:
      IDs[0] = LTEXT_C_PARAMS; IDs[1] = LTEXT_C_WAIT  ;
      IDs[2] = LTEXT_C_XVOLTS; IDs[3] = LTEXT_C_YVOLTS;
      IDs[4] = BTN_C_ADD     ; IDs[5] = BTN_C_DELETE  ;
      IDs[6] = LTEXT_C_ORDER ; IDs[7] = LTEXT_C_BIDIR;
      break;
  }
}


void updateActiveParams(int changedParam)
{
  int idx = -1;
  HMENU menu = GetMenu(hwnd);
  
  switch(changedParam) {
    case MI_SWEEP_OVER_X:
      idx = SWEEP_X;
      break;
    case MI_SWEEP_OVER_Y:
      idx = SWEEP_Y;
      break;
    case MI_SWEEP_OVER_A:
      idx = SWEEP_A;
      break;
    case MI_SWEEP_OVER_F:
      idx = SWEEP_F;
      break;
    case MI_CUSTOM_BLANK: case MI_CUSTOM_XY: case MI_CUSTOM_FROMFILE:
      idx = SWEEP_CUSTOM;
      break;
    case MI_AUTO_SENS:
      sweepSetup.autoSens = !sweepSetup.autoSens;
      if(sweepSetup.autoSens) {
        CheckMenuItem(menu, changedParam, MF_CHECKED);
      } else {
        CheckMenuItem(menu, changedParam, MF_UNCHECKED);
      }
      return;
    case MI_AVERAGING:
      sweepSetup.autoSens = !sweepSetup.autoSens;
      if(sweepSetup.autoSens) {
        CheckMenuItem(menu, changedParam, MF_CHECKED);
      } else {
        CheckMenuItem(menu, changedParam, MF_UNCHECKED);
      }
      return;
  }

  if(activeParams[idx]) {
    logger << "Active Parameter; de-activating" << std::endl;
    activeParams[idx] = FALSE;
    numActiveParams--;
    if(idx == SWEEP_CUSTOM) {
      EnableMenuItem(menu, MI_CUSTOM_DISABLE, MF_DISABLED);
    } else {
      CheckMenuItem(menu, changedParam, MF_UNCHECKED);
    }
  } else {
    logger << "Inactive Parameter; activating" << std::endl;
    activeParams[idx] = TRUE;
    numActiveParams++;
    if(idx == SWEEP_CUSTOM) {
      EnableMenuItem(menu, MI_CUSTOM_DISABLE, MF_ENABLED);
    } else {
      CheckMenuItem(menu, changedParam, MF_CHECKED);
    }
  }

  uintptr_t controlIDs[CONTROLS_PER_PARAM];
  getControlIDs(controlIDs, idx);
  for(int i = 0; i < CONTROLS_PER_PARAM; i++) {
    HWND ctrl = GetDlgItem(hwnd, controlIDs[i]);
    EnableWindow(ctrl, activeParams[idx]);
  }
  if(sweepSetup.autoTimeConst) {
    HWND ctrl = GetDlgItem(hwnd, LTEXT_F_WAIT);
    EnableWindow(ctrl, FALSE);
  }
  
  if(idx == SWEEP_F) {
    HWND ctrl = GetDlgItem(hwnd, LTEXT_F_AUTOTC);
    EnableWindow(ctrl, activeParams[idx]);
    ctrl = GetDlgItem(hwnd, LTEXT_F_LOG);
    EnableWindow(ctrl, activeParams[idx]);
  }
  else if(idx == SWEEP_A) {
    HWND ctrl = GetDlgItem(hwnd, LTEXT_A_LOG);
    EnableWindow(ctrl, activeParams[idx]);
  }

  EnableWindow(GetDlgItem(hwnd, BTN_RUN_SWEEP),numActiveParams > 0);

  //TODO: update sweep overview graphics
}


bool validateSweepParams(bool runSweep)
{
  HWND tree = GetDlgItem(hwnd, TVIEW_SWEEP_ORDER);
  SendMessage(tree, TVM_DELETEITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)TVI_ROOT);
  HWND lblSummary = GetDlgItem(hwnd, LTEXT_SUMMARY);
  SetWindowText(lblSummary,
      (connReady)?"GPIB Ready\r\n":"GPIB Disconnected!\r\n");
  EnableWindow(GetDlgItem(hwnd, BTN_RUN_SWEEP), FALSE);
  int order = -1;

  sweepSetup.maxRecursionLevel = numActiveParams - 1;
  for(int i = 0; i < NUM_AVAIL_PARAMS; i++) {
    sweepSetup.parameters[i] = -1;
  }

  uintptr_t controlIDs[CONTROLS_PER_PARAM];
  STRCONV_ERROR res = CONV_SUCCESS;
  HWND ctrl;

  for(int i = 0; i < NUM_AVAIL_PARAMS; i++) {
    char content[80];
    if(i == SWEEP_CUSTOM && activeParams[i]) {
      logger << "validating; custom sweep is active";
      ctrl = GetDlgItem(hwnd, LTEXT_C_ORDER);
      GetWindowText(ctrl, content, 80);
      res = str2int(order, content);
      if(order >= 1 && order <= numActiveParams && res == CONV_SUCCESS) {
        if(sweepSetup.parameters[order - 1] != -1) {
          return false;
        } else {
          sweepSetup.parameters[order - 1] = i;
        }
      } else {
        return false;
      }
      ctrl = GetDlgItem(hwnd, LTEXT_C_WAIT);
      GetWindowText(ctrl, content, 80);
      res = str2long(sweepSetup.waits[i], content);
      if(sweepSetup.waits[i] < 1) {
        return false;
      }
      sweepSetup.repeats[i] = 1;
    }
    else if(activeParams[i]) { //Only validate parameters which are set as active
      getControlIDs(controlIDs, i);

      // Check that the input to "Order" is valid
      ctrl = GetDlgItem(hwnd, controlIDs[6]);
      GetWindowText(ctrl, content, 80);
      res = str2int(order, content);
      if(order < 1 || order > numActiveParams || res != CONV_SUCCESS) {
        //Given order is out of range; input is invalid
        return false;
      }
      else {
        if(sweepSetup.parameters[order - 1] != -1) {
          //The given order is a duplicate; input is invalid
          return false;
        } else {
          sweepSetup.parameters[order - 1] = i;
        }
      }

      // Check that all the other inputs are valid
      for(int j = 1; j < CONTROLS_PER_PARAM - 1; j++) {
        ctrl = GetDlgItem(hwnd, controlIDs[j]);
        GetWindowText(ctrl, content, 80);
        switch(j) {
          case 1: {
            // Check the "start" field
            res = str2dbl(sweepSetup.starts[i], content);
            break;
          }
          case 2: {
            // Check the "end" field
            res = str2dbl(sweepSetup.ends[i], content);
            break;
          }
          case 3: {
            // Check the "steps" field
            res = str2int(sweepSetup.steps[i], content);
            if(sweepSetup.steps[i] < 1) {
              //Must have at least one step in the sweep;
              //this is an invalid value
              return false;
            }
            break;
          }
          case 4: {
            // Check the "repeats" field
            res = str2int(sweepSetup.repeats[i], content);
            if(sweepSetup.repeats[i] < 1) {
              //Must perform the sweep at least once; this
              //is an invalid value
              return false;
            }
            break;
          }
          case 5: {
            if(i == SWEEP_F && sweepSetup.autoTimeConst) {
              res = CONV_SUCCESS;
            }
            else {
              res = str2long(sweepSetup.waits[i], content);
              if(sweepSetup.waits[i] < 1) {
                //Wait time cannot be less than 0; this is
                //invalid value
                return false;
              }
            }
            break;
          }
        }
        if(res != CONV_SUCCESS) { // Found an invalid value
          return false;
        }
        else {
          if(i == SWEEP_X || i == SWEEP_Y) {
            if(sweepSetup.starts[i] > VOLTAGE_MAX) {
              sweepSetup.starts[i] = VOLTAGE_MAX;
            } else if(sweepSetup.starts[i] < VOLTAGE_MIN) {
              sweepSetup.starts[i] = VOLTAGE_MIN;
            }
            if(sweepSetup.ends[i] > VOLTAGE_MAX) {
              sweepSetup.ends[i] = VOLTAGE_MAX;
            } else if(sweepSetup.ends[i] < VOLTAGE_MIN) {
              sweepSetup.ends[i] = VOLTAGE_MIN;
            }
          }
        }
      }
    }
  }

  EnableWindow(GetDlgItem(hwnd,BTN_RUN_SWEEP), TRUE);

  populateTree(tree);

  long dur = calculateSweepDuration();
  int hours =  dur / (3600000);
  int minutes = (dur % 3600000) / 60000;
  double seconds = (dur % 60000) / (1000.0);
  std::ostringstream desc;
  desc << ((connReady)?"GPIB Ready\r\n":"GPIB Disconnected!\r\n");
  desc << "Estimated Duration:\r\n";
  desc << hours << "hr " << minutes << "min " << seconds << "s";

  if(runSweep && connReady && !settingCustomSweep) {
    LPDWORD threadId = NULL;
    cancelSweep = false;
    ResetEvent(cancelSweepEvent);
    bgThreadHandle = CreateThread(NULL, 0, SweepThreadFunction,
        NULL, 0, threadId);
    logger << bgThreadHandle << std::endl;
//    WaitForSingleObject(bgThreadHandle, INFINITE);
  }
  if(!cancelSweep) {
    desc << "\r\n" << "Sweeping...";
  }
  char charDesc[80];
  const char* s = desc.str().c_str();
  strcpy(charDesc, s);
  SetWindowText(lblSummary, s);
  EnableWindow(GetDlgItem(hwnd, BTN_RUN_SWEEP), TRUE);

  return true;
}


bool enableCustomSweep(bool useXY)
{
  if(useXY) {
    if(activeParams[SWEEP_X] && activeParams[SWEEP_Y] && validateSweepParams(false)) {
      numCustom = (sweepSetup.steps[SWEEP_X]+1)*(sweepSetup.steps[SWEEP_Y]+1)
          *(sweepSetup.repeats[SWEEP_X])*(sweepSetup.repeats[SWEEP_Y]);
      customX = new double[numCustom];
      customY = new double[numCustom];
      double xInterval = (sweepSetup.ends[SWEEP_X] - sweepSetup.starts[SWEEP_X])/sweepSetup.steps[SWEEP_X];
      double yInterval = (sweepSetup.ends[SWEEP_Y] - sweepSetup.starts[SWEEP_Y])/sweepSetup.steps[SWEEP_Y];
      int count = 0;
      for(int nx = 0; nx<sweepSetup.repeats[SWEEP_X]; nx++) {
        for(int ix = 0; ix<sweepSetup.steps[SWEEP_X]+1; ix++) {
          for(int ny = 0; ny<sweepSetup.repeats[SWEEP_Y]; ny++) {
            for(int iy = 0; iy<sweepSetup.steps[SWEEP_Y]+1; iy++) {
              customX[count] = sweepSetup.starts[SWEEP_X] + ix*xInterval;
              customY[count] = sweepSetup.starts[SWEEP_Y] + iy*yInterval;
              count++;
            }
          }
        }
      }
    } else {
      numCustom = -1;
      settingCustomSweep = FALSE;
      return false;
    }
  } else {
    customX = new double[numCustom];
    customY = new double[numCustom];
    for(int i = 0; i < numCustom; i++) {
      customX[i] = 0;
      customY[i] = 0;
    }
  }
  
  settingCustomSweep = true;
  currCustomStep = 0;
  
  if(activeParams[SWEEP_X]){
    updateActiveParams(MI_SWEEP_OVER_X);
  }
  if(activeParams[SWEEP_Y]) {
    updateActiveParams(MI_SWEEP_OVER_Y);
  }
  if(!activeParams[SWEEP_CUSTOM]){
    updateActiveParams(MI_CUSTOM_BLANK);
  }
  
  updateCustomXYText();
  return true;
}


DWORD WINAPI SweepThreadFunction( LPVOID lpParam )
{
  logger <<"Sweep called; lockin="<<lockin->address<< std::endl;
  int retVal = sweep();
  validateSweepParams(false);
  return retVal;
}


double getSensValue(int i)
{
  int m = i % 3;
  return (m*m + 2*m + 2) * pow(10.0, i/3 - 9.0);
}


double getTimeConstValue(int i)
{
  return (2*(i%2) + 1) * pow(10.0, i/2 - 5.0);
}


int getTimeConst(double reqTau)
{
  double logTau = log10(reqTau);
  double iPart = floor(logTau);
  //logger << "   getTimeConst: reqTau is " << reqTau <<std::endl;
  //logger << "   getTimeConst: logTau is " << logTau <<std::endl;
  //logger << "   getTimeConst: iPart is " << iPart <<std::endl;
  //logger << "   getTimeConst: rT / pow() is " << (reqTau / pow(10.0, iPart)) <<std::endl;
  
  return (2*iPart) + (((reqTau / pow(10.0, iPart)) > 3)?12:11);
}


double getFiltSlopeValue(int i)
{
  return -6.0 *(i+1);
}


double getWaitFactor(int filtSlope)
{
  switch(filtSlope) {
    case 0: return 5.0;
    case 1: return 7.0;
    case 2: return 9.0;
    case 3: return 10.0;
  }
}


bool outOfRange(double ampl, int sens)
{
  double fullScale = getSensValue(sens);
  double relAmpl   = ampl / fullScale;
  return (relAmpl < 0.0016) || (relAmpl > 1);
}


int getBestSens(double ampl, int currSens)
{
  // Calculate the minimum sensitivity we should jump to, which should not be smaller
  // than the resolution at the current sensitivity
  double currSensVal = getSensValue(currSens);
  double resolution = currSensVal / pow(2, 12);
  int minSens = currSens;
  for(int sens = currSens - 1; sens >= 0; sens--) {
    if(getSensValue(sens) < resolution) {
      break;
    }
    else {
      minSens = sens;
    }
  }
  double minVal = getSensValue(minSens);

  // Select the appropriate sensitivity
  if(ampl < 0.9 * minVal) {
    return minSens;
  }
  else if(ampl >= 0.9) {
    return 26;
  }
  else {
    double logAmp = log10(ampl);
    double base   = floor(logAmp);
    double frac   = logAmp - base;
    
    int decade = 3 * ( (int) base + 9);
    int offset;
    if(frac <= 0.25527) {
      offset = 0;
    } else if(frac <= 0.63521) {
      offset = 1;
    } else if(frac <= 0.95424) {
      offset = 2;
    } else {
      offset = 3;
    }
    logger << "getBestSens: for voltage " << ampl << ", best sensitivity is " << getSensValue(decade+offset) << std::endl;
    return decade + offset;
  }
}


void sweepInitOutput(std::ofstream& outps)
{
  // Parse output filename
  std::string ofilename(szFileName);
  int idx = ofilename.rfind(".txt");
  
  // Open the output file where results will be stored
  std::string outputFileName = ofilename.substr(0, idx);
  outputFileName.append(".txt");
  outps.open(outputFileName.c_str());
  
  // Modify the file name appropriately for the settings files, and
  // open the settings file. 
  std::string settingsFileName = ofilename.substr(0, idx);
  settingsFileName.append("_settings.txt");
  LockinSettings::settingsLogger = new std::ofstream(settingsFileName.c_str());
  
  // Write the lockin device description and current settings to the settings file
  (*LockinSettings::settingsLogger) << "SOFTWARE VERSION: " << FVXY_version << std::endl;
  (*LockinSettings::settingsLogger) << "LOCK-IN VERSION : " << lockin->get_device_description() << std::endl;
  lockin->settings.queryAllOptions(lockin->address);
  filterType = lockin->settings.get("Low Pass Filter Slope").intVal1;
  lockin->settings.writeAllOptions(&outps);
  
  // Write header to output file
  outps << std::endl;
  for(int i = 0; i < NUM_AVAIL_PARAMS; i++) {
    if(sweepSetup.parameters[i] == SWEEP_CUSTOM) {
      outps << "X\tY\t";
    } else if(sweepSetup.parameters[i] == SWEEP_F) {
      outps << "Frequency\t";
    } else if(sweepSetup.parameters[i] == SWEEP_A) {
      outps << "Amplitude\t";
    } else if(sweepSetup.parameters[i] == SWEEP_X) {
      outps << "X\t";
    } else if(sweepSetup.parameters[i] == SWEEP_Y) {
      outps << "Y\t";
    }
  }
  if(averaging) {
    outps << "R\tTheta\tStdDev\t" << std::endl;
  } else {
    outps << "R\tTheta\t" << std::endl;
  }
  
  // Log the start time of the sweep
  time_t now = time(0);
  struct tm* tmNow = localtime(&now);
  (*LockinSettings::settingsLogger) << std::endl << "=== SWEEP STARTED at " 
      << (1900 + tmNow->tm_year) << "-" << (1 + tmNow->tm_mon) << "-"
      << tmNow->tm_mday << " " << tmNow->tm_hour << ":" << tmNow->tm_min
      << ":" << tmNow->tm_sec << " ===" << std::endl;
}


void sweepFinalizeOutput(std::ofstream& outps)
{
  // Log the finish time of the sweep
  time_t now = time(0);
  struct tm* tmNow = localtime(&now);
  (*LockinSettings::settingsLogger) << std::endl << "=== SWEEP ENDED at " 
      << (1900 + tmNow->tm_year) << "-" << (1 + tmNow->tm_mon) << "-"
      << tmNow->tm_mday << " " << tmNow->tm_hour << ":" << tmNow->tm_min
      << ":" << tmNow->tm_sec << " ===" << std::endl;
  
  // Flush and close output file
  outps.flush();
  outps.close();
  
  // Flush and close logger
//  LockinSettings::settingsLogger->flush();
//  LockinSettings::settingsLogger->close();
//  delete LockinSettings::settingsLogger;
}


int sweepRepeatLoop(int recursionLevel, std::string prefix, std::ofstream& outps)
{
  if(cancelSweep) {
    logCanceledSweep();
    return 0;
  }
  
  // Identify the parameter being swept at the current level
  int currParam = sweepSetup.parameters[recursionLevel];
  
  // Calculate the number of steps, repeats, and wait times for the current parameter
  int currSteps;
  if(currParam == SWEEP_CUSTOM){
    currSteps = numCustom - 1;
  } else {
    currSteps = sweepSetup.steps[currParam];
  }
  int currRepeats = sweepSetup.repeats[currParam];
  long currWait = sweepSetup.waits[currParam];
  
  // Calculate all the parameter values
  double *currValues = new double[currSteps + 1];
  getCurrentValues(currParam, currValues);
  
  int exitVal = 1;
  int initialSens = 0;
  
  // Loop over _repeats_
  for(int r = 0; r < currRepeats; r++) {
    // Preliminaries: log the sweep level, check if the sweep has been canceled
    (*LockinSettings::settingsLogger) << "At level " << recursionLevel
        << " of sweep; starting loop #" << r << std::endl;
    if(cancelSweep) {
      logCanceledSweep();
      return 0;
    }
    
    // Set a pre-determined sensitivty if:
    //   1. auto-sensitivity is enabled
    //   2. this is the innermost parameter (where the measurements are made)
    //   3. this is a repeat of the sweep over the innermost parameter
    if(
      sweepSetup.autoSens
      && recursionLevel == sweepSetup.maxRecursionLevel
      && r > 0
    ) {
      (*LockinSettings::settingsLogger) << "Setting initial sensitivity; r = " << r << std::endl;
      lockin->set_sensitivity(initialSens);
    }
    
    // Do forward parameter sweep (currStart -> currEnd)
    int i = 0;
    exitVal = sweepParameterLoop(
      recursionLevel,
      currValues,
      currSteps + 1,
      currWait,
      false,
      r,
      i,
      initialSens,
      prefix,
      outps
    );
    (*LockinSettings::settingsLogger) << "Forward sweep finished; exitVal = " << exitVal << "; i = " << i << std::endl;
    if(exitVal == 0) {
      // Ramp back down to 0 if the sweep is canceled
      rampDown(recursionLevel, currParam, currValues, i);
      return 0;
    }
    
    // Do reverse parameter sweep, if enabled
    if(sweepSetup.bidirectional[currParam]) {
      exitVal = sweepParameterLoop(
        recursionLevel,
        currValues,
        currSteps + 1,
        currWait,
        true,
        r,
        i,
        initialSens,
        prefix,
        outps
      );
      if(exitVal == 0) {
        // Ramp back down to 0 if the sweep is canceled
        rampDown(recursionLevel, currParam, currValues, i);
        return 0;
      }
    }
    else {
      // If not a directional sweep, perform the appropriate ramp down
      i--;
      rampDown(recursionLevel, currParam, currValues, i);
    }
  }
  
  delete[] currValues;
  return 1;
}


void getCurrentValues(int currParam, double * currValues)
{
  double currStart, currEnd;
  if(sweepSetup.logSpacing[currParam]) {
    currStart = log10(sweepSetup.starts[currParam]);
    currEnd   = log10(sweepSetup.ends[currParam]);  
  } else {
    currStart = sweepSetup.starts[currParam];
    currEnd   = sweepSetup.ends[currParam];
  }
  
  int currSteps;
  if(currParam == SWEEP_CUSTOM){
    currSteps = numCustom - 1;
  } else {
    currSteps = sweepSetup.steps[currParam];
  }
  double stepSize = (currEnd - currStart) / currSteps;
  
  for(int i = 0; i < currSteps + 1; i++) {
    double val = currStart + i * stepSize;
    if(sweepSetup.logSpacing[currParam])
      currValues[i] = pow(10, val);
    else
      currValues[i] = val;
  }
}


void rampDown(int recursionLevel, int currParam, double * allSteps, int& i)
{
    (*LockinSettings::settingsLogger) << "At level " << recursionLevel
            << " of sweep; ramping down to initial value." << std::endl;
    if(currParam == SWEEP_CUSTOM) {
        if(rampType == 0) {
            sendCommandToLockin(SWEEP_X, customX[0]);
            sendCommandToLockin(SWEEP_Y, customY[0]);
        } else {
            for(--i; i >= 0; i--) {
                sendCommandToLockin(SWEEP_X, customX[i]);
                sendCommandToLockin(SWEEP_Y, customY[i]);
            }
        }
    }
    else {
        if(rampType == 0) {
            sendCommandToLockin(currParam, allSteps[0]);
        }
        else if(rampType == 1) {
            for( --i; i >= 0; i--) {
                sendCommandToLockin(currParam, allSteps[i]);
            }
        }
        else if(rampType == 2) {
            double endDecade = log10(allSteps[i]);
            double startDecade = log10(sweepSetup.starts[currParam]);
            double decades = abs(endDecade - startDecade);
            int steps = (int) ceil(STEPS_PER_RAMP_DECADE * decades);
            double rampStep = (endDecade - startDecade) / steps;
            for(int m = steps - 1; m >= 0; m--) {
                double currVal = pow(10, startDecade + rampStep*m);
                sendCommandToLockin(currParam, currVal);
            }
        }
    }
    i = 0;
}


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
)
{
  int currParam = sweepSetup.parameters[recursionLevel];
  int waitTime = currWait;
  int exitVal = 1;
  
  // Loop over parameter values
  for(i = 0; i < nValues; i++) {
    if(cancelSweep) {
      logCanceledSweep();
      return 0;
    }
    
    // Command the lockin to set the current parameter to the current value
    int currIndex;
    if(reversed)
      currIndex = nValues - i - 1;
    else
      currIndex = i;
    
    double currVal = 1;
    if(currParam == SWEEP_CUSTOM) {
      sendCommandToLockin(SWEEP_X, customX[currIndex]);
      sendCommandToLockin(SWEEP_Y, customY[currIndex]);
    } else {
      currVal = currValues[currIndex];
      sendCommandToLockin(currParam, currVal);
    }

    // AUTOMATICALLY SET THE TIME CONSTANT IF APPLICABLE
    if(currParam == SWEEP_F && sweepSetup.autoTimeConst) {
      if(sweepSetAutoTimeConst(currVal, &waitTime) == 0)
        return 0;
    }
    
    // Wait before proceeding to the measurement step
    // Note: must wait an additional amount of time on the first step of a sweep
    if(
      WaitForSingleObject(
        cancelSweepEvent, waitTime + ((i == 0) ? FIRST_STEP_WAIT : 0)
      ) == WAIT_OBJECT_0
    ) {
      logCanceledSweep();
      return 0;
    }

    // TAKE MEASUREMENT OR PROCEED TO NEXT SWEEP LEVEL
    if(recursionLevel < sweepSetup.maxRecursionLevel) {
      std::ostringstream prefixTmp;
      if(currParam == SWEEP_CUSTOM) {
        prefixTmp << prefix << customX[i] << "\t" << customY[i] << "\t";
      } else {
        prefixTmp << prefix << currVal << "\t";
      }
      exitVal = sweepRepeatLoop(recursionLevel + 1, prefixTmp.str(), outps);
      if(exitVal == 0)
        return 0;
    }
    else {
      double ampl = 0;
      double phs = 0;
      double stdDev = 0;
      
      if(
        sweepDoMeasurement(
          &ampl, &phs, waitTime, repeatNum, i, initialSens, prefix, currVal
        ) == 0
      )
        return 0;
      if(averaging) {
        sweepDoAveraging(ampl, phs, &ampl, &phs, &stdDev);
      }
      
      if(currParam == SWEEP_CUSTOM) {
        outps << prefix << customX[i] << '\t' << customY[i];
      } else {
        outps << prefix << currVal;
      }
      sweepWriteMeasurement(outps, prefix, ampl, phs, stdDev);
      if(i % 10 == 0) {
        outps.flush();
      }
    }
  }
    return 1;
}


int sweepDoMeasurement(
    double *ampl,
    double *phs,
    int waitTime,
    int r,
    int i,
    int& initialSens,
    std::string prefix,
    double currVal
)
{
    lockin->get_AmplPhase(*ampl, *phs);
    int ct = 0;
    int currSens = lockin->get_sensitivity();
    while(
        sweepSetup.autoSens
        && outOfRange(*ampl, currSens)
        && ct < 10
    ) {
        int sens = getBestSens(*ampl, currSens);
        lockin->set_sensitivity(sens);
        if(WaitForSingleObject(cancelSweepEvent, waitTime) == WAIT_OBJECT_0) {
            logCanceledSweep();
            return 0;
        }
        lockin->get_AmplPhase(*ampl, *phs);
        currSens = lockin->get_sensitivity();
        ct++;
    }
    
    if(ct == 10 && outOfRange(*ampl, lockin->get_sensitivity())) {
        // Sensitivity is invalid
        
        (*LockinSettings::settingsLogger) << "Unable to determine appropriate sensitivity for "
                << prefix << currVal << std::endl;
    } else {
        // Sensitivity is valid
        
        if(r == 0 && i == 0) {
            // Store this sensitivity so we can set it as a start value on
            // subsequent repeats of this parameter
            initialSens = lockin->get_sensitivity();
        }
    }
    return 1;
}


double getTauReq(double freq, int filtSlope)
{
  double tau_2f = pow(2, ATTEN_2F / getFiltSlopeValue(filtSlope)) / (4 * M_PI * freq);
  double tau_1f = pow(
    2,
    (sweepSetup.signalToDC + ATTEN_2F) / getFiltSlopeValue(filtSlope)
  ) / (2 * M_PI * freq);
  double tau_req;
  if(sweepSetup.ac_couple) {
    // ac-coupled input
    return tau_2f;
  }
  else {
    return fmax(tau_1f, tau_2f);
  }
}


int sweepSetAutoTimeConst(double currFreq, int *waitTime)
{
  int filtSlope = lockin->settings.get("Low Pass Filter Slope").intVal1;
  double tau_req = getTauReq(currFreq, filtSlope);
  int currTimeConst = lockin->settings.get("Time Constant").intVal1;
  int newTimeConst = getTimeConst(tau_req);
  
  if(newTimeConst != currTimeConst) {
    // The current setting for the time constant is not correct. Update
    // the lockin settings accordingly.
    lockin->settings.set(lockin->address, "Time Constant", newTimeConst);
    if(WaitForSingleObject(cancelSweepEvent, FIRST_STEP_WAIT) == WAIT_OBJECT_0) {
        logCanceledSweep();
        return 0;
    }
  }
  *waitTime = getWaitFactor(filtSlope)*getTimeConstValue(newTimeConst)*1000;
  return 1;
}

void sweepWriteMeasurement(
    std::ofstream& outps, std::string prefix, double ampl, double phs, double stdDev
)
{
    // WRITE MEASURED VALUE TO OUTPUT STREAM
    outps << '\t' << ampl;

    if(lockin->isPhaseAccessible()) {
        outps << '\t' << phs;
    }

    if(averaging) {
        outps << '\t' << stdDev;
    }
    
    outps<<std::endl;
}


void sweepDoAveraging(
  double ampl0, double phs0, double *ampl, double *phs, double *stdDev
)
{
  double* re = new double[numAvgPts];
  double* im = new double[numAvgPts];
  
  // Store the existing measurement
  re[0] = ampl0*cos(M_PI*phs0/180);
  im[0] = ampl0*sin(M_PI*phs0/180);

  // Take repeated measurements, convert to complex numbers and store,
  // and update the sum.
  double reTot = re[0];
  double imTot = im[0];
  double ampl_tmp, phs_tmp;
  for(int m = 1; m < numAvgPts; m++) {
    lockin->get_AmplPhase(ampl_tmp, phs_tmp);
    re[m] = ampl_tmp * cos(M_PI * phs_tmp / 180);
    im[m] = ampl_tmp * sin(M_PI * phs_tmp / 180);
    reTot += re[m];
    imTot += im[m];
  }
  
  // Compute the average: (sum)/(count)
  reTot /= numAvgPts;
  imTot /= numAvgPts;
  
  // Compute the variance: E[ (x-u)(x-u)* ]
  double varianceTot = 0;
  for(int m = 0; m < numAvgPts; m++) {
    double R = re[m] - reTot;
    double I = im[m] - imTot;
    varianceTot += R*R + I*I;
  }
  varianceTot /= numAvgPts;
  
  // Compute the standard deviation: the square root of the variance
  *stdDev = sqrt(varianceTot);
  
  // Convert the average value back to (amplitude-phase) form
  *ampl = sqrt(reTot*reTot + imTot*imTot);
  *phs  = 180 * atan2(imTot, reTot) / M_PI;
  
  // Clean up the dynamically allocated array.
  delete[] re;
  delete[] im;
}


LRESULT CALLBACK SignalDCDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  HWND ctrl = GetDlgItem(hwnd, LTEXT_SIGNAL_DC);
  char content[80];
  switch(Message) {
    case WM_INITDIALOG: {
      if(sweepSetup.ac_couple) {
        CheckDlgButton(hwnd, BTN_AC_COUPLE, BST_CHECKED);
        CheckDlgButton(hwnd, BTN_DC_COUPLE, BST_UNCHECKED);
        EnableWindow(GetDlgItem(hwnd, LTEXT_SIGNAL_DC), FALSE);
      }
      else {
        CheckDlgButton(hwnd, BTN_AC_COUPLE, BST_UNCHECKED);
        CheckDlgButton(hwnd, BTN_DC_COUPLE, BST_CHECKED);
        EnableWindow(GetDlgItem(hwnd, LTEXT_SIGNAL_DC), TRUE);
      }
      snprintf(content, 80, "%f", sweepSetup.signalToDC);
      SetWindowText(ctrl, content);
      return TRUE;
    }
    case WM_COMMAND: {
      switch(LOWORD(wParam)) {
        case BTN_AC_COUPLE: {
          CheckDlgButton(hwnd, BTN_AC_COUPLE, BST_CHECKED);
          CheckDlgButton(hwnd, BTN_DC_COUPLE, BST_UNCHECKED);
          EnableWindow(GetDlgItem(hwnd, LTEXT_SIGNAL_DC), FALSE);
          break;
        }
        case BTN_DC_COUPLE: {
          CheckDlgButton(hwnd, BTN_AC_COUPLE, BST_UNCHECKED);
          CheckDlgButton(hwnd, BTN_DC_COUPLE, BST_CHECKED);
          EnableWindow(GetDlgItem(hwnd, LTEXT_SIGNAL_DC), TRUE);
          break;
        }
        case IDOK: {
          int ac = IsDlgButtonChecked(hwnd, BTN_AC_COUPLE);
          if(ac) {
            sweepSetup.ac_couple = true;
          }
          else {
            sweepSetup.ac_couple = false;
            GetWindowText(ctrl, content, 80);
            STRCONV_ERROR res = str2dbl(sweepSetup.signalToDC, content);
            if(res == CONV_SUCCESS){
              EndDialog(hwnd, IDOK);
              return TRUE;
            }
            else {
              return FALSE;
            }
          }
        }
        case IDCANCEL: {
          EndDialog(hwnd, IDCANCEL);
          return TRUE;
        }
        default: return FALSE;
      }
    }
    default: return FALSE;
  }
}


int sweep()
{
  // Initialize output
  std::ofstream outps;
  sweepInitOutput(outps);
  std::string prefix;

  // Store initial settings so they can be reset
  int initialSens0 = lockin->get_sensitivity();
  int initialTC0 = lockin->get_time_constant();
  int initialCoupling = lockin->settings.get("Input Coupling").intVal1;

  // Setup lockin
  lockin->set_harmonic(sweepSetup.detHarm);
  if(sweepSetup.ac_couple) {
    lockin->AC_couple();
  }
  else {
    lockin->DC_couple();
  }

  // Do the parametric sweep
  int exitVal = sweepRepeatLoop(0, prefix, outps);

  // Cleanup
  sweepFinalizeOutput(outps);
  if(initialCoupling == 0) {
    lockin->AC_couple();
  }
  else {
    lockin->DC_couple();
  }
  if(sweepSetup.autoSens)
    lockin->set_sensitivity(initialSens0);
  if(sweepSetup.autoTimeConst)
    lockin->set_time_constant(initialTC0);

  // Finish
  cancelSweep = true;
  return exitVal;
}


/*
 * Log that a sweep was canceled and the time of cancellation
 */
void logCanceledSweep()
{
  time_t now = time(0);
  struct tm* tmNow = localtime(&now);
  (*LockinSettings::settingsLogger) << std::endl << "=== SWEEP CANCELED at " 
      << (1900 + tmNow->tm_year) << "-" << (1 + tmNow->tm_mon) << "-"
      << tmNow->tm_mday << " " << tmNow->tm_hour << ":" << tmNow->tm_min
      << ":" << tmNow->tm_sec << " ===" << std::endl;
}


void sendCommandToLockin(int currParam, double currVal)
{
  switch(currParam) {
    case SWEEP_F:
      lockin->set_frequency(currVal);
    break;
    case SWEEP_X:
      lockin->set_auxout1(currVal);
    break;
    case SWEEP_Y:
      lockin->set_auxout2(currVal);
    break;
    case SWEEP_A:
      lockin->set_reference_amplitude(currVal);
    break;
  }
}

long calculateSweepDuration()
{
  long millis = 0;
  for(int i = numActiveParams-1; i >= 0; i--) {
    if(sweepSetup.parameters[i] == SWEEP_CUSTOM) {
      millis += sweepSetup.waits[sweepSetup.parameters[i]];
      millis *= numCustom;
    }
    else {
      if(sweepSetup.parameters[i] == SWEEP_F && sweepSetup.autoTimeConst) {
        // Multiply the time for each sub-sweep by the number of steps at the
        // current sweep level
        millis *= sweepSetup.steps[sweepSetup.parameters[i]] + 1;
        
        // Calculate wait times for variable time constant
        double * currValues = new double[sweepSetup.steps[SWEEP_F] + 1];
        getCurrentValues(SWEEP_F, currValues);
        
        // Add up all the wait times for the steps at the current level
        int lastTimeConst = -1;
        for(int j = 0; j <= sweepSetup.steps[SWEEP_F]; j++) {
          double tau_req = getTauReq(currValues[j], filterType);
          int newTimeConst = getTimeConst(tau_req);
          if(lastTimeConst != -1 && newTimeConst != lastTimeConst)
            millis += FIRST_STEP_WAIT;
          millis += getWaitFactor(filterType)*getTimeConstValue(newTimeConst)*1000;
          lastTimeConst = newTimeConst;
        }
        delete[] currValues;
      }
      else {
        millis += sweepSetup.waits[sweepSetup.parameters[i]];
        millis *= sweepSetup.steps[sweepSetup.parameters[i]] + 1;
      }
    }
    millis += FIRST_STEP_WAIT;
    millis *= sweepSetup.repeats[sweepSetup.parameters[i]];
  
    if(sweepSetup.bidirectional[sweepSetup.parameters[i]]) {
      millis *= 2.0;
    }
  }
  
  return millis;
}

void populateTree(HWND tree)
{
  HTREEITEM parent;
  std::ostringstream desc;
  for(int i = 0; i < numActiveParams; i++) {
    desc << PARAM_DESCRIPTIONS[sweepSetup.parameters[i]];
    desc << " from " << sweepSetup.starts[sweepSetup.parameters[i]] << " to ";
    desc << sweepSetup.ends[sweepSetup.parameters[i]];
    char charDesc[80];
    const char* s = desc.str().data();
    strcpy(charDesc, s);

    TVITEM tvi;
    tvi.pszText = charDesc;
    tvi.mask = TVIF_TEXT;

    TVINSERTSTRUCT tviStruct;
    tviStruct.hParent = (i == 0)?TVI_ROOT:parent;
    tviStruct.hInsertAfter = TVI_FIRST;
    tviStruct.item = tvi;

    HTREEITEM tmp = (HTREEITEM)SendMessage(tree, TVM_INSERTITEM, 0,
        (LPARAM)(LPTVINSERTSTRUCT)&tviStruct);


    TreeView_Expand(tree, parent, TVE_EXPAND);
    parent = tmp;
    desc.str( std::string() );
    desc.clear();
  }
  desc << "Detection Harmonic: " << sweepSetup.detHarm;
  char charDesc[80];
  strcpy(charDesc, desc.str().data());
  TVITEM tvi;
  tvi.pszText = charDesc;
  tvi.mask = TVIF_TEXT;
  TVINSERTSTRUCT tviStruct;
  tviStruct.hParent = TVI_ROOT;
  tviStruct.hInsertAfter = TVI_LAST;
  tviStruct.item = tvi;
  SendMessage(tree, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tviStruct);
}

