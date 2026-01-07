#include <gba_base.h>
#include <gba_video.h>
#include <gba_dma.h>

#include <queue>

#include "micropolis.h"
#include "tile_engine.h"
#include "vram_queue.h"

IWRAM_DATA static volatile VRAMUpdateQueueItem queue[ 256 ];
IWRAM_DATA static volatile int queueLen = 0;

IWRAM_CODE void addVRAMUpdateToQueue( const void* src, void* dst, int len ) {
    assert( queueLen <= 255 );

    queue[ queueLen ].src = src;
    queue[ queueLen ].dst = dst;
    queue[ queueLen ].len = len;
    
    queueLen++;
}

IWRAM_CODE void processVRAMQueue( void ) {
    for ( int i = 0; i < queueLen; i++ ) 
        dmaCopy( queue[ i ].src, queue[ i ].dst, queue[ i ].len );
    
    queueLen = 0;
}
