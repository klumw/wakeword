

set(EI_SDK_FOLDER ${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk)

include(${EI_SDK_FOLDER}/cmake/utils.cmake)

RECURSIVE_FIND_FILE_APPEND(MODEL_SOURCE "tflite-model" "*.cpp")
RECURSIVE_FIND_FILE_APPEND(EI_SOURCE_FILES "${EI_SDK_FOLDER}" "*.cpp")
RECURSIVE_FIND_FILE_APPEND(EI_SOURCE_FILES "${EI_SDK_FOLDER}" "*.cc")
RECURSIVE_FIND_FILE_APPEND(EI_SOURCE_FILES "${EI_SDK_FOLDER}" "*.s")
RECURSIVE_FIND_FILE_APPEND(EI_SOURCE_FILES "${EI_SDK_FOLDER}/porting/espressif/ESP-NN" "*.c")
RECURSIVE_FIND_FILE_APPEND(EI_SOURCE_FILES "${EI_SDK_FOLDER}/porting/espressif" "*.cpp")
list(APPEND EI_SOURCE_FILES "${EI_SDK_FOLDER}/tensorflow/lite/c/common.c")

idf_component_register(SRCS ${EI_SOURCE_FILES} ${MODEL_SOURCE} INCLUDE_DIRS "." PRIV_REQUIRES esp_timer)




