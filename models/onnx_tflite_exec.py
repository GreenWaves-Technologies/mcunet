import numpy as np

def execute_tflite(model_path, input_array):
    import tensorflow as tf

    interpreter = tf.lite.Interpreter(model_path=model_path)
    interpreter.allocate_tensors()

    # Get input and output tensors.
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    # print('Input details: ', input_details)
    # print('Output details: ', output_details)

    scale, zero_point = input_details[0]['quantization']
    # input_h, input_w = input_details[0]['shape'][1], input_details[0]['shape'][2]
    input_array = input_array.reshape(input_details[0]['shape'])
    if scale != 0.0:
        dtype = input_details[0]['dtype']
        min_value = np.iinfo(dtype).min
        max_value = np.iinfo(dtype).max
        input_array = np.clip(np.floor(input_array / scale + 0.5) + zero_point, min_value, max_value).astype(dtype)

    interpreter.set_tensor(input_details[0]['index'], input_array.reshape(input_details[0]['shape']))
    interpreter.invoke()
    out_scale, out_zp = output_details[0]['quantization']
    return interpreter.get_tensor(output_details[0]['index']), out_scale, out_zp

def execute_onnx(model_path, input_array):
    import onnxruntime as ort
    sess = ort.InferenceSession(model_path)
    input_name = sess.get_inputs()[0].name
    output = sess.run(None, {input_name: input_array.reshape(sess.get_inputs()[0].shape)})
    output = np.asarray(output)
    return output

