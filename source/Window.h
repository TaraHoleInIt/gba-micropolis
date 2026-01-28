#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "w_micropolis.h"

class Window {
public:
    virtual ~Window( void ) = default;

    virtual void show( void ) { }
    virtual void hide( void ) { }
    virtual void draw( void ) { }

    virtual void tick( uint32_t tickNow, int keysDown, int keysHeld, int keysUp ) { };
};

#endif
