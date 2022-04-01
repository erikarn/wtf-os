#ifndef	__STM32F429_HW_MEM_MAP_H__
#define	__STM32F429_HW_MEM_MAP_H__

#define	FLASH_BASE		0x08000000UL /*!< FLASH(up to 2 MB) base address in the alias region                         */
#define	CCMDATARAM_BASE		0x10000000UL /*!< CCM(core coupled memory) data RAM(64 KB) base address in the alias region  */
#define	SRAM1_BASE		0x20000000UL /*!< SRAM1(112 KB) base address in the alias region                              */
#define	SRAM2_BASE		0x2001C000UL /*!< SRAM2(16 KB) base address in the alias region                              */
#define	SRAM3_BASE		0x20020000UL /*!< SRAM3(64 KB) base address in the alias region                              */
#define	PERIPH_BASE		0x40000000UL /*!< Peripheral base address in the alias region                                */
#define	BKPSRAM_BASE		0x40024000UL /*!< Backup SRAM(4 KB) base address in the alias region                         */
#define	FMC_R_BASE		0xA0000000UL /*!< FMC registers base address                                                 */
#define	SRAM1_BB_BASE		0x22000000UL /*!< SRAM1(112 KB) base address in the bit-band region                          */
#define	SRAM2_BB_BASE		0x22380000UL /*!< SRAM2(16 KB) base address in the bit-band region                           */
#define	SRAM3_BB_BASE		0x22400000UL /*!< SRAM3(64 KB) base address in the bit-band region                           */
#define	PERIPH_BB_BASE		0x42000000UL /*!< Peripheral base address in the bit-band region                             */
#define	BKPSRAM_BB_BASE		0x42480000UL /*!< Backup SRAM(4 KB) base address in the bit-band region                      */
#define	FLASH_END		0x081FFFFFUL /*!< FLASH end address                                                          */
#define	FLASH_OTP_BASE		0x1FFF7800UL /*!< Base address of : (up to 528 Bytes) embedded FLASH OTP Area                */
#define	FLASH_OTP_END		0x1FFF7A0FUL /*!< End address of : (up to 528 Bytes) embedded FLASH OTP Area                 */
#define	CCMDATARAM_END		0x1000FFFFUL /*!< CCM data RAM end address                                                   */

/*!< Peripheral memory map */
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000UL)

/*!< APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800UL)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00UL)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000UL)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400UL)
#define TIM12_BASE            (APB1PERIPH_BASE + 0x1800UL)
#define TIM13_BASE            (APB1PERIPH_BASE + 0x1C00UL)
#define TIM14_BASE            (APB1PERIPH_BASE + 0x2000UL)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800UL)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00UL)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000UL)
#define I2S2ext_BASE          (APB1PERIPH_BASE + 0x3400UL)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800UL)
#define SPI3_BASE             (APB1PERIPH_BASE + 0x3C00UL)
#define I2S3ext_BASE          (APB1PERIPH_BASE + 0x4000UL)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800UL)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00UL)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000UL)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800UL)
#define I2C3_BASE             (APB1PERIPH_BASE + 0x5C00UL)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400UL)
#define CAN2_BASE             (APB1PERIPH_BASE + 0x6800UL)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000UL)
#define DAC_BASE              (APB1PERIPH_BASE + 0x7400UL)
#define UART7_BASE            (APB1PERIPH_BASE + 0x7800UL)
#define UART8_BASE            (APB1PERIPH_BASE + 0x7C00UL)

/* APB2 peripherals */
#define USART1_BASE           (APB2PERIPH_BASE + 0x1000UL)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800UL)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00UL)

/*!< AHB1 peripherals */
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE            (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASE            (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASE            (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASE            (AHB1PERIPH_BASE + 0x1C00UL)
#define GPIOI_BASE            (AHB1PERIPH_BASE + 0x2000UL)
#define GPIOJ_BASE            (AHB1PERIPH_BASE + 0x2400UL)
#define GPIOK_BASE            (AHB1PERIPH_BASE + 0x2800UL)
#define CRC_BASE              (AHB1PERIPH_BASE + 0x3000UL)
#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800UL)
#define FLASH_R_BASE          (AHB1PERIPH_BASE + 0x3C00UL)
#define DMA1_BASE             (AHB1PERIPH_BASE + 0x6000UL)
#define DMA1_Stream0_BASE     (DMA1_BASE + 0x010UL)
#define DMA1_Stream1_BASE     (DMA1_BASE + 0x028UL)
#define DMA1_Stream2_BASE     (DMA1_BASE + 0x040UL)
#define DMA1_Stream3_BASE     (DMA1_BASE + 0x058UL)
#define DMA1_Stream4_BASE     (DMA1_BASE + 0x070UL)
#define DMA1_Stream5_BASE     (DMA1_BASE + 0x088UL)
#define DMA1_Stream6_BASE     (DMA1_BASE + 0x0A0UL)
#define DMA1_Stream7_BASE     (DMA1_BASE + 0x0B8UL)
#define DMA2_BASE             (AHB1PERIPH_BASE + 0x6400UL)
#define DMA2_Stream0_BASE     (DMA2_BASE + 0x010UL)
#define DMA2_Stream1_BASE     (DMA2_BASE + 0x028UL)
#define DMA2_Stream2_BASE     (DMA2_BASE + 0x040UL)
#define DMA2_Stream3_BASE     (DMA2_BASE + 0x058UL)
#define DMA2_Stream4_BASE     (DMA2_BASE + 0x070UL)
#define DMA2_Stream5_BASE     (DMA2_BASE + 0x088UL)
#define DMA2_Stream6_BASE     (DMA2_BASE + 0x0A0UL)
#define DMA2_Stream7_BASE     (DMA2_BASE + 0x0B8UL)
#define ETH_BASE              (AHB1PERIPH_BASE + 0x8000UL)
#define ETH_MAC_BASE          (ETH_BASE)
#define ETH_MMC_BASE          (ETH_BASE + 0x0100UL)
#define ETH_PTP_BASE          (ETH_BASE + 0x0700UL)
#define ETH_DMA_BASE          (ETH_BASE + 0x1000UL)
#define DMA2D_BASE            (AHB1PERIPH_BASE + 0xB000UL)

#endif
