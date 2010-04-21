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
#include "PICInjDriver.h"
#include "TaskSerialCmdProc.h"
#include "TaskInj.h"
#include "fp.h"

#pragma CLOCK_FREQ 40000000

#pragma DATA _CONFIG1H, _IESO_OFF_1H & _FCMEN_OFF_1H & _OSC_HSPLL_1H
#pragma DATA _CONFIG2H, _WDT_OFF_2H
#pragma DATA _CONFIG3H, _MCLRE_ON_3H & _LPT1OSC_OFF_3H & _PBADEN_OFF_3H

#ifdef DEBUG
  #include <icd2.h>
  #pragma DATA _CONFIG2L, _BORV_1_2L & _BOREN_BOHW_2L & _PWRT_OFF_2L
  #pragma DATA _CONFIG4L, _DEBUG_ON_4L & _XINST_OFF_4L & _BBSIZ_2048_4L & _LVP_OFF_4L & _STVREN_ON_4L
  #pragma DATA _CONFIG5L, _CP0_OFF_5L & _CP1_OFF_5L & _CP2_OFF_5L & _CP3_OFF_5L
  #pragma DATA _CONFIG5H, _CPB_OFF_5H & _CPD_OFF_5H
  #pragma DATA _CONFIG6L, _WRT0_OFF_6L & _WRT1_OFF_6L & _WRT2_OFF_6L & _WRT3_OFF_6L
  #pragma DATA _CONFIG6H, _WRTB_OFF_6H & _WRTC_OFF_6H & _WRTD_OFF_6H
  #pragma DATA _CONFIG7L, _EBTR0_OFF_7L & _EBTR1_OFF_7L & _EBTR2_OFF_7L & _EBTR3_OFF_7L
  #pragma DATA _CONFIG7H, _EBTRB_OFF_7H
#else
  #pragma DATA _CONFIG2L, _BORV_1_2L & _BOREN_BOHW_2L & _PWRT_ON_2L
  #pragma DATA _CONFIG4L, _DEBUG_OFF_4L & _XINST_OFF_4L & _BBSIZ_2048_4L & _LVP_OFF_4L & _STVREN_ON_4L
  #pragma DATA _CONFIG5L, _CP0_ON_5L & _CP1_ON_5L & _CP2_ON_5L & _CP3_ON_5L
  #pragma DATA _CONFIG5H, _CPB_ON_5H & _CPD_OFF_5H
  #pragma DATA _CONFIG6L, _WRT0_ON_6L & _WRT1_ON_6L & _WRT2_ON_6L & _WRT3_ON_6L
  #pragma DATA _CONFIG6H, _WRTB_ON_6H & _WRTC_ON_6H & _WRTD_OFF_6H
  #pragma DATA _CONFIG7L, _EBTR0_ON_7L & _EBTR1_ON_7L & _EBTR2_ON_7L & _EBTR3_ON_7L
  #pragma DATA _CONFIG7H, _EBTRB_ON_7H
#endif //DEBUG

#pragma DATA _IDLOC0, 0         // readable under all conditions, good place for checksums
#pragma DATA _IDLOC1, 0
#pragma DATA _IDLOC2, 0
#pragma DATA _IDLOC3, 0
#pragma DATA _IDLOC4, 0
#pragma DATA _IDLOC5, 0
#pragma DATA _IDLOC6, 0
#pragma DATA _IDLOC7, 0

/**************************************************************************************
 *  DEVICE: 18F4580,  10mhz external ceramic osc, bumped to 40mhz internally by PLL
 *
 *              for use with PIC-MT-USB from olimex.com
 *                      info  - [http://www.olimex.com/dev/pic-mt-usb.html]
 *                      schem - [http://www.olimex.com/dev/images/pic-mt-usb-sch.gif]
 *
 *  REWORK: Pull 'test' pin on FT232BM/U3 low to avoid test mode and allow default
 *          enumeration to happen. On more recent boards I've ordered, it seems that
 *          they're coming w/the fix. You may not have to perform the rework.
 *
 *          see 'mods to use 18f4580.txt'
 *
 *  PINOUT:
 *                       +----U----+
 *             Vpp/MCLR  | 1    40 |  RB7 - ICSP PGD
 *                - RA0  | 2    39 |  RB6 - ICSP PGC
 *                - RA1  | 3    38 |  RB5 - ICSP PGM, if not PGM, then LVP canNOT be used
 *                - RA2  | 4    37 |  RB4 - Button B2, Bot
 *                - RA3  | 5    36 |  RB3 - CANRX <-
 *                - RA4  | 6    35 |  RB2 - CANTX ->
 *                - RA5  | 7    34 |  RB1 - LED2, Red LED
 *                - RE0  | 8    33 |  RB0 -
 *                - RE1  | 9    32 |  +V
 *                - RE1  | 10   31 |  GND
 *                   +V  | 11   30 |  RD7 -
 *                  GND  | 12   29 |  RD6 -
 *                  OSC  | 13   28 |  RD5 -
 *                  OSC  | 14   27 |  RD4 - INJ OUT
 *                - RC0  | 15   26 |  RC7 - Serial RX
 *                - RC1  | 16   25 |  RC6 - Serial TX
 *     FP PWM OUT - RC2  | 17   24 |  RC5 -
 *                - RC3  | 18   23 |  RC4 -
 *                - RD0  | 19   22 |  RD3 -
 *                - RD1  | 20   21 |  RD2 -
 *                       +---------+
 *
 *
 * function by pin:
 *
 *   RB1 - Red LED (LED2), active high/source
 *   RD4 - Injector Out to driver
 *   RC2 - Fuel Pump PWM out to driver
 *
 **************************************************************************************/

static void timeBaseStart(void);

void main(void) {

  SysInit();
  TaskSerialCmdProcInit(0);
  TaskInjInit(0);
  timeBaseStart();

  fpInit();

  while( true ) {
    // might be a good place to parse ser/can pkts ?
    // just sit back and yield until a pkt comes in
    // everything else is pretty much self sustaining
    Sys_Yield();
  }
}

void interrupt(void) {

  if( intcon.TMR0IF ) {
    intcon.TMR0IF = false;
    SysTimerUpdate();
  }

  if( pie1.TXIE && pir1.TXIF ) {
    serialTXInt();
  }

  if( pir1.RCIF ) {
    pir1.RCIF = false;
    serialRXInt();
  }

  if( pir2.ECCP1IF ) {
    pir2.ECCP1IF = false;
    injInt();
  }

}

static void timeBaseStart(void) {

  // 10mhz/256 = 39062.5Hz, tmr0 should roll @ 153.186 Hz (6.528ms)
  // good enough for a decent rate

  t0con         = 01000111b;    // tmr off, 8bit, internal clk src, prescale, /256
  tmr0l         = 0;            // reset
  intcon.TMR0IF = false;
  intcon.TMR0IE = true;
  intcon.PEIE   = true;
  t0con.TMR0ON  = 1;            // start

}
