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
#include <novocfg_pic18t4e4ts1.h>
#include <novo.h>
#include "types.h"
#include "handles.h"
#include "TaskInj.h"

#ifdef DEBUG
  #include <icd2.h>
#endif

static void TaskInj();

static bit busy = false;
static bit abort = false;

static uint32_t errors = 0;
static uint16_t cycles = 0;
static uint16_t accCycles = 0;

#define MIN_ON_TIME             (125)   // T3 rate is 1.25MHz - 800ns     125==100us

static uint16_t onTime = MIN_ON_TIME;
static const uint16_t offTime = 6250;   // 5ms
static uint16_t onTrailingEdge;

void TaskInjInit(uint8_t priority) {

  SysCreateTask(hTaskInj, priority, TaskInj);
  SysStartTask(hTaskInj);

  // setup T3 -
  // 16bit r/w, clksrc for eccp, 1:8 pre, int clk, STOP
  // freq = 1.25MHZ
  t3con = 0b10111000;
  tmr3h = 0;
  tmr3l = 0;              // clr counter

  // setup eccp - drive pin low, goes high on next interrupt
  eccp1con = 0b00001000;
  trisd.4 = TRISOutput;

}

uint8_t injStart() {
  SysCriticalSectionBegin();
  if( !busy ) {
    busy = true;
    SysCriticalSectionEnd();
    abort = false;
    SysSignalSemaphore(hInj);
    return INJ_OK;
  }
  SysCriticalSectionEnd();
  return INJ_BUSY;
}

uint8_t injIsBusy() {
  return busy?0x01:0x00;
}

uint8_t injAbort() {
  if( busy ) {
    abort=true;
    return INJ_OK;
  }
  return INJ_UNKNOWN;
}

uint8_t injIsAborting() {
  return abort?0x01:0x00;
}

uint8_t injGetCycles(uint16_t *c) {
  *c = (cycles+1);
  return INJ_OK;
}

uint8_t injSetCycles(uint16_t *c) {
  if( !busy ) {
    cycles = (*c-1);
    return INJ_OK;
  }
  return INJ_BUSY;
}

uint8_t injGetAccumulatedCycles(uint16_t *c) {
  *c = accCycles;
  return INJ_OK;
}

uint8_t injSetAccumulatedCycles(uint16_t *c) {
  if( !busy ) {
    accCycles = *c;
    return INJ_OK;
  }
  return INJ_BUSY;
}

uint8_t injSetOnWidth(uint16_t *w) {

  if( !busy ) {
    // anytime the ON width is changed .. accumulated
    // counts are no longer valid .. so reset it
    if( *w != onTime ) {
      if( *w < MIN_ON_TIME ) {
        return INJ_TOO_LITTLE;
      }
      if( *w > (0xffff-offTime) ) {
        return INJ_TOO_BIG;
      }
      accCycles = 0;
      onTime = *w;
    }
    return INJ_OK;
  }
  return INJ_BUSY;
}

uint8_t injGetOnWidth(uint16_t *w) {
  *w = onTime;
  return INJ_OK;
}

/*
uint8_t injSetOffWidth(uint16_t *w) {

  if(!busy) {
    if(*w < MIN_OFF_TIME) { return INJ_TOO_LITTLE; }
    offTime = *w;
    return INJ_OK;
  }
  return INJ_BUSY;
}
*/

uint8_t injGetOffWidth(uint16_t *w) {
  *w = offTime;
  return INJ_OK;
}

void injInt() {
  if( portd.4 ) {
    // leading edge just occured
    eccpr1l = (uint8_t)onTrailingEdge;
    eccpr1h = (uint8_t)(onTrailingEdge>>8); // load trailing edge
    eccp1con = 0b00001001;                  // low on next match
  } else {
    // trailing edge just occured - done
    pie2.ECCP1IE = false;
    t3con.TMR3ON = false;
    SysSignalSemaphoreIsr(hInj);
  }
}

static void TaskInj() {

  while( true ) {
    Sys_WaitSemaphore(hInj, 153);
    if( SysWaitTimedOut(hInj) == false ) {
      // start cycle
      tmr3h = 0;
      tmr3l = 0;                              // clr counter
      eccp1con = 0b00001000;                  // go high on next interrupt
      eccpr1l = (uint8_t)offTime;             // load leading edge (setting OFF period)
      eccpr1h = (uint8_t)(offTime>>8);
      t3con.TMR3ON = true;                    // go
      pir2.ECCP1IF = false;                   // clr and unmask int
      pie2.ECCP1IE = true;
      onTrailingEdge=onTime+offTime;          // calc trailing edge time
      Sys_WaitSemaphore(hInj, 15);            // wait for complete cycle to finish  15*6.5ms==97.5ms timeout (> than possible count)
      if( SysWaitTimedOut(hTaskInj) == false ) {
        accCycles++; // no roll check! by design
        if( cycles ) {
          // more cycles to go ...
          cycles--;
          if( abort ) {
            // ... unless told to abort
            abort = false;  // ack abort
            busy = false;
          } else {
            SysSignalSemaphore(hInj);
          }
        } else {
          // done - all cycles complete
          busy = false;
        }
      } else {
        // cycle never finished - timeout
        errors++;
        busy = false;
      }
    }
  }

}