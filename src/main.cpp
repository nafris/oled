#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "ssd1336.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "font.h"
#define tag "SSD1306"


extern "C"{
    void app_main();
}

int get_y8(int y64){
	return y64 / 8;
}

int get_b8(int y64){
	return y64 % 8;
}
int get_bit(unsigned char data, int bit){
	data = data >> bit;
	return data % 2;
}

class OLED {
    private:
		int I2C_ADDR;
		bool init_done;
		uint8_t buffer[128][8];
		uint8_t cursorX = 0;
		uint8_t cursorY = 0;
		
    public:
        int init(int SCL_PIN, int SDA_PIN, int I2C_ADDR);
		int clear(int I2C_ADDR);
		int clear_buffer(void);
		int line(int x1, int y1, int x2, int y2);
		int refresh(void);
		int print_buffer(void); // prints to terminal
		int pixel(int x, int y);
		int draw_h_line(int x1, int x2, int y);
		int draw_v_line(int y1, int y2, int x);
		int draw_rectangle(int x1, int y1, int x2, int y2);
		int draw_letter(char letter[8]);
		int write_text(char text[16]);
};

int OLED::init(int SCL_PIN, int SDA_PIN, int I2C_ADDR){
	this->I2C_ADDR = I2C_ADDR;
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master = {
			.clk_speed = 1000000,
		},
		.clk_flags = 0		
	};

	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0); 

   	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);

	vTaskDelay(1000);

	if (espRc == ESP_OK) {
		ESP_LOGI(tag, "OLED configured successfully");
        printf("OLED configuration done.\n");
		this->init_done = true;
	} else {
		ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
		printf("OLED configuration failed \n");
		this->init_done = false;
	}

    i2c_cmd_link_delete(cmd);
    return 1;   
}

int OLED::clear(int I2C_ADDR){
	if(!this->init_done){
		return -1;
	}
	i2c_cmd_handle_t cmd;

	uint8_t zero[128];
    for (uint8_t i = 0; i < 128; i++) zero[i] = 0;
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
	return 1;
}

int OLED::clear_buffer(){
	for(int i = 0; i < 128; i++){
		for(int j = 0; j < 8; j++){
			//change this to memset in future
			this->buffer[i][j] = 0;
		}
	}
	return 1;
}

int OLED::print_buffer(){
	//not working correctly yet
    for (int y = 0; y < 8; y++){
        for (int b = 0; b < 8; b++){
            for(int x = 0; x < 128; x++){
                //printf("%d", get_bit(buffer[x][y], b));
				if(get_bit(this->buffer[x][y], b)){
					printf("*");
				}else{
					printf(" ");
				}
            }
            printf("\n");
        }
    }
	return 1;	
}

int OLED::refresh(){
	if(!this->init_done){
		return -1;
	}
	i2c_cmd_handle_t cmd;
	
	unsigned char t[128];
	for (uint8_t i = 0; i < 8; i++) {
		for (int j = 0; j < 128; j++){
			t[j] = this->buffer[j][i];
		}
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, t, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
	return 1;
}

int OLED::line(int x1, int y1, int x2, int y2){

	return 1;
}

int OLED::pixel(int x, int y){
	//need to check for init done here
	
	int y8 = get_y8(y);
	int b8 = get_b8(y);
	this->buffer[x][y8] |= 1 <<b8;
	return 1;
}

int OLED::draw_h_line(int x1, int x2, int y){
	//need to add fix if x2 < x1
	int y8 = get_y8(y);
	int b8 = get_b8(y);
	for(int i = x1; i <= x2; i++){
		this->buffer[i][y8] |= 1 << b8;
	}
	return 1;	
}
int OLED::draw_v_line(int y1, int y2, int x){
	//need to add fix if y2 < y1
	int y8;
	int b8;
	for (int i = y1; i <= y2; i++){
		y8 = get_y8(i);
		b8 = get_b8(i);
		buffer[x][y8] |= 1 << b8;
	}
	return 1;
}
int OLED::draw_rectangle(int x1,int y1,int x2, int y2){
	this->draw_h_line(x1, x2, y1);
	this->draw_h_line(x1, x2, y2);
	this->draw_v_line(y1, y2, x1);
	this->draw_v_line(y1, y2, x2);
	return 1;
}

int OLED::draw_letter(char letter[8]){
	for (int i = 0; i < 8; i++){
		this->buffer[cursorX * 8 + i][cursorY] = letter[i];
	}
	cursorX++;
	return 1;
}
int OLED::write_text(char text[16]){
	for (int i = 0; i < strlen(text); i++){
		//this->draw_letter(font[text[i]]);
		this->draw_letter(font[40]);
	}
	return 1;
}

void app_main() {
    OLED ekrans;
    ekrans.init(23, 22, 0x3c);
	vTaskDelay(200);
	ekrans.clear(0x3c);
	ekrans.line(10, 10, 10, 50);
	ekrans.clear_buffer();
	ekrans.pixel(10, 10);
	ekrans.pixel(127, 63);
	ekrans.draw_rectangle(5, 10, 20, 40);
	ekrans.print_buffer();
	ekrans.refresh();
    while(1){
        vTaskDelay(1000);
		printf("New Line \n");
    }
}
