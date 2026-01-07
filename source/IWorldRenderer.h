#ifndef _IWORLDRENDERER_H_
#define _IWORLDRENDERER_H_

#include "w_micropolis.h"

class IWorldRenderer {
public:
    virtual void init( Micropolis* _sim )   = 0;
    virtual void deinit( void )             = 0;
    virtual void update( void )             = 0;

    virtual void getViewport( int& left, int& right, int& top, int& bottom )    = 0;

    virtual void scroll( int dx, int dy )   = 0;
};

#endif
