#ifndef __MOTOR_H
#define __MOTOR_H

typedef struct {
	GPIO_TypeDef* FORWARD1_GPIOx;    
	uint16_t FORWARD1_GPIO_Pin;      
	GPIO_TypeDef* BACK1_GPIOx; 
	uint16_t BACK1_GPIO_Pin;   

	GPIO_TypeDef* FORWARD2_GPIOx;    
	uint16_t FORWARD2_GPIO_Pin;      
	GPIO_TypeDef* BACK2_GPIOx; 
	uint16_t BACK2_GPIO_Pin;
	} Motort;
#endif

