#ifndef _LED_H
#define _LED_H

#include "gd32f4xx.h"
#include "systick.h"

/*开发板上提供4个LED灯*/
#define RCU_LED1  	RCU_GPIOE
#define PORT_LED1 	GPIOE	
#define PIN_LED1    GPIO_PIN_3  

#define RCU_LED2  	RCU_GPIOD
#define PORT_LED2 	GPIOD	
#define PIN_LED2    GPIO_PIN_7  

#define RCU_LED3  	RCU_GPIOG
#define PORT_LED3 	GPIOG	
#define PIN_LED3    GPIO_PIN_3

#define RCU_LED4  	RCU_GPIOA
#define PORT_LED4 	GPIOA	
#define PIN_LED4    GPIO_PIN_5


void led_gpio_config(void);

#endif
