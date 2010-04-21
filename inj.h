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

#ifndef __inj_h
#define __inj_h

enum injErr_t {

        INJ_OK = 0,
        INJ_BUSY,

        INJ_UNKNOWN

};

void injInit();
void inj();
void injInt();

uint8_t injIsBusy();

uint8_t injGo();        // start generating cycles (or single cycle if count is 0)
uint8_t injStop();      // finish up current cycle and go idle, poll injIsBusy to determine when cycle is finished

uint8_t injGetCycles(uint16_t *c);
uint8_t injSetCycles(uint16_t *c);

uint8_t injGetAccumulatedCycles(uint16_t *c);
uint8_t injSetAccumulatedCycles(uint16_t *c);

uint8_t injSetOnWidth(uint16_t *w);
uint8_t injGetOnWidth(uint16_t *w);

uint8_t injSetOffWidth(uint16_t *w);
uint8_t injGetOffWidth(uint16_t *w);

#endif //__inj_h