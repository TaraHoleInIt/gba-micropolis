#ifndef _DIALOGWINDOW_H_
#define _DIALOGWINDOW_H_

#include <tuple>

#include "TextWindow.h"

typedef std::tuple< const char*, ActionProc > MenuEntry;

TextWindow* makeDialog( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, const char* text );
TextWindow* makeMenu( int x, int y, int width, int height, int fgColor, int bgColor, const char* title, std::vector< MenuEntry > menuEntries );

#endif
