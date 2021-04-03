TrashOs
    This project started out as a simple botloader for the purpose of learning. I am slowly adding kernel functionailty and drivers.

Bootloader

VGA Drivers

Keyboard Interrupts and drivers:
    Essentially a keyboard works via a connection to a physical port, like a PS/2. The data from the keyboard it recieved by a microcontroller which is located on the motherboard(in the case of our dinosaur of an operating system anyways). When a key is pressed, this microcontroller stores data(in the from of a scancode) in memory(port) 0x60 and sends an interrupt request(IRQ(in our case, IRQ1)) to the Programmable Interrupt Controller(PIC). The PIC then interrupts the CPU with a predefined interrupt number based on the external IRQ. On receiveing the interrupt, the CPU will consult the Interrupt Descriptor Table(IDT), which we, the coders, structure ourselves. IDTs consitst of 256 entries know as gates. These gates are 64 bits. For details on bits, either consult the sources or look at comments in idt.h. The IDT will determine which handler to invoke, and then the CPU will resume regular exection before the interrupt was invoked. In a 16 bit system, the interrupt proccess is a little more straight foward, but since our system is desgined to run in Protected mode, which means first 32 interrupts are occupied by the CPU, things like Keyboard Interrupt Requests need to be remapped. This process is a convoluted mess invloving Assembly Code(x86 Intel Syntax) and some advanced C programming techniques(to me anyways). Below is an attempt to simplify the lifecycle of an interrupt:
        1.Setup Interrupts
            1.Create IDT table(idt.h)
            2.Set IDT entry # with 
            3. Load IDT address with lidt
            4.(optional)Send interrupt mask 0xfd to PIC1 to unmask(enable) IRQ1
            5.Enable interrupts with sti(inline assmebly in kernel.c)
        2.Human hits spouse(keyboard)
        3.Keyboard controller raises interrupt line IRQ1 in PIC1
        4.PIC checks if this line is maksed (its not)
        5.CPU checks if interrupts are disabled(they're not)
        6.Push EFLAGS, CS(Code Segment) and EIP(Instruction Pointer) on the stack
        7.Push error code(if appropriate) onto the stack
        8.look into idtr(interrupt descriptor table register(C struct idt_regitser_t in idt.h)) and fetch segment selector from IDT 
        10.Check privilege levels and load segment selector and ISR address into CS:EIP
        11.Clear IF flag because IDT entries are interrupt gates
        13.Pass Control to ISR
            1.Disable interrupt with cli
            2.Save interrupted procedure state with pusha
            3.Push current DS value on the stack
            4.Reload ds, es, fs, gs from kernel data segment
        14.Acknowledge interrupt by sending EOI (0x20) to Master PIC(I/O port 0x20)
        15.Read keyboard status from keyboard controller(I/O port 0x64)
        16.If status is 1 then read keycode from keyboard controller(I/O port 0x60)
        17.print cgar via VGA buffer or send via teletype
        18.Return from interrupt:
            1.pop from the stack and restore DS
            2.Restore interrupted procedure state with popa 
            3.enable interrupts with sti (inline in kernel.c)
            4.iret (interrupt return)

   
All Sources
    x86 Architecture:
        https://en.wikipedia.org/wiki/X86
    x86 Assembly:
        https://en.wikipedia.org/wiki/X86_assembly_language
    CPU Modes:
        http://flint.cs.yale.edu/feng/cos/resources/BIOS/procModes.htm
    Global Descriptor Tables:
        https://en.wikipedia.org/wiki/Global_Descriptor_Table
    Segmentation:
        https://en.wikipedia.org/wiki/Segment_descriptor
    Video Memory:
        https://wiki.osdev.org/Printing_To_Screen
    Memory Mapped Locations:
        http://www.brokenthorn.com/Resources/OSDev7.html
    C inline assembler
        https://www.codeproject.com/Articles/15971/Using-Inline-Assembly-in-C-C
    Programmable Interrupt Controller:
        https://en.wikipedia.org/wiki/Programmable_interrupt_controller
    8259 PIC:
        https://wiki.osdev.org/8259_PIC
    8259 Documentation(Master and Slave command words, etc):
        http://www.thesatya.com/8259.html
    Interrupt Requests (IRQs);
        https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)
    Interrupt Descriptor Table(idt);
        https://en.wikipedia.org/wiki/Interrupt_descriptor_table
    KeyBoard Scancodes:
        https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
    Repo for OS dev tutorial:
        https://github.com/cfenollosa/os-tutorial
    Tutorial:
        https://dev.to/frosnerd/writing-my-own-boot-loader-3mld
    BIOS Overview:
        https://whatis.techtarget.com/definition/BIOS-basic-input-output-system
    Interrupts Explained:
        https://alex.dzyoba.com/blog/os-interrupts/
    Typedef Function Pointers:
        https://riptutorial.com/c/example/31818/typedef-for-function-pointers
    Additional Learning:
        http://himmele.blogspot.com/2011/07/build-your-own-operating-system.html
    Paging:
        http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html