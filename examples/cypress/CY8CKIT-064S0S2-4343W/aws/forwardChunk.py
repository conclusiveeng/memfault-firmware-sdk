import json
import base64
import urllib3 

memfault_key = <INSERT KEY HERE> 
headers = { 
    "Memfault-Project-Key" : memfault_key,
    "Content-Type" : "application/octet-stream"
}
http = urllib3.PoolManager()

def lambda_handler(event, context):
    device_id = event["device_id"]
    chunk = event["chunk"]
    decoded_chunk = base64.b64decode(chunk)
    
    resp = http.request(
        "POST", 
        f"https://chunks.memfault.com/api/v0/chunks/{device_id}",
        body=decoded_chunk,
        headers=headers)
    
