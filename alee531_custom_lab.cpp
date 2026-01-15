#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "i2c.h"
#include "serialATmega.h"
#include "LCD.h"
#include "frames.h"
#include <stdlib.h> 
#include <stdio.h>

#define NUM_TASKS 5 //TODO: Change to the number of tasks being used
#define TASK1_TIME 80
#define TASK2_TIME 100
#define TASK3_TIME 50
#define TASK4_TIME 8
#define TASK5_TIME 500



//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
}task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 2;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Declare your tasks' function and their states here

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

int dino_width = 32; // 32
int dino_height = 16; // 16
int cactus_width = 16; // 16
int cactus_height = 8; // 8

const int dino_x = 10;  
int cac_x = 127;


bool end = false;


void clear_Display() {
    for (int row = 0; row < 8; row++) {
        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x00);      
        i2c_write(0xB0 + row); 
        i2c_write(0x00);        
        i2c_write(0x10);        
        i2c_stop();

        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x40);      

        for (int col = 0; col < 128; col++) {
            i2c_write(0x00);
        }
        i2c_stop();
    }
    

}

int pixel = 0;
void display_dino(const int* dino, int position) {
    int rows = 2; // 16 / 8
    for (int row = 0; row < rows; row++) {
        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x00);     
        i2c_write(0xB0 + row + position); 
        i2c_write(0x00);      
        i2c_write(0x10);      
        i2c_stop();

        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x40);      

        for (int col = 0; col < 32; col++) {
            pixel = pgm_read_byte(&dino[row * 32 + col]);
            i2c_write(pixel);
        }
        i2c_stop();
    }
}


int c_pixel = 0;
void display_cactus(const int* c, int position, int col) {
    for (int row = 0; row < 1; row++) {
        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x00);      
        i2c_write(0xB0 + row + position);
        i2c_write(0x00 | (col & 0x0F)); 
        i2c_write(0x10 | ((col >> 4) & 0x0F));

        i2c_stop();

        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x40);      
        for (int col = 0; col < 16; col++) {
            pixel = pgm_read_byte(&c[row * 16 + col]);
            i2c_write(pixel);
        }
        i2c_stop();
    }
}



int s_pixel = 0;
void display_screen(const int* screen) {
    int rows = 8; // 16 / 8
    for (int row = 0; row < rows; row++) {
        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x00);     
        i2c_write(0xB0 + row); 
        i2c_write(0x00);      
        i2c_write(0x10);      
        i2c_stop();

        i2c_start();
        i2c_write(0x3C << 1); 
        i2c_write(0x40);      
        for (int col = 0; col < 128; col++) {
            pixel = pgm_read_byte(&screen[row * 128 + col]);
            i2c_write(pixel);
        }
        i2c_stop();
    }
}

int hop[] = {
    523, 659, 784
};

int die[] = {
    784, 523, 392
};


int i = 0;
unsigned char button = 0;
bool is_right = false;
bool on = false;
bool start = true;
int j = 0;
char buffer[16];



enum Buzzer_States {SOUND_ON, SOUND_OFF};
int TickFct_Buzzer(int state) {
    if (end) {
  
        if (j < 3) {
            ICR1 = (16000000 / (8 * die[j])) - 1; 
            OCR1A = ICR1 / 200; 
            j++; 
        } else {
            OCR1A = ICR1; 
        }
    } 
    
    else if (on) {
    
        if (i < 3) {
            ICR1 = (16000000 / (8 * hop[i])) - 1; 
            OCR1A = ICR1 / 200; 
            i++; 
        } else {
            OCR1A = ICR1; 
        }
    } else {
       
        i = 0;
        j = 0;
        OCR1A = ICR1; 
    }

    
            
    return state;
}

int count = 0;
int jump_count = 0;
bool in_air = false;
unsigned long score = 0;

bool collision_detected = false;
void check_collision() {

    int margin_x = 6; 
    int dino_left = dino_x + margin_x;
    int dino_right = dino_x + dino_width - margin_x;


    
    int cactus_left = cac_x + margin_x;
    int cactus_right = cac_x + cactus_width - margin_x;
   
    
    
    if ((dino_right >= cactus_left) && (dino_left <= cactus_right ) && (!in_air)) {
        collision_detected = true;
    } else {
        collision_detected = false;
    }

    
}



enum Dino_States { RUN };
int TickFct_Dino(int state) {


    if (start || end) {
        return state;
    }

    check_collision(); 

    if (collision_detected) {
        end = true;
        serial_println("Collision Detected! Game Over.");
    }


    if ((on) || (in_air)) {

        if (count < 1) {
            clear_Display();
            display_dino(dino_left, 6);
            // serial_println("....");
        }

        else {  
            if (!in_air) { 
                in_air = true;
                clear_Display();
                display_dino(reg, 1); 
            }
        }

        if (in_air) {
            if (jump_count < 5) { 
                jump_count++;
            }
            else {
                on = false;
                jump_count = 0;
                clear_Display();
                // serial_println("done being in air");
                in_air = false;
                
            }

            // serial_println(jump_count);
        }

        count++;
        if (!start && !end) {
            score++;
        }
        

        // serial_println(score);
        

    }



    else  {
        count = 0;
        if (is_right == true) {
            display_dino(dino_right, 6);
            is_right = false;
        }
            
        else {
            display_dino(dino_left, 6);
            is_right = true;
        }

    }
    

    
    
    return state;
}



enum Button_States { OFF, ON_PRESS, ON_RELEASE, GAME_OVER };
int TickFct_Button(int state) {

    unsigned char jump_button = (PINC >> 1) & 0x01;
    
    
   

    if (end) {
        display_screen(end_screen);
        if (jump_button) {
            state = GAME_OVER;
        }
       
        // serial_println(end);
        
    }

    switch(state) {
        case OFF:  

            if (start) {
                score = 0;
                display_screen(start_screen);
        
            }

            if (jump_button && !on) {
                on = true;
                state = ON_PRESS;
                start = false;
            }



            break;

        case ON_PRESS:

            

            if (!jump_button) {
                state = ON_RELEASE;
            }
            break;

        case ON_RELEASE:
            state = OFF;

            break;

        

        
        case GAME_OVER:
            if (jump_button) {
                start = true;
                end = false;
                on = false;

                state = OFF;
            
            }

        default:
            break;


    }

    return state;
}




enum Cactus_States { WAIT, MOVE};
int cactus_delay = 81; 
int x = 0; 

int TickFct_Cactus(int state) {
    switch (state) {
        case WAIT:
            if (!start && cactus_delay > 0) { 
                cactus_delay--;
                
            } else if (cactus_delay == 0) {
                state = MOVE;
                cactus_delay = 81;
            }


            break;

        case MOVE:
            // Move the cactus
            serial_println(x);
            if (!end) {

                if (x >= 3) {
                    display_cactus(cactus, 7, cac_x);

                    cac_x--;

                    

                    if (cac_x < -16) { 
                        cac_x = 127;
                    }

                }

                x++;

            }

            else {
                x = 0;
             
            }



            

            
            
            break;

        default:
            state = WAIT;
            break;
    }

    

    return state;
}


enum Display_States { ON };
int TickFct_Display(int state) {
    lcd_goto_xy(0, 0);

    if (start) {
        lcd_write_str((char*)"Start the Game"); 
        lcd_goto_xy(1, 0);
        lcd_write_str(" :) "); 
        return state;
    }


    lcd_write_str((char*)"Score: "); 
    sprintf(buffer, "%lu          ", score); 
    lcd_write_str(buffer);


    return state;
}


int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRB = 0xFF ; PORTB = 0x00;
    DDRC = 0x00;  PORTC = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;

    

    ADC_init();   // initializes ADC
    i2c_init(); 
    
    
    serial_init(9600);


    

    
    i2c_start();
    i2c_write(0x3C << 1); // address
    i2c_write(0x00);      // write commands
    i2c_write(0xAE); // off
    i2c_write(0x8D); 
    i2c_write(0x14); 
    i2c_write(0xAF); // on
    i2c_stop();

    clear_Display();


    TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the p

    
    
    lcd_init(); // Initialize the LCD screen
    
    lcd_clear();

    


    
    
    //TODO: Initialize the buzzer timer/pwm(timer0)

    //TODO: Initialize the servo timer/pwm(timer1)


    //TODO: Initialize tasks here
    // e.g. 
    tasks[0].period = TASK1_TIME;
    tasks[0].state = RUN;
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &TickFct_Dino;

    tasks[1].period = TASK2_TIME;
    tasks[1].state = OFF;
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &TickFct_Button;

    tasks[2].period = TASK3_TIME;
    tasks[2].state = SOUND_OFF;
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &TickFct_Buzzer;

    tasks[3].period = TASK4_TIME;
    tasks[3].state = WAIT;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &TickFct_Cactus;

    tasks[4].period = TASK5_TIME;
    tasks[4].state = ON;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &TickFct_Display;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}