void main_asm();
void input();
char *collectArray();

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

int stage = 1;                      
int targetIndex;                 
PIO pio = pio0;                               
int correct, totalCorrect;  
int gameover = 0;         
int health, livesLost;

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
}


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
char morse_to_character(char* morse_in){
    
    //Iterating through morse codes to check for the value
    for(int i=0; i<36;i++){
        if (strcmp(morse_in,morse[i])==0) break;
    }   
    return letters[i]   //It contains the corresponding letter for the given morse input
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



void stageInput()
{
    printf("Please enter the level you would like to attempt:\n");
    printf("Enter: '.----' for level 1\n");
    printf("Enter: '..---' for level 2\n");
    input();
    watchdog_update();

    char *stageInput = collectArray();
    char morsereturn[6];
    for (int i = 0; i < 6; i++)
    {
        morsereturn[i] = *(stageInput + (i * 4));
    }
    char *string = morsereturn;

    if (strcmp(string, morse[1]) == 0)
    {
        stage = 1;
    }
    if (strcmp(string, morse[2]) == 0)
    {
        stage = 2;
    }
}

int modifyArray(char *input)
{
    char morsereturn[6];
    for (int i = 0; i < 6; i++)
    {
        morsereturn[i] = *(input + (i * 4));
    }
    char *string = morsereturn;
    int indexOfInput = morseSearch(string);
    if (indexOfInput == -1)
    {
        printf("Not found: ?\n");
        return 0;
    }
    if (targetIndex == indexOfInput)
    {
        printf("You entered: %s\n", letters[indexOfInput]);
        return 1;
    }
    else
    {
        printf("You entered: %s\n", letters[indexOfInput]);
        return 0;
    }
}

int checkAccuracy()
{
    int correct = modifyArray(collectArray());
    if (correct == 1)
    {
        printf("It is correct!!\n");
        return 1;
    }
    else
    {
        printf("It is incorrect!!\n");
        return 0;
    }
}

int stageOne()
{
    printf("Welcome to Level One!\n");
    health = 3;
    correct = 0;
    while (1)
    {
        targetIndex = rand() % 36;
        printf("Please enter the following Character: %s, (Hint the Morse is : %s)\n", letters[targetIndex], morse[targetIndex]);
       

       input();
        watchdog_update();
        if (checkAccuracy() == 1)
        {
            correct;
            totalCorrect++;
            if (health < 3)
            {
                health++;
            }

            if (correct == 5)
            {
                return 1;
            }
        }
        else
        {
            health--;
            livesLost++;

            if (health == 0)
            {
                return -1;
            }
        }
    }
}

void level_2() {

  int value = (rand() % 37);
  int lives = 3;
  int correct_ans = 0;
  char input;
  char given_char = letters[value];

  // For stats
  float total_ans = 0;
  int total_correct_ans = 0;
  int total_incorrect_ans = 0;

  printf("%d lives remaining!\n", lives);

  while (lives != 0 && correct_ans != 5) {
    printf("Enter Morse code for: %c\n", given_char);
    input = getchar();
    getchar();
    printf("You entered \"%c\" which decodes to \"%c\"\n", input,
           morse_to_character(input));
    total_ans++;
    total_ans++;
    if (morse_to_character(input) == given_char) {
      printf("Correct Answer!\n");
      correct_ans++;
      total_correct_ans++;
      // final_total_correct_ans++;
      if (correct_ans == 1)
        printf("You have answered 1 correct question!\n");
      else if (correct_ans > 1)
        printf("You have answered %d correct questions in a row!\n",
               correct_ans);
      if (lives < 3) {
        lives++;
        printf("Life added, %d lives remaining!\n\n", lives);
      } else
        printf("\n");
      given_char = letters[value];

    } else if (lives > 0) {
      printf("Incorrect Answer!\n");
      lives--;
      total_incorrect_ans++;
      // final_total_incorrect_ans++;
      if (lives > 1)
        printf("%d lives remaining!\n\n", lives);
      else if (lives == 1)
        printf("%d life remaining!\n\n", lives);
      correct_ans = 0;
    }
  }
  if (correct_ans == 5) {
    printf("Congratulations! You have completed level 2\n\n");
    printf("Stats for Level 02 are:\n");
    printf("Total Answers: %d\n",total_ans);
    printf("Total Correct Answer: %d\n",total_correct_ans);
    printf("Total Incorrect Answer: %d/n", total_incorrect_ans);
  } else if (lives == 0) {
    printf("No lives remaining.\n");
    printf("Game Finished...\n\n");
    // printf("Final stats: \n");
    // print_stats(final_total_ans, final_total_correct_ans,
    // final_total_incorrect_ans);
    printf("Would you like to play again?   \"-.--\" - Y\n");
    printf("                                \"-.\"   - N\n");
    input = getchar();
    getchar();
    if (input == 'Y') {
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
    stageInput();
    return(0);
    
}
