#include "Disp.h"
#include <GPIO.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


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
	init_state = START;
	disp_state = INIT;
	out_state  = ROW_1_SET;

	settings1.prefix = PREFIX;
		settings1._8_4_bit = 1;
		settings1.dual_single_line = 0;
		settings1._5x10_5x8_dots = 0;
		settings1.postfix = POSTFIX;
	settings2.prefix = PREFIX;
		settings2.shift_display_cursor = 1;
		settings2.shift_right_left = 0;
		settings2.postfix = POSTFIX;
	settings3.prefix = PREFIX;
		settings3.increment_decrement = 1;
		settings3.display_shift_on = 0;
	settings4.prefix = PREFIX;
		settings4.display_on = 1;
		settings4.cursor_display_on = 0;
		settings4.cursor_blink_on = 0;
}

void Disp::instructDisplay()
{
	char tmp;

	switch( init_state )
	{
	case START:
		usleep(15000);
		break;

	case SET_INTERFACE_TO_8BIT_1:
		write( 0b00110000,CMD );
		init_state = SET_INTERFACE_TO_8BIT_2;
		usleep(4100);
		break;

	case SET_INTERFACE_TO_8BIT_2:
		write( 0b00110000,CMD );
		init_state = SET_INTERFACE_TO_8BIT_3;
		usleep(100);
		break;

	case SET_INTERFACE_TO_8BIT_3:
		write( 0b00110000,CMD );
		init_state = SETTING1;
		break;

	case SETTING1:
		memcpy( &tmp, &settings1, sizeof(char) );
		write( tmp,CMD );
		init_state = DISPLAY_OFF;
		break;

	case DISPLAY_OFF:
		write( 0b00001000,CMD );
		init_state = DISPLAY_CLEAR;
		break;

	case DISPLAY_CLEAR:
		write( 0b00000001,CMD );
		init_state = SETTING2;
		break;

	case SETTING2:
		memcpy( &tmp, &settings2, sizeof(char) );
		write( tmp,CMD );
		init_state = SETTING3;
		break;

	case SETTING3:
		memcpy( &tmp, &settings3, sizeof(char) );
		write( tmp,CMD );
		init_state = SETTING4;
		break;

	case SETTING4:
		memcpy( &tmp, &settings4, sizeof(char) );
		write( tmp,CMD );
		init_state = MAX_INIT_DISP;
		break;

	case MAX_INIT_DISP:
		disp_state = WAITING;
		disp_job = no_job;
		break;

	default:
		init_state = START;
		break;
	}
}

void Disp::process()
{
	static int i, e;

	// if( Timer_over(TIMER_LCD) )
	{
		switch (disp_state)
		{
		case INIT:
			instructDisplay();
			// Set_timer(TIMER_LCD, zeit2);
			break;

		case OPERATION:
			e++;
			switch (out_state)
			{
			case ROW_1_SET:
				write(0x80, CMD);
				out_state = ROW_1_WRITE;
				break;

			case ROW_1_WRITE:
				write(outArray[i], DATA);
				i++;
				if (e > DISP_ROW_LENGTH)
					out_state = MAX_OUT;
				break;

			case MAX_OUT:
				disp_state = WAITING;
				disp_job = no_job;
				break;
			}
			usleep(2000); // Set_timer(TIMER_LCD, zeit2);
			break;

		case WAITING:
			if( disp_job == text_job )
			{
				e = 0;
				i = 0;
				disp_state = OPERATION;
				out_state = ROW_1_SET;
			}
			// This "else" below is only for testing!!
			else
			{
				disp_job = text_job;

				char tmp[DISP_ROW_NUMBER][DISP_ROW_LENGTH] = {"123456789ABCDEF"};

				for(int x=0; x<DISP_ROW_NUMBER; ++x)
					for(int y=0; y<DISP_ROW_LENGTH; ++y)
						outArray[(x+1)*y] = tmp[x][y];
			}
			break;
		}
	}
}
