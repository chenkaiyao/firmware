# Define the prefix to this directory.
# Note: The name must be unique within this build and should be
#       based on the root of the project
TARGET_HAL_SRC_INC_PATH = $(TARGET_HAL_PATH)/src/$(PLATFORM_NAME)

INCLUDE_DIRS += $(TARGET_HAL_SRC_INC_PATH)
INCLUDE_DIRS += $(TARGET_HAL_SRC_INC_PATH)/socket

include $(TARGET_HAL_PATH)/src/stm32f1xx-share/include.mk

