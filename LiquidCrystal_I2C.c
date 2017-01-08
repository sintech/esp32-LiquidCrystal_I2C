#include <stdio.h>
#include "driver/i2c.h"
#include "LiquidCrystal_I2C.h"

#define I2C_MASTER_NUM I2C_NUM_1   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ    100000     /*!< I2C master clock frequency */


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//

LiquidCrystal_I2C_Def lcdi2c;

void i2c_master_init(gpio_num_t sda_pin, gpio_num_t scl_pin)
{
	i2c_port_t i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl_pin;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}


void LCDI2C_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows,uint8_t sda_pin, uint8_t scl_pin) //, uint8_t charsize) {
{
  lcdi2c.Addr = lcd_Addr;
  lcdi2c.cols = lcd_cols;
  lcdi2c.rows = lcd_rows;
  lcdi2c.backlightval = LCD_BACKLIGHT;

  i2c_master_init(sda_pin,scl_pin);
  LCDI2C_begin();
}

void LCDI2C_begin() {

   lcdi2c.displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (lcdi2c.rows > 1) {
		lcdi2c.displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
/*	if ((lcdi2c.charsize != 0) && (lines == 1)) {
		lcdi2c.displayfunction |= LCD_5x10DOTS;
	}*/

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	//Delay(50);
	vTaskDelay(50 / portTICK_RATE_MS);

	// Now we pull both RS and R/W low to begin commands
	LCDI2C_expanderWrite(lcdi2c.backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	//Delay(1000);
	vTaskDelay(1000 / portTICK_RATE_MS);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
   LCDI2C_write4bits(0x03 << 4);
   //DelayMC(4500); // wait min 4.1ms
   ets_delay_us(4500);

   // second try
   LCDI2C_write4bits(0x03 << 4);
   //DelayMC(4500); // wait min 4.1ms
   ets_delay_us(4500);

   // third go!
   LCDI2C_write4bits(0x03 << 4);
   //DelayMC(150);
   ets_delay_us(150);

   // finally, set to 4-bit interface
   LCDI2C_write4bits(0x02 << 4);


	// set # lines, font size, etc.
	LCDI2C_command(LCD_FUNCTIONSET | lcdi2c.displayfunction);

	// turn the display on with no cursor or blinking default
	lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LCDI2C_display();

	// clear it off
	LCDI2C_clear();

	// Initialize to default text direction (for roman languages)
	lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);

	LCDI2C_home();

}

/********** high level commands, for the user! */
void LCDI2C_clear(){
	LCDI2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	ets_delay_us(2000);
}

void LCDI2C_home(){
	LCDI2C_command(LCD_RETURNHOME);  // set cursor position to zero
	ets_delay_us(2000);
}

void LCDI2C_setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > lcdi2c.rows ) {
		row = lcdi2c.rows-1;    // we count rows starting w/0
	}
	LCDI2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDI2C_noDisplay() {
	lcdi2c.displaycontrol &= ~LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_display() {
	lcdi2c.displaycontrol |= LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turns the underline cursor on/off
void LCDI2C_noCursor() {
	lcdi2c.displaycontrol &= ~LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_cursor() {
	lcdi2c.displaycontrol |= LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turn on and off the blinking cursor
void LCDI2C_noBlink() {
	lcdi2c.displaycontrol &= ~LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_blink() {
	lcdi2c.displaycontrol |= LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDI2C_scrollDisplayLeft(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C_scrollDisplayRight(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDI2C_leftToRight(void) {
	lcdi2c.displaymode |= LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This is for text that flows Right to Left
void LCDI2C_rightToLeft(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'right justify' text from the cursor
void LCDI2C_autoscroll(void) {
	lcdi2c.displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'left justify' text from the cursor
void LCDI2C_noAutoscroll(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDI2C_createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	LCDI2C_command(LCD_SETCGRAMADDR | (location << 3));
	int i;
	for (i=0; i<8; i++) {
		LCDI2C_write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LCDI2C_noBacklight(void) {
	lcdi2c.backlightval=LCD_NOBACKLIGHT;
	LCDI2C_expanderWrite(0);
}

void LCDI2C_backlight(void) {
	lcdi2c.backlightval=LCD_BACKLIGHT;
	LCDI2C_expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

void LCDI2C_command(uint8_t value) {
	LCDI2C_send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void LCDI2C_send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
   LCDI2C_write4bits((highnib)|mode);
	LCDI2C_write4bits((lownib)|mode);
}

void LCDI2C_write4bits(uint8_t value) {
	LCDI2C_expanderWrite(value);
	LCDI2C_pulseEnable(value);
}

void LCDI2C_expanderWrite(uint8_t _data){
	i2c_port_t i2c_num=I2C_MASTER_NUM;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, lcdi2c.Addr << 1, 0);
	i2c_master_write_byte(cmd, (int) (_data) | lcdi2c.backlightval , 0);
	i2c_master_stop(cmd);
	int ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
}

void LCDI2C_pulseEnable(uint8_t _data){
	LCDI2C_expanderWrite(_data | En);	// En high
	ets_delay_us(1);		// enable pulse must be >450ns
	

	LCDI2C_expanderWrite(_data & ~En);	// En low
	ets_delay_us(50);		// commands need > 37us to settle
	
}


// Alias functions

void LCDI2C_cursor_on(){
	LCDI2C_cursor();
}

void LCDI2C_cursor_off(){
	LCDI2C_noCursor();
}

void LCDI2C_blink_on(){
	LCDI2C_blink();
}

void LCDI2C_blink_off(){
	LCDI2C_noBlink();
}

void LCDI2C_load_custom_character(uint8_t char_num, uint8_t *rows){
		LCDI2C_createChar(char_num, rows);
}

void LCDI2C_setBacklight(uint8_t new_val){
	if(new_val){
		LCDI2C_backlight();		// turn backlight on
	}else{
		LCDI2C_noBacklight();		// turn backlight off
	}
}

void LCDI2C_write(uint8_t value){
	LCDI2C_send(value, Rs);
}

void LCDI2C_print(char* str) {
  uint8_t i=0;
  while(str[i])
  {
    LCDI2C_write(str[i]);
    i++;
  }
}


