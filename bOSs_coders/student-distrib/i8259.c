/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

//#include <linux/spinlock.h>
#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

//spinlock_t i8259A_lock = SPIN_LOCK_UNLOCKED;


/* Initialize the 8259 PIC */
void
i8259_init(void) {
    unsigned long flags;
    cli_and_save(flags);

    // Acquire lock and save flags:
//    spin_lock_irqsave(&i8259A_lock, flags);

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

//    udelay(100);  // Wait for the PICs to initalize

    outb(0xff, MASTER_8259_DATA);           // Mask all of 8259A - 1
    outb(0xff, SLAVE_8259_DATA);            // Mask all of 8259A - 2
    enable_irq(2);

    /*i8259A_irq_type.ack = mask_and_ack8259A;*/
    restore_flags(flags);
    // Restore the masks
//    spin_unlock_irqrestore(&i8259A_lock, flags);
}



/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num) {
    unsigned long flags;
    unsigned char mask;
    cli_and_save(flags);
    // Acquire lock and save flags:
//    spin_lock_irqsave(&i8259A_lock, flags);

    if(irq_num >= 0 && irq_num <= 7) {
        // Mask on the master:
        mask = inb(MASTER_8259_DATA);
        outb(mask | (1<<irq_num), MASTER_8259_DATA);
    }
    else if(irq_num >= 8 && irq_num <= 15) {
        // Mask on the slave:
        mask = inb(SLAVE_8259_DATA);
        outb(mask | (1<<(irq_num - 8)), SLAVE_8259_DATA);
    }
    restore_flags(flags);
    // unlock and save flags:
//    spin_unlock_irqrestore(&i8259A_lock, flags);
}


/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num) {
    unsigned long flags;
    unsigned char mask;
    cli_and_save(flags);
    // Acquire lock and save flags:
//    spin_lock_irqsave(&i8259A_lock, flags);

    if(irq_num >= 0 && irq_num <= 7) {
        // Mask on the master:
        mask = inb(MASTER_8259_DATA);
        outb(mask & ~(1<<irq_num), MASTER_8259_DATA);
    }
    else if(irq_num >= 8 && irq_num <= 15) {
        // Mask on the slave:
        mask = inb(SLAVE_8259_DATA);
        outb(mask & ~(1<<(irq_num - 8)), SLAVE_8259_DATA);
    }
    restore_flags(flags);
    // unlock and save flags:
//    spin_unlock_irqrestore(&i8259A_lock, flags);
}



/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num) {
    unsigned long flags;
    cli_and_save(flags);
    // Acquire lock and save flags:
//    spin_lock_irqsave(&i8259A_lock, flags);

    if(irq_num < 8){
        outb(EOI | irq_num, MASTER_8259_COMMAND);
    }
    else if(irq_num <= 15) {
        outb(EOI | (irq_num-8), SLAVE_8259_COMMAND);
        outb(EOI | ICW3_SLAVE, MASTER_8259_COMMAND);


    }
    restore_flags(flags);
    // unlock and save flags:
//    spin_unlock_irqrestore(&i8259A_lock, flags);
}
