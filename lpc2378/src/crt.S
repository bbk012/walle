/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        crt.S
* Description: Module includes start-up code for LPC 2378 uP
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008 - Initial version created based on demo2378_blink_flash from OLIMEX
*********************************************************************************************************
*/

/* Stack Sizes */
/* IMPORTANT! Stack size MUST be multiple of 4 bytes i.e. 4, 8, C etc.*/
/* IMPORTANT! Stacks' size is defined in linker command file          */


/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs (program status registers) */
.set  MODE_USR, 0x10            		/* Normal User Mode 										*/
.set  MODE_FIQ, 0x11            		/* FIQ Processing Fast Interrupts Mode 						*/
.set  MODE_IRQ, 0x12            		/* IRQ Processing Standard Interrupts Mode 					*/
.set  MODE_SVC, 0x13            		/* Supervisor Processing Software Interrupts Mode 			*/
.set  MODE_ABT, 0x17            		/* Abort Processing memory Faults Mode 						*/
.set  MODE_UND, 0x1B            		/* Undefined Processing Undefined Instructions Mode 		*/
.set  MODE_SYS, 0x1F            		/* System Running Priviledged Operating System Tasks  Mode	*/

.set  I_BIT, 0x80               		/* when I bit is set, IRQ is disabled (program status registers) */
.set  F_BIT, 0x40               		/* when F bit is set, FIQ is disabled (program status registers) */

.text
.arm

.global	Reset_Handler
.global _startup
.func   _startup

_startup:

# Exception Vectors

_vectors:       ldr     PC, Reset_Addr         
                ldr     PC, Undef_Addr
                ldr     PC, SWI_Addr
                ldr     PC, PAbt_Addr
                ldr     PC, DAbt_Addr
                nop							/* Reserved Vector (holds Philips ISP checksum) */
                ldr     PC, IRQ_Addr        
                ldr     PC, FIQ_Addr

Reset_Addr:     .word   Reset_Handler		                  /* defined in this module below  */
Undef_Addr:     .word   OS_CPU_ARM_ExceptUndefInstrHndlr      /* uCOS-II interrupt handler defined in os_cpu_a.S  */
SWI_Addr:       .word   OS_CPU_ARM_ExceptSwiHndlr		      /* uCOS-II interrupt handler defined in os_cpu_a.S  */
PAbt_Addr:      .word   OS_CPU_ARM_ExceptPrefetchAbortHndlr	  /* uCOS-II interrupt handler defined in os_cpu_a.S  */
DAbt_Addr:      .word   OS_CPU_ARM_ExceptDataAbortHndlr		  /* uCOS-II interrupt handler defined in os_cpu_a.S  */
IRQ_Addr:       .word   OS_CPU_ARM_ExceptIrqHndlr			  /* uCOS-II interrupt handler defined in os_cpu_a.S  */
FIQ_Addr:       .word   OS_CPU_ARM_ExceptFiqHndlr             /* uCOS-II interrupt handler defined in os_cpu_a.S  */
                .word   0					                  /* rounds the vectors and ISR addresses to 64 bytes total  */
          

# Reset Handler

Reset_Handler:  

				.extern Lpc2378ResetInit    @ reference to C defined LPC 2378 uP initialization function
                 ldr     SP, =_stack_end    @ temporary stack at Stack_Top
                 LDR R0, =Lpc2378ResetInit  @ setup very basic LPC 2378 functions like MEM, PLL etc.
                 MOV LR, PC
                 BX R0
				

				/* Setup a stack for each mode - stacks' size is defined in linker command file
				Also each mode is setup with interrupts initially disabled. */
    			  
    			msr   CPSR_c, #MODE_UND|I_BIT|F_BIT 	/* Undefined Instruction Mode  */
    			ldr   sp, =_und_stack_top	
    			
    			msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT 	/* Abort Mode */
    			ldr   sp, =_abt_stack_top
    			
    			msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT 	/* FIQ Mode */
    			ldr   sp, =_fiq_stack_top
    				
    			msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT 	/* IRQ Mode */
    			ldr   sp, =_irq_stack_top
    			
    			msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT 	/* Supervisor Mode */
    			ldr   sp, =_svc_stack_top
    			
    			msr   CPSR_c, #MODE_SYS|I_BIT|F_BIT 	/* User Mode */
    			ldr   sp, =_sys_stack_top
				
				/* copy .data section (Copy from ROM to RAM) */
                ldr     R1, =_etext
                ldr     R2, =_data
                ldr     R3, =_edata
1:        		cmp     R2, R3
                ldrlo   R0, [R1], #4
                strlo   R0, [R2], #4
                blo     1b

				/* Clear .bss section (Zero init)  */
                mov     R0, #0
                ldr     R1, =_bss_start
                ldr     R2, =_bss_end
2:				cmp     R1, R2
                strlo   R0, [R1], #4
                blo     2b

/*
   Call C++ constructors (for objects in "global scope")
   ctor loop added by Martin Thomas 4/2005 
   based on a Anglia Design example-application for ST ARM
*/

                LDR     r0, =__ctors_start__
                LDR     r1, =__ctors_end__
ctor_loop:
                CMP     r0, r1
                BEQ     ctor_end
                LDR     r2, [r0], #4
                STMFD   sp!, {r0-r1}
                MOV     lr, pc
                BX R2   //B.K. original code of Martin Thomas used below instruction but this did not work with THUMB constructors
//              MOV     pc, r2
                LDMFD   sp!, {r0-r1}
                B       ctor_loop
ctor_end:

// Call main program: main(0)
// --------------------------
                mov   r0,#0                     // no arguments (argc = 0)
                mov   r1,r0
                mov   r2,r0
                mov   fp,r0                     // null frame pointer
                mov   r7,r0                     // null frame pointer for thumb
                ldr   r10,=main
                mov   lr,pc
                bx    r10                       // enter main()
	
                SWI     0xFFFFFF        /* cause exception if main() ever returns */
        
/* "global object"-dtors are never called and it should not be 
   needed since there is no OS to exit to. */
/* Call destructors */ 
# LDR r0, =__dtors_start__ 
# LDR r1, =__dtors_end__ 
dtor_loop: 
# CMP r0, r1 
# BEQ dtor_end 
# LDR r2, [r0], #4 
# STMFD sp!, {r0-r1} 
# MOV lr, pc
# BX R2 //B.K. original code of Martin Thomas used below instruction but this did not work with THUMB constructors
# //MOV pc, r2 
# LDMFD sp!, {r0-r1} 
# B dtor_loop 
dtor_end: 
.endfunc
.end
