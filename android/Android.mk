#
# Copyright (C) 2011 Dmitry Skiba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

ITOA_DROPINS_ROOT := ..
ITOA_DROPINS_ROOT_PATH := $(LOCAL_PATH)/$(ITOA_DROPINS_ROOT)

include $(CLEAR_VARS)

LOCAL_MODULE := itoa-dropins

LOCAL_SRC_FILES := $(ITOA_DROPINS_ROOT)/src/dropins.c
LOCAL_EXPORT_C_INCLUDES := $(ITOA_DROPINS_ROOT_PATH)/include

include $(BUILD_STATIC_LIBRARY)

