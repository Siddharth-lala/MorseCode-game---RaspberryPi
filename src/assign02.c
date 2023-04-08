
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
//#include "ws2812.pio.h"
#include "hardware/watchdog.h"
#include "string.h"

#define CHECK_RGBW true 
#define NO_PIXELS 1
#define WS2812_PIN 28 



//Defination of alphabets in morse code
//1: .
//2: -
char *morse[36] = {
"12", "2111", "2121", "211", "1", "1121", "221", "1111", "11", "1222", "212", "1211", "22", //A2Z
"21", "222", "1221", "2212", "121", "111", "2", "112", "1112", "122", "2112", "2122", "2211",
"22222","12222","11222","11122","11112","11111","21111","22111","22211","22221" //Numbers 0-9
};
char *letters[37]={
"A","B","C","D","E","F","G","H","I","J","K","L","M"
"N","O","P","Q","R","S","T","U","V","W","X","Y","Z"
"0","1","2","3","4","5","6","7","8","9","?"
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
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

/**
 * @brief generate a 32-bit int composit RGB value by combining individual 8-bit params
 *
 * @param r
 * @param g
 * @param b
 * @return uint32_t
 */

/**
 * @brief ensures that the first 8 bits affect the red of the LED, middle 8 the green and last on blue
 *
 * @param r
 * @param g
 * @param b
 * @return uint32_t
 */
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b);
}

/**
 * @brief fetches and stores the current time
 *
 */



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

void life_indicator(int lives)
{
    if (lives == 3)
    {
        put_pixel(urgb_u32(0x00, 0x2F, 0x00)); // green
    }
    else if (lives == 2)
    {
       put_pixel(urgb_u32(0x00, 0x00, 0x2F));// blue
        
    }
    else if (lives == 1)
    {
      put_pixel(urgb_u32(0x2F, 0x2F, 0x00)); // yellow
       
    }
    else if (lives == 0)
    {
        put_pixel(urgb_u32(0x2F, 0x00, 0x00)); // red
    }
    else
       put_pixel(urgb_u32(0x2F, 0xC, 0x00)); // orange
      
}


//Code for Morse code converter
char* characater_to_morse(char character){
    //For numbers
    if (character >= 48 && character <= 57){
        return morse[character- 48+26];
    }
    //For A-Z
    if (character >=65 && character <=90){
        return morse[character-65];
    }
    //For a-z, keeping the same morse code for both cases of characters
    if (character >= 97 && character <= 122){
        return morse[character - 97];
    }


}

//Code for morse code to character conversion
char morse_to_character(char* morse_in){
    int i;
    //Iterating through morse codes to check for the value
    for(i=0; i<36;i++){
        if (strcmp(morse_in,morse[i])==0) break;
    }   
    return letters[i];   //It contains the corresponding letter for the given morse input
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



void level_1() {

  int value = (rand() % 36);
  int lives = 3;
  int correct = 0;
  char input;
  char given_char = letters[value];

  
  int total_correct = 0;
  int total_incorrect = 0;
  int total_ans = 0;
  
  printf("You have %d lives for this level\n", lives);
  life_indicator(lives);
  
  while (lives != 0 && correct_ans != 5) {
    printf("Enter Morse code for - %c( Hint: %s)\n", given_char,characater_to_morse(given_char));
    input = getchar();
    printf("You have entered \"%c\" which can be decoded to \"%c\"\n", input,morse_to_character(input));
    total_ans++;
   // final_total_ans++;
    if (morse_to_character(input) == given_char) {
      printf("Correct Answer!\n");
      correct++;
      total_correct++;
      // final_total_correct++;
      if (correct == 1)
        printf("You have answered 1 correct question!\n");
      else if (correct > 1)
        printf("You have answered %d correct questions in a row!\n",
               correct);
      if (lives < 3) {
        lives++;
        life_indicator(lives);
        printf("Life has been added, %d lives are remaining!\n\n", lives);
      } else
        printf("\n");
      given_char = letters[value];

    } else if (lives > 0) {
      printf("Incorrect Answer!\n");
      lives--;
      life_indicator(lives);
      total_incorrect++;
      // final_total_incorrect++;
      if (lives > 1)
        printf("%d lives are remaining!\n\n", lives);
      else if (lives == 1)
        printf("%d life is remaining!\n\n", lives);
      correct = 0;
    }
  }
  if (correct == 5) {
    printf("Congratulations! You progressed to level 2!\n\n");
    printf("Stats for Level 1:\n");
    printf("Total Answers: %f\n", total_ans);
    printf("Total Correct Answer: %d\n", total_correct);
    printf("Total Incorrect Answer: %d/n", total_incorrect);
    printf("Now starting Level 2...\n\n");
    level_2();
  } else if (lives == 0) {
    printf("No lives remaining.\n");
    printf("Game Finished...\n\n");
    printf("Do you want to play again?   \"-.--\" - Y\n");
    printf("                                \"-.\"   - N\n");
    input = getchar();
    if (morse_to_character(input) == 'Y') {
      printf("Restarting game...\n");
      welcome_screen();
    } else {
      printf("Thank you for playing!\n");
    }
  }
}



void level_2() {

  int value = (rand() % 36);
  int lives = 3;
  int correct = 0;
  char input;
  char given_char = letters[value];

  
  int total_correct = 0;
  int total_incorrect = 0;
  int total_ans = 0;
  
  printf("You have %d lives for this level\n", lives);
  life_indicator(lives);
  
  while (lives != 0 && correct_ans != 5) {
    printf("Enter Morse code for - %c\n", given_char);
    input = getchar();
    printf("You have entered \"%c\" which can be decoded to \"%c\"\n", input,morse_to_character(input));
    total_ans++;
   // final_total_ans++;
    if (morse_to_character(input) == given_char) {
      printf("Correct Answer!\n");
      correct++;
      total_correct++;
      // final_total_correct++;
      if (correct == 1)
        printf("You have answered 1 correct question!\n");
      else if (correct > 1)
        printf("You have answered %d correct questions in a row!\n",
               correct);
      if (lives < 3) {
        lives++;
        life_indicator(lives);
        printf("Life has been added, %d lives are remaining!\n\n", lives);
      } else
        printf("\n");
      given_char = letters[value];

    } else if (lives > 0) {
      printf("Incorrect Answer!\n");
      lives--;
      life_indicator(lives);
      total_incorrect++;
      // final_total_incorrect++;
      if (lives > 1)
        printf("%d lives are remaining!\n\n", lives);
      else if (lives == 1)
        printf("%d life is remaining!\n\n", lives);
      correct = 0;
    }
  }
  if (correct == 5) {
    printf("Congratulations! You have completed level 2\n\n");
    printf("Statistics for Level 02 are:\n");
    printf("Total Answers: %d\n",total_ans);
    printf("Total Correct Answer: %d\n",total_correct);
    printf("Total Incorrect Answer: %d/n", total_incorrect);
  } else if (lives == 0) {
    printf("No lives remaining.\n");
    printf("Game Finished...\n\n");
    printf("Do you want to play again?   \"-.--\" - Y\n");
    printf("                                \"-.\"   - N\n");
    input = getchar();
    if (morse_to_character(input) == 'Y') {
      printf("Restarting game...\n");
      welcome_screen();
    } else {
      printf("Thank you for playing!\n");
    }
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

/*
 * Main entry point for the code - simply calls the main assembly function.
 */

int main() {
    main_asm();
    printf("=====================\n");
    printf("| LEARN MORSE CODE  |\n");
    printf("|Created By Group 33|\n ");
    printf("=====================\n");
    printf("\nInstrunctions:\n");
    printf("-Choose the difficulty from the given options:\n")
    printf("\tLevel 1: Morse code will be provided\n")
    printf("\tLevel 2: Morse coed will not be provided\n")
    printf("-A character will be printed on screen and you will have to input the correct sequence\n")
    printf("\tPress GP21 for short duration to register a dot\n")
    printf("\tPress GP21 for long duration to register a dash\n")
    printf("You have 3 lives and you lose 1 life for every incorrect answer and gain 1 life for every correct answer(max. 3 lives)\n")
    printf("Enter 5 fully correct sequences in a row to advance to the next level\n")
    
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);
  
    stageInput();
    return(0);
    
}
