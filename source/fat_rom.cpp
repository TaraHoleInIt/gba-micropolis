#include <gba_base.h>
#include <fat.h>
#include <string.h>

#include "fat_rom.h"
#include "fs_bin.h"

static bool fatROMStart( void ) {
	return true;
}

static bool fatROMIsInserted( void ) {
	return true;
}

static bool fatROMReadSector( sec_t sector, sec_t numSectors, void* buffer ) {
	memcpy( buffer, &fs_bin[ sector * 512 ], 512 * numSectors );
	return true;
}

static bool fatROMClearStatus( void ) {
	return true;
}

static bool fatROMShutdown( void ) {
	return true;
}

static DISC_INTERFACE di = {
	0,	// Type?
	FEATURE_MEDIUM_CANREAD | FEATURE_SLOT_GBA,
	fatROMStart,		// Startup
	fatROMIsInserted,	// isInserted
	fatROMReadSector,	// readSectors
	nullptr,			// writeSectors
	fatROMClearStatus,	// clearStatus
	fatROMShutdown,		// shutdown
};

bool fatROMInit( void ) {
    return fatMountSimple( "rom", &di );
}
