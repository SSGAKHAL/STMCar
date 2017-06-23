#ifndef __HCSR04_H
#define __HCSR04_H

#define SR04_OFFSET 0.8


typedef struct {
	GPIO_TypeDef* ECHO_GPIOx;    /* Pointer to GPIOx PORT for ECHO pin. */
	uint16_t ECHO_GPIO_Pin;      /* GPIO Pin for ECHO pin. */
	GPIO_TypeDef* TRIGGER_GPIOx; /* Pointer to GPIOx PORT for TRIGGER pin. */
	uint16_t TRIGGER_GPIO_Pin;   /* GPIO Pin for ECHO pin.*/
} HCSRO4t;
#endif


