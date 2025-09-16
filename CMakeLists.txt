cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(blink 
	main.c
	inputs.c
	tcp.c
	tls.c
	txt/msc_disk.c
	txt/usb_descriptors.c 
	txt/wifi_setting.c
)

target_include_directories(blink 
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/txt/
)

# pull in common dependencies
target_link_libraries(blink 
	pico_stdlib
	FreeRTOS-Kernel 
	FreeRTOS-Kernel-Heap4

	pico_cyw43_arch_lwip_threadsafe_background
	pico_mbedtls
  	pico_lwip_mbedtls

	tinyusb_additions
	tinyusb_board
	tinyusb_device
	hardware_flash
	hardware_sync
)  

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink)
