// GPIB.h
// encoding: utf-8
//
// Interface for communication via NI GPIB.
//
// Author:   Connor D. Pierce
// Created:  2018-02-02
// Modified: 2023-04-05 21:32:09
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


#ifndef GPIB_H
#define GPIB_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <exception>

#include <ni4882.h>

#define NUM_DEVICES 31
#define BUF_SIZE 1024

class DisconnectedException: public std::exception {
	char message[80];
	
public:
	DisconnectedException(std::string msg) {
		strcpy(message, msg.c_str());
	}
	
	const char* what() {
		return message;
	}
};


class GPIBInterface {
    int id;
    char *command;
    char deviceDesc[BUF_SIZE];
    int num_listeners;
    Addr4882_t instruments[NUM_DEVICES], result[NUM_DEVICES];

    void gpib_error(int errnum, std::string errmsg) {
        std::cout << "Error #" << errnum << ": " << errmsg << std::endl;
        ibonl(0,0); //take the board offline
//        exit(1); //terminate program  //REMOVED BY CONNOR 2/5/2018
    }

public:
    GPIBInterface(int idd);

	/*
     * Copy the name of the active device into the character array "buffer".
	 */
    void getDeviceDesc(char* buffer, int len);


    /*
     * Get the full address for the device with the given primary address.
     */
    Addr4882_t DeviceAddress(unsigned short GPIBaddress);


    /*
     * Disconnect the GPIB interface.
     */
    void disconnect_gpib();


    /*
     * Send command to lockin (no response).
     */
    void send_command(Addr4882_t address, char *command);

    /*
     * Send command to lockin and get the real-valued response.
     */
    double numerical_response_command(Addr4882_t address, char *command);


    /*
     * Send command to lockin and get the integer response.
     */
    int integer_response_command(Addr4882_t address, char *command);
    
    
    /*
     * Send command to lockin and get the raw string response.
     */
    void string_response_command(
        Addr4882_t address, char* command, char* result, int resultLen
    );

};


#endif
