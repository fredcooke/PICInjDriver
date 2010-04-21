/**
 *
 *   PICInjDriver: assisting diy efi efforts worldwide to
 *                 flow/clean your unknown injectors
 *
 *   Copyright (C) 2009 Sean Stasiak sstasiak@gmail.com
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __types_h
#define __types_h

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

#define TRISInput  (1)
#define TRISOutput (0)

inline void redOn(void) { trisb.1 = 0; portb.1 = 1; }
inline void redOff(void) { trisb.1 = 0; portb.1 = 0; }

#endif // __types_h
