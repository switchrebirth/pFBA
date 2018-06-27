PFBA: Portable Final Burn Alpha
===========================

![](https://github.com/Cpasjuste/pfba/raw/master/pfba/data/screenshot.png)


**The main goals of this [final burn alpha](http://www.fbalpha.com/) port are :**

- have a basic but user friendly user interface on non windows devices
- use a recent rendering interface for speed and effects support
- cross-platform and code portability
- easy updates of fba core (no modifications to the original sources)
- use arm asm m68k ([cyclone](https://github.com/notaz/cyclone68000)) core on arm devices

-----

**Currently supported devices**

- Linux (sdl1, sdl2)
- Sony Ps Vita (vita2d)
- Nintendo Switch (libnx, sdl2)

-----

**PS VITA**

- data is located at "ux0:/data/pfba"
- default rom path is "ux0:/data/pfba/roms/"
- rom path can be changed in "ux0:/data/pfba/pfba.cfg"
- preview images can be put in "ux0:/data/pfba/previews/rom-name.png"
- ui: START = general options + default roms configuration
- ui: SELECT = per rom configuration
- in game: SELECT+START = menu
- in game: SELECT+L = saves states
- in game: SELECT+SQUARE = service switch
- in game: SELECT+TRIANGLE = 1P+2P switch
- in game: SELECT+LEFT/RIGHT = change shader
- in game: SELECT+UP/DOWN = change scaling

-----

**Switch controls (in-game)**
```
(+)             : START
(-)             : COINS
(+/-) + (A)     : START
(+/-) + (B)     : COINS
(+/-) + (X)     : SERVICE SWITCH
(+/-) + (Y)     : 1P + 2P START
(+/-) + (L/R)   : PFBA MENU
(+)   + (-)     : UNIBIOS MENU (NEOGEO ONLY)
```

**Linux (SDL2) compilation**
- sudo apt-get install libsdl2-dev libfreetype6-dev libpng-dev zlib1g-dev
- git clone --recursive https://github.com/Cpasjuste/pfba.git
- cd pfba && git submodule update --init --recursive
- mkdir cmake-build
- cd cmake-build
- cmake -DBUILD_SDL2=ON -DBUILD_SDL2_GL=ON -DCMAKE_BUILD_TYPE=Release ..
- make pfba.deps
- make pfba.elf

**Ps Vita compilation**
- git clone --recursive https://github.com/Cpasjuste/pfba.git
- cd pfba && git submodule update --init --recursive
- mkdir cmake-build
- cd cmake-build
- cmake -DBUILD_PSP2=ON -DCMAKE_BUILD_TYPE=Release ..
- make pfba.deps
- make pfba.vpk

-----

**Thanks**
- Xavier for the support (http://www.consolex-bordeaux.fr/)
- ScHlAuChi for all the advices and testing
- fba/mame authors (http://www.fbalpha.com/, http://mamedev.org/)
- xerpi for vita2d (https://github.com/xerpi/libvita2d)
- frangarcj for vita2d shader support (https://github.com/frangarcj/vita2dlib)
- people behind the vitasdk (https://github.com/vitasdk)
