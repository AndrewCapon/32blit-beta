cmake_minimum_required(VERSION 3.1)
project (firmware-update)
include (../../32blit.cmake)

blit_executable_int_flash(firmware-update firmware-update.cpp)

target_compile_definitions(firmware-update
	PRIVATE
  	-DCDC_FIFO_BUFFERS=${CDC_FIFO_BUFFERS}
)

