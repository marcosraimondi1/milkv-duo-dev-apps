from message_pb2 import KWS_Result

def decode_msg(data:bytes):
  result = KWS_Result()
  result.ParseFromString(data)

  print(f"DSP {result.timing.dsp} ms, Classification {result.timing.classification} ms")

  for prediction in result.predictions:
    print(prediction.label, prediction.value)

  return result



