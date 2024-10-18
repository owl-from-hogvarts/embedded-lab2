#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "stm32f4xx_hal_def.h"
#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7


uint8_t Check_Row( uint8_t  Nrow );
HAL_StatusTypeDef Set_Keyboard( void );

#endif /* INC_KEYBOARD_H_ */
