# MCUNet on GAP

## Original repository

ONNX model is converted from PyTorch model which is both with quantized TFLite version are taken from the original
[MCUNet repo](https://github.com/mit-han-lab/mcunet/tree/master).
We are using [mcunet-512kb-2mb_imagenet.pth](https://hanlab.mit.edu/projects/tinyml/mcunet/release/mcunet-512kb-2mb_imagenet.pth)
and [mcunet-512kb-2mb_imagenet.pth](https://hanlab.mit.edu/projects/tinyml/mcunet/release/mcunet-512kb-2mb_imagenet.tflite) respectively. (The models might be available only from the github link)

## Supported Models

Depending on your needs, onnx model (which requires further quantization) and quantized tflite models are supported.

## Quantization Data

If you'd like to run onnx model on GAP, you first need to download data to quantize the model on.
To do so simply run:
```
./download_quant_data.sh
```

## Inference On GAP8

You can run the application on GAP8 (or platform simulator gvsoc).
If you want to use onnx model, you need first to download the post training quantization dataset and then run:

```
make clean all run platform=gvsoc MODEL_FORMAT=onnx
```
And for tflite model the command will change to:

```
make clean all run platform=gvsoc MODEL_FORMAT=tflite
```

## Evaluation

The comparison metrics table for all of the models is the following:
| MODEL | Top-1 Accuracy, % | Top-5 Accuracy, % |
|-------|------------------------|------------------------|
| MCUNet 512kB-2MB PyTorch Float32  |  68.2 |  88 |
| MCUNet 512kB-2MB ONNX Float32  |  68.09 |  87.97 |
| MCUNet 512kB-2MB TFlite Int8  |  67.68 |  87.64  |
| MCUNet 512kB-2MB ONNX GAP Int8  |  67.04 |  87.20 |
| MCUNet 512kB-2MB TFlite GAP Int8  |  67.07 |  87.25 |

New
| MODEL | Top-1 Accuracy, % | Top-5 Accuracy, % |
|-------|------------------------|------------------------|
| MCUNet 512kB-2MB TFlite GAP NE16 Int8  |  67.12 |  87.57 |
| MCUNet 512kB-2MB TFlite GAP SQ8 Int8  |  67.21 |  87.52 |
