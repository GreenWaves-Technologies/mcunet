#!/bin/bash

nntool -s convert_tflite_model.nnscript
nntool -g nnquant.json -m model.c -T weights
