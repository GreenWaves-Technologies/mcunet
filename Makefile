USE_PMSIS_BSP=1
export GAP_USE_OPENOCD=1
io=host

MODEL_GEN = modelKernels
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(DETECTOR_GEN_C) $(addsuffix .h, $(MODEL_GEN))

GenNet: model.c
	gcc -fcommon -o $@ $(TILER_PLATFORM_FLAGS) \
	-I$(TILER_EMU_INC) \
	-I"$(TILER_INC)" \
	-I"$(TILER_CNN_GENERATOR_PATH)" \
	-I"$(TILER_CNN_GENERATOR_PATH_SQ8)" \
	-I"$(TILER_CNN_KERNEL_PATH_SQ8)" \
	-I"$(NNTOOL_GENERATOR_PATH)" \
	-I"$(TILER_CNN_KERNEL_PATH)" \
	-I"$(PWD)/../" \
	model.c \
	"$(TILER_CNN_GENERATOR_PATH_SQ8)/CNN_Generators_SQ8.c" \
	"$(TILER_CNN_GENERATOR_PATH_SQ8)/RNN_Generators_SQ8.c" \
	"$(TILER_CNN_GENERATOR_PATH)/CNN_Generator_Util.c" \
	"$(NNTOOL_GENERATOR_PATH)/nntool_extra_generators.c" \
	$(TILER_LIB)

$(MODEL_GEN_C): GenNet
	./GenNet

tiler_model: $(MODEL_GEN_C)

build: tiler_model

CNN_KERNELS_SRC = \
  $(wildcard $(TILER_CNN_KERNEL_PATH_SQ8)/CNN_*SQ8.c) \
  $(TILER_CNN_KERNEL_PATH_SQ8)/CNN_AT_Misc.c \
  $(wildcard $(NNTOOL_KERNELS_PATH)/*.c)

APP_SRCS = \
  main.c \
  modelKernels.c \
  $(CNN_KERNELS_SRC)\
  $(GAP_LIB_PATH)/img_io/ImgIO.c

APP_INC += "$(PWD)" \
	"$(TILER_INC)" \
	"$(TILER_CNN_KERNEL_PATH_SQ8)" \
	"$(NNTOOL_KERNELS_PATH)" \
	"$(TILER_CNN_KERNEL_PATH)" \
    

DATA_FILES = model_L3_Flash_Const.dat
APP_CFLAGS += -g -Os

SRCS = 

ifeq ($(ALREADY_FLASHED),1)
# everything is already on board
else
  READFS_FILES+=$(realpath $(DATA_FILES))
endif

clean::
	rm -f GenNet
	rm -f $(MODEL_GEN_CLEAN)
	rm -f model_L3_Flash_Const.dat

.PHONY: tiler_model all clean

include $(RULES_DIR)/pmsis_rules.mk
    
