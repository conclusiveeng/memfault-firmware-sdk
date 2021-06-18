import json
import boto3

mqtt = boto3.client('iot-data')

def lambda_handler(event, context):
    device_id = event["device_id"]
    chunk = event["chunk"]
    topic = f"device/{device_id}/chunk"
    
    response = mqtt.publish(topic=topic, qos=1, payload=chunk)
