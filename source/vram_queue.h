#ifndef _VRAM_QUEUE_H_
#define _VRAM_QUEUE_H_

typedef struct {
    const void* src;
    void* dst;
    int len;
} VRAMUpdateQueueItem;

IWRAM_CODE void addVRAMUpdateToQueue( const void* src, void* dst, int len );
IWRAM_CODE void processVRAMQueue( void );

#endif
