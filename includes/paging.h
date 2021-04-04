#ifndef PAGING_H
#define PAGING_H

#include "common.h"
#include "isr.h"

typedef struct page {
    u32int present       :1;  //page present in memory
    u32int rw            :1;  //read-only if clear, read-write if set
    u32int user          :1;  //supervisor level only if clear
    u32int accessed      :1;  //has page been accessed since last refesh?
    u32int dirty         :1;  //has page been written to since last refresh?
    u32int usused        :7;  //Amalgamation of unsused and reserved bits
    u32int frame         :20; //frame adress (right shifted 12 bits);
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    /*Array of pointers to pagetables*/
    page_table_t *tables[1024];

    /*Array of pointers to page tables above but gives physical address*/
    u32int tablesPhysical[1024];

    /*physical address of tablesPhysical. Comes into play
    when i get kernel heap allocated and the directory may 
    be in a different location in virtual memory*/
} page_directory_t;

/*Sets up the environment, page directories etc and enables paging*/
void initialize_paging();

/*Causes the specified page directory to be loaded into the CR3 register*/
void switch_page_directory(page_directory_t *neew);

/*Retrives a pointer to the page required
 *if make == 1, if the page_table in which this page should rside isn't created,
 *then create it
 */
page_t *get_page(u32int address, int make, page_directory_t *dir);

/*Hanlder for page fault*/
void page_fault(registers_t regs);

#endif