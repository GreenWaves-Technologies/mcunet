
from onnxruntime.quantization import QuantFormat, QuantType, quantize_static
from onnxruntime.quantization import CalibrationDataReader

from tqdm import tqdm
import numpy as np
import os
import onnxruntime
from PIL import Image
from generate_at_model import representative_dataset

class QuantDataReader(CalibrationDataReader):
    def __init__(self, calibration_image_folder: str, model_path):
        self.enum_data = None

        self.dataset = iter([os.path.join(calibration_image_folder, file) for file in os.listdir(calibration_image_folder)])
        # Use inference session to get input shape.
        session = onnxruntime.InferenceSession(model_path, None)
        shape = session.get_inputs()[0].shape

        # Convert image to input data
        self.input_name = session.get_inputs()[0].name
        self.nhwc_data_list = [(np.array(Image.open(f).resize((shape[2], shape[3]))).astype(np.float32).reshape(shape) / 255 - 0.5) / 0.5 for f in self.dataset]

    def get_next(self):
        if self.enum_data is None:
            self.enum_data = iter(
                [{self.input_name: nhwc_data} for nhwc_data in self.nhwc_data_list]
            )

        return next(self.enum_data, None)


if __name__ == "__main__":
    input_model_path = "models/mcunet-512kb-2mb_imagenet.onnx"
    output_model_path = "models/mcunet-512kb-2mb_imagenet_quant.onnx"
    quant_dataset_path = "quant_data_ppm"

    dr = QuantDataReader(quant_dataset_path, input_model_path)

    # Calibrate and quantize model
    # Turn off model optimization during quantization
    quantize_static(
        input_model_path,
        output_model_path,
        dr,
        quant_format=QuantFormat.QOperator,
        per_channel=True,
        weight_type=QuantType.QInt8,
    )
    print("Calibrated and quantized model saved.")
