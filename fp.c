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
#include "types.h"
#include "handles.h"
#include "fp.h"

#ifdef DEBUG
  #include <icd2.h>
#endif

void fpInit() {

  t2con = 0b0000011;              // off, /16 - > 625KHz
  tmr2 = 0;
  pr2 = 0xff;                             // 625KHz -> /256 == 2.4KHz period
  ccpr1l = 0;                             // dc=0 by default
  ccp1con = 0b00001100;   // ccp1 == pwm
  t2con.TMR2ON = true;    // go
  trisc.2 = TRISOutput;   // RC2 - CCP1

}

uint8_t fpSet(uint16_t *dc) {

  uint16_t pwm = *dc;
  if( pwm > 0x3ff ) {
    pwm = 0x3ff;
  }        // 10bit
  ccp1con.DC1B0 = pwm.0;
  ccp1con.DC1B1 = pwm.1;
  pwm >>= 2;
  ccpr1l = (uint8_t)pwm;          // ccpr1l is 'shadow'
  return FP_OK;

}

uint8_t fpGet(uint16_t *dc) {

  uint16_t pwm = ccpr1h;          // ccpr1h is where shadow is xfered to (r/o)
  pwm <<= 2;
  pwm.0 = ccp1con.DC1B0;
  pwm.1 = ccp1con.DC1B1;
  *dc = pwm;
  return FP_OK;

}