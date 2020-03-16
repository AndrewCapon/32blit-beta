#pragma once

#include "graphics/surface.hpp"

bool dma2d_clear(blit::Surface *pSurface);
void dma2d_RGBA_RGB(const blit::Pen* pen, const blit::Surface* dest, uint32_t off, uint32_t cnt);
void dma2d_hires_flip(const blit::Surface &source);
void dma2d_lores_flip(const blit::Surface &source); 
