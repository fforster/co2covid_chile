import json
import boto3
from boto3.dynamodb.conditions import Key
from datetime import datetime, timedelta
import ast

dynamodb_client = boto3.client("dynamodb", region_name="us-east-2")
dynamodb = boto3.resource("dynamodb")
table = dynamodb.Table("co2covid")

import decimal

# Helper class to convert a DynamoDB item to JSON.
class DecimalEncoder(json.JSONEncoder):
    def default(self, o):
        if isinstance(o, decimal.Decimal):
            if o % 1 > 0:
                return float(o)
            else:
                return int(o)
        return super(DecimalEncoder, self).default(o)

# event handler
def lambda_handler(event, context):
    
    # output response
    output = {}
    
    # if max_isotime is not given assume now, if min_isotime is not given assume one hour before now
    now = datetime.now().isoformat()
    try:
        if "max_isotime" in event["queryStringParameters"].keys():
            max_timestamp = str(int(datetime.fromisoformat(event["queryStringParameters"]["max_isotime"]).timestamp() * 1000))
        else:
            max_timestamp = str(int(datetime.now().timestamp() * 1000))
        if "min_isotime" in event["queryStringParameters"].keys():
            min_timestamp = str(int(datetime.fromisoformat(event["queryStringParameters"]["min_isotime"]).timestamp() * 1000))
        else:
            min_timestamp = str(int((datetime.fromtimestamp(int(max_timestamp) / 1000.) - timedelta(hours=1, minutes=0)).timestamp() * 1000))
    except:
        output["body"] = json.dumps({"status": "fail", "data": {"code": "400", "message": "Time must be given in iso format, e.g., 2021-03-25, 2021-03-25T12, 2021-03-25T12:00, or 2021-03-25T12:00:00"}})
        return output
        
    if max_timestamp <= min_timestamp:
        output["body"] = json.dumps({"status": "fail", "data": {"code": "400", "message": "The maximum time (now if not given) must be larger than then minimum time."}})
        return output
        

    # check if last_key available
    if "last_device_id" in event["queryStringParameters"].keys():
        ExclusiveStartKey = {}
        ExclusiveStartKey['device_id'] = {"S": event["queryStringParameters"]["last_device_id"]}
        ExclusiveStartKey['sample_time'] = {"N": event["queryStringParameters"]["last_timestamp"]}
    else:
        ExclusiveStartKey = None
    
    # do query
    if ExclusiveStartKey is None:
        response = dynamodb_client.query(
            TableName="co2covid",
            KeyConditionExpression='device_id = :device_id AND sample_time BETWEEN :min_timestamp AND :max_timestamp',
            ExpressionAttributeValues={
                ':device_id': {'S': event["queryStringParameters"]["device_id"]},
                ':min_timestamp': {'N': min_timestamp},
                ':max_timestamp': {'N': max_timestamp}},
            Limit=500)
    else:
        response = dynamodb_client.query(
            TableName="co2covid",
            KeyConditionExpression='device_id = :device_id AND sample_time BETWEEN :min_timestamp AND :max_timestamp',
            ExpressionAttributeValues={
                ':device_id': {'S': event["queryStringParameters"]["device_id"]},
                ':min_timestamp': {'N': min_timestamp},
                ':max_timestamp': {'N': max_timestamp}},
            Limit=500,
            ExclusiveStartKey=ExclusiveStartKey)
            
    # prepare output
    items = []
    
    compact = False
    if compact:
        items["timestamp"] = [i["sample_time"]["N"] for i in response["Items"]]
        items["isotime"] = [datetime.fromtimestamp(int(i["sample_time"]["N"]) / 1000.).isoformat() for i in response["Items"]]
        items["device_id"] = [i["device_id"]["S"] for i in response["Items"]]
        items["co2"] = [i["device_data"]["M"]["co2"]["N"] for i in response["Items"]]
        items["temperature"] = [i["device_data"]["M"]["temperature"]["N"] for i in response["Items"]]
        items["humidity"] = [i["device_data"]["M"]["humidity"]["N"] for i in response["Items"]]
    else:
        for idx, i in enumerate(response["Items"]):
            items.append({"timestamp": i["sample_time"]["N"],
                        "isotime": datetime.fromtimestamp(int(i["sample_time"]["N"]) / 1000.).isoformat(),
                        "device_id": i["device_id"]["S"],
                        "co2": i["device_data"]["M"]["co2"]["N"],
                        "temperature": i["device_data"]["M"]["temperature"]["N"],
                        "humidity": i["device_data"]["M"]["humidity"]["N"]
                        })
        
    # add last evaluated key
    if "LastEvaluatedKey" in response.keys():
        next_url = "https://" + event["requestContext"]["apiId"] + ".execute-api.us-east-2.amazonaws.com/dev" \
                        + event["requestContext"]["resourcePath"]
        next_url += "?device_id=" + event["queryStringParameters"]["device_id"]
        next_url += "&min_isotime=" + event["queryStringParameters"]["min_isotime"]
        if "max_isotime" in event["queryStringParameters"].keys():
            next_url += "&max_isotime=" + event["queryStringParameters"]["max_isotime"]
        next_url += "&last_device_id=" + response["LastEvaluatedKey"]["device_id"]["S"]
        next_url += "&last_timestamp=" + response["LastEvaluatedKey"]["sample_time"]["N"]
        print(next_url)
        output["body"] = json.dumps({"data": items, "now": now, "next_url": next_url}, cls=DecimalEncoder)
    else:
        output["body"] = json.dumps({"data": items, "now": now}, cls=DecimalEncoder)
    
    # make sure the headers are correct ('access-control-allow-origin': '*' allows for access from anywhere)
    output["statusCode"] = '200'
    output["headers"] = {
                        "Access-Control-Allow-Headers" : "Content-Type",
                        "Access-Control-Allow-Origin": "*",
                        "Access-Control-Allow-Methods": "OPTIONS,POST,GET"
                        }
    
    return output
