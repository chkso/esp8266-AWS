# esp8266-AWS
A DIY project for measuring humidity and temperature and notifying the user when the humidity threshold is exceeded. An ESP8266 device with a BME sensor is used for taking the measurements and sending them to the cloud. AWS services such as API Gateway, Lambda, DynamoDB and SNS are used for processing the data, storing them and for notifications.

## Contents
The following files are present:
1. **A CloudFormation template** which will provision all necessary resources. During the "startup" phase, the script will request a humidity threshold for creating a notification and an email address where the notifications will be sent.
Following are the resources provisioned by the template:
* DynamoDB table
* SNS Topic
* Rest API (API Gateway) with all "sub-resources" such as method, resource and the deployment itself
* Lambda function

After successful execution, the script will return the API endpoint URL and the DynamoDB table name. 

2. **An Arduino source file** where the WiFi credentials and the API endpoint URL need to be set. After that, the code can be compiled and deployed on an ESP8266 device - NodeMCU v0.9 and BME 280 sensor were used in my case. 
