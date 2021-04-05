#include "../includes/common.h"
#include "../includes/display.h"
#include "../includes/paging.h"
#include "../includes/kheap.h"

u32int *frames;
u32int nframes;

//defined in kheap.c
extern u32int placement_address;

page_directory_t *kernel_directory=0;
page_directory_t *current_directory=0;

//Macros used in the bitset algorithm
#define INDEX_FROM_BIT(a)(a/(8*4))
#define OFFSET_FROM_BIT(a)(a%(8*4))
#define PANIC(string)(printk)

//Prototypes
void switch_page_directory(page_directory_t *dir);
page_t *get_page(u32int address, int make, page_directory_t *dir);
void page_fault(registers_t regs);

static void panic(char string[])
{
    printk(string);
    for(;;);
}

//static function to set a bit in the frams bitset
static void set_frame(u32int frame_addr)
{
    u32int frame = frame_addr/0x1000;
    u32int idx = INDEX_FROM_BIT(frame);
    u32int off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// static function to clear a bit in the frame address
static void clear_frame(u32int frame_addr)
{
    u32int frame = frame_addr/0x1000;
    u32int idx = INDEX_FROM_BIT(frame);
    u32int off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

static u32int test_frame(u32int frame_addr)
{
    u32int frame = frame_addr/0x1000;
    u32int idx = INDEX_FROM_BIT(frame);
    u32int off = OFFSET_FROM_BIT(frame);
    return(frames[idx] & (0x01 << off));
}

//static function to find the first free frame
static u32int first_frame()
{
    u32int i, j;
    for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if(frames[i] != 0xFFFFFFFF) //nothing free, exit early 
        {
            //atleast one bit is free here
            for(j = 0; j < 32; j++)
            {
                u32int toTest = 0x1 << j;
                if( !(frames[i]&toTest))
                {
                    return i*4*8+j;
                }
            }
        }
    }
}

// Function to allocate a frame
void alloc_frame(page_t *page, int is_kernel, int is_writable)
{
    if(page->frame != 0)
    {
        return; // Frame already allocated. return straight away
    }
    else
    {
        u32int idx = first_frame(); // idx is now the index of the first free frame
        if(idx == (u32int)-1)
        {
            panic("No free frames");
        }
        set_frame(idx*0x1000); // This frame is now obtained
        page->present = 1;     // Mark as present
        page->rw = (is_writable)?1:0; // Ternary. Should page be wrtiable?
        page->user = (is_kernel)?0:1; //should the page be in user-mode?
        page->frame = idx;
    }
}

// Function to deallocate a frame
void free_frame(page_t *page)
{
    u32int frame;
    if(!(frame=page->frame))
    {
        return; // This given page didnt actually have an allocated frame
    }
    else
    {
        clear_frame(frame);
        page->frame = 0x0;  //page now doesnt have a frame
    }
}

void initialize_paging()
{
    //The size of physical memory. 16mb
    u32int mem_end_page = 0x1000000; //16 mb

    nframes = mem_end_page/0x1000;
    frames = (u32int*)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Making of age directory
    kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;
    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_address
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    int i = 0;
    while(i < placement_address)
    {
        // Kernel code is readable but not writable from userspace
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }
    // Before we enable paging, we must register our page fault handler.
    register_interrupt_handlers(14, page_fault);

    //enable paging
    switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    asm volatile("mov %0, %%cr3"::"r"(&dir->tablesPhysical));
    u32int cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0"::"r"(cr0));
}

page_t *get_page(u32int address, int make, page_directory_t *dir)
{
    // Turn the address into an index
    address /= 0x1000;
    // Find page table containing this address
    u32int table_idx = address / 1024;
    if(dir->tables[table_idx]) // id this table is already assigned
    {
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else if(make)
    {
        u32int tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7; // Present, RW, US
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else
    {
        return 0;
    }
}

void page_fault(registers_t regs)
{
    //page fault has occured
    //The faulting address is stored in the cr2 register
    u32int faulting_addres;
    asm volatile("mov %%cr2, %0": "=r"(faulting_addres));

    //error code gives us details of what happened
    int present = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;         //Write operation?
    int us = regs.err_code & 0x4;         //Processor was in usermode?
    int reserved = regs.err_code & 0x8;   //OverWritten CPU-reserved bits of page entry?
    int id = regs.err_code & 0x10;        //Caused by an instruction fetch?

   // Output an error message.
   printk("Page fault! ( ");
   if (present) {printk("present ");}
   if (rw) {printk("read-only ");}
   if (us) {printk("user-mode ");}
   if (reserved) {printk("reserved ");}
//   printk(") at 0x");
//   printk_hex(faulting_address);
   printk("\n");
   panic("Page fault");
}