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

#include <system.h>
#include <icd2.h>
#include "types.h"
#include "inj.h"

static bit busy=false;

static uint16_t cycles=0;
static uint16_t accCycles=0;

void injInit() {
  // setup eccp
}

void inj() {
  // housekeeping
}

void injInt() {
  // edge interrupt
}

uint8_t injIsBusy() {
  return busy?0x01:0x00;
}

uint8_t injGetCycles(uint16_t *c) {
  *c = cycles;
  return INJ_OK;
}

uint8_t injSetCycles(uint16_t *c) {

  uint8_t retval = INJ_BUSY;
  if( !busy ) {
    cycles = *c;
    retval = INJ_OK;
  }
  return retval;
}

uint8_t injGetAccumulatedCycles(uint16_t *c) {
  *c = accCycles;
  return INJ_OK;
}

uint8_t injSetAccumulatedCycles(uint16_t *c) {

  uint8_t retval = INJ_BUSY;
  if( !busy ) {
    accCycles = *c;
    retval = INJ_OK;
  }
  return retval;
}