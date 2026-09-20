/*
 * PSP Software Development Kit - http://www.psvdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * scr_printf.c - Debug screen functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: scr_printf.c 2450 2009-01-04 23:53:02Z oopo $
 */
#include <vitasdk.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#include "pspdebug.h"

#define PSV_SCREEN_WIDTH  960
#define PSV_SCREEN_HEIGHT 544
#define PSV_LINE_SIZE     960

#define FONT_WIDTH        16
#define FONT_HEIGHT       16

#define FONT_ADVANCE_X    14
#define FONT_ADVANCE_Y    16

#define SCREEN_COLUMNS    68
#define SCREEN_ROWS       34

/* baseado nas libs do Duke... */

void _psvDebugScreenClearLine(int Y);

static int X = 0, Y = 0;
static int MX = SCREEN_COLUMNS;
static int MY = SCREEN_ROWS;
static uint32_t bg_col = 0, fg_col = 0xFFFFFFFF;
static int bg_enable = 1;
static void *g_vram_base = NULL;
static int g_vram_offset = 0;
static int g_vram_mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
static int init = 0;
static int clearline_en = 1;
extern uint16_t msx16[256][FONT_HEIGHT];
extern void msxFontInit16(void);

static void clear_screen_32(uint32_t color) {
  int x;
  uint32_t *vram = g_vram_base;
  vram +=  (g_vram_offset>>2);

  for(x = 0; x < (PSV_LINE_SIZE * PSV_SCREEN_HEIGHT); x++)
  {
    *vram++ = color; 
  }
}

static void clear_screen(uint32_t color) {
  if (g_vram_mode == SCE_DISPLAY_PIXELFORMAT_A8B8G8R8) {
    clear_screen_32(color);
  }
}

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

void psvDebugScreenInitEx(void *vram_base, int mode, int setup) {
  switch(mode) {
    case SCE_DISPLAY_PIXELFORMAT_A8B8G8R8:
      break;
    default: mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
  };

  X = Y = 0;

  msxFontInit16();

  if(vram_base == NULL) {
    int block = sceKernelAllocMemBlock("", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, ALIGN(PSV_LINE_SIZE * PSV_SCREEN_HEIGHT * 4, 256 * 1024), NULL);
    sceKernelGetMemBlockBase(block, &vram_base);
  }
  g_vram_base = vram_base;
  g_vram_offset = 0;
  g_vram_mode = mode;
  if(setup)
  {
    SceDisplayFrameBuf framebuf;
    framebuf.size = sizeof(SceDisplayFrameBuf);
    framebuf.base = (void *)g_vram_base;
    framebuf.pitch = PSV_LINE_SIZE;
    framebuf.width = PSV_SCREEN_WIDTH;
    framebuf.height = PSV_SCREEN_HEIGHT;
    framebuf.pixelformat = mode;
    sceDisplaySetFrameBuf(&framebuf, 1);
  }
  clear_screen(bg_col);
  init = 1;
}

void psvDebugScreenInit() {
  X = Y = 0;
  psvDebugScreenInitEx(NULL, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, 1);
}

void psvDebugScreenEnableBackColor(int enable) {
  bg_enable = enable;
}

void psvDebugScreenSetBackColor(uint32_t colour) {
  bg_col = colour;
}

void psvDebugScreenSetTextColor(uint32_t colour) {
  fg_col = colour;
}

void psvDebugScreenSetColorMode(int mode) {
  switch(mode)
  {
    case SCE_DISPLAY_PIXELFORMAT_A8B8G8R8:
      break;
    default: mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
  };

  g_vram_mode = mode;
}

int psvDebugScreenGetX() {
  return X;
}

int psvDebugScreenGetY() {
  return Y;
}

void psvDebugScreenClear() {
  int y;

  if(!init) {
    return;
  }

  for(y=0;y<MY;y++) {
    _psvDebugScreenClearLine(y);
  }

  psvDebugScreenSetXY(0,0);
  clear_screen(bg_col);
}

void psvDebugScreenSetXY(int x, int y) {
  if( x<MX && x>=0 ) X=x;
  if( y<MY && y>=0 ) Y=y;
}

void psvDebugScreenSetOffset(int offset) {
  g_vram_offset = offset;
}

void psvDebugScreenSetBase(uint32_t* base) {
  g_vram_base = base;
}

extern uint8_t msx[]; 
static void debug_put_char_32(int x, int y, uint32_t color, uint32_t bgc, uint8_t ch) {
    int row;
    int column;
    uint32_t *vram;
    uint32_t *vram_ptr;

    if (!init || g_vram_base == NULL)
        return;
  
    if (x < 0 || y < 0)
        return;

    if (x >= PSV_SCREEN_WIDTH || y >= PSV_SCREEN_HEIGHT)
        return;

    vram = (uint32_t *)g_vram_base;
    vram += (g_vram_offset >> 2);
    vram += x;
    vram += y * PSV_LINE_SIZE;

    for (row = 0; row < FONT_HEIGHT; row++) {
        uint16_t font_row;

        if ((y + row) >= PSV_SCREEN_HEIGHT)
            break;

        font_row = msx16[ch][row];
        vram_ptr = vram;

        for (column = 0; column < FONT_WIDTH; column++) {
            if ((x + column) >= PSV_SCREEN_WIDTH)
                break;

            if (font_row & ((uint16_t)0x8000U >> column)) {
                *vram_ptr = color;
            } else if (bg_enable) {
                *vram_ptr = bgc;
            }

            vram_ptr++;
        }

        vram += PSV_LINE_SIZE;
    }
}

void psvDebugScreenPutChar( int x, int y, uint32_t color, uint8_t ch) {
  if(g_vram_mode == SCE_DISPLAY_PIXELFORMAT_A8B8G8R8)
  {
    debug_put_char_32(x, y, color, bg_col, ch);
  }
}

void _psvDebugScreenClearLine(int line) {
    int i;

    if (!clearline_en || !bg_enable)
        return;

    for (i = 0; i < MX; i++)
        psvDebugScreenPutChar(i * FONT_ADVANCE_X, line * FONT_ADVANCE_Y, bg_col, 219);
}

void psvDebugScreenClearLineEnable(void) {
  clearline_en = 1;
}

void psvDebugScreenClearLineDisable(void) {
  clearline_en = 0;
}

static void debug_next_line(void) {
    X = 0;
    Y++;

    if (Y >= MY)
        Y = 0;

    _psvDebugScreenClearLine(Y);
}

/* Print non-nul terminated strings */
int psvDebugScreenPrintData(const char *buff, int size) {
  int i;
  int j;

  if (!init || buff == NULL || size <= 0) {
    return 0;
  }
  
    for (i = 0; i < size; i++) {
        uint8_t c = (uint8_t)buff[i];

        switch (c) {
            case '\r':
                X = 0;
                break;

            case '\n':
                debug_next_line();
                break;

            case '\t':
                for (j = 0; j < 5; j++) {
                    psvDebugScreenPutChar(X * FONT_ADVANCE_X, Y * FONT_ADVANCE_Y, fg_col, (uint8_t)' ');
                    X++;
                    if (X >= MX) {
                        debug_next_line();
                    }
                }
                break;

            default:
                psvDebugScreenPutChar(X * FONT_ADVANCE_X, Y * FONT_ADVANCE_Y, fg_col, c);
                X++;
                if (X >= MX) {
                    debug_next_line();
                }
                break;
        }
    }

    return i;
}

int psvDebugScreenPuts(const char *str) {
    if (str == NULL)
        return 0;

    return psvDebugScreenPrintData(str, sceClibStrnlen(str, 2048));
}

void psvDebugScreenPrintf(const char *format, ...) {
    va_list opt;
    char buff[2048];
    int bufsz;

    if (format == NULL)
        return;

    va_start(opt, format);

    bufsz = sceClibVsnprintf(buff, (size_t)sizeof(buff), format, opt);

    va_end(opt);

    if (bufsz < 0)
        return;

    if (bufsz >= (int)sizeof(buff)) {
        bufsz = (int)sizeof(buff) - 1;
    }

  (void) psvDebugScreenPrintData(buff, bufsz);
}