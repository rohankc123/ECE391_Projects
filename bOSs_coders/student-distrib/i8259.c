/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */
#define BIT_MASK_8_BITS 0xFF
#define SLAVE_MAX 15
#define MASTER_MAX 7
#define SLAVE_MIN 8
#define MASTER_MIN 0

/* Initialize the 8259 PIC */
void
i8259_init(void) {
    unsigned long flags;
    cli_and_save(flags);

    // ---------------- initialize master ---------------------
    outb(ICW1, MASTER_8259_COMMAND);      // set init routine
    outb(ICW2_MASTER, MASTER_8259_DATA);  // 8259A - 1 IR0-7 mapped to 0x20-0x27
    outb(ICW3_MASTER, MASTER_8259_DATA);  // 8259A - 1 (master has a slave on IR2)
    outb(ICW4, MASTER_8259_DATA);         // Master expects normal EOI

    // ---------------- initialize slave ----------------------
    outb(ICW1, SLAVE_8259_COMMAND);       // set init routine
    outb(ICW2_SLAVE, SLAVE_8259_DATA);    // 8259A - 2 IR0-7 mapped to 0x28-0x2f
    outb(ICW3_SLAVE, SLAVE_8259_DATA);    // 8259A - 2 slave on master's IR2
    outb(ICW4, SLAVE_8259_DATA);          // Master expects normal EOI

    outb(BIT_MASK_8_BITS, MASTER_8259_DATA);           // Mask all of 8259A - 1
    outb(BIT_MASK_8_BITS, SLAVE_8259_DATA);            // Mask all of 8259A - 2
    //enable pic
    enable_irq(2);

    restore_flags(flags);
}



/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num) {
    unsigned long flags;
    unsigned char mask;
    cli_and_save(flags);

    if(irq_num >= MASTER_MIN && irq_num <= MASTER_MAX) {
        // Mask on the master:
        mask = inb(MASTER_8259_DATA);
        outb(mask | (1<<irq_num), MASTER_8259_DATA);
    }
    else if(irq_num >= SLAVE_MIN && irq_num <= SLAVE_MAX) {
        // Mask on the slave:
        mask = inb(SLAVE_8259_DATA);
        outb(mask | (1<<(irq_num - SLAVE_MIN)), SLAVE_8259_DATA);
    }
    restore_flags(flags);

}


/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num) {
    unsigned long flags;
    unsigned char mask;
    cli_and_save(flags);
    if(irq_num >= MASTER_MIN && irq_num <= MASTER_MAX) {
        // Mask on the master:
        mask = inb(MASTER_8259_DATA);
        outb(mask & ~(1<<irq_num), MASTER_8259_DATA);
    }
    else if(irq_num >= SLAVE_MIN && irq_num <= SLAVE_MAX) {
        // Mask on the slave:
        mask = inb(SLAVE_8259_DATA);
        outb(mask & ~(1<<(irq_num - SLAVE_MIN)), SLAVE_8259_DATA);
    }
    restore_flags(flags);

}



/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num) {
    unsigned long flags;
    cli_and_save(flags);
    if(irq_num < SLAVE_MIN){
        outb(EOI | irq_num, MASTER_8259_COMMAND);
    }
    else if(irq_num <= SLAVE_MAX) {
        outb(EOI | (irq_num-SLAVE_MIN), SLAVE_8259_COMMAND);
        outb(EOI | ICW3_SLAVE, MASTER_8259_COMMAND);
    }
    restore_flags(flags);
}
