#ifndef _VRAM_QUEUE_H_
#define _VRAM_QUEUE_H_

typedef struct {
    const void* src;
    void* dst;
} TileUpdateQueueItem;

IWRAM_CODE void addTileUpdateToQueue( const void* src, void* dst );
IWRAM_CODE void processTileQueue( void );

#endif
