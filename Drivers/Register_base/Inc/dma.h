// filepath: /Users/haoyi/Documents/Code/Electronics_for_embedded_systems/Motor_monitor/Software/Inc/dma.h
/**
 * @file dma.h
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 DMA register-level driver header
 *
 * @details This file contains DMA function declarations, structures, and macro definitions
 * for STM32F407 series microcontrollers.
 */

#ifndef DMA_H
#define DMA_H

#include "stm32f407xx.h"

/**
 * @brief DMA initialization configuration structure
 */
typedef struct {
    uint32_t Channel;            /**< DMA channel selection */
    uint32_t Direction;          /**< Data transfer direction */
    uint32_t PeriphInc;          /**< Peripheral increment mode */
    uint32_t MemInc;             /**< Memory increment mode */
    uint32_t PeriphDataAlign;    /**< Peripheral data width */
    uint32_t MemDataAlign;       /**< Memory data width */
    uint32_t Mode;               /**< DMA mode normal/circular */
    uint32_t Priority;           /**< DMA priority level */
    uint32_t FIFOMode;           /**< FIFO mode enable/disable */
    uint32_t FIFOThreshold;      /**< FIFO threshold level */
    uint32_t MemBurst;           /**< Memory burst transfer configuration */
    uint32_t PeriphBurst;        /**< Peripheral burst transfer configuration */
} DMA_InitTypeDef;

/**
 * @name DMA Channels
 * @{
 */
#define DMA_CHANNEL_0        0x00000000U /**< DMA Channel 0 (CHSEL=000) */
#define DMA_CHANNEL_1        0x02000000U /**< DMA Channel 1 (CHSEL=001) */
#define DMA_CHANNEL_2        0x04000000U /**< DMA Channel 2 (CHSEL=010) */
#define DMA_CHANNEL_3        0x06000000U /**< DMA Channel 3 (CHSEL=011) */
#define DMA_CHANNEL_4        0x08000000U /**< DMA Channel 4 (CHSEL=100) */
#define DMA_CHANNEL_5        0x0A000000U /**< DMA Channel 5 (CHSEL=101) */
#define DMA_CHANNEL_6        0x0C000000U /**< DMA Channel 6 (CHSEL=110) */
#define DMA_CHANNEL_7        0x0E000000U /**< DMA Channel 7 (CHSEL=111) */
/** @} */

/**
 * @name DMA Data Transfer Direction
 * @{
 */
#define DMA_PERIPH_TO_MEMORY     0x00000000U      /**< Peripheral to memory direction */
#define DMA_MEMORY_TO_PERIPH     0x00000040U      /**< Memory to peripheral direction */
#define DMA_MEMORY_TO_MEMORY     0x00000080U      /**< Memory to memory direction */
/** @} */

/**
 * @name DMA Peripheral Increment Mode
 * @{
 */
#define DMA_PINC_ENABLE          0x00000200U     /**< Peripheral increment mode enable */
#define DMA_PINC_DISABLE         0x00000000U     /**< Peripheral increment mode disable */
/** @} */

/**
 * @name DMA Memory Increment Mode
 * @{
 */
#define DMA_MINC_ENABLE          0x00000400U     /**< Memory increment mode enable */
#define DMA_MINC_DISABLE         0x00000000U     /**< Memory increment mode disable */
/** @} */

/**
 * @name DMA Peripheral Data Size
 * @{
 */
#define DMA_PDATAALIGN_BYTE      0x00000000U     /**< Peripheral data alignment: Byte */
#define DMA_PDATAALIGN_HALFWORD  0x00000800U     /**< Peripheral data alignment: Half Word */
#define DMA_PDATAALIGN_WORD      0x00001000U     /**< Peripheral data alignment: Word */
/** @} */

/**
 * @name DMA Memory Data Size
 * @{
 */
#define DMA_MDATAALIGN_BYTE      0x00000000U     /**< Memory data alignment: Byte */
#define DMA_MDATAALIGN_HALFWORD  0x00002000U     /**< Memory data alignment: Half Word */
#define DMA_MDATAALIGN_WORD      0x00004000U     /**< Memory data alignment: Word */
/** @} */

/**
 * @name DMA Operation Mode
 * @{
 */
#define DMA_NORMAL               0x00000000U     /**< Normal mode */
#define DMA_CIRCULAR             0x00000100U     /**< Circular mode */
/** @} */

/**
 * @name DMA Priority Level
 * @{
 */
#define DMA_PRIORITY_LOW         0x00000000U     /**< Priority level: Low */
#define DMA_PRIORITY_MEDIUM      0x00010000U     /**< Priority level: Medium */
#define DMA_PRIORITY_HIGH        0x00020000U     /**< Priority level: High */
#define DMA_PRIORITY_VERY_HIGH   0x00030000U     /**< Priority level: Very High */
/** @} */

/**
 * @name DMA FIFO Mode
 * @{
 */
#define DMA_FIFOMODE_DISABLE     0x00000000U     /**< FIFO mode disable */
#define DMA_FIFOMODE_ENABLE      0x00000004U     /**< FIFO mode enable */
/** @} */

/**
 * @name DMA FIFO Threshold Level
 * @{
 */
#define DMA_FIFO_THRESHOLD_1QUARTERFULL    0x00000000U     /**< FIFO threshold 1 quart full configuration */
#define DMA_FIFO_THRESHOLD_HALFFULL        0x00000001U     /**< FIFO threshold half full configuration */
#define DMA_FIFO_THRESHOLD_3QUARTERSFULL   0x00000002U     /**< FIFO threshold 3 quarts full configuration */
#define DMA_FIFO_THRESHOLD_FULL            0x00000003U     /**< FIFO threshold full configuration */
/** @} */

/**
 * @name DMA Memory Burst
 * @{
 */
#define DMA_MBURST_SINGLE        0x00000000U     /**< Single transfer configuration */
#define DMA_MBURST_INC4          0x00800000U     /**< Incremental burst of 4 beats */
#define DMA_MBURST_INC8          0x01000000U     /**< Incremental burst of 8 beats */
#define DMA_MBURST_INC16         0x01800000U     /**< Incremental burst of 16 beats */
/** @} */

/**
 * @name DMA Peripheral Burst
 * @{
 */
#define DMA_PBURST_SINGLE        0x00000000U     /**< Single transfer configuration */
#define DMA_PBURST_INC4          0x00200000U     /**< Incremental burst of 4 beats */
#define DMA_PBURST_INC8          0x00400000U     /**< Incremental burst of 8 beats */
#define DMA_PBURST_INC16         0x00600000U     /**< Incremental burst of 16 beats */
/** @} */

/**
 * @name DMA Stream Selection
 * @{
 */
#define DMA_STREAM0              0      /**< DMA Stream 0 */
#define DMA_STREAM1              1      /**< DMA Stream 1 */
#define DMA_STREAM2              2      /**< DMA Stream 2 */
#define DMA_STREAM3              3      /**< DMA Stream 3 */
#define DMA_STREAM4              4      /**< DMA Stream 4 */
#define DMA_STREAM5              5      /**< DMA Stream 5 */
#define DMA_STREAM6              6      /**< DMA Stream 6 */
#define DMA_STREAM7              7      /**< DMA Stream 7 */
/** @} */


/**
 * @brief Initialize DMA stream with the specified parameters
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param init Pointer to DMA initialization structure
 * 
 * @note DMA clock must be enabled separately via RCC registers before calling this function
 */
void dma_init(DMA_TypeDef *DMAx, uint32_t stream, DMA_InitTypeDef *init);

/**
 * @brief Configure DMA transfer parameters
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param SrcAddress Source address
 * @param DstAddress Destination address
 * @param DataLength Number of data items to transfer
 * 
 * @note This function should be called after dma_init
 */
void dma_config_transfer(DMA_TypeDef *DMAx, uint32_t stream, uint32_t SrcAddress, uint32_t DstAddress, uint16_t DataLength);

/**
 * @brief Enable DMA stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_enable(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Disable DMA stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_disable(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Enable DMA interrupt
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param interrupt Interrupts to enable (combination of DMA_IT_xx flags)
 */
void dma_enable_interrupt(DMA_TypeDef *DMAx, uint32_t stream, uint32_t interrupt);

/**
 * @brief Disable DMA interrupt
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param interrupt Interrupts to disable (combination of DMA_IT_xx flags)
 */
void dma_disable_interrupt(DMA_TypeDef *DMAx, uint32_t stream, uint32_t interrupt);

/**
 * @brief Get DMA transfer complete flag status
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_tc_flag_status(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Get DMA half transfer flag status
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_ht_flag_status(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Get DMA transfer error flag status
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_te_flag_status(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Clear DMA transfer complete flag
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_tc_flag(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Clear DMA half transfer flag
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_ht_flag(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Clear DMA transfer error flag
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_te_flag(DMA_TypeDef *DMAx, uint32_t stream);

/**
 * @brief Enable DMA clock
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 */
void dma_enable_clock(DMA_TypeDef *DMAx);

/**
 * @brief Get number of data items left to transfer
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint16_t Number of data items left to transfer
 */
uint16_t dma_get_counter(DMA_TypeDef *DMAx, uint32_t stream);

#endif /* DMA_H */
