
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.global  g_pfnVectors
.global  _arm_m4_default_interrupt_handler
.global  _arm_m4_default_exception_handler
.global Default_Interrupt_Handler
.global Default_Exception_Handler

/*
 * start address for the initialization values of the .data section,
 * BSS sections, as defined in the linker script.
 */
.word  _sidata
/* start address for the .data section. defined in linker script */
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss

/**
 * Entry point after a reset/power-on.
 *
 * This is what the STM32 HAL does.
 */
.section  .text.Reset_Handler
.weak  Reset_Handler
.type  Reset_Handler, %function

Reset_Handler:

	/* Copy the data segment initializers from flash to SRAM */
	movs  r1, #0
	b  LoopCopyDataInit

CopyDataInit:
	ldr  r3, =_sidata
	ldr  r3, [r3, r1]
	str  r3, [r0, r1]
	adds  r1, r1, #4

LoopCopyDataInit:
	ldr  r0, =_sdata
	ldr  r3, =_edata
	adds  r2, r0, r1
	cmp  r2, r3
	bcc  CopyDataInit
	ldr  r2, =_sbss
	b  LoopFillZerobss

/* Zero fill the bss segment. */
FillZerobss:
	movs  r3, #0
	str  r3, [r2], #4

LoopFillZerobss:
	ldr  r3, = _ebss
	cmp  r2, r3
	bcc  FillZerobss

	/*
	 * Initialise the clock, flash, power regulator, etc
	 * stuff required for bootstrapping the system.
	 */
	bl  SystemInit

	/* Call static constructors */
	bl __libc_init_array

	/* Call the application's entry point.*/
	bl  main
	bx  lr
.size  Reset_Handler, .-Reset_Handler

/*
 * Default implementations of interrupt/exception handlers.
 * These exist purely to be weak symbols to be overridden
 * by implementations that define the STM32 HAL style function
 * names rather than needing the vector table to be moved
 * completely into RAM.
 */
.section  .text.Default_Interrupt_Handler,"ax",%progbits
Default_Interrupt_Handler:
	b _arm_m4_default_interrupt_handler
.size  Default_Interrupt_Handler, .-Default_Interrupt_Handler

.section  .text.Default_Exception_Handler,"ax",%progbits
Default_Exception_Handler:
	b _arm_m4_default_exception_handler
.size  Default_Exception_Handler, .-Default_Exception_Handler

/*
 * This is the vector table for the Cortex M-4 SoC.
 *
 * It should be arranged to appear at address 0x00000000
 * for reboot; although the bootstrap code above will
 * eventually relocate it into a flash alias memory address
 * area.
 */
.section  .isr_vector,"a",%progbits
.type  g_pfnVectors, %object
.size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
	.word  _estack
	.word  Reset_Handler
	.word  NMI_Handler
	.word  HardFault_Handler
	.word  MemManage_Handler
	.word  BusFault_Handler
	.word  UsageFault_Handler
	.word  0
	.word  0
	.word  0
	.word  0
	.word  SVC_Handler
	.word  DebugMon_Handler
	.word  0
	.word  PendSV_Handler
	.word  SysTick_Handler

  /* External Interrupts */
	.word     WWDG_IRQHandler
	.word     PVD_IRQHandler
	.word     TAMP_STAMP_IRQHandler
	.word     RTC_WKUP_IRQHandler
	.word     FLASH_IRQHandler
	.word     RCC_IRQHandler
	.word     EXTI0_IRQHandler
	.word     EXTI1_IRQHandler
	.word     EXTI2_IRQHandler
	.word     EXTI3_IRQHandler
	.word     EXTI4_IRQHandler
	.word     DMA1_Stream0_IRQHandler
	.word     DMA1_Stream1_IRQHandler
	.word     DMA1_Stream2_IRQHandler
	.word     DMA1_Stream3_IRQHandler
	.word     DMA1_Stream4_IRQHandler
	.word     DMA1_Stream5_IRQHandler
	.word     DMA1_Stream6_IRQHandler
	.word     ADC_IRQHandler
	.word     CAN1_TX_IRQHandler
	.word     CAN1_RX0_IRQHandler
	.word     CAN1_RX1_IRQHandler
	.word     CAN1_SCE_IRQHandler
	.word     EXTI9_5_IRQHandler
	.word     TIM1_BRK_TIM9_IRQHandler
	.word     TIM1_UP_TIM10_IRQHandler
	.word     TIM1_TRG_COM_TIM11_IRQHandler
	.word     TIM1_CC_IRQHandler
	.word     TIM2_IRQHandler
	.word     TIM3_IRQHandler
	.word     TIM4_IRQHandler
	.word     I2C1_EV_IRQHandler
	.word     I2C1_ER_IRQHandler
	.word     I2C2_EV_IRQHandler
	.word     I2C2_ER_IRQHandler
	.word     SPI1_IRQHandler
	.word     SPI2_IRQHandler
	.word     USART1_IRQHandler
	.word     USART2_IRQHandler
	.word     USART3_IRQHandler
	.word     EXTI15_10_IRQHandler
	.word     RTC_Alarm_IRQHandler
	.word     OTG_FS_WKUP_IRQHandler
	.word     TIM8_BRK_TIM12_IRQHandler
	.word     TIM8_UP_TIM13_IRQHandler
	.word     TIM8_TRG_COM_TIM14_IRQHandler
	.word     TIM8_CC_IRQHandler
	.word     DMA1_Stream7_IRQHandler
	.word     FSMC_IRQHandler
	.word     SDIO_IRQHandler
	.word     TIM5_IRQHandler
	.word     SPI3_IRQHandler
	.word     UART4_IRQHandler
	.word     UART5_IRQHandler
	.word     TIM6_DAC_IRQHandler
	.word     TIM7_IRQHandler
	.word     DMA2_Stream0_IRQHandler
	.word     DMA2_Stream1_IRQHandler
	.word     DMA2_Stream2_IRQHandler
	.word     DMA2_Stream3_IRQHandler
	.word     DMA2_Stream4_IRQHandler
	.word     ETH_IRQHandler
	.word     ETH_WKUP_IRQHandler
	.word     CAN2_TX_IRQHandler
	.word     CAN2_RX0_IRQHandler
	.word     CAN2_RX1_IRQHandler
	.word     CAN2_SCE_IRQHandler
	.word     OTG_FS_IRQHandler
	.word     DMA2_Stream5_IRQHandler
	.word     DMA2_Stream6_IRQHandler
	.word     DMA2_Stream7_IRQHandler
	.word     USART6_IRQHandler
	.word     I2C3_EV_IRQHandler
	.word     I2C3_ER_IRQHandler
	.word     OTG_HS_EP1_OUT_IRQHandler
	.word     OTG_HS_EP1_IN_IRQHandler
	.word     OTG_HS_WKUP_IRQHandler
	.word     OTG_HS_IRQHandler
	.word     DCMI_IRQHandler
	.word     CRYP_IRQHandler
	.word     HASH_RNG_IRQHandler
	.word     FPU_IRQHandler
	.word     UART7_IRQHandler
	.word     UART8_IRQHandler
	.word     SPI4_IRQHandler
	.word     SPI5_IRQHandler
	.word     SPI6_IRQHandler
	.word     SAI1_IRQHandler
	.word     LTDC_IRQHandler
	.word     LTDC_ER_IRQHandler
	.word     DMA2D_IRQHandler

/*
 * Provide weak aliases for each interrupt handler to
 * _arm_m4_default_interrupt_handler.  This allows us to
 * use default handlers in C for some VTOR-in-RAM, but also
 * allow for flash implementations of the above symbols
 * so we don't need to waste RAM on a completely separate
 * VTOR if we don't want to.
 */
	.weak		NMI_Handler
	.thumb_set 	NMI_Handler,Default_Exception_Handler

	.weak		HardFault_Handler
	.thumb_set 	HardFault_Handler,Default_Exception_Handler

	.weak		MemManage_Handler
	.thumb_set 	MemManage_Handler,Default_Exception_Handler

	.weak		BusFault_Handler
	.thumb_set 	BusFault_Handler,Default_Exception_Handler

	.weak		UsageFault_Handler
	.thumb_set 	UsageFault_Handler,Default_Exception_Handler

	.weak		SVC_Handler
	.thumb_set 	SVC_Handler,Default_Exception_Handler

	.weak		DebugMon_Handler
	.thumb_set 	DebugMon_Handler,Default_Exception_Handler

	.weak		PendSV_Handler
	.thumb_set 	PendSV_Handler,Default_Exception_Handler

	.weak		SysTick_Handler
	.thumb_set 	SysTick_Handler,Default_Interrupt_Handler

	.weak		WWDG_IRQHandler
	.thumb_set 	WWDG_IRQHandler,Default_Interrupt_Handler

	.weak		PVD_IRQHandler
	.thumb_set 	PVD_IRQHandler,Default_Interrupt_Handler

	.weak		TAMP_STAMP_IRQHandler
	.thumb_set 	TAMP_STAMP_IRQHandler,Default_Interrupt_Handler

	.weak		RTC_WKUP_IRQHandler
	.thumb_set 	RTC_WKUP_IRQHandler,Default_Interrupt_Handler

	.weak		FLASH_IRQHandler
	.thumb_set 	FLASH_IRQHandler,Default_Interrupt_Handler

	.weak		RCC_IRQHandler
	.thumb_set 	RCC_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI0_IRQHandler
	.thumb_set 	EXTI0_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI1_IRQHandler
	.thumb_set 	EXTI1_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI2_IRQHandler
	.thumb_set 	EXTI2_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI3_IRQHandler
	.thumb_set 	EXTI3_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI4_IRQHandler
	.thumb_set 	EXTI4_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream0_IRQHandler
	.thumb_set 	DMA1_Stream0_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream1_IRQHandler
	.thumb_set 	DMA1_Stream1_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream2_IRQHandler
	.thumb_set 	DMA1_Stream2_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream3_IRQHandler
	.thumb_set 	DMA1_Stream3_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream4_IRQHandler
	.thumb_set 	DMA1_Stream4_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream5_IRQHandler
	.thumb_set 	DMA1_Stream5_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream6_IRQHandler
	.thumb_set 	DMA1_Stream6_IRQHandler,Default_Interrupt_Handler

	.weak		ADC_IRQHandler
	.thumb_set 	ADC_IRQHandler,Default_Interrupt_Handler

	.weak		CAN1_TX_IRQHandler
	.thumb_set 	CAN1_TX_IRQHandler,Default_Interrupt_Handler

	.weak		CAN1_RX0_IRQHandler
	.thumb_set 	CAN1_RX0_IRQHandler,Default_Interrupt_Handler

	.weak		CAN1_RX1_IRQHandler
	.thumb_set 	CAN1_RX1_IRQHandler,Default_Interrupt_Handler

	.weak		CAN1_SCE_IRQHandler
	.thumb_set 	CAN1_SCE_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI9_5_IRQHandler
	.thumb_set 	EXTI9_5_IRQHandler,Default_Interrupt_Handler

	.weak		TIM1_BRK_TIM9_IRQHandler
	.thumb_set 	TIM1_BRK_TIM9_IRQHandler,Default_Interrupt_Handler

	.weak		TIM1_UP_TIM10_IRQHandler
	.thumb_set 	TIM1_UP_TIM10_IRQHandler,Default_Interrupt_Handler

	.weak		TIM1_TRG_COM_TIM11_IRQHandler
	.thumb_set 	TIM1_TRG_COM_TIM11_IRQHandler,Default_Interrupt_Handler

	.weak		TIM1_CC_IRQHandler
	.thumb_set 	TIM1_CC_IRQHandler,Default_Interrupt_Handler

	.weak		TIM2_IRQHandler
	.thumb_set 	TIM2_IRQHandler,Default_Interrupt_Handler

	.weak		TIM3_IRQHandler
	.thumb_set 	TIM3_IRQHandler,Default_Interrupt_Handler

	.weak		TIM4_IRQHandler
	.thumb_set 	TIM4_IRQHandler,Default_Interrupt_Handler

	.weak		I2C1_EV_IRQHandler
	.thumb_set 	I2C1_EV_IRQHandler,Default_Interrupt_Handler

	.weak		I2C1_ER_IRQHandler
	.thumb_set 	I2C1_ER_IRQHandler,Default_Interrupt_Handler

	.weak		I2C2_EV_IRQHandler
	.thumb_set 	I2C2_EV_IRQHandler,Default_Interrupt_Handler

	.weak		I2C2_ER_IRQHandler
	.thumb_set 	I2C2_ER_IRQHandler,Default_Interrupt_Handler

	.weak		SPI1_IRQHandler
	.thumb_set 	SPI1_IRQHandler,Default_Interrupt_Handler

	.weak		SPI2_IRQHandler
	.thumb_set 	SPI2_IRQHandler,Default_Interrupt_Handler

	.weak		USART1_IRQHandler
	.thumb_set 	USART1_IRQHandler,Default_Interrupt_Handler

	.weak		USART2_IRQHandler
	.thumb_set 	USART2_IRQHandler,Default_Interrupt_Handler

	.weak		USART3_IRQHandler
	.thumb_set 	USART3_IRQHandler,Default_Interrupt_Handler

	.weak		EXTI15_10_IRQHandler
	.thumb_set 	EXTI15_10_IRQHandler,Default_Interrupt_Handler

	.weak		RTC_Alarm_IRQHandler
	.thumb_set 	RTC_Alarm_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_FS_WKUP_IRQHandler
	.thumb_set 	OTG_FS_WKUP_IRQHandler,Default_Interrupt_Handler

	.weak		TIM8_BRK_TIM12_IRQHandler
	.thumb_set 	TIM8_BRK_TIM12_IRQHandler,Default_Interrupt_Handler

	.weak		TIM8_UP_TIM13_IRQHandler
	.thumb_set 	TIM8_UP_TIM13_IRQHandler,Default_Interrupt_Handler

	.weak		TIM8_TRG_COM_TIM14_IRQHandler
	.thumb_set 	TIM8_TRG_COM_TIM14_IRQHandler,Default_Interrupt_Handler

	.weak		TIM8_CC_IRQHandler
	.thumb_set 	TIM8_CC_IRQHandler,Default_Interrupt_Handler

	.weak		DMA1_Stream7_IRQHandler
	.thumb_set 	DMA1_Stream7_IRQHandler,Default_Interrupt_Handler

	.weak		FSMC_IRQHandler
	.thumb_set 	FSMC_IRQHandler,Default_Interrupt_Handler

	.weak		SDIO_IRQHandler
	.thumb_set 	SDIO_IRQHandler,Default_Interrupt_Handler

	.weak		TIM5_IRQHandler
	.thumb_set 	TIM5_IRQHandler,Default_Interrupt_Handler

	.weak		SPI3_IRQHandler
	.thumb_set 	SPI3_IRQHandler,Default_Interrupt_Handler

	.weak		UART4_IRQHandler
	.thumb_set 	UART4_IRQHandler,Default_Interrupt_Handler

	.weak		UART5_IRQHandler
	.thumb_set 	UART5_IRQHandler,Default_Interrupt_Handler

	.weak		TIM6_DAC_IRQHandler
	.thumb_set 	TIM6_DAC_IRQHandler,Default_Interrupt_Handler

	.weak		TIM7_IRQHandler
	.thumb_set 	TIM7_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream0_IRQHandler
	.thumb_set 	DMA2_Stream0_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream1_IRQHandler
	.thumb_set 	DMA2_Stream1_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream2_IRQHandler
	.thumb_set 	DMA2_Stream2_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream3_IRQHandler
	.thumb_set 	DMA2_Stream3_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream4_IRQHandler
	.thumb_set 	DMA2_Stream4_IRQHandler,Default_Interrupt_Handler

	.weak		ETH_IRQHandler
	.thumb_set 	ETH_IRQHandler,Default_Interrupt_Handler

	.weak		ETH_WKUP_IRQHandler
	.thumb_set 	ETH_WKUP_IRQHandler,Default_Interrupt_Handler

	.weak		CAN2_TX_IRQHandler
	.thumb_set 	CAN2_TX_IRQHandler,Default_Interrupt_Handler

	.weak		CAN2_RX0_IRQHandler
	.thumb_set 	CAN2_RX0_IRQHandler,Default_Interrupt_Handler

	.weak		CAN2_RX1_IRQHandler
	.thumb_set 	CAN2_RX1_IRQHandler,Default_Interrupt_Handler

	.weak		CAN2_SCE_IRQHandler
	.thumb_set 	CAN2_SCE_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_FS_IRQHandler
	.thumb_set 	OTG_FS_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream5_IRQHandler
	.thumb_set 	DMA2_Stream5_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream6_IRQHandler
	.thumb_set 	DMA2_Stream6_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2_Stream7_IRQHandler
	.thumb_set 	DMA2_Stream7_IRQHandler,Default_Interrupt_Handler

	.weak		USART6_IRQHandler
	.thumb_set 	USART6_IRQHandler,Default_Interrupt_Handler

	.weak		I2C3_EV_IRQHandler
	.thumb_set 	I2C3_EV_IRQHandler,Default_Interrupt_Handler

	.weak		I2C3_ER_IRQHandler
	.thumb_set 	I2C3_ER_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_HS_EP1_OUT_IRQHandler
	.thumb_set 	OTG_HS_EP1_OUT_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_HS_EP1_IN_IRQHandler
	.thumb_set 	OTG_HS_EP1_IN_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_HS_WKUP_IRQHandler
	.thumb_set 	OTG_HS_WKUP_IRQHandler,Default_Interrupt_Handler

	.weak		OTG_HS_IRQHandler
	.thumb_set 	OTG_HS_IRQHandler,Default_Interrupt_Handler

	.weak		DCMI_IRQHandler
	.thumb_set 	DCMI_IRQHandler,Default_Interrupt_Handler

	.weak		CRYP_IRQHandler
	.thumb_set 	CRYP_IRQHandler,Default_Interrupt_Handler

	.weak		HASH_RNG_IRQHandler
	.thumb_set 	HASH_RNG_IRQHandler,Default_Interrupt_Handler

	.weak		FPU_IRQHandler
	.thumb_set 	FPU_IRQHandler,Default_Interrupt_Handler

	.weak		UART7_IRQHandler
	.thumb_set 	UART7_IRQHandler,Default_Interrupt_Handler

	.weak		UART8_IRQHandler
	.thumb_set 	UART8_IRQHandler,Default_Interrupt_Handler

	.weak		SPI4_IRQHandler
	.thumb_set 	SPI4_IRQHandler,Default_Interrupt_Handler

	.weak		SPI5_IRQHandler
	.thumb_set 	SPI5_IRQHandler,Default_Interrupt_Handler

	.weak		SPI6_IRQHandler
	.thumb_set 	SPI6_IRQHandler,Default_Interrupt_Handler

	.weak		SAI1_IRQHandler
	.thumb_set 	SAI1_IRQHandler,Default_Interrupt_Handler

	.weak		LTDC_IRQHandler
	.thumb_set 	LTDC_IRQHandler,Default_Interrupt_Handler

	.weak		LTDC_ER_IRQHandler
	.thumb_set 	LTDC_ER_IRQHandler,Default_Interrupt_Handler

	.weak		DMA2D_IRQHandler
	.thumb_set 	DMA2D_IRQHandler,Default_Interrupt_Handler

