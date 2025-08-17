/**
 * @file dma.c
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 DMA register-level driver
 *
 * @details This file provides basic DMA initialization, configuration and operation
 * functions for STM32F407 series microcontrollers. All operations are performed at the 
 * register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#include "../Inc/dma.h"

/**
 * @brief Get DMA stream register pointer
 * 
 * @details Helper function to get pointer to the right stream register set
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return DMA_Stream_TypeDef* Pointer to the DMA stream registers
 */
static DMA_Stream_TypeDef* dma_get_stream(DMA_TypeDef *DMAx, uint32_t stream) {
    return (DMA_Stream_TypeDef *)((uint32_t)((uint32_t)DMAx + 0x10 + (0x18 * stream)));
}

/**
 * @brief Initialize DMA stream with the specified parameters
 * 
 * @details Configures DMA stream by setting channel, direction, increment modes,
 *          data alignment, mode, priority, and FIFO settings.
 *
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param init Pointer to DMA initialization structure
 * 
 * @note DMA clock must be enabled separately via RCC registers before calling this function
 */
void dma_init(DMA_TypeDef *DMAx, uint32_t stream, DMA_InitTypeDef *init) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    
    /* Disable the DMA Stream first */
    DMAStream->CR &= ~DMA_SxCR_EN;
    
    /* Wait until the stream is effectively disabled */
    while(DMAStream->CR & DMA_SxCR_EN);
    
    /* Clear all interrupt flags for the selected stream */
    /* This is critical for proper DMA operation */
    if (DMAx == DMA1) {
        if (stream < 4) {
            /* Streams 0-3 use LIFCR register */
            DMA1->LIFCR = (0x3F << (stream * 6)); /* Clear all flags for this stream */
        } else {
            /* Streams 4-7 use HIFCR register */
            DMA1->HIFCR = (0x3F << ((stream - 4) * 6)); /* Clear all flags for this stream */
        }
    } else { /* DMA2 */
        if (stream < 4) {
            /* Streams 0-3 use LIFCR register */
            DMA2->LIFCR = (0x3F << (stream * 6)); /* Clear all flags for this stream */
        } else {
            /* Streams 4-7 use HIFCR register */
            DMA2->HIFCR = (0x3F << ((stream - 4) * 6)); /* Clear all flags for this stream */
        }
    }
    
    /* Configure the source, destination and buffer size */
    DMAStream->PAR = 0;
    DMAStream->M0AR = 0;
    DMAStream->NDTR = 0;
    
    /* Configure CR register - do this in one write operation */
    DMAStream->CR = init->Channel | init->Direction | init->PeriphInc | init->MemInc |
                    init->PeriphDataAlign | init->MemDataAlign | init->Mode | init->Priority;
    
    /* Configure FIFO */
    DMAStream->FCR = init->FIFOMode | init->FIFOThreshold | init->MemBurst | init->PeriphBurst;
}

/**
 * @brief Configure DMA transfer parameters
 * 
 * @details Sets up source and destination addresses and transfer count for DMA operation
 *
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param SrcAddress Source address
 * @param DstAddress Destination address
 * @param DataLength Number of data items to transfer
 * 
 * @note This function should be called after dma_init
 */
void dma_config_transfer(DMA_TypeDef *DMAx, uint32_t stream, uint32_t SrcAddress, uint32_t DstAddress, uint16_t DataLength) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    
    /* Make sure the stream is disabled */
    if (DMAStream->CR & DMA_SxCR_EN) {
        DMAStream->CR &= ~DMA_SxCR_EN;
        while(DMAStream->CR & DMA_SxCR_EN);
    }
    
    /* Clear any pending flags for this stream before configuration */
    if (DMAx == DMA1) {
        if (stream < 4) {
            DMA1->LIFCR = (0x3F << (stream * 6)); /* Clear all flags for this stream */
        } else {
            DMA1->HIFCR = (0x3F << ((stream - 4) * 6)); /* Clear all flags for this stream */
        }
    } else { /* DMA2 */
        if (stream < 4) {
            DMA2->LIFCR = (0x3F << (stream * 6)); /* Clear all flags for this stream */
        } else {
            DMA2->HIFCR = (0x3F << ((stream - 4) * 6)); /* Clear all flags for this stream */
        }
    }
    
    /* Configure source, destination and data length according to direction */
    uint32_t direction = DMAStream->CR & DMA_SxCR_DIR;
    
    if (direction == DMA_PERIPH_TO_MEMORY) {
        /* Peripheral to memory (e.g., ADC to RAM) */
        DMAStream->PAR = SrcAddress;    /* Peripheral is source (e.g., ADC data register) */
        DMAStream->M0AR = DstAddress;   /* Memory is destination (e.g., buffer array) */
    } else if (direction == DMA_MEMORY_TO_PERIPH) {
        /* Memory to peripheral (e.g., RAM to DAC) */
        DMAStream->PAR = DstAddress;    /* Peripheral is destination (e.g., DAC data register) */
        DMAStream->M0AR = SrcAddress;   /* Memory is source (e.g., buffer array) */
    } else if (direction == DMA_MEMORY_TO_MEMORY) {
        /* Memory to memory */
        DMAStream->PAR = SrcAddress;    /* Source memory address */
        DMAStream->M0AR = DstAddress;   /* Destination memory address */
    }
    
    /* Set data length - verify it's within limits first */
    if (DataLength > 0 && DataLength <= 65535) {
        DMAStream->NDTR = DataLength;   /* Number of data items to transfer */
    } else {
        /* Handle invalid length - default to a safe value */
        DMAStream->NDTR = 1;            /* Set to minimum safe value */
    }
}

/**
 * @brief Enable DMA stream
 * 
 * @details Sets the EN bit in the DMA_SxCR register to start DMA transfer
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_enable(DMA_TypeDef *DMAx, uint32_t stream) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    DMAStream->CR |= DMA_SxCR_EN;
}

/**
 * @brief Disable DMA stream
 * 
 * @details Clears the EN bit in the DMA_SxCR register to stop DMA transfer
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_disable(DMA_TypeDef *DMAx, uint32_t stream) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    DMAStream->CR &= ~DMA_SxCR_EN;
}

/**
 * @brief Enable DMA interrupt
 * 
 * @details Enables specified DMA interrupts by setting corresponding bits in the CR register
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param interrupt Combination of interrupt flags to enable, use system macros directly:
 *                  DMA_SxCR_TCIE - Transfer Complete Interrupt
 *                  DMA_SxCR_HTIE - Half Transfer Interrupt
 *                  DMA_SxCR_TEIE - Transfer Error Interrupt
 *                  DMA_SxCR_DMEIE - Direct Mode Error Interrupt
 *                  DMA_SxFCR_FEIE - FIFO Error Interrupt
 *                  Multiple interrupts can be combined with bitwise OR (|)
 */
void dma_enable_interrupt(DMA_TypeDef *DMAx, uint32_t stream, uint32_t interrupt) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    
    /* Process interrupt flags in CR register */
    uint32_t cr_interrupts = interrupt & (DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
    if (cr_interrupts) {
        DMAStream->CR |= cr_interrupts;
    }
    
    /* Process interrupt flags in FCR register */
    if (interrupt & DMA_SxFCR_FEIE) {
        DMAStream->FCR |= DMA_SxFCR_FEIE;
    }
}

/**
 * @brief Disable DMA interrupt
 * 
 * @details Disables specified DMA interrupts by clearing corresponding bits in the CR register
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @param interrupt Combination of interrupt flags to disable, use system macros directly:
 *                  DMA_SxCR_TCIE - Transfer Complete Interrupt
 *                  DMA_SxCR_HTIE - Half Transfer Interrupt
 *                  DMA_SxCR_TEIE - Transfer Error Interrupt
 *                  DMA_SxCR_DMEIE - Direct Mode Error Interrupt
 *                  DMA_SxFCR_FEIE - FIFO Error Interrupt
 *                  Multiple interrupts can be combined with bitwise OR (|)
 */
void dma_disable_interrupt(DMA_TypeDef *DMAx, uint32_t stream, uint32_t interrupt) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    
    /* Process interrupt flags in CR register */
    uint32_t cr_interrupts = interrupt & (DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
    if (cr_interrupts) {
        DMAStream->CR &= ~cr_interrupts;
    }
    
    /* Process interrupt flags in FCR register */
    if (interrupt & DMA_SxFCR_FEIE) {
        DMAStream->FCR &= ~DMA_SxFCR_FEIE;
    }
}

/**
 * @brief Get DMA transfer complete flag status
 * 
 * @details Checks if the transfer complete flag (TCIF) is set for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_tc_flag_status(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (6 + stream*6) : (6 + (stream-4)*6 + 16);
    
    if (DMAx == DMA1) {
        return (DMA1->LISR & (1 << flag_pos)) ? 1 : 0;
    } else {
        return (DMA2->LISR & (1 << flag_pos)) ? 1 : 0;
    }
}

/**
 * @brief Get DMA half transfer flag status
 * 
 * @details Checks if the half transfer flag (HTIF) is set for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_ht_flag_status(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (4 + stream*6) : (4 + (stream-4)*6 + 16);
    
    if (DMAx == DMA1) {
        return (DMA1->LISR & (1 << flag_pos)) ? 1 : 0;
    } else {
        return (DMA2->LISR & (1 << flag_pos)) ? 1 : 0;
    }
}

/**
 * @brief Get DMA transfer error flag status
 * 
 * @details Checks if the transfer error flag (TEIF) is set for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint8_t Flag status: 1 if flag is set, 0 otherwise
 */
uint8_t dma_get_te_flag_status(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (3 + stream*6) : (3 + (stream-4)*6 + 16);
    
    if (DMAx == DMA1) {
        return (DMA1->LISR & (1 << flag_pos)) ? 1 : 0;
    } else {
        return (DMA2->LISR & (1 << flag_pos)) ? 1 : 0;
    }
}

/**
 * @brief Clear DMA transfer complete flag
 * 
 * @details Clears the transfer complete flag (TCIF) for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_tc_flag(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (6 + stream*6) : (6 + (stream-4)*6);
    
    if (DMAx == DMA1) {
        if (stream < 4) {
            DMA1->LIFCR = (1 << flag_pos);
        } else {
            DMA1->HIFCR = (1 << flag_pos);
        }
    } else {
        if (stream < 4) {
            DMA2->LIFCR = (1 << flag_pos);
        } else {
            DMA2->HIFCR = (1 << flag_pos);
        }
    }
}

/**
 * @brief Clear DMA half transfer flag
 * 
 * @details Clears the half transfer flag (HTIF) for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_ht_flag(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (4 + stream*6) : (4 + (stream-4)*6);
    
    if (DMAx == DMA1) {
        if (stream < 4) {
            DMA1->LIFCR = (1 << flag_pos);
        } else {
            DMA1->HIFCR = (1 << flag_pos);
        }
    } else {
        if (stream < 4) {
            DMA2->LIFCR = (1 << flag_pos);
        } else {
            DMA2->HIFCR = (1 << flag_pos);
        }
    }
}

/**
 * @brief Clear DMA transfer error flag
 * 
 * @details Clears the transfer error flag (TEIF) for the specified stream
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 */
void dma_clear_te_flag(DMA_TypeDef *DMAx, uint32_t stream) {
    uint32_t flag_pos = (stream < 4) ? (3 + stream*6) : (3 + (stream-4)*6);
    
    if (DMAx == DMA1) {
        if (stream < 4) {
            DMA1->LIFCR = (1 << flag_pos);
        } else {
            DMA1->HIFCR = (1 << flag_pos);
        }
    } else {
        if (stream < 4) {
            DMA2->LIFCR = (1 << flag_pos);
        } else {
            DMA2->HIFCR = (1 << flag_pos);
        }
    }
}

/**
 * @brief Enable DMA clock
 * 
 * @details Sets the appropriate bit in RCC_AHB1ENR register to enable DMA clock
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 */
void dma_enable_clock(DMA_TypeDef *DMAx) {
    if (DMAx == DMA1) {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    } else {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    }
}

/**
 * @brief Get number of data items left to transfer
 * 
 * @details Reads the current NDTR register value to get number of remaining transfers
 * 
 * @param DMAx DMA instance (DMA1 or DMA2)
 * @param stream Stream number (0-7)
 * @return uint16_t Number of data items left to transfer
 */
uint16_t dma_get_counter(DMA_TypeDef *DMAx, uint32_t stream) {
    DMA_Stream_TypeDef *DMAStream = dma_get_stream(DMAx, stream);
    return (uint16_t)DMAStream->NDTR;
}
