LOCAL_PATH := $(call my-dir)

#-------------------------------------------
#            Build HFP LIB
#-------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libhfp_pal
LOCAL_VENDOR_MODULE := true

ifeq ($(TARGET_BOARD_AUTO),true)
  LOCAL_CFLAGS += -DPLATFORM_AUTO
endif

LOCAL_SRC_FILES:= Hfp.cpp

LOCAL_CFLAGS += \
    -Wall \
    -Werror \
    -Wno-unused-function \
    -Wno-unused-variable

LOCAL_SHARED_LIBRARIES := \
    libaudioroute \
    libaudioutils \
    libcutils \
    libdl \
    libexpat \
    liblog \
    libar-pal

LOCAL_C_INCLUDES := \
    vendor/qcom/opensource/pal \
    vendor/qcom/opensource/audio-hal/primary-hal/hal \
    vendor/qcom/opensource/audio-hal/primary-hal/hal/audio_extn \
    external/expat/lib \
    system/media/audio_utils/include \
    $(call include-path-for, audio-route) \

LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_HEADER_LIBRARIES += libsystem_headers
LOCAL_HEADER_LIBRARIES += libarpal_headers
include $(BUILD_SHARED_LIBRARY)

#-------------------------------------------
#            Build FM LIB
#-------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libfmpal
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES:= FM.cpp

LOCAL_CFLAGS += \
    -Wall \
    -Werror \
    -Wno-unused-function \
    -Wno-unused-variable

LOCAL_SHARED_LIBRARIES := \
    libaudioroute \
    libaudioutils \
    libcutils \
    libdl \
    libexpat \
    liblog \
    libar-pal

LOCAL_C_INCLUDES := \
    vendor/qcom/opensource/pal \
    vendor/qcom/opensource/audio-hal/primary-hal/hal \
    vendor/qcom/opensource/audio-hal/primary-hal/hal/audio_extn \
    external/expat/lib \
    system/media/audio_utils/include \
    $(call include-path-for, audio-route) \

LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_HEADER_LIBRARIES += libsystem_headers
LOCAL_HEADER_LIBRARIES += libarpal_headers
include $(BUILD_SHARED_LIBRARY)

#-------------------------------------------
#            Build BATTERY_LISTENER
#-------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libbatterylistener
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES:= battery_listener.cpp

LOCAL_CFLAGS := \
    -Wall \
    -Werror \
    -Wno-unused-function \
    -Wno-unused-variable

# Define HEALTH_AIDL for targets other than anorak.
ifneq ($(TARGET_BOARD_PLATFORM), anorak)
    LOCAL_CFLAGS += -DHEALTH_AIDL
    $(warning "Using AIDL HEALTH")
endif

LOCAL_SHARED_LIBRARIES := \
    android.hardware.health@1.0 \
    android.hardware.health@2.0 \
    android.hardware.health@2.1 \
    android.hardware.power@1.2 \
    android.hardware.health-V1-ndk \
    libbinder_ndk \
    libaudioutils \
    libbase \
    libcutils \
    libdl \
    libhidlbase \
    liblog \
    libutils \

ifneq ($(TARGET_BOARD_PLATFORM), anorak)
    LOCAL_SHARED_LIBRARIES += \
        android.hardware.health-V1-ndk \
        libbinder_ndk
endif

LOCAL_STATIC_LIBRARIES := libhealthhalutils

include $(BUILD_SHARED_LIBRARY)
