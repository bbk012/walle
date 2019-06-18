# This makefile was created for GNUARM GCC 4.1.1 based on
# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith@heizung-thomas.de>
# and based on the WinAVR makefile written by Eric B. Weddington, J�rg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# This make assumes following project directory:
#
# <root>
#		.dep - genereted dependency files included later on by makefile
#		.rel - genertaed executables of final release without debug information
#		lib - local library files if any
#       lst - generated listing files from compilation
#		obj - generated object files
#		src - source files
#			include - include files
#       Lpc2378-rom.cmd - linker command file
#		Makefile - this makefile in root directory
# IMPORTANT!
# All compilation files (obj files, lst files, elf etc. are generated in the src file and
# after that are moved to their destination directory to keep src directory clean.
# Because of that before building all compilation files are moved back to src directory to
# let makefile roles work O.K.
# 17-Oct-2012: I have decided to remove file movement to src dirs before buliding so only
#              after building generated files are move to their destination directories.
#
# This makefile assumes cygwin is running and GNUARM toolset is used
#

# Project directories

RELDIR = ./.rel
LLIBDIR = ./lib
OBJDIR = ./obj
LSTDIR = ./lst
SRCDIR = ./src
INCDIR = ./src/include

# MCU name and submodel
MCU = arm7tdmi-s
SUBMDL = LPC2378
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork


## Create ROM-Image (final)
RUN_MODE=ROM_RUN
## Create RAM-Image (debugging)
#RUN_MODE=RAM_RUN


# Output format. (can be srec, ihex, binary)
FORMAT = ihex


# Target file name (without extension).
TARGET = main


# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
# SRC =  $(SRCDIR)/$(TARGET).c
SRC  = $(SRCDIR)/lib_memalloc.c
SRC += $(SRCDIR)/lib_dbg.c
SRC += $(SRCDIR)/lib_std.c
SRC += $(SRCDIR)/lib_time.c
SRC += $(SRCDIR)/hw_uart.c
SRC += $(SRCDIR)/hw_uart1.c
SRC += $(SRCDIR)/lib_error.c
SRC += $(SRCDIR)/hw_adc.c
SRC += $(SRCDIR)/hw_gpio.c
SRC += $(SRCDIR)/hw_hcsr04.c
SRC += $(SRCDIR)/hw_rtc.c
SRC += $(SRCDIR)/hw_wdt.c
SRC += $(SRCDIR)/hw_timer.c
SRC += $(SRCDIR)/hw_pwm1.c
SRC += $(SRCDIR)/hw_spi.c
SRC += $(SRCDIR)/hw_sram.c
SRC += $(SRCDIR)/ctr_lcd.c
SRC += $(SRCDIR)/ctr_gp2d12.c
SRC += $(SRCDIR)/ctr_f_sens.c
SRC += $(SRCDIR)/os_core.c 
SRC += $(SRCDIR)/os_cpu_c.c
SRC += $(SRCDIR)/os_mbox.c
SRC += $(SRCDIR)/os_mem.c
SRC += $(SRCDIR)/os_q.c
SRC += $(SRCDIR)/os_sem.c
SRC += $(SRCDIR)/os_task.c
SRC += $(SRCDIR)/os_time.c
SRC += $(SRCDIR)/os_exch.c
SRC += $(SRCDIR)/tsk_tracks.c




# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
# SRCARM =  $(SRCDIR)/$(TARGET).c
SRCARM = $(SRCDIR)/hw_lpc2378.c


# List C++ source files here.
# use file-extension cpp for C++-files
CPPSRC = $(SRCDIR)/$(TARGET).cpp 
CPPSRC += $(SRCDIR)/lib_new.cpp
CPPSRC += $(SRCDIR)/lib_g_ctrl.cpp
CPPSRC += $(SRCDIR)/lib_g_text.cpp
CPPSRC += $(SRCDIR)/lib_g_icon.cpp
CPPSRC += $(SRCDIR)/lib_g_window.cpp
CPPSRC += $(SRCDIR)/lib_e_tmr.cpp
CPPSRC += $(SRCDIR)/wrp_sem.cpp
CPPSRC += $(SRCDIR)/wrp_mbox.cpp
CPPSRC += $(SRCDIR)/wrp_queue.cpp
CPPSRC += $(SRCDIR)/wrp_thread.cpp
CPPSRC += $(SRCDIR)/wrp_kernel.cpp
CPPSRC += $(SRCDIR)/mw_smart_ptr.cpp
CPPSRC += $(SRCDIR)/mw_notifier.cpp
CPPSRC += $(SRCDIR)/mw_subscriber.cpp
CPPSRC += $(SRCDIR)/mw_publisher.cpp
CPPSRC += $(SRCDIR)/mw_dispatcher.cpp
CPPSRC += $(SRCDIR)/mng.cpp
CPPSRC += $(SRCDIR)/mng_exe.cpp
CPPSRC += $(SRCDIR)/mng_ctx.cpp 
CPPSRC += $(SRCDIR)/mng_brain.cpp 
CPPSRC += $(SRCDIR)/mng_motion.cpp
CPPSRC += $(SRCDIR)/mng_leftarm.cpp
CPPSRC += $(SRCDIR)/mng_rightarm.cpp
CPPSRC += $(SRCDIR)/mng_indicator.cpp
CPPSRC += $(SRCDIR)/mng_monitor.cpp
CPPSRC += $(SRCDIR)/mng_rtc.cpp
CPPSRC += $(SRCDIR)/mng_rmt.cpp
CPPSRC += $(SRCDIR)/mng_vrm.cpp
CPPSRC += $(SRCDIR)/mng_keypad.cpp
CPPSRC += $(SRCDIR)/mng_display.cpp

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files
#CPPSRCARM = $(TARGET).cpp


# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = $(SRCDIR)/crt.S 
ASRCARM += $(SRCDIR)/os_cpu_a.S

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
#OPT = s
OPT = 0

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = $(INCDIR)


# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS =  -D$(RUN_MODE)

# Place -I options here
CINCS =

# Place -D or -U options for ASM here
ADEFS =  -D$(RUN_MODE)


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -Wall -Wcast-align -Wcast-qual -Wimplicit 
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

# flags only for C
CONLYFLAGS  = -Wstrict-prototypes -Wmissing-declarations
CONLYFLAGS += -Wmissing-prototypes -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

# flags only for C++ (arm-elf-g++)
# CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS = -fno-rtti -fno-exceptions

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
##ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)

# Additional libraries.
#NEWLIBLPC = -lnewlib-lpc
#MATH_LIB = -lm
#CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref
#BKO: none standard library is used at that moment
#LDFLAGS += -lc
#LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
#LDFLAGS += -lc -lgcc 
#LDFLAGS += $(CPLUSPLUS_LIB)

# Set Linker-Script Depending On Selected Memory
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T$(SUBMDL)-RAM.cmd
else 
LDFLAGS +=-T$(SUBMDL)-ROM.cmd
endif

# ---------------------------------------------------------------------------
# Define programs and commands.
SHELL = sh
CC = arm-elf-gcc
CPP = arm-elf-g++
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump
SIZE = arm-elf-size
NM = arm-elf-nm
REMOVE = rm -f
MOVE = mv -f
COPY = cp


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = Compiling C++:
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.
MSG_MOVE_TO_SRC = Move files to SRC dir
MSG_MOVE_FROM_SRC = Move files from SRC dir


# Define all object files.
COBJ      = $(SRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)
CPPOBJ    = $(CPPSRC:.cpp=.o) 
CPPOBJARM = $(CPPSRCARM:.cpp=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -fomit-frame-pointer -mcpu=$(MCU) $(THUMB_IW)  -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: begin move_to_src gccversion sizebefore build sizeafter move_from_src finished end
#all: begin gccversion sizebefore build sizeafter move_from_src finished end
#move files from supportive dirs to src dir
# '-' means ignore commands which fails
move_to_src:
	@echo
	@echo $(MSG_MOVE_TO_SRC)
	-$(MOVE) $(RELDIR)/* ./
	-$(MOVE) $(OBJDIR)/* $(SRCDIR)
	-$(MOVE) $(LSTDIR)/* $(SRCDIR)

#move files from scr dir to supportive dirs
# '-' means ignore commands which fails	
move_from_src:
	@echo
	@echo $(MSG_MOVE_FROM_SRC)
	-$(MOVE) $(TARGET).elf $(RELDIR)
	-$(MOVE) $(TARGET).hex $(RELDIR)
	-$(MOVE) $(TARGET).lss $(RELDIR)
	-$(MOVE) $(TARGET).sym $(RELDIR)
	-$(MOVE) $(TARGET).map $(RELDIR)
	-$(MOVE) $(SRCDIR)/*.o $(OBJDIR)
	-$(MOVE) $(SRCDIR)/*.lst $(LSTDIR)

	

build: elf hex lss sym

elf: $(TARGET).elf
hex: $(TARGET).hex
lss: $(TARGET).lss 
sym: $(TARGET).sym

# Eye candy.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi


# Display compiler version information.
gccversion : 
	@$(CC) --version

# Create final output files (.hex, .eep) from ELF output file.
# TODO: handling the .eeprom-section should be redundant
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C $< > $@


# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
#	$(CPP) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP) $<
	$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $<
	$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 


# Compile: create assembler files from C source files. ARM/Thumb
## does not work - TODO - hints welcome
##$(COBJ) : %.s : %.c
##	$(CC) $(THUMB) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean: begin move_to_src clean_list finished end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).lst
	$(REMOVE) $(COBJ)
	$(REMOVE) $(CPPOBJ)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(CPPOBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(CPPSRC:.cpp=.s) 
	$(REMOVE) $(CPPSRCARM:.cpp=.s) 
	$(REMOVE) .dep/*


# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex lss sym clean clean_list program

