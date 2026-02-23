#include <gba_interrupt.h>
#include "DialogWindow.h"
#include "Game.h"
#include "scenarios.h"
#include "save.h"

TextWindow* makeDialog( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, const char* text ) {
    TextWindow* tw = new TextWindow( x, y, width, height, fgColor, bgColor, title );

    tw->addItem( new TextWindowItem( 2, 2, width - 2, height - 2, text ) );

    return tw;
}

TextWindow* makeMenu( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, std::vector< MenuEntry > menuEntries ) {
    TextWindow* tw = new TextWindow( x, y, width, height, fgColor, bgColor, title );
    const char* text = nullptr;
    ActionProc action = nullptr;
    int yPos = 1;

    for ( MenuEntry i : menuEntries ) {
        text = std::get< 0 >( i );
        action = std::get< 1 >( i );

        tw->addItem(
            new TextWindowButton(
                1,
                yPos++,
                strlen( text ),
                1,
                text,
                action
            )
        );
    }

    return tw;
}

TextWindow* makeScenarioMenu( void ) {
    return makeMenu(
        0,
        0,
        TextConsoleWidth,
        10,
        Color_White,
        Color_Blue,
        "Select a Scenario",
        {
            { "DULLSVILLE, USA        1900", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_DULLSVILLE, dullsville_bin, dullsville_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "SAN FRANCISCO, CA.     1906", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_SAN_FRANCISCO, sanfran_bin, sanfran_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "HAMBURG, GERMANY       1944", 
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        game.getSim( ).loadScenario( SC_HAMBURG, hamburg_bin, hamburg_bin_size );
                        game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                    irqEnable( IRQ_VBLANK );
            } },
            { "BERN, SWITZERLAND      1965", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_BERN, bern_bin, bern_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "TOKYO, JAPAN           1957",
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_TOKYO, tokyo_bin, tokyo_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "DETROIT, MI.           1972", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_DETROIT, detroit_bin, detroit_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "BOSTON, MA.            2010", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_BOSTON, boston_bin, boston_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
            { "RIO DE JANEIRO, BRAZIL 2047", 
            [ ] ( void ) { 
                irqDisable( IRQ_VBLANK );
                    game.getSim( ).loadScenario( SC_RIO, rio_bin, rio_bin_size );
                    game.showMessage( MESSAGE_LOADED_SAVED_CITY );
                irqEnable( IRQ_VBLANK );
            } },
        }
    );
}

TextWindow* makeOptionsMenu( void ) {
    return makeMenu(
        0,
        0,
        20,
        10,
        Color_White,
        Color_Blue,
        "Options",
        {
            { "Renderer: MCGA",
                [ ] ( void ) {
                    game.setMCGARenderer( );
                    game.showMessage( "Using MCGA tileset" );
                }
            },
            { "Renderer: Tandy",
                [ ] ( void ) {
                    game.setTandyRenderer( );
                    game.showMessage( "Using Tandy tileset" );
                }
            },
            {
                "Auto goto ON",
                [ ] ( void ) {
                    game.getSim( ).setAutoGoto( true );
                    game.showMessage( "Auto goto ON" );
                }
            },
            {
                "Auto goto OFF",
                [ ] ( void ) {
                    game.getSim( ).setAutoGoto( false );
                    game.showMessage( "Auto goto OFF" );
                }
            },
            {
                "Auto budget ON",
                [ ] ( void ) {
                    game.getSim( ).setAutoBudget( true );
                    game.showMessage( "Auto budget ON" );
                }
            },
            {
                "Auto budget OFF",
                [ ] ( void ) {
                    game.getSim( ).setAutoBudget( false );
                    game.showMessage( "Auto budget OFF" );
                }
            }
        }
    );
}

TextWindow* makeDisastersMenu( void ) {
    return makeMenu(
        0,
        0,
        20,
        10,
        Color_White,
        Color_Blue,
        "Disasters",
        { 
            { "Earthquake", [ ] { game.getSim( ).makeEarthquake( ); } },
            { "Fire", [ ] { game.getSim( ).makeFire( ); } },
            { "Fire bombs", [ ] { game.getSim( ).makeFireBombs( ); } },
            { "Flood", [ ] { game.getSim( ).makeFlood( ); } },
            { "Monster", [ ] { game.getSim( ).makeMonster( ); } },
            { "Tornado", [ ] { game.getSim( ).makeTornado( ); } },
            { "Meltdown", [ ] { game.getSim( ).makeMeltdown( ); } }
        }
    );
}

TextWindow* makeFileMenu( void ) {
    return makeMenu(
        0,
        0,
        25,
        10,
        Color_White,
        Color_Blue,
        "File",
        {
            {
                "Save city",
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        game.getSim( ).saveCity( ( unsigned char* ) SRAM );
                        game.showMessage( "Saved city" );
                    irqEnable( IRQ_VBLANK );
                }
            },
            {
                "Load city",
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        game.getSim( ).loadCity( ( const unsigned char* ) SRAM );
                        game.showMessage( "Loaded saved city" );
                    irqEnable( IRQ_VBLANK );
                }
            },
            {
                "Start new city (Easy)",
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        Micropolis& sim = game.getSim( );
                        
                        // hackhackhack
                        sim.generateSomeCity( rand( ) );
                        sim.setCityName( "New City" );
                        sim.setGameLevel( LEVEL_EASY );
                        sim.setFunds( 20000 );

                        game.showMessage( "Started a new city" );
                    irqEnable( IRQ_VBLANK );
                },
            },
            {
                "Start new city (Medium)",
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        Micropolis& sim = game.getSim( );
                        
                        // hackhackhack
                        sim.generateSomeCity( rand( ) );
                        sim.setCityName( "New City" );
                        sim.setGameLevel( LEVEL_MEDIUM );
                        sim.setFunds( 10000 );

                        game.showMessage( "Started a new city" );
                    irqEnable( IRQ_VBLANK );
                },
            },
            {
                "Start new city (Hard)",
                [ ] ( void ) {
                    irqDisable( IRQ_VBLANK );
                        Micropolis& sim = game.getSim( );
                        
                        // hackhackhack
                        sim.generateSomeCity( rand( ) );
                        sim.setCityName( "New City" );
                        sim.setGameLevel( LEVEL_HARD );
                        sim.setFunds( 5000 );

                        game.showMessage( "Started a new city" );
                    irqEnable( IRQ_VBLANK );
                },
            },
            {
                "Load a scenario",
                [ ] ( void ) {
                    game.wmShowWindow( makeScenarioMenu( ) );
                }
            }
        }
    );
}

TextWindow* makeWindowsMenu( void ) {
    return makeMenu(
        0,
        0,
        20,
        5,
        Color_White,
        Color_Blue,
        "Windows",
        {
            {
                "City evaluation",
                [ ] {
                    const char* problemStr[ 10 ] = {
                        "CRIME        ", 
                        "POLLUTION    ", 
                        "HOUSING COSTS", 
                        "TAXES        ",
                        "TRAFFIC      ", 
                        "UNEMPLOYMENT ", 
                        "FIRES        "
                    };
                    const char* classStr[ 10 ] = {
                        "VILLAGE", 
                        "TOWN", 
                        "CITY", 
                        "CAPITAL", 
                        "METROPOLIS", 
                        "MEGALOPOLIS"
                    };
                    const char* levelStr[ 3 ] = {
                        "Easy",
                        "Medium",
                        "Hard"
                    };
                    auto& sim = game.getSim( );
                    int problemNo = 0;

                    irqDisable( IRQ_VBLANK );
                        sim.cityEvaluation( );

                        // hackhackhack
                        // Run one frame so we get the do evaluation message
                        sim.simTick( );
                    irqEnable( IRQ_VBLANK );

                    TextWindow* tw = makeDialog(
                        0,
                        0,
                        TextConsoleWidth,
                        TextConsoleHeight,
                        Color_White,
                        Color_Blue,
                        "City Evaluation",
                        ""
                    );

                    if ( tw != nullptr ) {
                        tw->addItem( new TextWindowItem( 1, 1, "Approval rating:" ) );

                        tw->addItem( new TextWindowItem( 20, 2, "YEA: %d%%", sim.cityYes ) );
                        tw->addItem( new TextWindowItem( 20, 3, "NAY: %d%%", 100 - sim.cityYes ) );

                        tw->addItem( new TextWindowItem( 1, 5, "What are the worst problems?" ) );

                        for ( int i = 0; i < CVP_PROBLEM_COMPLAINTS; i++ ) {
                            if ( ( problemNo = sim.getProblemNumber( i ) ) != -1 ) {
                                tw->addItem( new TextWindowItem( 10, 6 + i, "%s: %d%%", problemStr[ problemNo ], sim.getProblemVotes( i ) ) );
                            }
                        }

                        tw->addItem( new TextWindowItem( 1, 12, "Statistics:" ) );
                        tw->addItem( new TextWindowItem( 2, 13, "Population    : %d", sim.cityPop ) );
                        tw->addItem( new TextWindowItem( 2, 14, "Net migration : %d", sim.cityPopDelta ) );
                        tw->addItem( new TextWindowItem( 2, 15, "Assessed value: $%lu", sim.cityAssessedValue ) );
                        tw->addItem( new TextWindowItem( 2, 16, "Category      : %s", classStr[ sim.cityClass ] ) );
                        tw->addItem( new TextWindowItem( 2, 17, "Level         : %s", levelStr[ sim.gameLevel ] ) );

                        game.wmShowWindow( tw );
                    }
                }
            },
            {
                "Budget",
                [ ] {
                }
            }
        }
    );
}

TextWindow* makeMainMenu( void ) {
    return makeMenu(
        0,
        0,
        15,
        7,
        Color_White,
        Color_Blue,
        "Main menu",
        {
            {
                "File",
                [ ] {
                    game.wmShowWindow( makeFileMenu( ) );
                }
            },
            {
                "Options",
                [ ] {
                    game.wmShowWindow( makeOptionsMenu( ) );
                }
            },
            {
                "Disasters",
                [ ] {
                    game.wmShowWindow( makeDisastersMenu( ) );
                }
            },
            {
                "Windows",
                [ ] {
                    game.wmShowWindow( makeWindowsMenu( ) );
                }
            }
        }
    );
}
