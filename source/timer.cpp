#include <gba_base.h>
#include <gba_timers.h>
#include <gba_interrupt.h>
#include "timer.h"
#include <stdio.h>

static volatile uint32_t tickCount = 0;

static void irqTimer1( void ) {
	tickCount+= 65536;
}

void timerInit( void ) {
	irqSet( IRQ_TIMER1, irqTimer1 );
	irqEnable( IRQ_TIMER1 );

	REG_TM0CNT_L = 65536 - 262;
	REG_TM0CNT_H = 1 | TIMER_START;

	REG_TM1CNT_L = 0;
	REG_TM1CNT_H = TIMER_COUNT | TIMER_IRQ | TIMER_START;
}

uint32_t timerMillis( void ) {
	return REG_TM1CNT_L + tickCount;
}
