#!/bin/bash

nntool -s convert_onnx_model.nnscript
nntool -g nnquant.json -m model.c -T weights
