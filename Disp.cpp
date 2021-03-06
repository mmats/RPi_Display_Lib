#include "Disp.h"
#include <GPIO.h>
#include <ctime>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <initializer_list>
#include <vector>

Disp::Disp(std::initializer_list<int> il)
{
	std::vector<int> pins(il);
	RS  = new GPIO(pins.at(0),OUT);
	RW  = new GPIO(pins.at(1),OUT);
	E   = new GPIO(pins.at(2),OUT);
	DB0 = new GPIO(pins.at(3),OUT);
	DB1 = new GPIO(pins.at(4),OUT);
	DB2 = new GPIO(pins.at(5),OUT);
	DB3 = new GPIO(pins.at(6),OUT);
	DB4 = new GPIO(pins.at(7),OUT);
	DB5 = new GPIO(pins.at(8),OUT);
	DB6 = new GPIO(pins.at(9),OUT);
	DB7 = new GPIO(pins.at(10),OUT);

	init_state = START;
	disp_state = INIT;
	out_state  = ROW_1_SET;
	disp_job   = no_job;

	settings1.prefix = PREFIX;
		settings1._8_4_bit = 1;
		settings1.dual_single_line = 1;
		settings1._5x10_5x8_dots = 0;
		settings1.postfix = POSTFIX;
	settings2.prefix = PREFIX;
		settings2.shift_display_cursor = 0;
		settings2.shift_right_left = 1;
		settings2.postfix = POSTFIX;
	settings3.prefix = PREFIX;
		settings3.increment_decrement = 1;
		settings3.display_shift_on = 0;
	settings4.prefix = PREFIX;
		settings4.display_on = 1;
		settings4.cursor_display_on = 0;
		settings4.cursor_blink_on = 0;
}
Disp::~Disp()
{
	displayClear();
	displayOff();
	delete RS; delete RW; delete E;
	delete DB0; delete DB1; delete DB2; delete DB3;
	delete DB4; delete DB5; delete DB6; delete DB7;
}

void Disp::process()
{
	static int character;

	switch (disp_state)
	{
	case INIT:
		instructDisplay();
		break;

	case OPERATION:
		switch (out_state)
		{
		case ROW_1_SET:
			character = 0;
			write(0x80|LINE_1_ADDR, CMD);
			out_state = ROW_1_WRITE;
			break;

		case ROW_1_WRITE:
			write(outArray[character], DATA);
			character++;
			if( character >= DISP_LINE_LENGTH )
				out_state = ROW_2_SET;
			break;

		case ROW_2_SET:
			write(0x80|LINE_2_ADDR, CMD);
			out_state = ROW_2_WRITE;
			break;

		case ROW_2_WRITE:
			write(outArray[character], DATA);
			character++;
			if( character >= 2*DISP_LINE_LENGTH )
				out_state = MAX_OUT;
			break;

		case MAX_OUT:
		default:
			disp_state = WAITING;
			disp_job = no_job;
			break;
		}
		while( isBusy() )
		{
			usleep(1);
		}
		break;

	case WAITING:
		if( disp_job == text_job )
		{
			disp_state = OPERATION;
			out_state = ROW_1_SET;
		}
		break;
	}
}

void Disp::writeText( unsigned char* textptr, int lineNr )
{
	unsigned i;

	if( lineNr>=1 && lineNr<=DISP_LINES )
	{
		for(i=0; i<DISP_LINE_LENGTH; ++i)
			outArray[i+(lineNr-1)*DISP_LINE_LENGTH] = textptr[i];
	}
	else
	{
		// ERROR
		for(i=0; i<DISP_LINE_LENGTH*DISP_LINES; ++i)
			outArray[i] = '!';
	}

	disp_job = text_job;
}
void Disp::writeText( std::string* textptr, int lineNr )
{
	unsigned i;

	if( lineNr>=1 && lineNr<=DISP_LINES )
	{
		// cut, if text is too long
		unsigned max = textptr->length();
		if( max > DISP_LINE_LENGTH )
			max = DISP_LINE_LENGTH;

		for(i=0; i<max; ++i)
			outArray[i+(lineNr-1)*DISP_LINE_LENGTH] = textptr->at(i);
		for(i=textptr->length(); i<DISP_LINE_LENGTH; ++i)
			outArray[i+(lineNr-1)*DISP_LINE_LENGTH] = ' ';
	}
	else
	{
		// ERROR
		for(i=0; i<DISP_LINE_LENGTH*DISP_LINES; ++i)
			outArray[i] = '!';
	}

	disp_job = text_job;
}

void Disp::displayClear()
{
	write(0b00000001, CMD);
}
void Disp::displayOn()
{
	displayOnOff(1, settings4.cursor_display_on, settings4.cursor_blink_on);
}
void Disp::displayOff()
{
	displayOnOff(0, settings4.cursor_display_on, settings4.cursor_blink_on);
}
void Disp::displayCursorHome()
{
	write(0b00000010, CMD);
}
void Disp::entryModeSet(bool ID, bool S)
{
	settings3.prefix = PREFIX;
	settings3.increment_decrement = ID;
	settings3.display_shift_on = S;

	char tmp;
	memcpy( &tmp, &settings3, sizeof(char) );
	write( tmp,CMD );
}
void Disp::displayOnOff(bool D, bool C, bool B)
{
	settings4.prefix = PREFIX;
	settings4.display_on = D;
	settings4.cursor_display_on = C;
	settings4.cursor_blink_on = B;

	char tmp;
	memcpy( &tmp, &settings4, sizeof(char) );
	write( tmp,CMD );
}
void Disp::displayCursorShift(bool SC, bool RL)
{
	settings2.prefix = PREFIX;
	settings2.shift_display_cursor = SC;
	settings2.shift_right_left = RL;
	settings2.postfix = POSTFIX;

	char tmp;
	memcpy( &tmp, &settings2, sizeof(char) );
	write( tmp,CMD );
}
void Disp::functionSet(bool DL, bool N, bool F)
{
	settings1.prefix = PREFIX;
	settings1._8_4_bit = DL;
	settings1.dual_single_line = N;
	settings1._5x10_5x8_dots = F;
	settings1.postfix = POSTFIX;

	char tmp;
	memcpy( &tmp, &settings1, sizeof(char) );
	write( tmp,CMD );
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
void Disp::instructDisplay()
{
	char tmp;
	int i;

	switch( init_state )
	{
	case START:
		usleep(30000);
		init_state = SET_INTERFACE_TO_8BIT_1;
		break;

	case SET_INTERFACE_TO_8BIT_1:
		write( 0b00110000,CMD );
		init_state = SET_INTERFACE_TO_8BIT_2;
		usleep(5);
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
		displayOff();
		init_state = DISPLAY_CLEAR;
		break;

	case DISPLAY_CLEAR:
		displayClear();
		for(i=0; i<DISP_LINE_LENGTH*DISP_LINES; ++i)
			outArray[i] = ' ';
		displayCursorHome();
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
		settings4.display_on=1;
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
