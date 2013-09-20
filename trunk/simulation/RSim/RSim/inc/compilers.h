//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru   
//
//  Target(s)...: ATMega
//
//  Compiler....: IAR, GCC, CodeVision
//
//  Description.: ���� ��� ������������ ��������, ���������� ���������
//
//  Data........: 08.03.13
//
//	Updated.....: 29.03.2013 by Avega
//***************************************************************************
#ifndef COMPILERS_H
#define COMPILERS_H


#define F_CPU 16000000UL    // AVR clock frequency in Hz, used by util/delay.h



//________________________________________
#ifdef  __ICCAVR__
#include <ioavr.h>
#include <inavr.h>
#include <intrinsics.h>
#include <stdint.h>
#define read_byte_flash(x) (x)
#endif

//________________________________________
#ifdef  __GNUC__
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
//#include <stdint.h>

#define __save_interrupt() SREG
#define __restore_interrupt(var) SREG = (var)
#define __disable_interrupt() cli()
#define __enable_interrupt() sei()
#define __delay_cycles(var) _delay_us((unsigned int)(var)/(F_CPU/1000000))
#define __flash PROGMEM const
#define read_byte_flash(x) pgm_read_byte(&(x)) 
#endif

//________________________________________
#ifdef __CODEVISIONAVR__
#include <io.h>
#include <delay.h>
#include <stdint.h>

#define __save_interrupt() SREG
#define __restore_interrupt(var) SREG = (var)
#define __disable_interrupt() #asm("cli")
#define __enable_interrupt() #asm("sei")
#define __delay_cycles(var) delay_us((unsigned int)(var)/(_MCU_CLOCK_FREQUENCY_/1000000))
#define read_byte_flash(x) (x)
#endif

//____________________________________________


//________________________________________
#ifdef WIN32
#include "stdint.h"
#include "taps.h"
#endif

//____________________________________________



#endif //COMPILERS_H

