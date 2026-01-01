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

EWRAM_DATA Micropolis sim;

static volatile uint32_t hblankCount = 0;

volatile uint32_t frameCount = 0;

void irqHBlank( void ) {
	hblankCount++;
}

static uint32_t tslowest = 0;
static uint32_t tfastest = 0;
static uint32_t tLast = 0;

int gettimeofday( struct timeval* tv, void* tzp ) {
	uint32_t timeNow = timerMillis( );

	tv->tv_sec = timeNow / 1000;
	tv->tv_usec = timeNow * 1000;

	return 0;
}

void irqVBlank( void ) {
	uint32_t a = 0;
	uint32_t b = 0;
	uint32_t t = 0;

	inputUpdateVBlank( );
	tileEngineVBlank( );

	a = timerMillis( );
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

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main( void ) {
//---------------------------------------------------------------------------------
	uint32_t nextAnimationTime = 0;
	uint32_t nextSimTick = 0;
	uint32_t tickNow = 0;
	uint32_t a = 0;
	uint32_t b = 0;

	mgba_console_open( );

	iprintf( "Built %s at %s\nReady...\n\n", __DATE__, __TIME__ );

	fatROMInit( );

	//FILE* fp = nullptr;

	//fp = fopen( "rom:/snro.111", "rb" );

	//iprintf( "fp = %p\n", fp );

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqSet( IRQ_VBLANK, irqVBlank );
	irqEnable( IRQ_VBLANK );

	timerInit( );

	for ( int i = 0; i < REG_VCOUNT; i++ )
		asm volatile( "nop" );

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

		a = timerMillis( );
		if ( tickNow >= nextSimTick ) {
			sim.simTick( );
			sim.simUpdate( );

			nextSimTick = tickNow + 100;
		}

		if ( tickNow >= nextAnimationTime ) {
			// nothing yet
			nextAnimationTime = tickNow + 200;
			
			sim.animateTiles( );
		}
		b = timerMillis( );

		//iprintf( "Sim tick took %lums\n", ( b - a ) );

		a = timerMillis( );
		//tileEngineUpdate( sim );
		b = timerMillis( );

		//iprintf( "Tile engine took %lums\n", ( b - a ) );

		//iprintf( "Tile updates times (taken, slowest, fastest): [%lu, %lu, %lu]\n", tLast, tslowest, tfastest );
	}
}


