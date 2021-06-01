# MCUNet

## Convert model with NNTool
Change model path or some other steps for NNTool in convert_model.nnscript. Then run

```
./convert_model.sh
```
## Build model and run the app

After conversion, select source in GAP SDK(8 - GAPUINO_V2
) and run

```
make clean all run platform=gvsoc
```

## Issues

**Model doesn't initializing with error 3**

In that case,change 963 row in `model.c` to
```
modelModel(52000, 250*1024, 8*1024*1024, 20*1024*1024);
```
Probably, we can fix it before compilation step?

**After prediction, all network_output array has only zeros**

We need to check that input is correctly passed and the model is actually running.

**Do we need add additional channel for batch_size?**

Right now input for a model has (3x160x160) size.