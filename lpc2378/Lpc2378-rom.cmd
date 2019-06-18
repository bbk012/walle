/* ****************************************************************************************************** */
/*                                            LINKER  SCRIPT                                              */
/*   B.K. Based on  original: demo2106_blink_flash.cmd				                                      */
/*                                                                                                        */
/*                                                                                                        */
/*   The Linker Script defines how the code and data emitted by the GNU C compiler and assembler are  	  */
/*   to be loaded into memory (code goes into FLASH, variables go into RAM).                 			  */
/*                                                                                                        */
/*   Any symbols defined in the Linker Script are automatically global and available to the rest of the   */
/*   program.                                                                                             */
/*                                                                                                        */
/*   To force the linker to use this LINKER SCRIPT, just add the -T lpc2378-rom.cmd directive             */
/*   to the linker flags in the makefile.                                                                 */
/*                                                                                                        */
/*                                                                                                        */
/*   The Philips boot loader supports the ISP (In System Programming) via the serial port and the IAP     */
/*   (In Application Programming) for flash programming from within your application.                     */
/*                                                                                                        */
/*   When bootloader runs ISP or when your application is using IAP some areas of RAM as specified below  */
/*   are in use and your application MUST NOT load variables or code in those areas.                      */
/*   Lpc2378 RAM ussage by ISP and IAP                                                                    */
/* [0x4000 0000 - 0x4000 003F] - interrupt vectors if RAM mode used                                       */
/* [0x4000 0040 - 0x4000 011F] - RealMonitor RAM when RealMonitor debugger used                           */
/* [0x4000 0120 - 0x4000 01FF] - ISP RAM occupied when ISP used                                           */
/* [0x4000 7FE0 - 0x4000 7FFF] - RAM reserved for flash programming commands when either ISP or IAP is use*/
/* [0x4000 7EE0 - 0x4000 7FDF] - stack area when ISP is used (256 bytes)                                  */
/* [0x4000 7F60 - ox4000 7FDF] - stack area when IAP is used (128 bytes)                                  */
/*                                                                                                        */
/* IMPORTANT! Depending of planned ISP or IAP ussage above areas of RAM need to be exluded                */
/*            so linker to not place any data in those areas.                                             */
/*                                                                                                        */
/*  Below is MEM map used by LPC2378 as setup by this linker command file.                                */
/*  even when current version of my program is not using ISP and IAP memory is reserved                   */
/*                                                                                                        */
/*                              MEMORY MAP                                                                */
/*                      |                                 |0x40008000                                     */
/*            .-------->|---------------------------------|                                               */
/*            .         |                                 |0x40007FFF                                     */
/*         ram_isp_high |     variables and stack         |                                               */
/*            .         |     for Philips boot loader     |                                               */
/*            .         |         288 bytes               |                                     		  */
/*            .         |   Do not put anything here      |0x40007EE0                                     */
/*            .-------->|---------------------------------|                                               */
/*                      |    UDF Stack  see below         |0x40007EDC  <---------- _stack_end             */
/*            .-------->|---------------------------------|                                               */
/*                      |    ABT Stack  see below         |                                               */
/*            .-------->|---------------------------------|                                               */
/*                      |    FIQ Stack  see below         |                                               */
/*            .-------->|---------------------------------|                                               */
/*                      |    IRQ Stack  see below         |                                               */
/*            .-------->|---------------------------------|                                               */
/*                      |    SVC Stack  see below         |  when uCOS-II is run it switches from SYS     */
/*                      |                                 |  to SVC mode, this mode is also used          */
/*                      |                                 |  just after reset                             */
/*            .-------->|---------------------------------|                                               */
/*            .         |    SYS Stack  see below         | main() is called in this mode                 */
/*            .         |---------------------------------|                                               */
/*            .         |                                 | <---------- _stack_end                        */
/*            .         |                                 |            ^                                  */
/*            .         |                                 |            |                                  */
/*            .         |                                 |            |                                  */
/*            .         |                                 |            |                                  */
/*            .         |                                 |        Heap Area (from _bss_end to _stack_end */
/*            .         |                                 |            |                                  */
/*            .         |          free ram               |            |                                  */
/*           ram        |                                 |            |                                  */
/*            .         |                                 |            |                                  */
/*            .         |                                 |            V                                  */
/*            .         |.................................|   <---------- _bss_end                        */
/*            .         |                                 |                                               */
/*            .         |  .bss   uninitialized variables |                                               */
/*            .         |.................................|           <---------- _bss_start, _edata      */
/*            .         |                                 |                                               */
/*            .         |                                 |                                               */
/*            .         |                                 |                                               */
/*            .         |  .data  initialized variables   |                                               */
/*            .         |                                 |                                               */
/*            .         |                                 |                                               */
/*            .         |                                 |           <---------- _data                   */
/*            .-------->|---------------------------------|                                               */
/*            .         |     variables used by           |0x400001FF                                     */
/*         ram_isp_low  |     Philips ISP                 |                                               */
/*            .         |                                 |0x40000120                                     */
/*            .-------->|---------------------------------|                                               */
/*            .         |                                 |0x4000011F                                     */
/*            .         |                                 |                                               */
/*            .         |          free ram               |0x40000040                                     */
/*            .         |---------------------------------|                                               */
/*            .-------->|                                 |0x4000003F                                     */
/*            .         |  Interrupt Vectors (re-mapped)  |                                               */
/*       ram_vectors    |  64 bytes (can be used in flash |                                               */
/*            .         |  mode but need to be reserved   |                                               */
/*            .         |  only for RAM mode )            |0x40000000                                     */
/*            .-------->|---------------------------------|                                               */
/*                      |                                 |                                               */
/*                                                                                                        */
/*                                                                                                        */
/*                                                                                                        */
/*                      |                                 |                                               */
/*           .--------> |---------------------------------|                                               */
/*           .          |                                 |0x0007FFFF                                     */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |       unused flash eprom        |                                               */
/*           .          |                                 |                                               */
/*           .          |.................................|                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |      copy of .data area         |                                               */
/*         flash        |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |---------------------------------|                                               */
/*           .          |                                 |  <----------- _etext                          */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |            C code               |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |                                 |                                               */
/*           .          |---------------------------------|                                               */
/*           .          |                                 |                                               */
/*           .          |         Startup Code            |                                               */
/*           .          |         (assembler)             |                                               */
/*           .          |                                 |0x00000040                                     */
/*           .          |---------------------------------|                                               */
/*           .          |                                 |0x0000003F                                     */
/*           .          | Interrupt Vector Table          |                                               */
/*           ---------> |          64 bytes               |0x00000000 _startup                            */
/*                      |---------------------------------|                                               */
/*                                                                                                        */
/*                                                                                                        */
/*    The easy way to prevent the linker from loading anything into a memory area is to define            */
/*    a MEMORY region for it and then avoid assigning any .text, .data or .bss sections into it.          */
/*                                                                                                        */
/*                                                                                                        */
/*             MEMORY                                                                                     */
/*             {                                                                                          */
/*                ram_isp_low(A)  : ORIGIN = 0x40000120, LENGTH = 223                                     */
/*                                                                                                        */
/*             }                                                                                          */
/*                                                                                                        */
/*                                                                                                        */
/*  Author:    James P. Lynch                                                                             */
/*  IMPORTANT! Adapted by B.K. to include C++ startup sections as defined by Martin Thomas                */
/* ****************************************************************************************************** */

/* identify the Entry Point  */
ENTRY(_startup)

/* specify the LPC2378 memory areas  */
MEMORY 
{
	flash     			: ORIGIN = 0,          LENGTH = 512K	/* FLASH ROM                            	*/	
	ram_isp_low(A)		: ORIGIN = 0x40000120, LENGTH = 223		/* variables used by Philips ISP bootloader	*/		 
	ram   				: ORIGIN = 0x40000200, LENGTH = 32224	/* free RAM area							*/
	ram_isp_high(A)		: ORIGIN = 0x40007FE0, LENGTH = 32		/* variables used by Philips ISP bootloader	*/
}/* MEMORY */

/* Stack Sizes */
/* IMPORTANT! Stack size MUST be multiple of 4 bytes i.e. 4, 8, C etc. because each stack entry is 4 byte long  */
/* IMPORTANT! Below define initial stack before uCOS-II is initialized                                          */
/*            uCOS-II is using his own defined stack for both task's stack and for exception handling stack     */
/* After reset uP is working in SVC mode, in startup after initialization we switch to SYS and main is call     */
/* in SYS mode uCOS-II is switchin back to SVC mode and all tasks are executed in this mode                     */

/* Stack size was evaluated based on asm calls and some assumptions:                                            */
/* - Nested interrupts are disabled.                                                                            */
/* - when particular interrupt is generated responsible                                                         */
/* handler is called from os_cpu_a.S OS_CPU_ARM_ExceptXXXXX....Hndlr:                                           */
/* this handler saves R0-R3 on interrupt stack so 4*4 but for safety we put more 4*8                            */
/* - next from every specific interrupt handler so named global handler OS_CPU_ARM_ExceptHndlr: is callled      */
/*   it switches to supervisior mode and saves current context on supervsior stack and next calls C function    */
/*   interrupt handler OS_CPU_ExceptHndlr which uses two 32 bits long (2*4 bytes) variables in addition to      */
/*   saved contexts I dicaded for 4*32 bytes should be enought to preserve context and local variables          */
/* - when task are running and interrupt are generated uCOS-II Exception Task is used during interrupt handling */
/*   its size in stack entries is defined by OS_CPU_EXCEPT_STK_SIZE (default 128 4 bytes entries)               */


UND_STACK_SIZE = 4*8;         /* stack for "undefined instruction" interrupts is 32 bytes */
ABT_STACK_SIZE = 4*8;        /* stack for "abort" interrupts is 32 bytes                 */
FIQ_STACK_SIZE = 4*8;        /* stack for "FIQ" interrupts  is 32 bytes                  */
IRQ_STACK_SIZE = 4*8;        /* stack for "IRQ" normal interrupts is 32 bytes            */
SVC_STACK_SIZE = 32*8;       /* stack for "SVC" supervisor mode is 128 bytes             */
SYS_STACK_SIZE = 64*8;        /* stack for "SYS" just starts below above stacks           */


/* define a global symbol for stack based on _stack_end  stack sizes                                        */
/* IMPORTANT! Stack in ARM must be 4 BYTE ALIGNED i.e. LSB of its adress must end with 0 or 4 or 8 or C etc.*/
/*                                                                                                          */
/* _stack_end points to the place in RAM the highest address of the uP stack area                           */
/* this is de-facto beginning of the stack because stacks grows towards lower addresses                     */
_stack_end = 0x40007EDC;
_und_stack_top = _stack_end;
_abt_stack_top = _und_stack_top - UND_STACK_SIZE;
_fiq_stack_top = _abt_stack_top - ABT_STACK_SIZE; 
_irq_stack_top = _fiq_stack_top - FIQ_STACK_SIZE;
_svc_stack_top = _irq_stack_top - IRQ_STACK_SIZE;
_sys_stack_top = _svc_stack_top - SVC_STACK_SIZE;
_stack_start   = _sys_stack_top - SYS_STACK_SIZE;

/* now define the output sections  */
SECTIONS
{
    . = 0;                                          /* set location counter to address zero  */
    
    startup : { *(.startup)} >flash                             /* the startup code goes into FLASH */
    
    /* first section is .text which is used for code */
    .text :
    {
        *(.text .text.*)                            /* remaining code */
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.rodata)                                  /* read-only data (constants) */
        *(.rodata*)
        *(.gnu.linkonce.r.*)
    } > flash
  
  . = ALIGN(4);                                     /* advance location counter to the next 32-bit boundary */
  
    /* .ctors .dtors are used for c++ constructors/destructors */
    /* added by Martin Thomas 4/2005 based on Anglia Design example */
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(__ctors_end__ = .);
    } > flash
    
    .dtors :
    {
        PROVIDE(__dtors_start__ = .); 
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);
    } > flash
  
    . = ALIGN(4);                                   /* advance location counter to the next 32-bit boundary */
    /* mthomas - end */
       _etext = .;                                  /* define a global symbol _etext just after the last code byte */
       
   /* .data section which is used for initialized data */
  .data :                                           /* collect all initialized .data sections that go into RAM  */
  {
    _data = .;                                      /* create a global symbol marking the start of the .data section  */
    *(.data)                                        /* all .data sections  */
    *(.data.*)
    *(.gnu.linkonce.d*)
    SORT(CONSTRUCTORS) /* mt 4/2005 */
  } >ram AT >flash                                  /* put all the above into RAM (but load the LMA copy into FLASH) */
 
  . = ALIGN(4);                                     /* advance location counter to the next 32-bit boundary */
  _edata = . ;                                      /* define a global symbol marking the end of the .data section  */
  
  /* .bss section which is used for uninitialized data those go to RAM*/
  .bss (NOLOAD) :                                   /* collect all uninitialized .bss sections that go into RAM  */
  {
    _bss_start = . ;                                /* define a global symbol marking the start of the .bss section */
    *(.bss)                                         /* all .bss sections  */
    *(.gnu.linkonce.b*)
    *(COMMON)
  } > ram                                           /* put all the above in RAM (it will be cleared in the startup code */
  
  . = ALIGN(4);                                     /* advance location counter to the next 32-bit boundary */
  _bss_end = . ;                                    /* define a global symbol marking the end of the .bss section */
  
  /* this allocates space for stack at top of availiable RAM area */
   
}/* SECTIONS */
_end = . ;                      /* define a global symbol marking the end of application RAM */
PROVIDE (end = .);

/* define HEAP area and make it avaliable through global vars */
_heapstart = _bss_end ;         
_heapend = _stack_start;
