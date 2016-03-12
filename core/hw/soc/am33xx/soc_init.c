#include "hw.h"
#include <soc_cpsw.h>

void irq_handler();
/* context */
void soc_init()
{
	soc_uart_init();
	printf("Uart initialized\n");
	soc_ctrl_init();
	printf("Soc CTRL initialized\n");
	soc_interrupt_init();
	printf("Interrupt initialized\n");
//    soc_gpio_init();

	soc_timer_init();
	printf("Timer initialized\n");
	soc_cpsw_reset(1);
	printf("CPSW DMA Logic initialized\n");
}
