#include "V_Button.h"
// ==================== CONSTRUCTOR ====================

void NewButton (Button* button, GPIO_TypeDef* _port, uint16_t _pin)
{
	// Настройки по умолчанию
	if (_pin != BTN_NO_PIN)										//Проверка, что _pin != 0xFFFF (что указан пин)
	{
		button->Port 						= _port;				//Порт кнопки
		button->Pin 						= _pin;					//Пин кнопки
		button->conf.noPin 			= false;				//Флаг виртуальной кнопки
	} 
	else 
	{
		button->conf.noPin = true;									//Если _pin == 0xFFFF имеем виртуальную кнопку (пока не реализовано)
	}
	button->conf.tickMode 		= AUTO;							//AUTO - Вызов tick() при вызове ф-ций is...()/MANUAL - вызов в цикле
	button->conf.type 				= HIGH_PULL;				//Подтянута к питанию (HIGH_PULL) или к земле (LOW_PULL)
	button->conf.inv_state 		= NORM_OPEN;				// NORM_OPEN - кнопка по умолчанию разомкнута (по умолчанию) / NORM_CLOSE - кнопка по умолчанию замкнута
	button->debounce 					= 60;								//Время на подавление дребезга
	button->hold_timeout 			= 500;							//Таймаут на удержание кнопки
	button->click_timeout			= 500;							//Таймаут для накликивания 
	button->step_timeout 			= 400;							//Время изменения инкремента при вызове функции isStep(Button* button, uint8_t clicks)
	button->btn_counter 			= 0;								//Обнуляем счетчик кликов
	button->last_counter			= 0;								//Эта перемнная принимает кол-во кликов. Обнуляем
	button->last_hold_counter = 0;								//Тоже принимает кол-во кликов, но тех, что до удержания. Обнуляем
	button->btn_timer 				= 0;								//Таймер для сравнения с HAL_GetTick()
	button->status.btn_state 	= false;						//Обнуляем флаг состояния кнопки
	button->status.btn_flag 	= false;						//Обнуляем флаг нажатия кнопки
}

// ==================== TICK ====================

void tick(Button* button) 
	{	
	// читаем пин
	if (!(button->conf.noPin)) 
	{
		button->status.btn_state = !(HAL_GPIO_ReadPin(button->Port, button->Pin)) ^ ((button->conf.inv_state) ^ (button->conf.type));
	}
	
	uint32_t thisMls = HAL_GetTick();
	
	// нажатие
	if ((button->status.btn_state) && !(button->status.btn_flag)) // Если кнопку нажали и btn_flag опущен
	{
		if (!(button->status.btn_deb)) 															// Если флаг btn_deb опущен 
		{													
			button->status.btn_deb = true;														//Поднять флаг btn_deb, чтобы начать борьбу с дребезгом
			button->btn_timer = thisMls;															//Зарядить таймер
		} 													
		else 																												// Если флаг btn_deb поднят
		{
			if (thisMls - (button->btn_timer) >= button->debounce) 		// Опрашиваем с интервалом debounce (боримся с дребезгом)
			{
				button->status.btn_flag = true;		
				button->status.isPress_f = true;	
				button->status.oneClick_f = true;	
			}
		}    
	} else {
		button->status.btn_deb = false;
	}

	// отпускание
	if (!(button->status.btn_state) && (button->status.btn_flag)) {
		button->status.btn_flag = false;
		if (!button->status.hold_flag) 
		{
			button->btn_counter++;
		}
		button->status.hold_flag = false;
		button->status.isRelease_f = true;
		button->btn_timer = thisMls;
		if (button->status.step_f) {
			button->last_counter = 0;
			button->btn_counter = 0;
			button->status.step_f = false;	
		}		
		if (button->status.oneClick_f) {
			button->status.oneClick_f = false;
			button->status.isOne_f = true;
		}
	}

	// кнопка удерживается
	if ((button->status.btn_flag) && (button->status.btn_state) && (thisMls - (button->btn_timer) >= (button->hold_timeout)) && !(button->status.hold_flag)) {
		button->status.hold_flag = true;
		button->last_hold_counter = button->btn_counter;
		button->btn_counter = 0;
		button->last_counter = 0;
		button->status.isHolded_f = true;
		button->status.step_f = true;
		button->status.oneClick_f = false;
		button->btn_timer = thisMls;
	}

	// обработка накликивания
	if ((thisMls - (button->btn_timer) >= (button->click_timeout)) && (button->btn_counter > 0) ) {    
		button->last_counter = button->btn_counter;
		button->btn_counter = 0;
		button->status.counter_flag = true;
	}
}
/*void tickALL (int n, ...)
{
	va_list factor;         //указатель va_list
	va_start(factor, n);    // устанавливаем указатель
	
	for(int i=0; i<n; i++)
	{
		Button SW = va_arg(factor, Button);
		tick(&SW);
	}
	va_end(factor); // завершаем обработку параметров
}*/
// ==================== SET ====================
void setDebounce(Button* button, uint16_t _debounce) 
{
	button->debounce = _debounce;
}
void setHoldTimeout(Button* button, uint16_t _new_timeout) {
	button->hold_timeout = _new_timeout;
}
void setClickTimeout(Button* button, uint16_t new_timeout) {
	button->click_timeout = new_timeout;
}
void setStepTimeout(Button* button, uint16_t _step_timeout) {
	button->step_timeout = _step_timeout;
}
void setType(Button* button, uint8_t _type) {
	button->conf.type = _type;
}
void setDirection(Button* button, uint8_t dir) {
	button->conf.inv_state = dir;
}
void setTickMode(Button* button, uint8_t _tickMode) {
	button->conf.tickMode = _tickMode;
}

// ==================== IS ====================
uint8_t isPress(Button* button) 
	{
	if (button->conf.tickMode) tick(button);
	if (button->status.isPress_f) {
		button->status.isPress_f = false;
		return true;
	} else return false;
}
uint8_t isRelease(Button* button) {
	if (button->conf.tickMode) tick(button);
	if (button->status.isRelease_f) {
		button->status.isRelease_f = false;
		return true;
	} else return false;
}
uint8_t isClick(Button* button) {	
	if (button->conf.tickMode) tick(button);
	if (button->status.isOne_f) {
		button->status.isOne_f = false;
		return true;
	} else return false;
}
uint8_t isHolded(Button* button) {
	if (button->conf.tickMode) tick(button);
	if (button->status.isHolded_f) {
		button->status.isHolded_f = false;
		return true;
	} else return false;
}
uint8_t isHold(Button* button) {
	if (button->conf.tickMode) tick(button);
	if (button->status.step_f) return true;
	else return false;
}
uint8_t state(Button* button) {
	if (button->conf.tickMode) tick(button);
	return button->status.btn_state;
}
uint8_t isSingle(Button* button) {
	if (button->conf.tickMode) tick(button);
	if (button->status.counter_flag && (button->last_counter == 1)) {
		button->last_counter = 0;
		button->status.counter_flag = false;
		return true;
	} else return false;
}
uint8_t isDouble(Button* button) {
	if (button->conf.tickMode) tick(button);
	if ((button->status.counter_flag) && (button->last_counter) == 2) {
		button->status.counter_flag = false;
		button->last_counter = 0;
		return true;
	} else return false;
}
uint8_t isTriple(Button* button) {
	if (button->conf.tickMode) tick(button);
	if ((button->status.counter_flag) && (button->last_counter) == 3) {
		button->status.counter_flag = false;
		button->last_counter = 0;
		return true;
	} else return false;
}
uint8_t hasClicks(Button* button) {
	if (button->conf.tickMode) tick(button);
	if (button->status.counter_flag) {
		button->status.counter_flag = false;
		return true;
	} else return false;
}
uint8_t getClicks(Button* button) {
	uint8_t thisCount = button->last_counter;
	button->last_counter = 0;
	return thisCount;	
}
uint8_t getHoldClicks(Button* button) {
	if (button->conf.tickMode) tick(button);
	return (button->status.hold_flag) ? (button->last_hold_counter) : 0;	
}
uint8_t isStep(Button* button, uint8_t clicks) {
	if (button->conf.tickMode) tick(button);
	if (((button->last_hold_counter) == clicks) && (button->status.step_f) && (HAL_GetTick() - (button->btn_timer) >= (button->step_timeout))) {
		button->btn_timer = HAL_GetTick();		
		return true;
	}
	else return false;
}

void resetStates(Button* button) {
	button->status.isPress_f = false;
	button->status.isRelease_f = false;
	button->status.isOne_f = false;
	button->status.isHolded_f = false;
	button->status.step_f = false;
	button->status.counter_flag = false;
	button->last_hold_counter = 0;
	button->last_counter = 0;
}

#ifdef BUTTON_IRQ
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/*if(GPIO_Pin == s[0]->Pin)
	{
		if(HAL_GetTick() - s[0]->tmr > s[0]->time_debounce)
		{
			s[0]->f1 = 1;
		}
	}*/
}
#endif
