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
#include <ctype.h>
#include <stdlib.h>
#include "types.h"
#include "handles.h"
#include "TaskSerialCmdProc.h"
#include "TaskInj.h"
#include "fp.h"

#ifdef DEBUG
  #include <icd2.h>
#endif

static void uartInit(void);
static void TaskSerialCmdProc(void);

// commands ----------------------------------------------

enum {

  SERCMD_LEDON        = '0',
  SERCMD_LEDOFF       = '1',
  SERCMD_CHKBUSY      = '2',
  SERCMD_GETCYCLES    = '3',
  SERCMD_SETCYCLES    = '4',
  SERCMD_GETACCCYCLES = '5',
  SERCMD_SETACCCYCLES = '6',
  SERCMD_START        = '7',
  SERCMD_SETONWIDTH   = '8',
  SERCMD_GETONWIDTH   = '9',
  SERCMD_ABORT        = 'a',
  SERCMD_GETOFFWIDTH  = 'b',
  SERCMD_GETFPPWM     = 'c',
  SERCMD_SETFPPWM     = 'd'

};

// --------------------------------------------------------

#define BUFFSIZE        (8)
static uint8_t buffer[BUFFSIZE];
static uint8_t index=0;

static uint32_t errors=0;
static uint32_t txPktCnt=0;
static uint32_t rxPktCnt=0;

void TaskSerialCmdProcInit(uint8_t priority) {

  SysCreateTask(hTaskSerialCmdProc, priority, TaskSerialCmdProc);
  SysStartTask(hTaskSerialCmdProc);

}

static void TaskSerialCmdProc(void) {

  uartInit();

  uint16_t uint16;

  while( true ) {

    Sys_WaitSemaphore(hSerial, 153);
    if( SysWaitTimedOut(hTaskSerialCmdProc) == false ) {

      // pkt is ready to parse
      rxPktCnt++;

      switch( buffer[0] ) {
        case SERCMD_LEDON:
          redOn();
          buffer[1] = '0';
          buffer[2] = '\n';
          break;
        case SERCMD_LEDOFF:
          redOff();
          buffer[1] = '0';
          buffer[2] = '\n';
          break;
        case SERCMD_CHKBUSY:
          buffer[1] = '0';
          buffer[2] = '0'+injIsBusy();
          buffer[3] = '\n';
          break;
        case SERCMD_GETCYCLES:
          buffer[1] = '0'+injGetCycles(&uint16);
          uitoa_hex(&buffer[2], uint16, 4);
          buffer[6] = '\n';
          break;
        case SERCMD_SETCYCLES:
          uint16 = atoui_hex(&buffer[1]);
          buffer[1] = '0'+injSetCycles(&uint16);
          buffer[2] = '\n';
          break;
        case SERCMD_GETACCCYCLES:
          buffer[1] = '0'+injGetAccumulatedCycles(&uint16);
          uitoa_hex(&buffer[2], uint16, 4);
          buffer[6] = '\n';
          break;
        case SERCMD_SETACCCYCLES:
          uint16 = atoui_hex(&buffer[1]);
          buffer[1] = '0'+injSetAccumulatedCycles(&uint16);
          buffer[2] = '\n';
          break;
        case SERCMD_START:
          buffer[1] = '0'+injStart();
          buffer[2] = '\n';
          break;
        case SERCMD_SETONWIDTH:
          uint16 = atoui_hex(&buffer[1]);
          buffer[1] = '0'+injSetOnWidth(&uint16);
          buffer[2] = '\n';
          break;
        case SERCMD_GETONWIDTH:
          buffer[1] = '0'+injGetOnWidth(&uint16);
          uitoa_hex(&buffer[2], uint16, 4);
          buffer[6] = '\n';
          break;
        case SERCMD_ABORT:
          buffer[1] = '0'+injAbort();
          buffer[2] = '\n';
          break;
        case SERCMD_GETOFFWIDTH:
          buffer[1] = '0'+injGetOffWidth(&uint16);
          uitoa_hex(&buffer[2], uint16, 4);
          buffer[6] = '\n';
          break;
        case SERCMD_SETFPPWM:
          uint16 = atoui_hex(&buffer[1]);
          buffer[1] = '0'+fpSet(&uint16);
          buffer[2] = '\n';
          break;
        case SERCMD_GETFPPWM:
          buffer[1] = '0'+fpGet(&uint16);
          uitoa_hex(&buffer[2], uint16, 4);
          buffer[6] = '\n';
          break;
        default:
          errors++;
          buffer[1] = 'F';
          buffer[2] = '\n';
          break;
      }

      // buffer already contains response, trigger
      // a send and wait for it to complete
      pie1.TXIE = true;       // goes straight to int and loads the first item
      Sys_WaitSemaphore(hSerial, 153);

      if( SysWaitTimedOut(hTaskSerialCmdProc) == false ) {
        txPktCnt++;
      } else {
        // tx never finished - got a timeout
        errors++;
      }

    }
  }
}

void serialRXInt(void) {

  // keep it short, this is from the interrupt
  if( index < BUFFSIZE ) {
    const char c = rcreg;
    buffer[index++]=c;
    if( c=='\n' ) {
      index=0;
      SysSignalSemaphoreIsr(hSerial);
    }
  } else {
    errors++;
    index=0;
  }
}

void serialTXInt(void) {

  // keep it short, this is from the interrupt
  if( index < BUFFSIZE ) {
    const char c = buffer[index++];
    if( c=='\n' ) {
      pie1.TXIE = false;                              // done
      index=0;
      SysSignalSemaphoreIsr(hSerial); // terminator
    }
    txreg = c;
  } else {
    errors++;
    index=0;
  }

}

static void uartInit(void) {

// 18f - 40MHz

  trisc.6 = TRISOutput;
  trisc.7 = TRISInput;

  //txsta
  txsta.BRGH = 1;
  txsta.TXEN = 1;         // enable tx

  //rcsta
  rcsta.SPEN = 1;         // enable sp
  rcsta.CREN = 1;         // enable rx

  // baudcon
  baudcon.BRG16 = 1;

  // spbrg = 68;          // 115200 @ 32mhz, brgh = 1, brg16 = 1
  spbrg = 86;             // 115200 @ 40mhz, brgh = 1, brg16 = 1

  pie1.TXIE = false;
  pir1.RCIF = false;
  pie1.RCIE = true;

}