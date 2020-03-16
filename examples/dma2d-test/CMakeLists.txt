cmake_minimum_required(VERSION 3.1)
project (dma2d-test)
include (../../32blit.cmake)
blit_executable (dma2d-test dma2d-test.cpp)
blit_assets_yaml (dma2d-test assets.yml)

target_compile_definitions(dma2d-test
  PRIVATE
 	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)
