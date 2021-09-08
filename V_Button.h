#ifndef V_BUTTON_H
#define V_BUTTON_H

//#include "stm32l4xx_hal.h"
#include "main.h"
//#include "stdio.h"
#include "stdarg.h"

/*Задание
Библиотека обработки нажатий кнопок на прерываниях
*/
//#define millis() HAL_GetTick()
//#define BUTTON_IRQ

#define true	1
#define TRUE	1
#define false	0
#define FALSE	0

#define BTN_NO_PIN	0xFFFF
#define HIGH_PULL		0
#define LOW_PULL		1
#define NORM_OPEN		0
#define NORM_CLOSE	1
#define MANUAL			0
#define AUTO				1

// Сруктура с флагами состояния кнопки
typedef struct	{
	uint8_t btn_state			: 1;	//Флаг состояния кнопки
	uint8_t btn_flag			: 1;	//Флаг нажатия кнопки
	uint8_t btn_deb				: 1;	//Флаг начала борьбы с дребезгом
	uint8_t hold_flag			: 1;	//Флаг удержани
	uint8_t counter_flag	: 1;	//Флаг счета
	
	uint8_t isHolded_f		: 1;	//Флаг "Кнопка удерживается"
	uint8_t isRelease_f		: 1;	//Флаг "Кнопка отпущена"
	uint8_t isPress_f			: 1;	//Флаг "Кнопка нажата"
	uint8_t step_f				: 1;	//Флаг "Кнопка удержана, изменение с шагом"
	uint8_t oneClick_f		: 1;	//Флаг "Кнопка была нажата один раз"
	uint8_t isOne_f				: 1;	//Флаг "Кнопка была нажата и отпущена"
}tStatusButton;

// Сруктура с флагами конфигурации кнопки
typedef struct	{
	uint8_t type					: 1;	// установка типа кнопки (HIGH_PULL - подтянута к питанию, LOW_PULL - к gnd)
	uint8_t inv_state			: 1;	// установка направления (разомкнута/замкнута по умолчанию - NORM_OPEN, NORM_CLOSE)
	uint8_t tickMode			: 1;	// (MANUAL / AUTO) ручной или автоматический опрос кнопки функцией tick()
	uint8_t noPin					: 1;	// флаг виртуальной кнопки
}tConfigButton;

// Структура кнопки
// Одна такая структура занимает 24 байта
typedef struct	{
	GPIO_TypeDef* 	Port;			// Порт, к которому подключена кнопка
	uint16_t 				Pin;			// Пин, к которому подключена кнопка
	tStatusButton 	status;		// Флаги состояния кнопки
	tConfigButton 	conf;			// Флаги конфигурации кнопки
	
	uint8_t 				btn_counter;				// Счетчик кликов
	uint8_t 				last_counter; 			// Эта перемнная принимает кол-во кликов
	uint8_t 				last_hold_counter;	// Тоже принимает кол-во кликов, но тех, что до удержания
	uint32_t 				btn_timer;					// Переменная для таймера
	
	uint16_t 				debounce;						// Время на подавление дребезга
	uint16_t 				hold_timeout;				// Таймаут на удержание кнопки
	uint16_t 				click_timeout;			// Таймаут для накликивания
	uint16_t 				step_timeout;				// Время изменения инкремента при вызове функции isStep(Button* button, uint8_t clicks)
}Button;

/**
	* @brief  Функция создания новой кнопки
	* @note   Функция принимает адрес на структуру типа Button с именем кнопки и
  *         заполляет ее значениями по умолчанию.
  * @param  Адрес на структуру типа Button
	* @param  _port вида GPIOx, где x может быть (A..H) для выбора GPIO порта
	* @param  _pin вида GPIO_Pin_x, где x может быть (0..15) для выбора ножки порта
  * @retval None
  */
void NewButton (Button* button, GPIO_TypeDef* _port, uint16_t _pin);

/**
	* @brief  Функция опроса кнопки
	* @note   Функция опрашивет состояние кнопки и расставляет необходимые флаги
	*					По умолчанию tick() входит во все остальные функции (AUTO).
  * @param  Адрес на структуру типа Button
  * @retval None
  */
void tick(Button* button) ;					// опрос кнопки	
/*void tickALL (int n, ...);				//опрос ВСЕХ кнопок. Сначала указывается количество, потом имена. Временно не работает*/

// ==================== SET ====================
/**
	* @brief  Функция установка времени антидребезга
	* @note   По умолчанию интервал опроса кнопки 60 мс
  * @param  Адрес на структуру типа Button
	* @param  Время debounce, мс
  * @retval None
  */
void setDebounce(Button* button, uint16_t debounce);

/**
	* @brief  Функция установка таймаута удержания
	* @note   По умолчанию таймаут удержания кнопки 500 мс
  * @param  Адрес на структуру типа Button
	* @param  Время new_timeout, мс
  * @retval None
  */
void setHoldTimeout(Button* button, uint16_t new_timeout);

/**
	* @brief  Функция установка таймаута между кликами
	* @note   По умолчанию таймаут между кликами кнопки 500 мс
  * @param  Адрес на структуру типа Button
	* @param  Время new_timeout, мс
  * @retval None
  */
void setClickTimeout(Button* button, uint16_t new_timeout);

/**
	* @brief  Функция установка таймаута между инкрементами
	* @note   По умолчанию таймаут между инкрементами кнопки 400 мс
  * @param  Адрес на структуру типа Button
	* @param  Время new_timeout, мс
  * @retval None
  */
void setStepTimeout(Button* button, uint16_t step_timeout);

/**
	* @brief  Функция установки типа кнопки
	* @note   По умолчанию тип кнопки HIGH_PULL - подтянута к питанию
  * @param  Адрес на структуру типа Button
	* @param  Тип кнопки type
  *            @arg HIGH_PULL	:	кнопка подтянута к питанию
  *            @arg LOW_PULL	:	кнопка подтянута к GND
  * @retval None
  */
void setType(Button* button, uint8_t type);

/**
	* @brief  Функция установки направления кнопки
	* @note   По умолчанию кнопка нормально разомкнутая (NORM_OPEN)
  * @param  Адрес на структуру типа Button
	* @param  Тип кнопки dir
  *            @arg NORM_OPEN		:	кнопка нормально разомкнута
  *            @arg NORM_CLOSE	:	кнопка нормально замкнута
  * @retval None
  */
void setDirection(Button* button, uint8_t dir);

/**
	* @brief  Функция установки режима опроса функции tick()
	* @note   По умолчанию tick() входит во все остальные функции (AUTO)
  * @param  Адрес на структуру типа Button
	* @param  Тип кнопки dir
  *            @arg MANUAL	:	нужно вызывать функцию tick() вручную
  *            @arg AUTO		:	tick() входит во все остальные функции и опрашивается сама
  * @retval None
  */
void setTickMode(Button* button, uint8_t _tickMode);

// ==================== IS ====================
/**
	* @brief  Функция анализа флага "Кнопка нажата"
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при нажатии на кнопку. Сбрасывается после вызова
  */
uint8_t isPress(Button* button);

/**
	* @brief  Функция анализа флага "Кнопка отпущена"
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при отпускании кнопки. Сбрасывается после вызова
  */
uint8_t isRelease(Button* button);

/**
	* @brief  Функция анализа флага "Кнопка была нажата"
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при клике. Сбрасывается после вызова
  */
uint8_t isClick(Button* button);

/**
	* @brief  Функция анализа флага "Кнопка была удержена"
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при удержании дольше hold_timeout. Сбрасывается после вызова
  */
uint8_t isHolded(Button* button);

/**
	* @brief  Функция анализа удержания кнопки
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при удержании кнопки, не сбрасывается
  */
uint8_t isHold(Button* button);

/**
	* @brief  Функция анализа состояния кнопки
  * @param  Адрес на структуру типа Button
  * @retval возвращает состояние кнопки
  */
uint8_t state(Button* button);

/**
	* @brief  Функция анализа кол-ва кликов
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при одиночном клике. Сбрасывается после вызова
  */
uint8_t isSingle(Button* button);

/**
	* @brief  Функция анализа кол-ва кликов
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при двойном клике. Сбрасывается после вызова
  */
uint8_t isDouble(Button* button);

/**
	* @brief  Функция анализа кол-ва кликов
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при тройном клике. Сбрасывается после вызова
  */
uint8_t isTriple(Button* button);

/**
	* @brief  Функция проверки на наличие кликов
  * @param  Адрес на структуру типа Button
  * @retval возвращает true при наличии кликов. Сбрасывается после вызова
  */
uint8_t hasClicks(Button* button);

/**
	* @brief  Функция проверки кол-ва кликов
  * @param  Адрес на структуру типа Button
  * @retval возвращает количество кликов
  */
uint8_t getClicks(Button* button);

/**
	* @brief  Функция проверки кол-ва кликов, предшествующее удержанию
  * @param  Адрес на структуру типа Button
  * @retval возвращает количество кликов, предшествующее удерживанию
  */
uint8_t getHoldClicks(Button* button);

/**
	* @brief  Функция проверки кол-ва кликов, предшествующее удержанию
  * @param  Адрес на структуру типа Button
  * @param  Количество кликов до удержания clicks
  * @retval возвращает true по таймеру step_timeout, если перед удержанием кнопка была нажата clicks раз
  */
uint8_t isStep(Button* button, uint8_t clicks);

/**
	* @brief  Функция сбрасывает все is-флаги и счётчики
  * @param  Адрес на структуру типа Button
  * @retval None
  */
void resetStates(Button* button);

#endif

