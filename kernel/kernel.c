#include <stdbool.h>
#include <stddef.h>
#include "../includes/display.h"
#include "../includes/kernel.h"
#include "../includes/gdt.h"
#include "../includes/common.h"
#include "../includes/timer.h"
#include "../includes/keyboard.h"
#include "../includes/banner.h"
#include "../includes/paging.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
//#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
//#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/****************************************************************************
 *                              Kernel Main                                 *
 ****************************************************************************/

void kernel_main(void) 
{
	banner_init();
	
	init_descriptor_tables();

	printk("\n>");

	enable_interrupts();
	
	init_keyboard();

	initialize_paging();
	//testing 
	u32int *ptr = (u32int*)0xA00000000000000000000000000;
	u32int do_page_fault = *ptr;
	/*Loops cpu*/
	cpu_continue();
}

/****************************************************************************
 *                              Kernel Main                                 *
 ****************************************************************************/
 

void enable_interrupts()
{
	asm volatile("sti");
}

void cpu_continue()
{
	for(;;)
		asm volatile("hlt");
}
