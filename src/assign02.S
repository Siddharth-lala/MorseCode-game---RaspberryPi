#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified                         
.cpu    cortex-m0plus                   
.thumb                                  
.global main_asm                        
.align 4                               

.equ    DFLT_ALARM_TIME, 1000000        @ Specify the default alarm timeout
.equ    GPIO_ISR_OFFSET, 0x40           @ GPIO is int #0 (vector table entry 16)
.equ    ADC, 4                          @ input 4 = temperature sensor



.equ    SLEEP_TIME, 5000              @ Specify the amount of ms that we want to sleep for in the loop
.equ    DFLT_STATE_STRT, 1            @ Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            @ Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 1000000      @ Specify the default alarm timeout

.equ    GPIO_BTN_DN_MSK, 0x00800000   @ Bit-23 for falling-edge event on GP21
.equ    GPIO_BTN_UP_MSK, 0x00400000   @ Bit-22 for rising-edge event on GP21

.equ    DOT, 255                                                @ Input time for a dot is between 0 and 255 ms
                                                                @ Input time for a dash is 255 ms and greater


.equ    GPIO_BTN_EN,  21              @ Specify pin for the "enter" button
.equ    GPIO_LED_PIN, 28             @ Specify pin for the built-in LED
.equ    GPIO_DIR_IN,   0              @ Specify input direction for a GPIO pin
.equ    GPIO_DIR_OUT,  1              @ Specify output direction for a GPIO pin

.equ    LED_VAL_ON,    1              @ Specify value that turns the LED "on"
.equ    LED_VAL_OFF,   0              @ Specify value that turns the LED "off"

.equ    GPIO_ISR_OFFSET, 0x74         @ GPIO is int #13 (vector table entry 29)
.equ    ALRM_ISR_OFFSET, 0x40         @ ALARM0 is int #0 (vector table entry 16)



@ Entry point to the ASM portion of the program
main_asm:
    bl      init_gpio_btn                @ Use asm_gpio_set_irq C function to setup falling-edge interrutps
    ldr     r0, =msg
    bl      printf
    
    bl      inst_svc_gpio_hnd                              @ initialise buttons
    bl      inst_svc_isr                                   @ initialise alarm interrupts                   
         
begin_game:
    bl      initInputArray                                
    movs    R4, #10                                             @ so R4 will only equal ten when dot or dash hasnt been picked.. like a reset btn 
    movs    R7, #0                                              @ R7 show many interrups/seconds have passed
    
sub_toggle:
    bl      set_alarm                                           @ alarm
    wfi                                                         @ wait interrupt
    bl      user_input                                          @ user input into array
    cmp     R4, #10                                             @ has the button been pressed yet??
    beq     check_input_level                                               
    movs    R4, #10                                             @ reset r4,
check_input_level
    cmp     R7, #2                                              @ loop if the input is not completed
    bne     sub_toggle
    cmp     R0, #1
    beq     lv1
    cmp     R0, #2
    beq     lv2
    cmp     R0, #3
    beq     lv3
    cmp     R0, #4
    beq     lv4
    cmp     R0, #5
lv1:
    // code here

lv2:
    // code here

lv3:
    // code here

lv4:
    // code here

level_selected:
    bl      initalizeInputArray                                 @ Initialize array
    pop     {pc}                                                
 
user_input:
    Push    {lr}                                                @ Store the link register to the stack as we will call the init_btns subroutine

    cmp     R7, #1                                              @ if alarm interrupt has gone off once
    beq     space_occured                                       @ add space to input
    cmp     R7, #2                                              @ alarm int x2
    beq     enter                                               @ player finished their input
    cmp     R4, #1                                              @ dash = 1
    beq     input_dash                                          @ add dash to seq
    cmp     R4, #2                                              @ dot=2
    beq     input_dot                                           @ add dot to seq
    b       pop_stack                                           @ input finished

input_dot:
    ldr     R0, =2                                              @ 1 = "."
    bl      add_to_input                                         
    b       pop_stack                                          

input_dash:
    ldr     R0, =1                                              @ 2 = "-"
    bl      add_to_input                                          
    b       pop_stack                                           

space_occured:
    ldr     R0, =4                                              @ 3 = " "
    bl      add_to_input                                          
    b       pop_stack                                            

enter:
    ldr     R0, =3                                              @ 4 = "enter"
    bl      add_to_input                                        
    pop     {pc}   

sub_toggle:
    ldr     R2, =(TIMER_BASE + TIMER_INTE_OFFSET)       @ Enable alarm timer interrupts using the (TIMER_BASE + TIMER_INTE_OFFSET) register
    movs    R3, #1
    str     R3, [R2]

    ldr     R2, =(TIMER_BASE + TIMER_TIMELR_OFFSET)     @ Get the current timer count from (TIMER_BASE + TIMER_TIMELR_OFFSET) register
    ldr     R4, [R2]
    
    movs    R3, R7 
    add     R3, R4                                      @ Add the time delay you want to wait for to the current timer count
    
    ldr     R2, =(TIMER_BASE + TIMER_ALARM0_OFFSET)     @ Push the updated value to (TIMER_BASE + TIMER_ALARM0_OFFSET)
    str     R3, [R2]
    bx      lr                                          @ Exit subroutine

init_gpio_btn:
        push    {lr}

        movs    R0, GPIO_BTN_EN             @ This value is the down button on the PI Pico board
        bl      asm_gpio_init               @ Call the subroutine to initialise the GPIO pin specified by r2
        movs    R0, #GPIO_BTN_EN            @ This value is the GPIO ENTER pin on the PI PICO board
        movs    R1, #GPIO_DIR_IN            @ We want this GPIO pin to be setup as an input pin
        bl      asm_gpio_set_dir            @ Call the subroutine to set the GPIO pin specified by r0 to state specified by r1

        movs    R0, #GPIO_BTN_EN            @ This value is the initalised enter button on the PI Pico board
        bl      asm_gpio_set_irq            @ Enable falling-edge interrupt

        pop     {pc}
inst_svc_isr:
    
        ldr     R6, =(M0PLUS_VTOR_OFFSET + PPB_BASE)        @ R6 = address PPB_BASE + M0PLUS_VTOR_OFFSET 
        ldr     R5, [R6]                                    
        movs    R6, #GPIO_ISR_OFFSET                        @ PPB_BASE + M0PLUS_VTOR_OFFSET + ALARM_ISR_OFFSET
        adds    R6, R5
        ldr     R0, =svc_gpio_isr                           @ call svc_pio_isr funcion
        str     R0, [R6]                                    @ return goes to addy at R2
        ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)   @ Disable GPIO IRQ                                    
        movs    R5, #1                                      @ put right val to PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET
        str     R5,  [R6]                                   @ 
        ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)   @ Enable GPIO IRQ 
        movs    R5, #1                                      @ correct value 
        str     R5, [R6]                                    @ to PPB_BASE + M0PLUS_NVIC_ISER_OFFSET
        bx      lr   

svc_gpio_isr:
        push    {lr}                        @ Store the link register to the stack as we will call nested subroutines

        @ Disable pending interrupt from TIMER by writing correct value to (TIMER_BASE + TIMER_INTR_OFFSET)
        ldr     R3, =(TIMER_BASE + TIMER_INTR_OFFSET)
        movs    R2, #1
        str     R2, [R3]

        cmp     R8, #0                      @ If there is a time stored in r7, then the button is still being held
        bne     pop
        cmp     R4, #3
        beq     pop

        @ Add 1 to the number of iterations the timer has gone off in r5
        ldr     R5, =1
        add     R6, R6, R5
pop:
    pop     {pc}                        @ Pop the link register from the stack to the program counter 
inst_svc_gpio_hnd:

        ldr     R6, =(M0PLUS_VTOR_OFFSET + PPB_BASE)        @ R6 = address PPB_BASE + M0PLUS_VTOR_OFFSET 
        ldr     R5, [R6]                                    
        movs    R6, #GPIO_ISR_OFFSET                        @ PPB_BASE + M0PLUS_VTOR_OFFSET + ALARM_ISR_OFFSET
        adds    R6, R5
        ldr     R0, =svc_gpio_isr                           @ call svc_pio_isr funcion
        str     R0, [R6]                                    @ return goes to addy at R2
        ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)   @ Disable GPIO IRQ                                    
        movs    R5, =0x2000                                      @ put right val to PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET
        str     R5,  [R6]                                   @ 
        ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)   @ Enable GPIO IRQ 
        movs    R5, =0x2000                                      @ correct value 
        str     R5, [R6]                                    @ to PPB_BASE + M0PLUS_NVIC_ISER_OFFSET
        bx      lr                                          

.thumb_func
svc_gpio_isr:

    push    {lr}

    ldr     R3, =(IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET)      @ get interrupt status event 
    ldr     R1, [R3]                                                @ wht button was pressed by comparing to massks
    
    ldr     R0, =0x2000
    str     R0, [R3]
   
    ldr     r2,=GPIO_BTN_RISE_MSK
    CMP     r1, r2                                              @ Detect if it is a rising edge
    beq     rising_edge

    ldr     r2, =GPIO_BTN_FALL_MSK
    CMP     r1, r2                                              @ Detect if it is a falling edge
    beq     falling_edge
    b       pop_stack

 
    
    ldr     R3, =GPIO_BTN_UP_MSK                                    @ else if(up == true){
    cmp     R1, R3                                                  @ up-btn();
    beq     UP_btn 

    ldr     R3, =GPIO_BTN_DN_MSK
    cmp     R3, R1                                                  @ if (down == true){
    bne     pop_stack                                      

HALF_btn:                                                           @ performHalfFlash(){
    ldr     R3, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)            @ correctVal = IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET.... putting the correct value to IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET
    ldr     R1, =0x00800000                                         @ GPIO_BTN_UP_MSK = 0x00040000   ; 
    str     R1, [R3]                                                @ RRRRR3

    bl      find_time
    movs    R1, R8                                                  @ Load the start time
    bl      time_difference
    movs    R8, R0
    bl      watchdog_update
    
    movs    R0, R8
    movs    R8, #0                                                  
    cmp     R0, #DOT                                           
    blt     dot                                                 
    bge     dash                                                
    b       pop_stack                                           

dot:
    ldr     R4, =1                                              @ 1 = "."
    b       pop_stack
dash:
    ldr     R4, =2                                              @ 2 = "-"
    bl      pop_stack        

UP_btn:                                                            
    ldr     R3, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)            @ correctVal = IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET
    ldr     R1, =0x0040000                                          @ GPIO_BTN_UP_MSK = 0x0040000 ; 
    str     R1, [R3]                                                @( ... correct val (GPIO_BTN_EN_MSK) to IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)
    bl      find_time
    movs    R8, R0
    bl      pop_stack 

pop_stack:
    pop     {pc}
.align 4
msg:    .asciz "initalisation of btn complete!\n"
.data
lstate: .word   DFLT_STATE_STRT
ltimer: .word   DFLT_ALARM_TIME
