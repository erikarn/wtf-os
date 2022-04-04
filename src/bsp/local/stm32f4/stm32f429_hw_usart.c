#include <stdint.h>

#include "../os/reg.h"
#include "../os/bit.h"
#include "../os/bitmask.h"

#include "stm32f429_hw_map.h"
#include "stm32f429_hw_usart_reg.h"
#include "stm32f429_hw_usart.h"

#include <core/platform.h>

/*
 * This driver is purely for USART1.  Right now it doesn't
 * "know" about the mappings between APB1/APB2 and the RCC
 * registers to poke.  That can come later.
 */

static int16_t rx_char = -1;

static void
stm32f429_uart_set_baud(uint32_t baud, uint32_t apbclock,
    int is_oversampling)
{
	uint32_t pre, m, frac, reg;

	/* Calculate the pre scaled value */
	if (is_oversampling) {
		pre = (apbclock * 25) / (16 * baud);
	} else {
		pre = (apbclock * 25) / (8 * baud);
	}

	/* Calculate mantissa value to use */
	m = pre / 25;

	/* calculate fractional part to use */
	frac = pre - (m * 25);

	if (is_oversampling) {
		frac = frac & 0xf;
	} else {
		frac = frac & 0x7;
	}

	/* program it in */
	reg = os_reg_read32(USART1_BASE, USART_BRR);
	reg = RMW(reg, USART_BRR_DIV_MANTISSA, m);
	reg = RMW(reg, USART_BRR_DIV_FRACTION, frac);
	os_reg_write32(USART1_BASE, USART_BRR, reg);
}

/*
 * Setup the UART for the given baud rate with the given
 * AHB clock.  For now the AHB clock for this USART comes
 * from the STM32 vendor BSP and I don't want to call it
 * from here.
 */
void
stm32f429_uart_init(uint32_t baud, uint32_t apbclock)
{
	uint32_t reg;

	/* disable tx/rx first, prescalers/outputs */
	reg = os_reg_read32(USART1_BASE, USART_CR1);
	reg &= ~ (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
	os_reg_write32(USART1_BASE, USART_CR1, reg);

	/* Set configuration, 8N1, 16x sampling, no flow control */
	reg = os_reg_read32(USART1_BASE, USART_CR1);
	reg &= ~ USART_CR1_OVER8; /* 16x */
	reg &= ~ USART_CR1_M; /* 8 bit */
	reg &= ~ USART_CR1_WAKE; /* wake on idle transition */
	reg &= ~ USART_CR1_PCE; /* no parity */
	reg &= ~ (USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE |
	    USART_CR1_RXNEIE | USART_CR1_IDLEIE | USART_CR1);
		/* disable interrupt generation */
	reg &= ~ USART_CR1_RWU; /* leave it in active mode */
	reg &= ~ USART_CR1_SBK; /* we're not sending a break right now */
	os_reg_write32(USART1_BASE, USART_CR1, reg);

	reg = os_reg_read32(USART1_BASE, USART_CR2);
	reg &= ~ USART_CR2_LINEN; /* disable break send/receive for now */
	reg = RMW(reg, USART_CR2_STOP, 0x0); /* 1 stop bit */
	reg &= ~ USART_CR2_CLKEN; /* disable sync clock (CK) pin */
	reg &= ~ (USART_CR2_CPOL | USART_CR2_CPHA | USART_CR2_LBCL);
	    /* sync serial: low polarity, first edge = capture; last bit
	     * clock pulse */
	reg &= ~ USART_CR2_LBDIE; /* don't interrupt on break detect */
	reg &= ~ USART_CR2_LBDL; /* 10 bit break detection */
	reg = RMW(reg, USART_CR2_ADD, 0);
	    /* address 0 for multi-proc UART communication */
	os_reg_write32(USART1_BASE, USART_CR2, reg);

	reg = os_reg_read32(USART1_BASE, USART_CR3);
	reg &= ~ USART_CR3_ONEBIT; /* 3 sample bit for data bits */
	reg &= ~ (USART_CR3_CTSIE | USART_CR3_EIE);
	    /* disable CTS/error interrupts */
	reg &= ~ (USART_CR3_CTSE | USART_CR3_RTSE);
	    /* disable RTS/CTS */
	reg &= ~ (USART_CR3_DMAT | USART_CR3_DMAR);
	    /* disable DMA for TX/RX */
	reg &= ~ USART_CR3_SCEN; /* disable smartcard mode */
	reg &= ~ USART_CR3_NACK;
	    /* smartcard NACK tx on parity error disabled */
	reg &= ~ USART_CR3_HDSEL; /* disable half-duplex operation */
	reg &= ~ USART_CR3_IRLP; /* disable IrDA low power */
	reg &= ~ USART_CR3_IREN; /* disable IrDA */
	os_reg_write32(USART1_BASE, USART_CR3, reg);

	/* Set baud rate, we'll use oversampling */
	stm32f429_uart_set_baud(baud, apbclock, 1);

	/* Enable TX/RX */
	reg = os_reg_read32(USART1_BASE, USART_CR1);
	reg |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
	os_reg_write32(USART1_BASE, USART_CR1, reg);
}

/*
 * Write a byte to the UART and wait for it to complete.
 */
void
stm32f429_uart_tx_byte(uint8_t c)
{

	/*
	 * There's no "I'm busy" bit, only "I've sent it to the
	 * transmit shift register" bit.
	 */
	os_reg_write32(USART1_BASE, USART_DR, c);

	/* Wait until it's going out to the shift register */
	while ((os_reg_read32(USART1_BASE, USART_SR) & USART_SR_TXE) == 0)
		;
}

/**
 * UART interrupt handler.
 *
 * This is called by the UART interrupt to process an interrupt.
 */
void
stm32f429_uart_interrupt(void)
{
	uint32_t reg;
	uint32_t r;

	reg = os_reg_read32(USART1_BASE, USART_SR);

	/*
	 * Test OE before RXNE, as reading from DR will clear both
	 * conditions.
	 */
	if (reg & USART_SR_ORE) {
		/* XXX TODO */
	}
	if (reg & USART_SR_RXNE) {
		r = os_reg_read32(USART1_BASE, USART_DR);
		rx_char = (uint16_t) (MS(r, USART_DR_DATA)) & 0xff;
	}
}

/**
 * Return if there's a character in the input buffer, or
 * -1 if there isn't.
 *
 * This routine is non-blocking, and it designed to be called
 * from the interrupt context after the UART interrupt routine
 * has run.
 *
 * @retval -1 if no character, or >= 0 if a character has been read.
 */
int16_t
stm32f429_uart_try_read(void)
{
	int16_t r;

	r = rx_char;
	rx_char = -1;
	return (r);
}

/**
 * Enable interrupts for receive.
 */
void
stm32f429_uart_enable_rx_intr(void)
{
	uint32_t reg;

	reg = os_reg_read32(USART1_BASE, USART_CR1);
	reg |= USART_CR1_RXNEIE;
	os_reg_write32(USART1_BASE, USART_CR1, reg);

	platform_irq_enable(37); /* XXX TODO: hard-coded; move it */
}

/**
 * Disable interrupts for receive.
 */
void
stm32f429_uart_disable_rx_intr(void)
{
	uint32_t reg;

	platform_irq_disable(37); /* XXX TODO: hard-coded; move it */

	reg = os_reg_read32(USART1_BASE, USART_CR1);
	reg &= ~USART_CR1_RXNEIE;
	os_reg_write32(USART1_BASE, USART_CR1, reg);

}
