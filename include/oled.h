#include <stdio.h>

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
		int draw_line(int x1, int y1, int x2, int y2);
		int draw_rectangle(int x1, int y1, int x2, int y2);
		int draw_letter(char letter[8]);
		int write_text(char text[16]);
};