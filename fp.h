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

#ifndef __fp_h
#define __fp_h

enum {

  FP_OK,
  FP_UNKNOWN

};


void fpInit();

uint8_t fpSet(uint16_t *);
uint8_t fpGet(uint16_t *);

#endif // __fp_h