#ifndef DISP_H_
#define DISP_H_

#include "GPIO.h"

#define PREFIX 1
#define POSTFIX 0

enum regSel
{
	CMD = 0,
	DATA = 1
};

class Disp
{
public:
	Disp();
	Disp(int port[11]);
	/* RS, RW, E, DB[0..7] */
	~Disp();

	void process();	// State machine

	void displayClear();
	void displayCursorHome();
	void entryModeSet(bool ID, bool S);
	void displayOnOff(bool D, bool C, bool B);
	void displayCursorShift();
	void functionSet(bool DL, bool N, bool F);

private:
	void write(char data, regSel rs=DATA);
	void read(char& data, regSel rs=DATA);
	bool isBusy();
	void initDisplay();

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
		int display_shift_on : 1;		// 1:Display Shift On
		int increment_decrement : 1;	// 1:Increment,				0:Decrement
		int prefix : 6;
	}settings1;

	struct
	{
		int cursor_blink_on : 1;		// 1:Cursor Blink On
		int cursor_display_on : 1;		// 1:Cursor Display ON
		int display_on : 1;				// 1:Display On
		int prefix : 5;
	}settings2;

	struct
	{
		int postfix : 2;
		int shift_right_left : 1;		// 1:Shift Right			0:Shift Left
		int shift_display_cursor : 1;	// 1:Shift Display			0:Move Cursor
		int prefix : 4;
	}settings3;

	struct
	{
		int postfix : 2;
		int _5x10_5x8_dots : 1;			// 1:5x10 Dots				0:5x8 Dots
		int dual_single_line : 1;		// 1:Dual Line				0:Single Line
		int _8_4_bit : 1;				// 1:8-Bit					0:4-Bit
		int prefix : 3;
	}settings4;

	enum InitState
	{
		START,
		SETTING1,
		SETTING2,
		SETTING3,
		SETTING4,
		MAX_INIT_DISP
	};

    InitState init_state;

};

#endif /* DISP_H_ */
