#ifndef DISP_H_
#define DISP_H_

#include "GPIO.h"

#define PREFIX  0x01
#define POSTFIX 0x00
#define DISP_LINES 2
#define DISP_LINE_LENGTH 16
#define LINE_1_ADDR 0x00
#define LINE_2_ADDR 0x40

class Disp
{
public:
	Disp(int* pins);
	/* RS, RW, E, DB[0..7] */
	~Disp();

	void process();	// State machine

	void writeText( unsigned char* textptr, int lineNr );

	void displayClear();
	void displayOn();
	void displayOff();
	void displayCursorHome();
	void entryModeSet(bool ID, bool S);
	void displayOnOff(bool D, bool C, bool B);
	void displayCursorShift(bool SC, bool RL);
	void functionSet(bool DL, bool N, bool F);

	enum regSel
	{
		CMD = 0,
		DATA = 1
	};

	enum DispJob
	{
		text_job,
		no_job
	}disp_job;

	enum DispState
	{
	    INIT,
	    OPERATION,
	    WAITING
	}disp_state;

private:
	void write(char data, regSel rs=DATA);
	void read(char& data, regSel rs=DATA);
	bool isBusy();
	void instructDisplay();

	GPIO* RS;	// register select.  H:data, L:cmd
	GPIO* RW;	// direction select. H:read, L:write
	GPIO* E;	// enable signal
	GPIO* DB0;	// data bus
	GPIO* DB1;
	GPIO* DB2;
	GPIO* DB3;
	GPIO* DB4;
	GPIO* DB5;
	GPIO* DB6;
	GPIO* DB7;

	struct
	{
		int postfix : 2;
		int _5x10_5x8_dots : 1;			// 1:5x10 Dots				0:5x8 Dots
		int dual_single_line : 1;		// 1:Dual Line				0:Single Line
		int _8_4_bit : 1;				// 1:8-Bit					0:4-Bit
		int prefix : 3;
	}settings1;

	struct
	{
		int postfix : 2;
		int shift_right_left : 1;		// 1:Shift Right			0:Shift Left
		int shift_display_cursor : 1;	// 1:Shift Display			0:Move Cursor
		int prefix : 4;
	}settings2;

	struct
	{
		int display_shift_on : 1;		// 1:Display Shift On
		int increment_decrement : 1;	// 1:Increment,				0:Decrement
		int prefix : 6;
	}settings3;

	struct
	{
		int cursor_blink_on : 1;		// 1:Cursor Blink On
		int cursor_display_on : 1;		// 1:Cursor Display ON
		int display_on : 1;				// 1:Display On
		int prefix : 5;
	}settings4;

	enum InitState
	{
		START,
		SET_INTERFACE_TO_8BIT_1,
		SET_INTERFACE_TO_8BIT_2,
		SET_INTERFACE_TO_8BIT_3,
		SETTING1,
		DISPLAY_OFF,
		DISPLAY_CLEAR,
		SETTING2,
		SETTING3,
		SETTING4,
		MAX_INIT_DISP
	}init_state;

	enum OutState
	{
	    ROW_1_SET,
	    ROW_1_WRITE,
	    ROW_2_SET,
	    ROW_2_WRITE,
	    MAX_OUT
	}out_state;

	unsigned char outArray[DISP_LINES * DISP_LINE_LENGTH];
};

#endif /* DISP_H_ */
