#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <fat.h>

#include "micropolis.h"
#include "mgba.h"

#include "tile_engine.h"
#include "input.h"
#include "timer.h"
#include "gfx.h"
#include "vram_queue.h"
#include "fat_rom.h"
#include "text_and_debug.h"

int gettimeofday( struct timeval* tv, void* tzp );
uint32_t generateEntropy( void );
void irqHBlank( void );

EWRAM_DATA Micropolis sim;

static volatile uint32_t hblankCount = 0;

volatile uint32_t frameCount = 0;

void irqHBlank( void ) {
	hblankCount++;
}

static uint32_t tslowest = 0;
static uint32_t tfastest = 0;
static uint32_t tLast = 0;

static uint32_t seed = 0;

int gettimeofday( struct timeval* tv, void* tzp ) {
	uint32_t timeNow = timerMillis( );

	tv->tv_sec = ( timeNow / 1000 ) + seed;
	tv->tv_usec = timeNow * 1000;

	return 0;
}

void irqVBlank( void ) {
	uint32_t a = 0;
	uint32_t b = 0;
	uint32_t t = 0;

	inputUpdateVBlank( );

	a = timerMillis( );
		tileEngineVBlank( );
		tileEngineUpdate( sim );
		processTileQueue( );
	b = timerMillis( );

	t = b - a;

	if ( t > tslowest )
		tslowest = t;
	
	if ( t < tfastest )
		t = tfastest;

	tLast = t;

	frameCount++;
}

uint32_t generateEntropy( void ) {
	int y = ( ( SCREEN_HEIGHT / 8 ) / 2 ) - 2;
	uint32_t tickNow = 0;
	uint32_t result = 0;
	uint32_t lastHeld = 0;
	uint32_t held = 0;

	textPrintfCenter( y, "Generating entropy..." );
	textPrintfCenter( y + 1, "Mash buttons for a while." );
	textPrintfCenter( y + 2, "Press START to finish." );

	do {
		VBlankIntrWait( );

		tickNow = timerMillis( );
		held = inputHeld( );

		if ( held != lastHeld ) {
			result |= tickNow;
			result <<= 8;
			result |= held;
			result <<= 8;

			lastHeld = held;
		}
	} while ( ! ( held & KEY_START) );

	// Clear the screen
	textPrintf( "\x1b[2J" );

	return result;
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main( void ) {
//---------------------------------------------------------------------------------
	uint32_t nextAnimationTime = 0;
	uint32_t nextSimTick = 0;
	uint32_t tickNow = 0;

	REG_DISPCNT = 0;

	fatROMInit( );

	irqInit();
	irqSet( IRQ_VBLANK, irqVBlank );
	irqEnable( IRQ_VBLANK );

	timerInit( );

	textAndDebugInit( );
	seed = generateEntropy( );

	tileEngineInit( );

	//sim.generateSomeCity( 3247283 );
	sim.resourceDir = "rom:/";
	sim.loadScenario( SC_TOKYO );
	sim.setSpeed( 1 );
	sim.setPasses( 1 );
	sim.simTick( );
	sim.simUpdate( );

	while (1) {
		VBlankIntrWait( );
		tickNow = timerMillis( );

		if ( tickNow >= nextSimTick ) {
			sim.simTick( );
			sim.simUpdate( );

			nextSimTick = tickNow + 100;
		}

		if ( tickNow >= nextAnimationTime ) {
			nextAnimationTime = tickNow + 200;
			sim.animateTiles( );
		}

		textPrintfCenter( 0, "FS: %lu, SL: %lu, LST: %lu       ", tfastest, tslowest, tLast );
	}
}


