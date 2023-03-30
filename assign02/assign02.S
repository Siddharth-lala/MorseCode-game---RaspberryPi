#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified
.cpu    cortex-m0plus
.thumb
.global main_asm
.align  4

.equ    SLEEP_TIME, 5000              @ Specify the amount of ms that we want to sleep for in the loop
.equ    DFLT_STATE_STRT, 1            @ Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            @ Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 1000000      @ Specify the default alarm timeout


.equ    GPIO_BTN_UP_MSK, 0x00400000                           @ Bit-22 for RISING-edge GP21
.equ    GPIO_BTN_DN_MSK, 0x00800000                           @ Bit-23 for falling-edg GP21


.equ    DOT, 255                                                @ 0<dot<255ms
                                                                @ dash=>255ms

.equ    GPIO_BTN_EN,  21                                        @ Specify pin for the "enter" button
.equ    GPIO_LED_PIN, 25                                        @ Specify pin for the built-in LED
.equ    GPIO_DIR_IN,   0                                        @ Specify input direction for a GPIO pin
.equ    GPIO_DIR_OUT,  1                                        @ Specify output direction for a GPIO pin


.equ    GPIO_ISR_OFFSET, 0x74                                   @ GPIO is int #13 (vector table entry 29)
.equ    ALRM_ISR_OFFSET, 0x40                                   @ ALARM0 is int #0 (vector table entry 16)


@ Entry point to the ASM portion of the program
main_asm:

    bl      init_gpio_btn                @ Use asm_gpio_set_irq C function to setup falling-edge interrutps
    ldr     r0, =msg
    bl      printf
    
    bl      inst_svc_gpio_hnd                              @ initialise buttons
    bl      inst_svc_isr                                   @ initialise alarm interrupts


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

    ldr     R3, =(PPB_BASE + M0PLUS_VTOR_OFFSET)                        @  the sum of "M0PLUS_VTOR_OFFSET"  and "PPB_BASE" goes into R2  
    ldr     R2, [R3]                                                    @  loads address from r2 ( a 32 bit word from memory) and stores it in reg r1
    movs    R3, #ALRM_ISR_OFFSET                                        @ loads address from r2 ( a 32 bit word from memory) and stores it in reg r1
    add     R3, R2                                                      @ adds values stored in R1 and R2 and stores them in R2,, therefore adds "PPB_BASE" + "M0PLUS_VTOR_OFFSET" + "SVC_ISR_OFFSET" together and stores in R2
    ldr     R0, =svc_isr                                                @ assign r0 to "svc_isr"
    str     R0, [R3]  
    ldr     R3, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)                   @  the sum of PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET (note has chnaged ) goes into -- right value goes in                                                 @  for bitwise ops
    movs    R2, #1
    str     R2, [R3]                                                    @ enable the specific interrupt level by writing to the appropriate bit in (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
    ldr     R3, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
    movs    R2, #1
    str     R3, [R3]
    bx      lr    

.thumb_func 
svc_isr:
    ldr     R3, =(TIMER_BASE + TIMER_INTE_OFFSET)
    movs    R2, #1
    ldr     R2, [R3]
    cmp     R7, #0                     
    bne     pop_stack
    cmp     R4, #3
    beq     pop_stack
    add     R5, R5, #1
    bl      pop_stack
inst_svc_gpio_hnd:

    ldr     R6, =(M0PLUS_VTOR_OFFSET + PPB_BASE)        @ R6 = address PPB_BASE + M0PLUS_VTOR_OFFSET 
    ldr     R5, [R6]                                    
    movs    R6, #GPIO_ISR_OFFSET                        @ PPB_BASE + M0PLUS_VTOR_OFFSET + ALARM_ISR_OFFSET
    adds    R6, R5
    ldr     R0, =svc_gpio_isr                           @ call svc_pio_isr funcion
    str     R0, [R6]                                    @ return goes to addy at R2
    ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)   @ Disable GPIO IRQ                                    
    ldr     R5, =0x2000                                 @ put right val to PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET
    str     R5,  [R6]                                   @ 
    ldr     R6, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)   @ Enable GPIO IRQ 
    ldr     R5, =0x2000                                 @ correct value 
    str     R5, [R6]                                    @ to PPB_BASE + M0PLUS_NVIC_ISER_OFFSET
    bx      lr              

.thumb_func
svc_gpio_isr:

    push    {lr}

    ldr     R3, =(IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET)      @ get interrupt status event 
    ldr     R1, [R3]                                                @ wht button was pressed by comparing to massks
    
    ldr     R0, =0x2000
    str     R0, [R3]
   
    ldr     R3, =(TIMER_BASE + TIMER_INTE_OFFSET)
    ldr     R0, [R3]
    
    ldr     R3, =GPIO_BTN_UP_MSK                                    @ else if(up == true){
    cmp     R3, R1                                                  @ up-btn();
    beq     UP_btn 

    ldr     R3, =GPIO_BTN_DN_MSK
    cmp     R3, R1                                                  @ if (down == true){
    bne     pop_stack                                      

HALF_btn:                                                           @ performHalfFlash(){
    ldr     R3, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)            @ correctVal = IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET.... putting the correct value to IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET
    ldr     R1, =0x00800000                                         @ GPIO_BTN_UP_MSK = 0x00040000   ; 
    str     R1, [R3]                                                @ RRRRR3

    bl      find_time
    movs    r1, r7                                                  @ Load the start time
    bl      time_difference
    movs    r7, r0
    bl      watchdog_update
    
    movs    r0, r7
    movs    r7, #0                                                  
    cmp     r0, #DOT                                           
    blt     dot                                                 
    bge     dash                                                
    b       pop_stack                                           

dot:
    ldr     r4, =1                                              @ 1 = "."
    b       end_input
dash:
    ldr     r4, =2                                              @ 2 = "-"
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