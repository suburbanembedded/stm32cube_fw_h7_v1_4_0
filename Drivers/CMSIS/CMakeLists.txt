add_library(stm32h7xx_cmsis
	Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c
)

target_include_directories(stm32h7xx_cmsis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/
	${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/STM32H7xx/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../STM32H7xx_HAL_Driver/Inc
)

add_library(stm32h743xx_cmsis 
	Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s
)

target_include_directories(stm32h743xx_cmsis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/STM32H7xx/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../STM32H7xx_HAL_Driver/Inc
)

add_library(stm32h750xx_cmsis 
	Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h750xx.s
)

target_include_directories(stm32h750xx_cmsis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/STM32H7xx/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../STM32H7xx_HAL_Driver/Inc
)


add_library(stm32h753xx_cmsis 
	Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h753xx.s
)

target_include_directories(stm32h753xx_cmsis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/STM32H7xx/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../STM32H7xx_HAL_Driver/Inc
)
