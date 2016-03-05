#include <hw.h>

#include "soc_defs.h"

#define GTIMER_COUNT 7

#define GTIMER_1MS_FIRST 0
#define GTIMER_1MS_LAST 0

#define GTIMER_TCLR_START_STOP_CTRL (1 << 0)
#define GTIMER_TCLR_AUTO_RELOAD (1 << 1)

#define GTIMER_TISR_MAT_IT_FLAG_CLEAR (1 << 0)
#define GTIMER_TISR_OVT_IT_FLAG_CLEAR (1 << 1)
#define GTIMER_TISR_TCAR_IT_FLAG_CLEAR (1 << 2)

#define GTIMER_TIER_MAT_IT_EN  (1 << 0)
#define GTIMER_TIER_OVF_IT_EN  (1 << 1)
#define GTIMER_TIER_TCAR_IT_EN (1 << 2)

#define TICK_COUNTER 0xFF

typedef struct {
	uint32_t tidr;
	uint32_t unused1[3];
	uint32_t tiocp_cfg;
	uint32_t unused2[3];

	uint32_t irq_eoi;	//20
	uint32_t irqstatus_raw;	//24
	uint32_t irqstatus;	//28
	uint32_t irqenable_set;
	uint32_t irqenable_clr;
	uint32_t irqwakeen;

	uint32_t tclr;
	uint32_t tcrr;
	uint32_t tldr;
	uint32_t ttgr;
	uint32_t twps;
	uint32_t tmar;
	uint32_t tcar1;
	uint32_t tsicr;
	uint32_t tcar2;

} volatile gtimer;

typedef struct {
	uint32_t tidr;
	uint32_t unused1[3];
	uint32_t tiocp_cfg;
	uint32_t tistat;
	uint32_t tisr;
	uint32_t tier;
	uint32_t twer;
	uint32_t tclr;
	uint32_t tcrr;
	uint32_t tldr;
	uint32_t ttgr;
	uint32_t twps;
	uint32_t tmar;
	uint32_t tcar1;
	uint32_t tsicr;
	uint32_t tcar2;

	uint32_t tpir;
	uint32_t tnir;
	uint32_t tcvr;
	uint32_t tocr;
	uint32_t towr;
} volatile gtimer_1ms;

static uint32_t TIMER_BASES_1MS[] = {
	0x44E31000,
};

static uint32_t TIMER_BASES[] = {
	0x44E05000,		/*DMTIMER1 */
	0x48040000,		/*DMTIMER2 */
	0x48042000,		/* TODO: WHY IS TIMER3 TURNED OFF? */
	0x48044000,
	0x48046000,
	0x48048000,		/* TODO: WHY IS TIMER6 TURNED OFF? */
	0x4804A000
};

/* --------------------------------------------------------- */
cpu_callback tick_handler = 0;

return_value timer_tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2);

// ------------------------------------------------------
// timer support functions
// ------------------------------------------------------

static gtimer *timer;

gtimer *timer_get(int n)
{
	if (n < 0 || n >= GTIMER_COUNT)
		return 0;
	return (gtimer *) IO_VA_OMAP2_L4_ADDRESS(TIMER_BASES[n]);
}

static gtimer_1ms *timer_tick_get1ms()
{
	return (gtimer_1ms *) IO_VA_ADDRESS(TIMER_BASES_1MS[0]);
}

void timer_tick1ms_clear_interrupt()
{

	gtimer_1ms *timer = timer_tick_get1ms();
	volatile uint32_t what;

	what = timer->tisr;
	timer->tisr = what;
}

void timer_tick_clear_interrupt()
{
	timer->irqstatus = GTIMER_TISR_OVT_IT_FLAG_CLEAR;	// | GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
}

void gp_timer_tick_start(cpu_callback handler)
{
	/*DMTIMER as timer tick */

	timer = timer_get(1);	/*Use DMTIMER2, same as Linux */

	cpu_irq_set_enable(INTC_IRQ_TIMER2, FALSE);

	timer->tsicr = 6;	/*reset */
	int l;
	l = timer->tiocp_cfg;
	l |= (0x2 << 3);	/* Smart idle mode */
	l |= (0x2 << 8);	/* Perserve f-clock on idle */
	timer->tiocp_cfg = l;
	timer->tsicr = (1 << 2);	/*Timer control, posted */

	/*Period = 0x100 -1 = 0xff, load = 0xFFFFFFFF - period */

	timer->tldr = 0xfffff000;
	timer->tcrr = 0xfffff000;	/*counter reg */

	tick_handler = handler;
	cpu_irq_set_handler(INTC_IRQ_TIMER2, timer_tick_handler_stub);
	cpu_irq_set_enable(INTC_IRQ_TIMER2, TRUE);

	/* clear old status bits, set interrupt type and start it */
	timer->tclr |= GTIMER_TCLR_START_STOP_CTRL | GTIMER_TCLR_AUTO_RELOAD;	/*Auto reload enable */
	timer->irqstatus |=
	    GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_OVT_IT_FLAG_CLEAR |
	    GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
	timer->irqenable_set = GTIMER_TIER_OVF_IT_EN;	/*TIMER INTERRUPT ENABLE */
	timer->irqwakeen = GTIMER_TIER_OVF_IT_EN;	/*TIMER WAKEUP ENABLE */

}

void timer_tick_start(cpu_callback handler)
{
	gtimer_1ms *timer = timer_tick_get1ms();
	/*1MS timer, used from Linux as time measurement and clocksource, */
#if 0
	cpu_irq_set_enable(INTC_IRQ_TIMER1_MS, FALSE);
	timer->tpir = 232000;
	timer->tnir = -768000;
#if 0
	timer->tldr = timer->tcrr = 0xFFFFFFE0;

	// XXX: for some reason this gives us 1ms instead of the value above 
	// timer->tldr = timer->tcrr = -(32 * 256 * 5  / 17);    
	timer->tldr = timer->tcrr = -1024 * 64;
	// timer->tldr = timer->tcrr = 0xFFFFE6FF;    
	// timer->tldr = timer->tcrr = -1024 * 4;
#endif
	/*lower values make linux boot faster as it is not interrupted all the time, higher original values 0xff
	 *makes the kernel stop working */
	timer->tldr = 0xfffff000;
	timer->tcrr = 0xfffff000;	/*counter reg */

	// 
	tick_handler = handler;
	cpu_irq_set_handler(INTC_IRQ_TIMER1_MS, timer_tick_handler_stub);
#endif
	cpu_irq_set_enable(INTC_IRQ_TIMER1_MS, TRUE);
	/* Load counter reg and start free running 1ms timer */
	timer->tcrr = 0;	/*Load counter reg */
	timer->tclr |= GTIMER_TCLR_START_STOP_CTRL | GTIMER_TCLR_AUTO_RELOAD;	/*Start timer */

}

void timer_tick_stop()
{
	timer->tclr &= ~GTIMER_TCLR_START_STOP_CTRL;
}

/* ----------------------------------------------------- */

/* XXX: this handler is shared between MTU0:0 to MTU0:3,
 *      we should check for MTU0:0 before dispatching the timer callback
 */
return_value timer_tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2)
{
	if (tick_handler)
		tick_handler(r0, r1, r2);

	/* clear interrupt: */
	timer_tick_clear_interrupt();

	return RV_OK;
}

void soc_timer_init()
{
	int i;
	gtimer *timer;

#if 0
	// NORMAL TIMERS
	for (i = 0; i < GTIMER_COUNT; i++) {
		timer = timer_get(i);

		// XXX: stupid bug (?) work around, for now
		if ((uint32_t) timer == 0x48042000
		    || (uint32_t) timer == 0x48048000) {
			printf
			    ("[timer_init] FPTIMER%d at 0x08lx was BYPASSED due to bug\n",
			     i, timer);
			continue;
		}

		printf("[timer_init] GPTIMER%d at 0x%x is version %d.%d\n", i,
		       timer, (timer->tidr >> 8) & 0x7, timer->tidr & 0x3F);

		// turn it off and clear interrupt
		timer->tclr &= ~GTIMER_TCLR_START_STOP_CTRL;
		timer->irqenable_set = 0;
	}
#endif
	timer_tick_start(0);

}
