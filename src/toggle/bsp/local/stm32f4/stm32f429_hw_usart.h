#ifndef	__STM32F429_USART_H__
#define	__STM32F429_USART_H__

extern	void stm32f429_uart_init(int baud, int apbclock);
extern	void stm32f429_uart_tx_byte(uint8_t c);

#endif	/* __STM32F429_USART_H__ */
