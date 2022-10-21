NNTOOL=nntool
MODEL_SQ8=1



MODEL_SUFFIX?=
MODEL_PREFIX?=model
MODEL_PYTHON=python3
MODEL_BUILD=BUILD_MODEL$(MODEL_SUFFIX)
$(info  MODEL_FORMAT is $(MODEL_FORMAT))
ifeq ($(MODEL_FORMAT), onnx)
  TRAINED_MODEL = models/mcunet-512kb-2mb_imagenet.onnx
  MODEL_PATH = $(MODEL_BUILD)/$(MODEL_PREFIX).onnx
else ifeq ($(MODEL_FORMAT), tflite)
  TRAINED_MODEL = models/mcunet-512kb-2mb_imagenet.tflite
  MODEL_PATH = $(MODEL_BUILD)/$(MODEL_PREFIX).tflite
  MODEL_QUANTIZED=1
else
  $(error MODEL_FORMAT can only be tflite or onnx)
endif

TENSORS_DIR = $(MODEL_BUILD)/tensors
MODEL_TENSORS = $(MODEL_BUILD)/$(MODEL_PREFIX)_L3_Flash_Const.dat

MODEL_STATE = $(MODEL_BUILD)/$(MODEL_PREFIX).json
MODEL_SRC = $(MODEL_PREFIX)Model.c
MODEL_HEADER = $(MODEL_PREFIX)Info.h
MODEL_GEN = $(MODEL_BUILD)/$(MODEL_PREFIX)Kernels $(MODEL_BUILD)/Expression_Kernels
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(MODEL_GEN_C) $(addsuffix .h, $(MODEL_GEN))
MODEL_GEN_EXE = $(MODEL_BUILD)/GenTile

MODEL_EXPRESSIONS = #$(MODEL_BUILD)/Expression_Kernels.c

# Cluster stack size for master core and other cores
CLUSTER_STACK_SIZE=2048
CLUSTER_SLAVE_STACK_SIZE=512
# Memory sizes for cluster L1, SoC L2 and Flash
ifeq '$(TARGET_CHIP_FAMILY)' 'GAP9'
  CLUSTER_NUM_CORES=8
  TOTAL_STACK_SIZE = $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* 8)
  FREQ_CL?=370
  FREQ_FC?=370
  FREQ_PE?=370
  MODEL_L1_MEMORY=$(shell expr 128000 \- $(TOTAL_STACK_SIZE))
  MODEL_L2_MEMORY=1350000
  MODEL_L3_MEMORY=8000000
  USE_SECONDARY_FLASH?=1
else
  CLUSTER_NUM_CORES=7
  TOTAL_STACK_SIZE = $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* 7)
  FREQ_CL?=175
  FREQ_FC?=250
  FREQ_PE?=250
  MODEL_L1_MEMORY=$(shell expr 64000 \- $(TOTAL_STACK_SIZE))
  MODEL_L2_MEMORY?=200000
  MODEL_L3_MEMORY=8000000
  USE_SECONDARY_FLASH=0
endif

ifeq ($(USE_SECONDARY_FLASH), 1)
MODEL_SEC_L3_FLASH=AT_MEM_L3_MRAMFLASH
else
MODEL_SEC_L3_FLASH=''
endif

#VOLTAGE?=800
ifdef VOLTAGE
  APP_CFLAGS += -DVOLTAGE=$(VOLTAGE)
endif

ifeq ($(MODEL_FORMAT), onnx)
  NNTOOL_SCRIPT = models/nntool_scripts/nntool_script_onnx
  MODEL_QUANTIZED=0
else ifeq ($(MODEL_FORMAT), tflite)
  NNTOOL_SCRIPT = models/nntool_scripts/nntool_script_tflite
  MODEL_QUANTIZED=1
endif

ifeq ($(MODEL_NE16), 1)
  NNTOOL_SCRIPT := $(NNTOOL_SCRIPT)_ne16
  MODEL_SUFFIX = _NE16
  APP_CFLAGS += -DMODEL_NE16
endif
$(info GEN ... $(CNN_GEN))
