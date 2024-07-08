/*!
    \file    main.c
    \brief   led spark with systick

    \version 2024-01-15, V3.2.0, firmware for GD32F4xx
*/
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "SEGGER_RTT.h"
#include "exmc_sdram.h"
#include "lcd.h"

/**********************************************************
 * 函 数 名 称：tli_draw_point
 * 函 数 功 能：画点
 * 传 入 参 数：(x,y)：起点坐标
 * 				color：点的颜色
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
**********************************************************/
void tli_draw_point(uint16_t x,uint16_t y,uint16_t color)
{ 
    *(ltdc_lcd_framebuf0[0] + (LCD_WIDTH * y + x ) ) = color;
}

/**********************************************************
 * 函 数 名 称：tli_draw_line
 * 函 数 功 能：画线
 * 传 入 参 数：(sx,sy)：起点坐标
 * 				(ex,ey)：终点坐标
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
**********************************************************/
void tli_draw_line(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=ex-sx; //计算坐标增量 
	delta_y=ey-sy; 
	uRow=sx; 
	uCol=sy; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		tli_draw_point(uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 
/**********************************************************
 * 函 数 名 称：tli_draw_Rectangle
 * 函 数 功 能：画矩形填充
 * 传 入 参 数：(sx,sy) ：起点坐标
 * 			    (sx,sy) ：终点坐标
 * 				color：笔画颜色
* 				fill：填充标志  =1填充颜色  =0不填充
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
**********************************************************/
void tli_draw_Rectangle(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color, uint16_t fill)
{
	int i=0, j=0;
	if( fill )
	{
		for( i = sx; i < ex; i++ )
		{
			for( j = sy; j < ey; j++ )
			{
				tli_draw_point(i,j,color);
			}
		}
	}
	else
	{
		tli_draw_line(sx,sy,ex,sy,color);
		tli_draw_line(sx,sy,sx,ey,color);
		tli_draw_line(sx,ey,ex,ey,color);
		tli_draw_line(ex,sy,ex,ey,color);
	}
}


int8_t a = 0;

int main(void)
{
    ErrStatus init_state;
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    systick_config();
    led_gpio_config();
    init_state = exmc_synchronous_dynamic_ram_init(EXMC_SDRAM_DEVICE0);
    if(init_state == ERROR)
    {
        SEGGER_RTT_printf(0, "a = %d\n", init_state);
    }
    lcd_disp_config();

    tli_draw_Rectangle(0,0,400,480,0x8800,1);
    tli_draw_Rectangle(400,0,800,480,0x001F,1);
    
   while(1)
   {
        gpio_bit_write(GPIOD, GPIO_PIN_7, SET); 
        delay_1ms(100);
        gpio_bit_write(GPIOD, GPIO_PIN_7, RESET); 
        delay_1ms(100);
        a++;    
   }
}
