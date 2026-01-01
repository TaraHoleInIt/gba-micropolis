#ifndef _TILE_ENGINE_H_
#define _TILE_ENGINE_H_

#define MaxScrollX ( ( WORLD_W * 16 ) - SCREEN_WIDTH )
#define MaxScrollY ( ( WORLD_H * 16 ) - SCREEN_HEIGHT )

#define TileSize ( ( 8 * 8 ) / 2 )
#define MetaTileSize ( TileSize * 4 )

#define TilesWide ( SCREEN_WIDTH / 8 )
#define TilesHigh ( SCREEN_HEIGHT / 8 )

void tileEngineInit( void );
void tileEngineTest( void );
void tileEngineVBlank( void );
void tileEngineUpdate( Micropolis& sim );

#endif
