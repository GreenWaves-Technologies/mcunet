# MCUNet on GAP

## Original repository

PyTorch model as well as its quantized TFLite version are taken from the original 
[MCUNet repo](https://github.com/mit-han-lab/tinyml/tree/master/mcunet)

## Quantization Data

Before NNTool conversion, you need to download data to quantize the model on.
Follow [the link](https://drive.google.com/drive/folders/1BLZJic1oSRe9z2NliH70_QiK8Qehtuc9?usp=sharing) to dowload prepared data and put it in the project folder so it has a location: `mcunet/quant_data_ppm`. It will be used on the next step during conversion process.

## Original repository

[MCUNet](https://github.com/mit-han-lab/tinyml/tree/master/mcunet)

## Convert model with NNTool
Change model path or some other steps for NNTool in `convert_model.nnscript`. Then run:

```
./convert_model.sh
```
## Build model and run the app

After conversion, select source in GAP SDK(8 - GAPUINO_V2) and run:

```
make clean all run platform=gvsoc
```

## Evaluation

To evaluate the model you first need to dump the results.
Make sure, you are passing images preprocessed the same way as for the original model.
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
| MCUNet 512kB-2MB GAP Int8  |  66.45 |  87.36 |
