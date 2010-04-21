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

#ifndef __taskinj_h
#define __taskinj_h

enum injErr_t {

  INJ_OK = 0,
  INJ_BUSY,
  INJ_TOO_BIG,
  INJ_TOO_LITTLE,

  INJ_UNKNOWN

};

void TaskInjInit(uint8_t priority);

void injInt();

uint8_t injStart();
uint8_t injIsBusy();            // !busy when x->0 cycle transition or abort completes

uint8_t injAbort();
uint8_t injIsAborting();        // abort clears when cycle is completely finished

uint8_t injGetCycles(uint16_t *c);
uint8_t injSetCycles(uint16_t *c);

uint8_t injGetAccumulatedCycles(uint16_t *c);
uint8_t injSetAccumulatedCycles(uint16_t *c);

uint8_t injSetOnWidth(uint16_t *w);
uint8_t injGetOnWidth(uint16_t *w);

//uint8_t injSetOffWidth(uint16_t *w);
uint8_t injGetOffWidth(uint16_t *w);

#endif //__taskinj_h