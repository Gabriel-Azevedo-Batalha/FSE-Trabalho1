
/*
*
* by Lewis Loflin www.bristolwatch.com lewis@bvu.net
* http://www.bristolwatch.com/rpi/i2clcd.htm
* Using wiringPi by Gordon Henderson
*
*
* Port over lcd_i2c.py to C and added improvements.
* Supports 16x2 and 20x4 screens.
* This was to learn now the I2C lcd displays operate.
* There is no warrenty of any kind use at your own risk.
*
*/

#include "lcd.h"

int fd;  // seen by all subroutines

// float to string
void typeFloat(float myFloat)   {
  char buffer[20];
  sprintf(buffer, "%4.1f",  myFloat);
  typeln(buffer);
}

// int to string
void typeInt(int i)   {
  char array1[20];
  sprintf(array1, "%d",  i);
  typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void)   {
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)   {

  lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char *s)   {

  while ( *s ) lcd_byte(*(s++), LCD_CHR);

}

void lcd_byte(int bits, int mode)   {

  //Send byte to data pins
  // bits = the data
  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)   {
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}


void lcd_init()   {

	if (wiringPiSetup () == -1) exit (1);
	fd = wiringPiI2CSetup(I2C_ADDR);

	// Initialise display
  	lcd_byte(0x33, LCD_CMD); // Initialise
	lcd_byte(0x32, LCD_CMD); // Initialise
	lcd_byte(0x06, LCD_CMD); // Cursor move direction
	lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
	lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
	lcd_byte(0x01, LCD_CMD); // Clear display
	delayMicroseconds(500);
}

void lcdWriteTemperatures(char on, char mode, float tr, float te, float ti){
	ClrLcd();
	lcdLoc(LINE1);
	if(on){
		if(mode == 0)
			typeln("UART");
		else if (mode == 1)
			typeln("CURVA");
		else
			typeln("TERM");
		typeln(" TR:");
		typeFloat(tr);
		lcdLoc(LINE2);
		typeln("TE:");
		typeFloat(te);
		typeln(" TI:");
		typeFloat(ti);
	}
	else{
		typeln("Sistema");
		lcdLoc(LINE2);
		typeln("Desligado");
	}
}
