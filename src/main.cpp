#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "ssd1336.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "oled.h"
#define tag "SSD1306"

extern "C"{
    void app_main();
}

void app_main() {
    OLED ekrans;
    ekrans.init(23, 22, 0x3c);
	vTaskDelay(200);
	ekrans.clear();
	ekrans.draw_line(10, 10, 10, 50);
	ekrans.clear_buffer();
	//ekrans.pixel(10, 10);
	//ekrans.pixel(127, 63);
	//ekrans.draw_rectangle(5, 10, 20, 40);
	ekrans.write_text("Chemtrails...");
	ekrans.draw_line(10, 30, 60, 60);
	ekrans.print_buffer();
	ekrans.refresh();
    while(1){
        vTaskDelay(1000);
		printf("New Line \n");
    }
}
