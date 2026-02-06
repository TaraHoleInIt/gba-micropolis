#include "DialogWindow.h"
#include "Game.h"
#include "scenarios.h"

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
                game.getSim( ).loadScenario( SC_DULLSVILLE, dullsville_bin, dullsville_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "SAN FRANCISCO, CA.     1906", 
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_SAN_FRANCISCO, sanfran_bin, sanfran_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "HAMBURG, GERMANY       1944", 
                [ ] ( void ) {
                game.getSim( ).loadScenario( SC_HAMBURG, hamburg_bin, hamburg_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "BERN, SWITZERLAND      1965", 
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_BERN, bern_bin, bern_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "TOKYO, JAPAN           1957",
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_TOKYO, tokyo_bin, tokyo_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "DETROIT, MI.           1972", 
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_DETROIT, detroit_bin, detroit_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "BOSTON, MA.            2010", 
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_BOSTON, boston_bin, boston_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
            } },
            { "RIO DE JANEIRO, BRAZIL 2047", 
            [ ] ( void ) { 
                game.getSim( ).loadScenario( SC_RIO, rio_bin, rio_bin_size );
                game.showMessage( MESSAGE_LOADED_SAVED_CITY );
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
        8,
        Color_White,
        Color_Blue,
        "File",
        {
            {
                "Save city",
                [ ] ( void ) {
                    game.showMessage( "Not yet implemented" );
                }
            },
            {
                "Start new city (Easy)",
                [ ] ( void ) {
                    Micropolis& sim = game.getSim( );
                    
                    // hackhackhack
                    sim.generateSomeCity( rand( ) );
                    sim.setCityName( "New City" );
                    sim.setGameLevel( LEVEL_EASY );
                    sim.setFunds( 20000 );

                    game.showMessage( "Started a new city" );
                },
            },
            {
                "Start new city (Medium)",
                [ ] ( void ) {
                    Micropolis& sim = game.getSim( );
                    
                    // hackhackhack
                    sim.generateSomeCity( rand( ) );
                    sim.setCityName( "New City" );
                    sim.setGameLevel( LEVEL_MEDIUM );
                    sim.setFunds( 10000 );

                    game.showMessage( "Started a new city" );
                },
            },
            {
                "Start new city (Hard)",
                [ ] ( void ) {
                    Micropolis& sim = game.getSim( );
                    
                    // hackhackhack
                    sim.generateSomeCity( rand( ) );
                    sim.setCityName( "New City" );
                    sim.setGameLevel( LEVEL_HARD );
                    sim.setFunds( 5000 );

                    game.showMessage( "Started a new city" );
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

TextWindow* makeMainMenu( void ) {
    return makeMenu(
        0,
        0,
        15,
        5,
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
            }
        }
    );
}
