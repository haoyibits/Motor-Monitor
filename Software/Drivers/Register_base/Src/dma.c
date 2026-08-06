/**
 * @file dma.c
 * @brief STM32F4 DMA register-level driver.
 */

#include <stddef.h>
#include <stdint.h>

#include "dma.h"

#define DMA_STREAM_FLAG_MASK 0x3DU

static uint8_t dma_valid(DMA_TypeDef *dma, uint32_t stream)
{
    return (((dma == DMA1) || (dma == DMA2)) && (stream < 8U)) ? 1U : 0U;
}

static DMA_Stream_TypeDef *dma_get_stream(DMA_TypeDef *dma, uint32_t stream)
{
    return (DMA_Stream_TypeDef *)((uintptr_t)dma + 0x10U + (0x18U * stream));
}

static uint32_t dma_flag_shift(uint32_t stream)
{
    static const uint8_t shifts[4] = {0U, 6U, 16U, 22U};
    return shifts[stream & 0x3U];
}

static volatile uint32_t *dma_status_register(DMA_TypeDef *dma,
                                              uint32_t stream)
{
    return (stream < 4U) ? &dma->LISR : &dma->HISR;
}

static volatile uint32_t *dma_clear_register(DMA_TypeDef *dma,
                                             uint32_t stream)
{
    return (stream < 4U) ? &dma->LIFCR : &dma->HIFCR;
}

static void dma_clear_all_flags(DMA_TypeDef *dma, uint32_t stream)
{
    *dma_clear_register(dma, stream) =
        DMA_STREAM_FLAG_MASK << dma_flag_shift(stream);
}

DriverStatus dma_disable(DMA_TypeDef *dma, uint32_t stream,
                         uint32_t timeout_cycles)
{
    if ((!dma_valid(dma, stream)) || (timeout_cycles == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    dma_stream->CR &= ~DMA_SxCR_EN;
    while (timeout_cycles > 0U) {
        if ((dma_stream->CR & DMA_SxCR_EN) == 0U) {
            return DRIVER_STATUS_OK;
        }
        --timeout_cycles;
    }
    return DRIVER_STATUS_TIMEOUT;
}

DriverStatus dma_init(DMA_TypeDef *dma, uint32_t stream,
                      const DMA_InitTypeDef *init, uint32_t timeout_cycles)
{
    if ((!dma_valid(dma, stream)) || (init == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = dma_disable(dma, stream, timeout_cycles);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    dma_clear_all_flags(dma, stream);
    dma_stream->PAR = 0U;
    dma_stream->M0AR = 0U;
    dma_stream->M1AR = 0U;
    dma_stream->NDTR = 0U;
    dma_stream->CR = init->Channel | init->Direction | init->PeriphInc |
                     init->MemInc | init->PeriphDataAlign |
                     init->MemDataAlign | init->Mode | init->Priority |
                     init->MemBurst | init->PeriphBurst;
    dma_stream->FCR = init->FIFOMode | init->FIFOThreshold;
    return DRIVER_STATUS_OK;
}

DriverStatus dma_config_transfer(DMA_TypeDef *dma, uint32_t stream,
                                 uint32_t source_address,
                                 uint32_t destination_address,
                                 uint16_t data_length,
                                 uint32_t timeout_cycles)
{
    if ((!dma_valid(dma, stream)) || (source_address == 0U) ||
        (destination_address == 0U) || (data_length == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = dma_disable(dma, stream, timeout_cycles);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    dma_clear_all_flags(dma, stream);
    uint32_t direction = dma_stream->CR & DMA_SxCR_DIR;
    if (direction == DMA_MEMORY_TO_PERIPH) {
        dma_stream->PAR = destination_address;
        dma_stream->M0AR = source_address;
    } else {
        dma_stream->PAR = source_address;
        dma_stream->M0AR = destination_address;
    }
    dma_stream->NDTR = data_length;
    return DRIVER_STATUS_OK;
}

DriverStatus dma_enable(DMA_TypeDef *dma, uint32_t stream)
{
    if (!dma_valid(dma, stream)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    if (dma_stream->NDTR == 0U) {
        return DRIVER_STATUS_NOT_READY;
    }
    dma_stream->CR |= DMA_SxCR_EN;
    return DRIVER_STATUS_OK;
}

DriverStatus dma_enable_interrupt(DMA_TypeDef *dma, uint32_t stream,
                                  uint32_t interrupt)
{
    if (!dma_valid(dma, stream)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    dma_stream->CR |= interrupt & (DMA_SxCR_TCIE | DMA_SxCR_HTIE |
                                   DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
    dma_stream->FCR |= interrupt & DMA_SxFCR_FEIE;
    return DRIVER_STATUS_OK;
}

DriverStatus dma_disable_interrupt(DMA_TypeDef *dma, uint32_t stream,
                                   uint32_t interrupt)
{
    if (!dma_valid(dma, stream)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    DMA_Stream_TypeDef *dma_stream = dma_get_stream(dma, stream);
    dma_stream->CR &= ~(interrupt & (DMA_SxCR_TCIE | DMA_SxCR_HTIE |
                                     DMA_SxCR_TEIE | DMA_SxCR_DMEIE));
    dma_stream->FCR &= ~(interrupt & DMA_SxFCR_FEIE);
    return DRIVER_STATUS_OK;
}

static uint8_t dma_get_flag(DMA_TypeDef *dma, uint32_t stream,
                            uint32_t offset)
{
    if (!dma_valid(dma, stream)) {
        return 0U;
    }
    uint32_t mask = 1UL << (dma_flag_shift(stream) + offset);
    return ((*dma_status_register(dma, stream) & mask) != 0U) ? 1U : 0U;
}

static void dma_clear_flag(DMA_TypeDef *dma, uint32_t stream,
                           uint32_t offset)
{
    if (dma_valid(dma, stream)) {
        *dma_clear_register(dma, stream) =
            1UL << (dma_flag_shift(stream) + offset);
    }
}

uint8_t dma_get_tc_flag_status(DMA_TypeDef *dma, uint32_t stream)
{
    return dma_get_flag(dma, stream, 5U);
}

uint8_t dma_get_ht_flag_status(DMA_TypeDef *dma, uint32_t stream)
{
    return dma_get_flag(dma, stream, 4U);
}

uint8_t dma_get_te_flag_status(DMA_TypeDef *dma, uint32_t stream)
{
    return dma_get_flag(dma, stream, 3U);
}

void dma_clear_tc_flag(DMA_TypeDef *dma, uint32_t stream)
{
    dma_clear_flag(dma, stream, 5U);
}

void dma_clear_ht_flag(DMA_TypeDef *dma, uint32_t stream)
{
    dma_clear_flag(dma, stream, 4U);
}

void dma_clear_te_flag(DMA_TypeDef *dma, uint32_t stream)
{
    dma_clear_flag(dma, stream, 3U);
}

void dma_enable_clock(DMA_TypeDef *dma)
{
    if (dma == DMA1) RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    else if (dma == DMA2) RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

uint16_t dma_get_counter(DMA_TypeDef *dma, uint32_t stream)
{
    return dma_valid(dma, stream) ?
           (uint16_t)dma_get_stream(dma, stream)->NDTR : 0U;
}
