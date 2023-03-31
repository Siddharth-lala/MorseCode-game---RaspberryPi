#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
//#include "ws2812.pio.h"
#include "hardware/watchdog.h"

#define CHECK_RGBW true 
#define NO_PIXELS 1
#define WS2812_PIN 28 
// Must declare the main assembly entry point before use.
void main_asm();
// GPIO INTIALIZATION

void asm_gpio_init(uint pin)
{
    gpio_init(pin);
}

void asm_gpio_init(uint pin)
{
    gpio_init(pin);
}

bool asm_gpio_get(uint pin)
{
    return gpio_get(pin);
}

void asm_gpio_put(uint pin, bool value)
{
    gpio_put(pin,value);
}
void asm_gpio_set_irq(uint pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL,true);
     gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE,true);

}

// WATCHDOG initalization

void watchdog_init()
{
    if(watchdog_caused_reboot())
    {
        printf(" No input was detected for timeout. Rebooted by watchdog...\n");
    }

    if(watchdog_enable_caused_reboot())
    {
        printf("Chip was rebooted due to watchdog enable\n");
    }
    watchdog_enable(0x7fffff,1);
    watchdog_update();
}
// variables used in more then one function
char arrayInput[20];
int inputLength = 0;
int inputComplete = 0;


// Initalize the global input array
void initInputArray()
{
    int maxsize = 20;
    for (int j = 0; j < maxsize; j++)
    {
        arrayInput[j] = NULL;
    }
    inputComplete = 0;
    inputLength = 0;
}

// check input dot, dash, enter, finish
void add_to_input(int input)
{
    if (input == 1){                                   // dot
        arrayInput[inputLength] = '-';  // add '.' to sequence
        inputLength++;
        printf(".");
    }
    else if (input == 2){                           // dash
        arrayInput[inputLength] = '.'; // add '-' to sequence
        inputLength++;
        printf("-");
    }
    else if (input == 3){                           // enter
        arrayInput[inputLength-1] = NULL;  // 4 is " "
        inputComplete = 1;  // finished their attempt;
    }
    else if (input == 4){           // space
        arrayInput[inputLength] = ' '; // add ' ' to sequence
        inputLength++;
        printf(" ");
    }
}


//timestamp ms
int find_time() {
    absolute_time_t time = get_absolute_time();
    return to_ms_since_boot(time);
    }

// time difference in ms
     int time_difference(int end_time, int start_time) {
        return (end_time - start_time);
     } 
/* -------------------------------------------- Main function -------------------------------------------- */

int main()
{
    main_asm();
    return(0);
}