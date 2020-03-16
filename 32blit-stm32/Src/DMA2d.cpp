#include "stm32h7xx_hal.h"
#include "DMA2D.hpp"

extern char __ltdc_start;

enum dma2d_state {d2d_idle, d2d_flip, d2d_color, d2d_color_alpha};

dma2d_state current_state = d2d_idle;


void dma2d_hires_flip(const blit::Surface &source) 
{
  current_state = d2d_flip;
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 3); 
  static DMA2D_HandleTypeDef hdma2d; // Do not place on stack

  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
  hdma2d.Init.OutputOffset = 0;

  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].InputOffset = 0;

  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, 1);

  HAL_DMA2D_Start(&hdma2d, (uintptr_t)source.data, (uintptr_t)&__ltdc_start, 320, 240);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);
  current_state = d2d_idle;
}

void dma2d_lores_flip(const blit::Surface &source) 
{
  current_state = d2d_flip;
  static DMA2D_HandleTypeDef hdma2d; // Do not place on stack
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(source.data), 320 * 240 * 3); 

  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.Init.OutputOffset = 1;//160+320;

  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].InputOffset = 0;

  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, 1);


  HAL_DMA2D_Start(&hdma2d, (uintptr_t)source.data, (uintptr_t)&__ltdc_start+(320*120*3), 1, 160*120);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);

  HAL_DMA2D_Start(&hdma2d, (uintptr_t)source.data, (uintptr_t)&__ltdc_start+(320*120*3)+3, 1, 160*120);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);

  // now we transfer screen to screen for every other row
  hdma2d.Init.OutputOffset = 320;
  hdma2d.LayerCfg[1].InputOffset = 0;
  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, 1);

  HAL_DMA2D_Start(&hdma2d, (uintptr_t)&__ltdc_start+(320*120*3), (uintptr_t)&__ltdc_start, 320, 120);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);

  // // now we duplicate each row
  hdma2d.LayerCfg[1].InputOffset = 320;
  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, 1);

  HAL_DMA2D_Start(&hdma2d, (uintptr_t)&__ltdc_start, (uintptr_t)&__ltdc_start+(320*3), 320, 240);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);

  current_state = d2d_idle;
}

void dma2d_RGBA_RGB(const blit::Pen* pen, const blit::Surface* dest, uint32_t off, uint32_t cnt) 
{
  static DMA2D_HandleTypeDef hdma2d;  // Do not place on stack
  static uint32_t dma2d_scanline[320] = {0};

  uint32_t addr = (off * 3) + (uint32_t)dest->data;
  uint32_t byte_count = cnt * 3;

  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, byte_count);


  uint32_t use_alpha   = ((pen->a + 1) * (dest->alpha + 1)) >> 8;
  uint32_t dma2d_color = (use_alpha << 24) | (pen->b << 16) | (pen->g << 8) | (pen->r);
  if(use_alpha < 255)
  {
    if(dma2d_scanline[0] != dma2d_color)
    {
      for(uint16_t u = 0; u < 320; u ++)
        dma2d_scanline[u] = dma2d_color;
      SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dma2d_scanline, 320*4);
    }

    if(current_state != d2d_color_alpha)
    {
      hdma2d.Instance = DMA2D;
      hdma2d.Init.Mode = DMA2D_M2M_BLEND;
      hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
      hdma2d.Init.OutputOffset = 0;

      hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
      hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB888;
      hdma2d.LayerCfg[0].InputOffset = 0;

      hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
      hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
      hdma2d.LayerCfg[1].InputOffset = 0;

      HAL_DMA2D_Init(&hdma2d);
      HAL_DMA2D_ConfigLayer(&hdma2d, 0);
      HAL_DMA2D_ConfigLayer(&hdma2d, 1);

      current_state = d2d_color_alpha;
    }

    HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)dma2d_scanline, addr, addr, cnt, 1);
  }
  else
  {
    if(current_state != d2d_color)
    {
      hdma2d.Instance = DMA2D;
      hdma2d.Init.Mode = DMA2D_R2M;
      hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
      hdma2d.Init.OutputOffset = 0;
      HAL_DMA2D_Init(&hdma2d);

      current_state = d2d_color;
    }
    HAL_DMA2D_Start(&hdma2d, dma2d_color, addr, cnt, 1);
  }

  HAL_DMA2D_PollForTransfer(&hdma2d, 10);

}

bool dma2d_clear(blit::Surface *pSurface)
{
  uint32_t uW = pSurface->bounds.w;
  uint32_t uH = pSurface->bounds.h;
  
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(pSurface->data), uW * uH * 3);
  static DMA2D_HandleTypeDef hdma2d;  // Do not place on stack
  static uint32_t scanline[320];

  uint32_t uColor = (pSurface->pen.b << 16) | (pSurface->pen.g << 8) | (pSurface->pen.r);
  uint32_t uAddr = (uint32_t)pSurface->data;

  if(pSurface->pen.a == 255)
  {
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
    hdma2d.Init.OutputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);

    HAL_DMA2D_Start(&hdma2d, uColor, uAddr, uW, uH);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);
  }
  else
  {
    // in R2M mode we have no alpha so create ARGB8888 scanline
    uint32_t uColor = (pSurface->pen.a << 24) | (pSurface->pen.b << 16) | (pSurface->pen.g << 8) | (pSurface->pen.r);
    for(uint16_t u = 0; u < uW; u ++)
      scanline[u] = uColor;
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)scanline, uW*4);

    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M_BLEND;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
    hdma2d.Init.OutputOffset = 0;

    hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB888;
    hdma2d.LayerCfg[0].InputOffset = 0;

    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].InputOffset = 0;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 0);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    for(uint16_t uR = 0; uR < uH; uR++)
    {
      HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)&scanline[0], uAddr, uAddr, uW, 1);
      HAL_DMA2D_PollForTransfer(&hdma2d, 10);
      uAddr+= uW * 3;
    }
  }
  return false;
}