
add_library(stm32_usb_lib_core
	#Core/Src/usbd_conf_template.c
	Core/Src/usbd_core.c
	Core/Src/usbd_ctlreq.c
	Core/Src/usbd_ioreq.c
)

target_include_directories(stm32_usb_lib_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
)

add_library(stm32_usb_lib_cdc
	#Class/CDC/Src/usbd_cdc_if_template.c
	Class/CDC/Src/usbd_cdc.c
)

target_include_directories(stm32_usb_lib_cdc PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Class/CDC/Inc
)

target_link_libraries(stm32_usb_lib_core
	stm32h750xx_cmsis
)

target_link_libraries(stm32_usb_lib_cdc
	stm32_usb_lib_core
)