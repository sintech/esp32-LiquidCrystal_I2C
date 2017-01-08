#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <string.h>

#include "LiquidCrystal_I2C.h"

#define LCD_ADDR  0x3f  /*!< slave address for LCD */
#define I2C_MASTER_SCL_IO    19    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO    18    /*!< gpio number for I2C master data  */


void app_main(void)
{
    nvs_flash_init();
	 LCDI2C_init(LCD_ADDR,16,2,I2C_MASTER_SDA_IO,I2C_MASTER_SCL_IO);
	
	// custom symbols
	 uint8_t home[8]    = {0x03, 0x04, 0x0E, 0x0E, 0x1F, 0x0A, 0x0e, 0x0e};
	 uint8_t thing[8]    = {0x00, 0x0e, 0x11, 0x1c, 0x12, 0x19, 0x14, 0x12};
    LCDI2C_createChar(0, home); 
	 LCDI2C_createChar(1, thing);
	 LCDI2C_clear();
	
	// display text
	LCDI2C_print("Hello ESP32!");
	vTaskDelay(2000 / portTICK_RATE_MS);
	
	// display custom symbols
	LCDI2C_setCursor(0,1);
	LCDI2C_write(0);
	LCDI2C_write(0);
	LCDI2C_write(0);
	LCDI2C_print(" ");
	LCDI2C_write(1);
	LCDI2C_write(1);
	LCDI2C_write(1);
	vTaskDelay(2000 / portTICK_RATE_MS);

	// display dynamic text
	//LCDI2C_clear();
	char str[3];
	for(int i=1; i<=100; i++) {
		LCDI2C_setCursor(8,1);
		LCDI2C_print("Test:");
		sprintf(str,"%d",i);
		LCDI2C_print(str);
		vTaskDelay(100 / portTICK_RATE_MS);
	}

}

