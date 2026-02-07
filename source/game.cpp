#include <gba.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_video.h>
#include <gba_dma.h>
#include <string.h>
#include "text_and_debug.h"
#include "Game.h"
#include "scenarios.h"
#include "timer.h"

#include "TextWindow.h"
#include "DialogWindow.h"

#define MosaicBGSizeH( size ) ( ( size ) & 0x0F )
#define MosaicBGSizeV( size ) ( ( ( size ) & 0x0F ) << 4 )
#define MosaicOBJSizeH( size ) ( ( ( size ) & 0x0F ) << 8 )
#define MosaicOBJSizeV( size ) ( ( ( size ) & 0x0F ) << 12 )

#define Config_SimTick_Time 100
#define Config_AnimationTick_Time 200
#define Config_ScrollTick_Time 100
#define Config_StatusBarTick_Time 500
#define Config_Message_Show_Time 5000

#define Message_Y ( TextConsoleHeight - 3 )

IWRAM_DATA Sprite Game::oamShadow[ 128 ];
EWRAM_DATA Game game;

TextWindow tw( 1, 1, 15, 10, Color_White, Color_Blue, "Test" );

const char* Game::monthNames[ 12 ] = {
    "Jan", 
    "Feb", 
    "Mar", 
    "Apr", 
    "May", 
    "Jun",
    "Jul", 
    "Aug", 
    "Sep", 
    "Oct", 
    "Nov", 
    "Dec"
};

const char* Game::messageTable[ MESSAGE_LAST + 1 ] = {
    "",
    "More residential zones needed",
    "More commercial zones needed",
    "More industrial zones needed",
    "More roads required",
    "Inadequate rail system",
    "Build a power plant",
    "Residents demand a stadium",
    "Industry requires a seaport",
    "Commerce requires an airport",
    "Pollution is very high",
    "Crime is very high",
    "Frequent traffic jams reported",
    "Citizens demand a fire dept",
    "Citizens demand a police dept",
    "Blackouts reported",
    "Citizens: taxes are too high",
    "Roads breaking due to funding",    // TARA: Something more clear?
    "Fire depts need funding",
    "Police depts need funding",
    "Fire reported",
    "A Monster has been sighted!",
    "Tornado reported!",
    "Major earthquake reported!",
    "A plane has crashed!",
    "Shipwreck reported!",
    "A train has crashed!",
    "A helicopter has crashed!",
    "Unemployment rate is high",
    "YOUR CITY HAS GONE BROKE!",
    "Firebombing reported!",
    "Need more parks",
    "Explosion detected!",
    "Insufficient funds",
    "Area must be bulldozed first",
    "Population has reached 2,000",
    "Population has reached 10,000",
    "Population has reached 50,000",
    "Population has reached 100,000",
    "Population has reached 500,000",
    "Brownouts; build more power",
    "Heavy traffic reported",
    "Flooding reported!",
    "Nuclear meltdown occurred!",
    "Citizens are rioting!",
    "Started a new city",
    "Restored a saved city",
    "You won the scenario",
    "You lose the scenario",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

Game::Game( void ) {
    nextTileAnimateTick = 0;
    nextSimTick = 0;
    nextScrollTick = 0;
    nextStatusBarTick = 0;
    eraseMessageTick = 0xFFFFFFFF;

    memset( gameMessage, 0, sizeof( gameMessage ) );

    gameReady = false;
    needsRedraw = true;

    sim.generateSomeCity( 0xAABBCCDD );
    //sim.loadScenario( SC_TOKYO, tokyo_bin, tokyo_bin_size );
    sim.setAutoGoto( true );
    sim.setSpeed( 1 );
    sim.setPasses( 1 );
    sim.simTick( );

    sim.callbackHook = simCallback;
    sim.callbackData = this;

    windows.push_back( this );

    setRenderer( &rendererMCGA );
    spriteInit( );

    gameRunning = true;
    gameReady = true;
    statusBarShown = false;
}

void Game::tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp ) {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    if ( gameRunning ) {
        if ( tickNow >= nextScrollTick ) {
            handleScrolling( keysHeld );
            nextScrollTick = tickNow + Config_ScrollTick_Time;
        }

        if ( tickNow >= nextSimTick ) {
            nextSimTick = tickNow + Config_SimTick_Time;
            sim.simTick( );

            needsRedraw = true;
        }

        if ( tickNow >= nextTileAnimateTick ) {
            nextTileAnimateTick = tickNow + Config_AnimationTick_Time;

            renderer->getViewport( left, right, top, bottom );

            left>>= 3;
            right>>= 3;
            top>>= 3;
            bottom >>= 3;

            sim.animateTiles( left, top, left + ( SCREEN_WIDTH / 8 ), top + ( SCREEN_HEIGHT / 8 ) );
            needsRedraw = true;
        }

        if ( tickNow >= nextStatusBarTick ) {
            nextStatusBarTick = tickNow + Config_StatusBarTick_Time;

            if ( statusBarShown )
                drawStatusBars( );
        }

        if ( tickNow >= eraseMessageTick && eraseMessageTick != 0xFFFFFFFF ) {
            eraseMessageTick = 0xFFFFFFFF;
            clearMessage( );
        }

        if ( keysDown & KEY_L )
        cursor.prevTool( );

        if ( keysDown & KEY_R )
            cursor.nextTool( );

        if ( keysDown & KEY_A )
            cursor.doTool( );
    }
}

void Game::vblank( void ) {
    if ( gameReady ) {
        if ( needsRedraw ) {
            renderer->update( sim.map );
            spriteUpdate( );
        }
    }
}

void Game::setRenderer( IWorldRenderer* newRenderer ) {
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;

	// Wait until in vblank
	while ( REG_VCOUNT >= SCREEN_HEIGHT );
	while ( REG_VCOUNT < SCREEN_HEIGHT );

	irqDisable( IRQ_VBLANK );
		if ( renderer != nullptr ) {
			// Save scrolling parameters
			renderer->getViewport( left, right, top, bottom );
			renderer->deinit( );
		}

		renderer = newRenderer;
		
		REG_DISPCNT |= LCDC_OFF;
			renderer->init( );
			renderer->scrollTo( left, top );
			textAndDebugInit( );
		REG_DISPCNT &= ~LCDC_OFF;
	irqEnable( IRQ_VBLANK );

    // Make sure to redraw message, current menu and status bar since this
    // will erase them
    if ( statusBarShown )
        drawStatusBars( );
    
    wmGetActiveWindow( )->show( );
    
    if ( eraseMessageTick != 0xFFFFFFFF )
        showMessage( gameMessage );
}

IWRAM_CODE std::vector< Sprite > Game::getGameSprites( void ) {
    std::vector< Sprite > res;
    Sprite newSprite;
    int left = 0;
    int top = 0;
    int bottom = 0;
    int right = 0;
    int sprX0 = 0;
    int sprY0 = 0;
    int sprX1 = 0;
    int sprY1 = 0;

    res.reserve( 128 );
    renderer->getViewport( left, right, top, bottom );
    
    for ( SimSprite* s = sim.spriteList; s != nullptr; s = s->next ) {
        sprX0 = ( ( s->x + s->xOffset ) / 2 ) - left;
        sprY0 = ( ( s->y + s->yOffset ) / 2 ) - top;

        sprX1 = sprX0 + ( s->width / 2 );
        sprY1 = sprY0 + ( s->height / 2 );

        if ( sprX1 >= 0 && sprX0 < SCREEN_WIDTH && sprY1 >= 0 && sprY0 < SCREEN_HEIGHT && s->frame > 0 ) {
            memcpy( &newSprite, &oamClearSpritesDisabled[ 0 ], sizeof( Sprite ) );

            switch ( s->type ) {
                case SPRITE_TRAIN:
                    newSprite = setupSprite( trainSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_HELICOPTER:
                    newSprite = setupSprite( heliSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_AIRPLANE:
                    newSprite = setupSprite( planeSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_SHIP:
                    newSprite = setupSprite( shipSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_MONSTER:
                    newSprite = setupSprite( monsterSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_TORNADO:
                    newSprite = setupSprite( tornadoSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                case SPRITE_EXPLOSION:
                    newSprite = setupSprite( explosionSprite[ s->frame - 1 ], sprX0, sprY0 );
                    break;
                default:
                    break;
            };

            res.push_back( newSprite );
        }
    }

    return res;
}

void Game::spriteInit( void ) {
    spriteClearAll( );
}

void Game::spriteClearAll( void ) {
    dmaCopy( oamClearSpritesDisabled, oamShadow, sizeof( oamClearSpritesDisabled ) );
}

IWRAM_CODE void Game::spriteUpdate( void ) {
    int spriteNo = 0;

    spriteClearAll( );

    for ( Sprite spr : cursor.getSprites( ) )
        oamShadow[ spriteNo++ ] = spr;

    for ( Sprite spr : getGameSprites( ) )
        oamShadow[ spriteNo++ ] = spr;

    dmaCopy( oamShadow, OAM, sizeof( oamShadow ) );
}

void Game::handleScrolling( int keysHeld ) {
    int cursorX = 0;
    int cursorY = 0;
    int scrollX = 0;
    int scrollY = 0;
    int dx = 0;
    int dy = 0;

    dx = ( keysHeld & KEY_LEFT ) ? -1 : 0;
    dx = ( keysHeld & KEY_RIGHT ) ? 1 : dx;

    dy = ( keysHeld & KEY_UP ) ? -1 : 0;
    dy = ( keysHeld & KEY_DOWN ) ? 1 : dy;

    if ( dx != 0 || dy != 0 ) {
		cursorX = ( cursor.getX( ) * 8 ) + 4;
		cursorY = ( cursor.getY( ) * 8 ) + 4;

		scrollX = cursorX - ( SCREEN_WIDTH / 2 );
		scrollY = cursorY - ( SCREEN_HEIGHT / 2 );

        cursor.moveBy( dx, dy );
        renderer->scrollTo( scrollX, scrollY );

        needsRedraw = true;
    }
}

void Game::show( void ) {
    REG_BG0CNT &= ~BG_MOSAIC;
    REG_MOSAIC = 0;

    REG_BLDCNT = 0;
    REG_BLDY = 0;

    gameRunning = true;
}

void Game::hide( void ) {
    REG_MOSAIC = MosaicBGSizeH( 1 ) | MosaicBGSizeV( 1 ) | MosaicOBJSizeH( 1 ) | MosaicOBJSizeV( 1 );
    REG_BG0CNT |= BG_MOSAIC;

    REG_BLDCNT = 1 | ( 1 << 8 ) | ( 3 << 6 );
    REG_BLDY = 8;

    gameRunning = false;
}

void Game::runFrame( void ) {
    Window* activeWin = nullptr;
    int down = 0;
    int held = 0;
    int up = 0;

    scanKeys( );

    down = keysDown( );
    held = keysHeld( );
    up = keysUp( );

    if ( down & KEY_SELECT ) {
        statusBarShown = ! statusBarShown;

        if ( statusBarShown )
            drawStatusBars( );
        else
            clearStatusBars( );
    }

    if ( ( down & KEY_START ) && ! ( ( held | down ) & KEY_SELECT ) ) {
        if ( wmGetActiveWindow( ) == this ) {
            wmShowWindow(
                makeMainMenu( )
            );
        }
    }

    if ( down & KEY_B )
        wmPopLastWindow( );

    activeWin = wmGetActiveWindow( );
    activeWin->tick( timerMillis( ), down, held, up );
}

bool Game::wmPopLastWindow( void ) {
    Window* win = nullptr;
    bool result = false;

    if ( windows.size( ) > 1UL ) {
        win = windows.back( );

        assert( win != nullptr );
        assert( win != this );

        win->hide( );
        delete win;

        windows.pop_back( );
        windows.back( )->show( );

        result = true;
    }

    return result;
}

void Game::wmShowWindow( Window* win ) {
    assert( win != nullptr );

    if ( windows.size( ) >= 1UL )
        windows.back( )->hide( );

    windows.push_back( win );
    win->show( );
}

Window* Game::wmGetActiveWindow( void ) {
    assert( windows.size( ) >= 1UL );

    return windows.back( );
}

void Game::drawStatusBars( void ) {
    char dateStr[ 32 ];
    char cashStr[ 32 ];
    char popStr[ 32 ];
    int dateLen = 0;
    int cashLen = 0;

    dateLen = snprintf( dateStr, sizeof( dateStr ), "%s %ld", Game::monthNames[ sim.cityMonth ], sim.cityYear );
    cashLen = snprintf( cashStr, sizeof( cashStr ), "Cash: $%ld", sim.totalFunds );
    snprintf( popStr, sizeof( popStr ), "Pop: %ld", sim.cityPop );

    textSaveColors( );
        textSetColor( Color_White, Color_Blue );

        textDrawLineH( 0, 0, TextConsoleWidth, ' ' );
        textDrawLineH( 0, TextConsoleHeight - 1, TextConsoleWidth, ' ' );

        textSetCursor( 0, 0 );
        textPuts( sim.cityName.c_str( ) );

        textSetCursor( TextConsoleWidth - dateLen, 0 );
        textPuts( dateStr );

        textSetCursor( 0, TextConsoleHeight - 1 );
        textPuts( popStr );

        textSetCursor( TextConsoleWidth - cashLen, TextConsoleHeight - 1 );
        textPuts( cashStr );
    textRestoreColors( );
}

void Game::clearStatusBars( void ) {
    textSaveColors( );
        textSetColor( Color_Transparent, Color_Transparent );
        textDrawLineH( 0, 0, TextConsoleWidth, ' ' );
        textDrawLineH( 0, TextConsoleHeight - 1, TextConsoleWidth, ' ' );
    textRestoreColors( );
}

void Game::simCallback( Micropolis* sim, void* data, const char* name, const char* params, va_list args ) {
    const char* updateType = nullptr;
    Game* gamePtr = nullptr;
    int isImportant = false;
    int hasPicture = false;
    int messageNo = 0;
    int mx = 0;
    int my = 0;

    if ( strcmp( name, "update" ) == 0 ) {
        updateType = va_arg( args, const char* );

        if ( updateType != nullptr ) {
            if ( strcmp( updateType, "message" ) == 0 ) {
                messageNo = va_arg( args, int );
                mx = va_arg( args, int );
                my = va_arg( args, int );
                hasPicture = va_arg( args, int );
                isImportant = va_arg( args, int );

                if ( ( gamePtr = ( Game* ) data ) != nullptr )
                    gamePtr->showMessage( messageNo );

                if ( sim->autoGoto && isImportant ) {
                    gamePtr->scrollTo( mx, my );
                }
            }
        }
    }
}

void Game::showMessage( int messageId ) {
    int len = 0;

    if ( messageId > 0 && messageId <= MESSAGE_LAST ) {
        if ( messageTable[ messageId ] != nullptr ) {
            showMessage( messageTable[ messageId ] );
        }
    }
}

void Game::showMessage( const char* message ) {
    int len = 0;

    strncpy( gameMessage, message, TextConsoleWidth );
    len = strlen( gameMessage );

    clearMessage( );

    textSaveColors( );
        textSetColor( Color_White, Color_Black );
        textSetCursor( ( TextConsoleWidth - len ) / 2, Message_Y );
        textPuts( gameMessage );
    textRestoreColors( );

    eraseMessageTick = timerMillis( ) + Config_Message_Show_Time;
}

void Game::clearMessage( void ) {
    textSaveColors( );
        textSetColor( Color_Transparent, Color_Transparent );
        textDrawLineH( 0, Message_Y, TextConsoleWidth, ' ' );
    textRestoreColors( );
}

void Game::scrollTo( int x, int y ) {
    int cursorX = 0;
    int cursorY = 0;
    int scrollX = 0;
    int scrollY = 0;

    cursorX = ( x * 8 ) + 4;
    cursorY = ( y * 8 ) + 4;

    scrollX = cursorX - ( SCREEN_WIDTH / 2 );
    scrollY = cursorY - ( SCREEN_HEIGHT / 2 );

    cursor.moveTo( x, y );
    renderer->scrollTo( scrollX, scrollY );

    needsRedraw = true;
}

void Game::setTandyRenderer( void ) {
    setRenderer( &rendererTandy );
}

void Game::setMCGARenderer( void ) {
    setRenderer( &rendererMCGA );
}
