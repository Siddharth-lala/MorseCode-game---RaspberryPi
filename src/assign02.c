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

//Defination of alphabets in morse code
char *morse[36] = {
".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", //A-Z
"-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
"-----",".----","..---","...--","....-",".....","-....","--...","---..","----." //Numbers 0-9
};

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

//Code for Morse code converter
char* characater_to_morse(char character){
    //For numbers
    if (character >= 48 && character <= 57){
        return morse[character- 48+26];
    }
    //For A-Z
    if (character >=65 && character <=90){
        return morse[character-65]
    }
    //For a-z, keeping the same morse code for both cases of characters
    if (character >= 97 && character <= 122){
        return morse[character - 97];
    }


}

//Code for morse code to character conversion
char morse_to_character(char* morse){
    //TODO
    return 'a'
}


/*
 * Main entry point for the code - simply calls the main assembly function.
 */
int main() {
    main_asm();
    return(0);
}
