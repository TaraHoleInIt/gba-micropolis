# AI Coding Agent Instructions for micropolis-gba

## Big Picture
- Micropolis (classic SimCity) engine runs on GBA via `devkitARM` + `libgba`. The original engine sources are included from `micropolis/MicropolisCore/src/MicropolisEngine/src` and the GBA client glue/rendering lives in `source/`.
- Frame loop is driven by GBA VBlank: `irqVBlank()` in [source/entry.cpp](../source/entry.cpp) calls `spriteEngineUpdate()`, `tileEngineVBlank()`, `tileEngineUpdate()`, and then flushes buffered VRAM writes via `processTileQueue()`.
- Simulation ticks and tile animations are scheduled in the main loop (outside VBlank) by time checks using `timerMillis()`.
- Rendering uses two subsystems:
  - Tile engine: caches 2x2 "metatiles" and maps Micropolis `sim.map` values to hardware tiles and palette in [source/tile_engine.cpp](../source/tile_engine.cpp).
  - Sprite engine: displays entities from `Micropolis::spriteList` using cached 64x64 16-color OBJ sprites in [source/sprite_engine.cpp](../source/sprite_engine.cpp).

## Build, Run, Debug
- Prereq: `DEVKITARM` must point to devkitPro’s ARM toolchain. Example: `export DEVKITARM=/opt/devkitpro/devkitARM`.
- Build: run `make` at repo root. Target ROM is `./micropolis-gba.gba`.
- Run (macOS): `/Applications/mGBA.app/Contents/MacOS/mGBA -l 255 ./micropolis-gba.gba` for verbose logging.
- Logs & text:
  - `mgbaPrintf()` writes to mGBA’s console; enable high log level (`-l 255`).
  - `textPrintf()`/`textPrintfCenter()` draw to the on-device console layer after `textAndDebugInit()`.

## Architecture & Data Flow
- Entry & timing: [source/entry.cpp](../source/entry.cpp) sets IRQs, initializes subsystems, creates `Micropolis`, sets `resourceDir="rom:/"`, loads a scenario (e.g., `SC_TOKYO`), and controls sim pace and animation cadence.
- Input: `inputUpdateVBlank()` captures key state each VBlank; helpers `inputIsHeld()/Down()/Up()` in [source/input.cpp](../source/input.cpp) drive scrolling and interactions.
- Tile engine:
  - `tileEngineVBlank()` updates smooth scroll offsets based on keys; `tileEngineUpdate(sim)` draws a 32x32 tilemap window (11x16 Micropolis tiles) around the viewport.
  - `MetaTile::metaTilePlace()` ensures a metatile exists in cache, queues VRAM copies with `addTileUpdateToQueue()`; actual VRAM DMA happens only in VBlank via `processTileQueue()`.
- Sprite engine:
  - Iterates `sim.spriteList`, culls to current viewport via `tileEngineGetViewport()`, and assigns/caches frames into OBJ VRAM slots.
  - LRU-like eviction uses `lastSeenTick` from `timerMillis()`; `spriteCacheEvictOld()` frees slots not seen for ~500ms.
- VRAM update protocol: never DMA to VRAM directly during game logic; enqueue with `addTileUpdateToQueue(src,dst)` and flush in VBlank.

## Assets & Resources
- Tile graphics/palette are embedded in [source/gfx.c](../source/gfx.c) (`__tiles_bin`, `__tiles_pal_bin`).
- Sprite frames are compiled into headers under `build/` (e.g., `objN_img_bin.h`, `sprites_pal_bin.h`) and used by `sprite_engine.cpp` via `spriteObjs[]` index mapping. Index 0 is `nullptr`; valid sprite types start at 1.
- File system: `rom:/` is a read-only FAT interface backed by a single binary blob `fs_bin` (see [build/fs_bin.h](../build/fs_bin.h)). GBA-side FAT implementation is in [source/fat_rom.cpp](../source/fat_rom.cpp).

## Conventions & Patterns
- Use `IWRAM_CODE` and `IWRAM_DATA` for hot paths and data that should reside in internal work RAM (performance-critical code like VBlank handlers, cache lookups, queues).
- Respect VBlank boundaries: heavy rendering and DMA must happen inside `irqVBlank()` through the tile/sprite engine and VRAM queue.
- Tile mapping: Micropolis tile IDs are masked with `& 0x03FF` and mapped to 4 hardware tiles per metatile; palette index 16+ contains inverted copies used for highlight effects.
- Screen layers: Console text runs on BG1 (`ConfigConsoleBG`), game map runs on BG0 (`ConfigGameBG`), both configured in [source/config.h](../source/config.h).

## Practical Examples
- Add a new sprite type: generate `objN_img_bin.h`, include it in `sprite_engine.cpp`, and add to `spriteObjs[]`. Ensure frame count matches `SpriteFrameSize` assumptions (64×64, 4bpp).
- Draw UI text: call `textAndDebugInit()` early, then `textPrintfCenter(y, "Your text")` in the main loop or on events. Avoid printing inside tight VBlank sections unless minimal.
- Access resources: place files into the ROM FS blob and load via `sim->resourceDir = "rom:/"`; use Micropolis engine APIs like `loadScenario()` and `animateTiles()`.

## Safety & Gotchas
- Do not DMA to VRAM outside VBlank; use the queue. Writing during HBlank or active display may corrupt output.
- Keep cache sizes in `MetaTile::tileCache[256]` and OBJ slots aligned with hardware limits; GBA has 1D OBJ mapping enabled (`OBJ_1D_MAP`).
- Time math uses `timerMillis()` (`timer.cpp`), not `gettimeofday()`; `gettimeofday()` is stubbed to derive entropy only.

If any section is unclear or you need deeper examples (e.g., adding inputs, extending Micropolis tool interactions, or modifying scenario loading), tell me what you’d like to do and I’ll refine these instructions.