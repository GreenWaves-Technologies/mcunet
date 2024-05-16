# Copyright (c) 2022 GreenWaves Technologies SAS
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of GreenWaves Technologies SAS nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.16)

###############################################################################
# Application related definitions
###############################################################################

set(TARGET_NAME         "MCUNet")
set(MAIN                main.c)

###############################################################################
# CMake pre initialization
###############################################################################

include($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)
set(APP_MSG_PREFIX "[${BoldRed}${TARGET_NAME}${ColourReset}]")

###############################################################################
# Application's kconfig options
###############################################################################

set(IMAGE dataset/goldfish.ppm)

###############################################################################
# NNTool Variables 
###############################################################################
if(CONFIG_MODEL_NE16)
  message(STATUS "${APP_MSG_PREFIX} >> Building NE16 model <<")
  set(QUANT_TYPE "ne16")
else()
  message(STATUS "${APP_MSG_PREFIX} >> Building SQ8 model <<")
  set(QUANT_TYPE "sq8")
endif()

if (CONFIG_MODEL_ONNX)
  set(MODEL_FORMAT onnx)
  set(GEN_FLAGS "--quant_dataset_path=${CMAKE_SOURCE_DIR}/quant_data_ppm/")
  message(STATUS "${APP_MSG_PREFIX} ONNX")
else()
  set(MODEL_FORMAT tflite)
  set(GEN_FLAGS "--load_quantization")
  message(STATUS "${APP_MSG_PREFIX} TFLITE")
endif()

set(TRAINED_MODEL_PATH "${CMAKE_SOURCE_DIR}/models/mcunet-512kb-2mb_imagenet.${MODEL_FORMAT}")


set(NN_NNTOOL_SCRIPT ${CMAKE_SOURCE_DIR}/models/generate_at_model.py)
set(NN_DIR  ${CMAKE_BINARY_DIR}/BUILD_MODEL_NN)
set(NN_AT_MODEL_PATH      ${NN_DIR}/MCUNetModel.c)
set(NN_AT_TENSORS_DIR     ${NN_DIR}/tensors)
set(NN_EXPRESSION_KERNELS ${NN_DIR}/NN_Expression_Kernels.c)
autotiler_add_model(
  TARGET_NAME "MCUNet"
  MODEL_PATH ${NN_AT_MODEL_PATH}
  MODEL_TENSORS_DIR ${NN_AT_TENSORS_DIR}
  MODEL_BUILDDIR ${NN_DIR}
  PRI1_USED TRUE
  PRI1_FLASH mram
  PRI2_FILENAME "L3_Flash_Const.dat"
  L1_MEMORY ${CONFIG_MODEL_L1_MEMORY}
  L2_MEMORY ${CONFIG_MODEL_L2_MEMORY}
  L3_MEMORY ${CONFIG_MODEL_L3_MEMORY}
  L2STATIC_MEMORY ${CONFIG_MODEL_L2STATIC_MEMORY}
  L3STATIC_MEMORY ${CONFIG_MODEL_L3STATIC_MEMORY}
)
add_custom_command(
  OUTPUT ${NN_AT_MODEL_PATH} ${NN_AT_TENSORS_DIR} ${NN_EXPRESSION_KERNELS}
  COMMENT "Generate the autotiler model: ${NN_AT_MODEL_PATH}"
  DEPENDS ${NN_NNTOOL_SCRIPT}
  COMMAND python3 ${NN_NNTOOL_SCRIPT} --trained_model=${TRAINED_MODEL_PATH}
                                      --quant_type=${QUANT_TYPE}
                                      --expressions_file_path=${NN_EXPRESSION_KERNELS}
                                      --at_model_path=${NN_AT_MODEL_PATH}
                                      --tensors_dir=${NN_AT_TENSORS_DIR}
                                      --flash_type="mram"
                                      ${GEN_FLAGS}
                                      --test_image=${CMAKE_CURRENT_SOURCE_DIR}/${IMAGE}
)
add_custom_target(clean_model
  COMMENT "clean_model: removing ${COMMON_MODEL_BUILDDIR}/${MODEL_GEN}"
  COMMAND rm -rf ${NN_AT_MODEL_PATH}
)

###############################################################################
# Application dependencies
###############################################################################

list(APPEND TARGET_SRCS   ${MAIN}
                          ${CNN_LIB}
                          ${KERNEL_C_PATHS}
                          ${NN_EXPRESSION_KERNELS}
)

list(APPEND TARGET_INCS -I${TILER_INC}
                        -I${TILER_EMU_INC}
                        -I${GAP_SDK_HOME}/libs/gap_lib/include
                          ${CNN_LIB_INCLUDE}
                        -I${COMMON_MODEL_BUILDDIR}
                        -I${CMAKE_CURRENT_SOURCE_DIR}
                        -I${GAP_SDK_HOME}/utils/power_meas_utils)

list(APPEND TARGET_CFLAGS   -gdwarf-2
                            -gstrict-dwarf
                            -O3
                            -mno-memcpy
                            -fno-tree-loop-distribute-patterns)

if (CONFIG_MAX_PERF)
  set(FREQ 370)
  set(VOLTAGE 800)
elseif(CONFIG_MAX_EFFICIENCY)
  set(FREQ 240)
  set(VOLTAGE 650)
endif()
list(APPEND TARGET_PREPROCESSOR -DPERF
                                -DAT_MODEL_PREFIX=${MODEL_NAME}
                                -DSTACK_SIZE=${CONFIG_CL_MASTER_CORE_STACK_SIZE}
                                -DSLAVE_STACK_SIZE=${CONFIG_CL_SLAVE_CORE_STACK_SIZE}
                                -DAT_IMAGE=${CMAKE_CURRENT_SOURCE_DIR}/${IMAGE}
                                -DFREQ_CL=${FREQ}
                                -DFREQ_FC=${FREQ}
                                -DFREQ_PE=${FREQ}
                                -DVOLTAGE=${VOLTAGE}
                                -DAT_CNN=${AT_CNN})
if (DEFINED PERF_CI)
list(APPEND TARGET_PREPROCESSOR -DPERF_CI=${PERF_CI})
endif()
if (CONFIG_MODEL_NE16)
list(APPEND TARGET_PREPROCESSOR -DMODEL_NE16)
endif()

project(${TARGET_NAME} C ASM)
add_executable(${TARGET_NAME} ${TARGET_SRCS})

target_compile_options(${TARGET_NAME} PUBLIC    ${TARGET_CFLAGS} 
                                                ${TARGET_INCS}
                                                ${TARGET_PREPROCESSOR})


###############################################################################
# CMake post initialization
###############################################################################
setupos(${TARGET_NAME})

