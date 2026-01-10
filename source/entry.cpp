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

#include <fat.h>

#include "w_micropolis.h"
#include "mgba.h"

#include "input.h"
#include "timer.h"
#include "gfx.h"
#include "vram_queue.h"
#include "fat_rom.h"
#include "text_and_debug.h"

#include "TandyWorldRenderer.h"
#include "MCGAWorldRenderer.h"

static void setRenderer( IWorldRenderer* newRenderer );
int gettimeofday( struct timeval* tv, void* tzp );
uint32_t generateEntropy( void );
void irqVBlankPreGame( void );
void irqVBlankGame( void );

IWRAM_DATA Micropolis* sim = nullptr;

volatile uint32_t frameCount = 0;

static MCGAWorldRenderer rendererMCGA;
static TandyWorldRenderer rendererTandy;

static IWorldRenderer* renderer = nullptr;

static uint32_t tslowest = 0;
static uint32_t tfastest = 0;
static uint32_t tLast = 0;

static uint32_t seed = 0;

static volatile int gameReady = 0;

static void setRenderer( IWorldRenderer* newRenderer ) {
	assert( newRenderer != nullptr );
	assert( sim != nullptr );

	// Wait until in vblank
	while ( REG_VCOUNT >= SCREEN_HEIGHT );
	while ( REG_VCOUNT < SCREEN_HEIGHT );

	irqDisable( IRQ_VBLANK );
		if ( renderer != nullptr )
			renderer->deinit( );

		renderer = newRenderer;
		
		REG_DISPCNT |= LCDC_OFF;
			renderer->init( sim );
			textAndDebugInit( );
		REG_DISPCNT &= ~LCDC_OFF;
	irqEnable( IRQ_VBLANK );
}

int gettimeofday( struct timeval* tv, void* tzp ) {
	uint32_t timeNow = timerMillis( );

	tv->tv_sec = ( timeNow / 1000 ) + seed;
	tv->tv_usec = timeNow * 1000;

	return 0;
}

void irqVBlankPreGame( void ) {
	inputUpdateVBlank( );
	frameCount++;
}

Sprite oamShadow[ 128 ];

void irqVBlankGame( void ) {
	uint32_t a = 0;
	uint32_t b = 0;
	uint32_t t = 0;
	int held = 0;
	int dx = 0;
	int dy = 0;
	int s = 0;

	dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );

	inputUpdateVBlank( );

	if ( gameReady ) {
		held = inputHeld( );

		dx = ( held & KEY_LEFT ) ? -1 : 0;
		dx = ( held & KEY_RIGHT ) ? 1 : dx;

		dy = ( held & KEY_UP ) ? -1 : 0;
		dy = ( held & KEY_DOWN ) ? 1 : dy;

		dx = ( held & KEY_R ) ? ( dx * 2 ) : dx;
		dy = ( held & KEY_R ) ? ( dy * 2 ) : dy;

		renderer->scroll( dx, dy );

		a = timerMillis( );
			renderer->update( );

			memset( oamShadow, 0, sizeof( oamShadow ) );

			for ( Sprite i : renderer->getSprites( ) ) {
				oamShadow[ s++ ] = i;
			}
		b = timerMillis( );

		t = b - a;

		if ( t > tslowest )
			tslowest = t;
		
		if ( t < tfastest )
			t = tfastest;

		tLast = t;
	}

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

	textClearScreen( );

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
	struct timeval tv;

	memset( ( void* ) oamShadow, 0, sizeof( oamShadow ) );

	irqInit();
	irqSet( IRQ_VBLANK, irqVBlankPreGame );
	irqEnable( IRQ_VBLANK );

	REG_DISPCNT = 0;

	fatROMInit( );

	timerInit( );

	textAndDebugInit( );

	seed = generateEntropy( );

	gettimeofday( &tv, nullptr );

	sim = new Micropolis( );
	assert( sim != nullptr );

	setRenderer( &rendererTandy );

	sim->resourceDir = "rom:/";

	//sim->generateSomeCity( 0xAABBCCDD );
	sim->loadScenario( SC_TOKYO );
	sim->setSpeed( 1 );
	sim->setPasses( 1 );
	sim->simTick( );
	sim->simUpdate( );

	irqDisable( IRQ_VBLANK );
		irqSet( IRQ_VBLANK, irqVBlankGame );
	irqEnable( IRQ_VBLANK );

	gameReady = 1;

	while ( true ) {
		VBlankIntrWait( );
		tickNow = timerMillis( );
	
		if ( tickNow >= nextSimTick ) {
			sim->simTick( );
			sim->simUpdate( );

			nextSimTick = tickNow + 100;
		}

		if ( tickNow >= nextAnimationTime ) {
			nextAnimationTime = tickNow + 200;
			sim->animateTiles( );
		}
	}
}
