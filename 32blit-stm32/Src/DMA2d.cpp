#include "stm32h7xx_hal.h"
#include "DMA2D.hpp"

bool blit_dma2d_clear(blit::Surface *pSurface)
{
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(pSurface->data), 320 * 240 * 3);
  static DMA2D_HandleTypeDef hdma2d;  // Do not place on stack

  uint32_t uColor = (pSurface->pen.r << 16) | (pSurface->pen.g << 8) | (pSurface->pen.b);
  uint32_t uAddr = (uint32_t)pSurface->data;

  if(pSurface->pen.a == 255)
  {
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
    hdma2d.Init.OutputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);

    HAL_DMA2D_Start(&hdma2d, uColor, uAddr, pSurface->bounds.w, pSurface->bounds.h);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);
  }
  else
  {
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
    hdma2d.Init.OutputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);

    HAL_DMA2D_Start(&hdma2d, uColor, uAddr, pSurface->bounds.w, pSurface->bounds.h);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);
  }
  return false;
}