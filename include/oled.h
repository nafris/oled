#include <stdio.h>

class OLED {
    private:
		int I2C_ADDR;
		bool init_done = false;
		uint8_t buffer[128][8];
		uint8_t cursorX = 0;
		uint8_t cursorY = 0;
		
    public:
        int init(int SCL_PIN, int SDA_PIN, int ADDR);
		int clear(void);
		int clear_buffer(void);
		int refresh(void);
		int print_buffer(void); // prints to terminal
		int pixel(int x, int y, bool colour);
		int draw_h_line(int x1, int x2, int y, bool colour);
		int draw_v_line(int y1, int y2, int x, bool colour);
		int draw_line(int x1, int y1, int x2, int y2, bool colour);
		int draw_rectangle(int x1, int y1, int x2, int y2, bool colour);
		int draw_letter(char letter[8]);
		int write_text(char text[16]);
};