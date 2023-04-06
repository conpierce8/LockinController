// GPIB.cpp
// encoding: utf-8
//
// Interface for communication via NI GPIB.
//
// Author:   Connor D. Pierce
// Created:  2018-02-02
// Modified: 2023-04-05 21:31:36
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


#include "GPIB.h"


GPIBInterface::GPIBInterface(int idd)
{
  id = idd;
  char buffer[BUF_SIZE];
  int i; //the number of listeners on the bus
  unsigned short address; //the address of a listener

  SendIFC(id);
  //check for an error
  if(ibsta & ERR) {
    gpib_error(1, "GPIB-Interface: Could not send IFC");
    throw DisconnectedException("GPIB disconnected; error in SendIFC");
  }

  for(i = 0; i < NUM_DEVICES - 1; i++) {
    instruments[i] = i + 1;
  }
  instruments[NUM_DEVICES - 1] = NOADDR;

  FindLstn(id, instruments, result, NUM_DEVICES);

  //check for error
  if(ibsta & ERR) {
    gpib_error(2, "GPIB-Interface: Could not find listeners");
    throw DisconnectedException("GPIB disconnected; error in FindLstn");
  }

  num_listeners = ibcnt;
  result[num_listeners] = NOADDR;

  std::cout << "GPIB-Interface: Found " << num_listeners << " GPIB devices." << std::endl;

  char *sIDN=new char[11];

  sIDN[0] = '*';
  sIDN[1] = 'I';
  sIDN[2] = 'D';
  sIDN[3] = 'N';
  sIDN[4] = '?';
  sIDN[5] = '\0';

  SendList(id, result, sIDN, 5L, NLend);

  if (ibsta & ERR) {
    gpib_error(3, "GPIB-Interface: Could not send *IDN? to devices");
  }

  std::cout << "GPIB-Interface: Device list:\n";
  std::cout << "----------------------------------------\n";
  for(int i = 0; i < num_listeners; i++) {
    Receive(id, result[i], buffer, BUF_SIZE, STOPend);

    if(ibsta & ERR) {
      gpib_error(4, "GPIB-Interface: Could not receive from device");
    }

    address = GetPAD(result[i]);

    buffer[ibcnt] = '\0';
    
    strcpy(deviceDesc, buffer);

    //Now output the results
    std::cout << ibcnt << std::endl;
    std::cout << "#" << i+1 << " ADDRESS: " <<  address << " ID: " << buffer;
  }
}


void GPIBInterface::getDeviceDesc(char* buffer, int len)
{
  strncpy(buffer, deviceDesc, len);
  buffer[len-1] = '\0';
}


Addr4882_t GPIBInterface::DeviceAddress(unsigned short GPIBaddress)
{
  for(int i=0;i<num_listeners;i++) {
      if(GetPAD(result[i]) == GPIBaddress) {
          return result[i];
      }
  }
  std::cout << "GPIB-Interface: Warining: There is no GPIB device at address " << GPIBaddress << "\n";
  return NOADDR;
}


void GPIBInterface::disconnect_gpib()
{
  ibonl(0,0);
}


void GPIBInterface::send_command(Addr4882_t address, char *command)
{
  Send(id, address, command, strlen(command), NLend);
}


double GPIBInterface::numerical_response_command(Addr4882_t address, char *command)
{
  char *result = new char[1024];
  Send(id, address, command, strlen(command), NLend);
  Receive(id, address, result, 1024, STOPend);
  result[ibcnt]='\0';
  double numval = atof(result);
  delete[] result;
  return numval;
}


int GPIBInterface::integer_response_command(Addr4882_t address, char *command)
{
  char *result = new char[1024];
  Send(id, address, command, strlen(command), NLend);
  Receive(id, address, result, 1024, STOPend);
  result[ibcnt]='\0';
  int numval = atoi(result);
  delete[] result;
  return numval;
}


void GPIBInterface::string_response_command(Addr4882_t address, char* command, char* result, int resultLen)
{
  Send(id, address, command, strlen(command), NLend);
//      std::cout << "string_response_command(): command = " << command << std::endl;
//      std::cout << "  string_response_command(): strlen(result) = " << strlen(result) << std::endl;
  Receive(id, address, result, resultLen, STOPend);
  result[ibcnt] = '\0';
//      std::cout << "  string_response_command(): result = " << result << std::endl;
}


