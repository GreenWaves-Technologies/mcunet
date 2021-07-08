# MCUNet on GAP

## Original repository

ONNX model is converted from PyTorch model which is both with quantized TFLite version are taken from the original
[MCUNet repo](https://github.com/mit-han-lab/tinyml/tree/master/mcunet).
We are using [mcunet-512kb-2mb_imagenet.pth](https://hanlab.mit.edu/projects/tinyml/mcunet/release/mcunet-512kb-2mb_imagenet.pth)
and [mcunet-512kb-2mb_imagenet.pth](https://hanlab.mit.edu/projects/tinyml/mcunet/release/mcunet-512kb-2mb_imagenet.tflite) respectively.

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
If you want to use onnx model, run:

```
make clean all run platform=gvsoc MODEL_FORMAT=onnx
```
And for tflite model the command will change to:

```
make clean all run platform=gvsoc MODEL_FORMAT=tflite
```

## Evaluation
Fist thing you need to do is to navigate to the `accuracy_on_validation_data`:
```
cd accuracy_on_validation_data
```

To evaluate the model you will need to dump the results on the properly preprocessed data which is preprocessed the same way as for the original model.
We are using a [3923 images subset](https://drive.google.com/file/d/1HZ1vgkJ2KYQpRzs0Y20aG8eUYkCb7ME0/) from validation imagenet dataset.
To preprocess them, run:
```
python3 preprocess_gap_images.py --val-dir /path/to/original/val/data  --out-dir /path/to/preprocessed/data
```
After that you can dump the results with:
```
python3 get_gap_results.py --in_dir /path/to/preprocessed/data--out_dir /path/to/results/folder

```
Next step is to evaluate the obtained for each image outputs with the command below:
```
python3 eval_gap.py --val-dir /path/to/preprocessed/data --gap-dir  /path/to/results/folder

```

For [3923 images subset](https://drive.google.com/file/d/1HZ1vgkJ2KYQpRzs0Y20aG8eUYkCb7ME0/) from validation imagenet dataset the metrics are the following:
| MODEL | Top-1 Accuracy, % | Top-5 Accuracy, % |
|-------|------------------------|------------------------|
| MCUNet 512kB-2MB PyTorch Float32  |  68.2 |  88 |
| MCUNet 512kB-2MB ONNX Float32  |  68.09 |  87.97 |
| MCUNet 512kB-2MB TFlite Int8  |  67.68 |  87.64  |
| MCUNet 512kB-2MB ONNX GAP Int8  |  66.45 |  87.36 |
| MCUNet 512kB-2MB TFlite GAP Int8  |  67.07 |  87.25 |
