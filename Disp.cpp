/*
 * Disp.cpp
 *
 *  Created on: 05.10.2013
 *      Author: mats
 */

#include "Disp.h"
#include <GPIO.h>
#include <ctime>
#include <stdio.h>
#include <string.h>

Disp::Disp()
{
	RS  = new GPIO(14,OUT);
	RW  = new GPIO(15,OUT);
	E   = new GPIO(18,OUT);
	DB0 = new GPIO(11,OUT);
	DB1 = new GPIO(23,OUT);
	DB2 = new GPIO(24,OUT);
	DB3 = new GPIO( 9,OUT);
	DB4 = new GPIO(25,OUT);
	DB5 = new GPIO( 8,OUT);
	DB6 = new GPIO( 7,OUT);
	DB7 = new GPIO(10,OUT);

	initDisplay();
}

Disp::Disp(int port[11])
{
	RS  = new GPIO(port[0],OUT);
	RW  = new GPIO(port[1],OUT);
	E   = new GPIO(port[2],OUT);
	DB0 = new GPIO(port[3],OUT);
	DB1 = new GPIO(port[4],OUT);
	DB2 = new GPIO(port[5],OUT);
	DB3 = new GPIO(port[6],OUT);
	DB4 = new GPIO(port[7],OUT);
	DB5 = new GPIO(port[8],OUT);
	DB6 = new GPIO(port[9],OUT);
	DB7 = new GPIO(port[10],OUT);

	initDisplay();
}

Disp::~Disp()
{
	delete RS; delete RW; delete E;
	delete DB0; delete DB1; delete DB2; delete DB3;
	delete DB4; delete DB5; delete DB6; delete DB7;
}

void Disp::write(char data,regSel rs)
{
	struct timespec delay;
	delay.tv_sec = 0;

	RS->setValue(rs);
	RW->setValue(0);

	delay.tv_nsec = 90L;
	nanosleep(&delay , NULL);

	E->setValue(1);

	delay.tv_nsec = 25L;
	nanosleep(&delay , NULL);

	DB0->setValue( data & 0x01 );
		data >>= 1;
	DB1->setValue( data & 0x01 );
		data >>= 1;
	DB2->setValue( data & 0x01 );
		data >>= 1;
	DB3->setValue( data & 0x01 );
		data >>= 1;
	DB4->setValue( data & 0x01 );
		data >>= 1;
	DB5->setValue( data & 0x01 );
		data >>= 1;
	DB6->setValue( data & 0x01 );
		data >>= 1;
	DB7->setValue( data & 0x01 );

	delay.tv_nsec = 300L;
	nanosleep(&delay , NULL);

	E->setValue(0);

	delay.tv_nsec = 35L;
	nanosleep(&delay , NULL);
}

void Disp::read(char& data,regSel rs)
{
	struct timespec delay;
	delay.tv_sec = 0;

	RS->setValue(rs);
	RW->setValue(1);

	delay.tv_nsec = 60L;
	nanosleep(&delay , NULL);

	E->setValue(1);

	delay.tv_nsec = 215L;
	nanosleep(&delay , NULL);

	DB7->setDirection(IN);
	DB6->setDirection(IN);
	DB5->setDirection(IN);
	DB4->setDirection(IN);
	DB3->setDirection(IN);
	DB2->setDirection(IN);
	DB1->setDirection(IN);
	DB0->setDirection(IN);

	data = DB7->getValue();
	data <<= 1;
	data |= DB6->getValue();
	data <<= 1;
	data |= DB5->getValue();
	data <<= 1;
	data |= DB4->getValue();
	data <<= 1;
	data |= DB3->getValue();
	data <<= 1;
	data |= DB2->getValue();
	data <<= 1;
	data |= DB1->getValue();
	data <<= 1;
	data |= DB0->getValue();

	DB7->setDirection(OUT);
	DB6->setDirection(OUT);
	DB5->setDirection(OUT);
	DB4->setDirection(OUT);
	DB3->setDirection(OUT);
	DB2->setDirection(OUT);
	DB1->setDirection(OUT);
	DB0->setDirection(OUT);

	delay.tv_nsec = 300L;
	nanosleep(&delay , NULL);

	E->setValue(0);

	delay.tv_nsec = 45L;
	nanosleep(&delay , NULL);
}

bool Disp::isBusy()
{
	char data;
	read(data,CMD);
	data >>= 7;
	data &= 0x01;
	return data;
}

void Disp::initDisplay()
{
	char tmp;

	switch( init_state )
	{
	case START:
		settings1.prefix = PREFIX;
			settings1.increment_decrement = 1;
			settings1.display_shift_on = 0;
		settings2.prefix = PREFIX;
			settings2.display_on = 1;
			settings2.cursor_display_on = 0;
			settings2.cursor_blink_on = 0;
		settings3.prefix = PREFIX;
			settings3.shift_display_cursor = 1;
			settings3.shift_right_left = 0;
			settings3.postfix = POSTFIX;
		settings4.prefix = PREFIX;
			settings4._8_4_bit = 1;
			settings4.dual_single_line = 0;
			settings4._5x10_5x8_dots = 0;
			settings4.postfix = POSTFIX;
//		Set_timer(TIMER_LCD, zeit1);
		init_state = SETTING1;
		break;

	case SETTING1:
		memcpy( &tmp, &settings1, sizeof(settings1) );
		write( tmp,CMD );
		init_state = SETTING2;
		break;

	case SETTING2:
		memcpy( &tmp, &settings2, sizeof(settings2) );
		write( tmp,CMD );
		init_state = SETTING3;
		break;

	case SETTING3:
		memcpy( &tmp, &settings3, sizeof(settings3) );
		write( tmp,CMD );
		init_state = SETTING4;
		break;

	case SETTING4:
		memcpy( &tmp, &settings4, sizeof(settings4) );
		write( tmp,CMD );
		init_state = MAX_INIT_DISP;
		break;

	case MAX_INIT_DISP:
//		disp_state = WARTEN_AUF_EINGABE;
//		Disp_job = no_job;
		break;

	default:
		init_state = START;
		break;
	}
}
