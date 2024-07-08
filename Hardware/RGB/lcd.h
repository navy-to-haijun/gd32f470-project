#ifndef GD32F450Z_LCD_H
#define GD32F450Z_LCD_H

#include <stdint.h>
#include "exmc_sdram.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_FB_BYTE_PER_PIXEL 1


#define HORIZONTAL_SYNCHRONOUS_PULSE 10
#define HORIZONTAL_BACK_PORCH 150
#define ACTIVE_WIDTH 800
#define HORIZONTAL_FRONT_PORCH 15

#define VERTICAL_SYNCHRONOUS_PULSE 10
#define VERTICAL_BACK_PORCH 140
#define ACTIVE_HEIGHT 480
#define VERTICAL_FRONT_PORCH 40

#define LCD_FRAME_BUF_ADDR 0XC0000000


extern uint16_t ltdc_lcd_framebuf0[800][480];
// extern uint16_t (*ltdc_lcd_framebuf0)[LCD_HEIGHT];

/*******************************************************************************
 * API
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void lcd_disp_config(void);

#if defined(__cplusplus)
}
#endif

#endif /* GD32F450Z_LCD_H */
