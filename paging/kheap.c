#include "../includes/common.h"
extern end;
u32int placement_address = (u32int)&end;
/***********************************************************************************
 *                 Helper Functions for paging and memory managment                *
 * *********************************************************************************/

/*Similar to malloc, except it creates the objest at address specified*/
/*Needs to be page aligned when we allocate tables and directories*/

u32int kmalloc_int(u32int size, int align, u32int *phys_addr)
{
    if(align = 1 && (placement_address & 0xFFFFF000)) //If the address is not already page aligned
    {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000; //0x1000 = 4kb. Each page is 4KB in size. Least significant 12 bits are always 0
    }

    if(phys_addr)
    {
        *phys_addr = placement_address;
    }
    u32int tmp = placement_address;
    placement_address += size;
    return tmp;
}
/*Page aligned kmalloc*/
u32int kmalloc_a(u32int size)
{
    return kmalloc_int(size, 1, 0);
}
/*returns a physical address*/
u32int kmalloc_p(u32int size, u32int *phys_addr)
{
    return kmalloc_int(size, 0, phys_addr);
}
/*Page aligned and returns physical address*/
u32int kmalloc_ap(u32int size, u32int *phys_addr)
{
    return kmalloc_int(size, 1, phys_addr);
}
/*Vanilla Malloc*/
u32int kmalloc(u32int size)
{
    return kmalloc_int(size, 0, 0);
}