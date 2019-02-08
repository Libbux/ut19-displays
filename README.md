# ut19-displays
Digital display programming for [UTFR](www.fsaeutoronto.ca)'s UT19 dashboard and steering wheel displays.

## Project structure
This repository contains two Arduino sketch folders, `wheel` and `dash`, for the steering wheel-mounted display and the dashboard-mounted display. These two sketches are independent of each other, and are compiled and deployed to separate Arduinos independently. They share some code and header files, however. See [config](#config) and [assets](#assets) below.

# Dependencies
### Required libraries
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) (latest)
- [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv) (latest)

### Development libraries
 - Python simulation script
  - [pySerial](https://github.com/pyserial/pyserial)

# Compiling
1. Install dependencies using Arduino IDE, or download library files manually and place them in the Arduino Libraries folder on the compiling machine.
2. Generate symlinks/shortcuts to shared directories for the two Arduino sketch folders

    For example, on a macOS/Linux system:

    `ln -s /absolute/path/to/ut19-displays/assets /absolute/path/to/ut19-displays/wheel/assets`

    `ln -s /absolute/path/to/ut19-displays/config /absolute/path/to/ut19-displays/wheel/config`

    The process should be repeated for the `dash` sketch directory. This will create links inside each of the sketch folders to the `assets` and `config` directories in the root of this repository.

    On Windows, folder shortcuts can be created via the file explorer. This is a bit dirty but necessary because the Arduino IDE does not allow linking to relative paths outside of the sketch folder (e.g. `../config/xyz.h`). These symlinks/shortcuts should **not** be committed to the repository.

3. Each Arduino sketch can now be opened and compiled in the Arduino IDE as normal.

# Shared code
## Config
Various configuration parameters for the Serial connection, car sensor threshold values for the display, display colours, etc. can all be configured in the relevant header files in the `config` directory, which both sketches link to once the symlinks/shortcuts have been set up (see above).

## Assets
Shared assets such as font and image bitmap byte arrays are stored in the `assets` folder and linked to by both sketches.

### Converting images for bitmap display
Images can be converted to PROGMEM-resident byte arrays for display using the
`MCUFRIEND_kbv`/`Adafruit_GFX` libraries' `drawBitmap()` method using [this
 tool](https://github.com/javl/image2cpp).

 Images take up a lot of program memory, so size them wisely.

 For example: The UT19 splash screen is 310x119 - much smaller than the screen size - but it is drawn in the centre of the screen rather than drawing a full screen bitmap at (0, 0).

### Fonts
Custom fonts were generated to improve the graphics of the display over the "standard" Adagruit GFX font (which is basically unreadable).

These fonts are stored as byte arrays in PROGMEM, and can very conveniently be created from font files (e.g. `.ttf`) using [this website](http://oleddisplay.squix.ch/#/home) or the `fontconvert` tool provided with the Adafruit GFX library.

Fonts take up a lot of space in the limited program memory, so it's best to keep their character size small (e.g. 32px) and remove any characters that aren't necessary (e.g most symbols, lowercase letters).

# Atom editor configuration
See the `.clang_complete` file for an example on how to set up the [clang linter](https://atom.io/packages/linter-clang)
in atom so it can find the libraries installed by the Arduino IDE. Note that the paths specified in that file must be absolute.
