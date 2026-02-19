#include <gba_console.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include "w_micropolis.h"

#include "timer.h"
#include "text_and_debug.h"
#include "save.h"

#include "Game.h"

#include "scenarios.h"

#include "maxmod.h"
#include "soundbank.h"
#include "soundbank_bin.h"

int gettimeofday( struct timeval* tv, void* tzp );
uint32_t generateEntropy( void );
void irqVBlankGame( void );
void maxmodSetup( void );

__attribute__( ( used ) ) __attribute__( ( aligned( 4 ) ) ) const char* saveTag = "SRAM_V110";

IWRAM_DATA Micropolis* sim = nullptr;

volatile uint32_t frameCount = 0;

static MCGAWorldRenderer rendererMCGA;
static TandyWorldRenderer rendererTandy;

static volatile int needsRedraw = 0;

IWorldRenderer* renderer = nullptr;

static uint32_t seed = 0;

static volatile int gameReady = 0;
static volatile int rngReady = 0;

int gettimeofday( struct timeval* tv, void* tzp ) {
	uint32_t timeNow = 0;

	if ( rngReady == 0 ) {
		seed = generateEntropy( );
		rngReady++;

		srand( seed );
	}
	
	timeNow = timerMillis( );

	tv->tv_sec = ( timeNow / 1000 ) + seed;
	tv->tv_usec = timeNow * 1000;

	return 0;
}

void irqVBlankGame( void ) {
	mmFrame( );

	game.vblank( );
	frameCount++;
}

uint32_t generateEntropy( void ) {
	int y = ( ( SCREEN_HEIGHT / 8 ) / 2 ) - 2;
	uint32_t tickNow = 0;
	uint32_t result = 0;
	uint32_t lastHeld = 0;
	uint32_t held = 0;

	// Make sure timer and text subsystems are up
	REG_DISPCNT = 0;

	irqInit( );
	irqEnable( IRQ_VBLANK );

	timerInit( );
	textAndDebugInit( );

	textPrintfCenter( y, "Generating randomness..." );
	textPrintfCenter( y + 1, "Mash buttons for a while." );
	textPrintfCenter( y + 2, "Press START to finish." );

	do {
		VBlankIntrWait( );
		scanKeys( );

		tickNow = timerMillis( );
		held = keysHeld( );

		if ( held != lastHeld ) {
			result |= tickNow;
			result <<= 8;
			result |= held;
			result <<= 8;

			lastHeld = held;
		}
	} while ( ! ( held & KEY_START ) );

	textClearScreen( );
	textPrintfCenter( y, "Loading..." );

	return result;
}

#define REG_WAITCNT ( *( volatile u16* ) 0x04000204 )

IWRAM_DATA static uint8_t mixBuffer[ MM_MIXLEN_16KHZ ] __attribute( ( aligned( 4 ) ) );

void maxmodSetup( void ) {
	uint8_t* musicBuffers = nullptr;
	mm_gba_system mmGBA;

	musicBuffers = ( uint8_t* ) malloc( ( 20  * ( MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH + MM_SIZEOF_MIXCH ) ) + MM_MIXLEN_16KHZ );
	assert( musicBuffers != nullptr );

	mmGBA.mixing_mode = MM_MIX_16KHZ;
	mmGBA.mod_channel_count = 20;
	mmGBA.mix_channel_count = 20;

	mmGBA.module_channels = ( mm_addr ) musicBuffers;
	mmGBA.active_channels = ( mm_addr ) ( musicBuffers + ( 20 * MM_SIZEOF_MODCH ) );
	mmGBA.mixing_channels = ( mm_addr ) ( musicBuffers + ( 20 * ( MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH ) ) );
	mmGBA.mixing_memory = ( mm_addr ) mixBuffer;
	mmGBA.wave_memory = ( mm_addr ) ( musicBuffers + ( 20 * ( MM_SIZEOF_MODCH + MM_SIZEOF_ACTCH + MM_SIZEOF_MIXCH ) ) );
	mmGBA.soundbank = ( mm_addr ) soundbank_bin;

	mmInit( &mmGBA );
	mmSetVBlankHandler( irqVBlankGame );
}

int main( void ) {
	REG_WAITCNT = ( 1 << 14 ) | 2;

	irqInit( );
	irqEnable( IRQ_VBLANK );

	textAndDebugInit( );
	timerInit( );

	maxmodSetup( );
 
	irqDisable( IRQ_VBLANK );
		irqSet( IRQ_VBLANK, mmVBlank );
	irqEnable( IRQ_VBLANK );

	while ( true ) {
		VBlankIntrWait( );
		game.runFrame( );
	}
}
