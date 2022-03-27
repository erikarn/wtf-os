#ifndef	__STM32F429_USART_H__
#define	__STM32F429_USART_H__

extern	void stm32f429_uart_init(uint32_t baud, uint32_t apbclock);
extern	void stm32f429_uart_tx_byte(uint8_t c);
extern	void stm32f429_uart_enable_rx_intr(void);
extern	void stm32f429_uart_disable_rx_intr(void);

#endif	/* __STM32F429_USART_H__ */
