add_library(freertos_v10

	Source/croutine.c
	Source/event_groups.c
	Source/list.c
	Source/queue.c
	Source/tasks.c
	Source/timers.c

	Source/portable/GCC/ARM_CM7/r0p1/port.c

	Source/portable/MemMang/heap_4.c
)

add_library(freertos_v10_heap1
	Source/portable/MemMang/heap_1.c
)

add_library(freertos_v10_heap2
	Source/portable/MemMang/heap_2.c
)

add_library(freertos_v10_heap3
	Source/portable/MemMang/heap_3.c
)

add_library(freertos_v10_heap4
	Source/portable/MemMang/heap_4.c
)

add_library(freertos_v10_heap5
	Source/portable/MemMang/heap_5.c
)

add_library(freertos_v10_cmsis
	Source/CMSIS_RTOS/cmsis_os.c
)

target_include_directories(freertos_v10 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Source/include
	${CMAKE_CURRENT_SOURCE_DIR}/Source/portable/GCC/ARM_CM7/r0p1
)

target_link_libraries(freertos_v10_heap1
	freertos_v10
)

target_link_libraries(freertos_v10_heap2
	freertos_v10
)

target_link_libraries(freertos_v10_heap3
	freertos_v10
)

target_link_libraries(freertos_v10_heap4
	freertos_v10
)

target_link_libraries(freertos_v10_heap5
	freertos_v10
)

target_link_libraries(freertos_v10_cmsis
	freertos_v10
	stm32h7xx_cmsis
)

target_include_directories(freertos_v10_cmsis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Source/CMSIS_RTOS
)
