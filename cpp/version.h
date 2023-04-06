// version.h
// encoding: utf-8
//
// Version information.
//
// Author:   Connor D. Pierce
// Created:  2023-04-05 21:37:22
// Modified: 2023-04-05 21:38:35
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


#ifndef VERSION_H_
#define VERSION_H_


// Version information
#define SOFTWARE_NAME "Lockin Controller"
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_BUGFIX 0
#define VERSION_ALPHA 23

#define _MK_VERSION(MAJ,MIN,BUG,ALPH) MAJ,MIN,BUG,ALPH
#define _MK_VERS_STR3(MAJ,MIN,BUG) #MAJ "." #MIN "." #BUG
#define _MK_VERS_STR4(MAJ,MIN,BUG,ALPH) #MAJ "." #MIN "." #BUG ".a" #ALPH
#define _MK_NAME_VERSION(NAME,VERS) NAME " v" VERS
#if VERSION_ALPHA > 0
  #define _MK_VERS_STR(MAJ,MIN,BUG,ALPH) _MK_VERS_STR4(MAJ,MIN,BUG,ALPH)
#else
  #define _MK_VERS_STR(MAJ,MIN,BUG,ALPH) _MK_VERS_STR3(MAJ,MIN,BUG)
#endif

#define VERSION_ALL _MK_VERSION(VERSION_MAJOR,VERSION_MINOR,VERSION_BUGFIX,VERSION_ALPHA)
#define VERSION_STRING _MK_VERS_STR(VERSION_MAJOR,VERSION_MINOR,VERSION_BUGFIX,VERSION_ALPHA)
#define SOFTWARE_NAME_VERSION _MK_NAME_VERSION(SOFTWARE_NAME,VERSION_STRING)


#endif
