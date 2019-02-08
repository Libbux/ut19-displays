#include <Adafruit_GFX.h>
/**
 * This function has been copied directly from the Adafruit GFX library,
 * with the indicated section modified to speed up text rendering.
 * It's *very* effective.
 * Source: https://github.com/adafruit/Adafruit-GFX-Library/issues/69
 * OR: https://forum.arduino.cc/index.php?topic=369222.msg2547389#msg2547389
 **/
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color, no background)
    @param    size  Font magnification level, 1 is 'original' size
*/
/**************************************************************************/
void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
  uint16_t color, uint16_t bg, uint8_t size) {

    if(!gfxFont) { // 'Classic' built-in font

        if((x >= _width)            || // Clip right
           (y >= _height)           || // Clip bottom
           ((x + 6 * size - 1) < 0) || // Clip left
           ((y + 8 * size - 1) < 0))   // Clip top
            return;

        if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        startWrite();
        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size == 1)
                        writePixel(x+i, y+j, color);
                    else
                        writeFillRect(x+i*size, y+j*size, size, size, color);
                } else if(bg != color) {
                    if(size == 1)
                        writePixel(x+i, y+j, bg);
                    else
                        writeFillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size == 1) writeFastVLine(x+5, y, 8, bg);
            else          writeFillRect(x+5*size, y, size, 8*size, bg);
        }
        endWrite();

    } else { // Custom font

        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;

        if(size > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        /** === BEGIN: Fast font rendering === **/
        /**
         * Taken directly from the following issue on GitHub
         * https://github.com/adafruit/Adafruit-GFX-Library/issues/69
         **/
        startWrite();
        #define FAST_TEXT // Comment to quickly disable
        #ifdef FAST_TEXT
            // Custom "fast font rendering" from https://github.com/adafruit/Adafruit-GFX-Library/issues/69
            uint16_t hpc = 0; // Horizontal foreground pixel count
            for(yy=0; yy<h; yy++) {
                for(xx=0; xx<w; xx++) {
                    if(bit == 0) {
                        bits = pgm_read_byte(&bitmap[bo++]);
                        bit  = 0x80;
                    }
                    if(bits & bit) hpc++;
                    else {
                        if (hpc) {
                            if(size == 1) drawFastHLine(x+xo+xx-hpc, y+yo+yy, hpc, color);
                            else fillRect(x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
                            hpc=0;
                        }
                    }
                    bit >>= 1;
                }
                // Draw pixels for this line as we are about to increment yy
                if (hpc) {
                    if(size == 1) drawFastHLine(x+xo+xx-hpc, y+yo+yy, hpc, color);
                    else fillRect(x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
                    hpc=0;
                }
            }
        #else
            // FAST_TEXT rendering disabled
            // This should be identical to the default implementation
            for(yy=0; yy<h; yy++) {
                for(xx=0; xx<w; xx++) {
                    if(!(bit++ & 7)) {
                        bits = pgm_read_byte(&bitmap[bo++]);
                    }
                    if(bits & 0x80) {
                        if(size == 1) {
                            writePixel(x+xo+xx, y+yo+yy, color);
                        } else {
                            writeFillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,
                              size, size, color);
                        }
                    }
                    bits <<= 1;
                }
            }
        #endif
        /** === END fast text rendering === **/
        endWrite();
    }
}
