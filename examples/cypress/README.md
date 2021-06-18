# Memfault for Cypress CY8CKIT-064S0S2-4343W kit with Amazon-FreeRTOS

This folder contains an integration example for the Cypress CY8CKIT-064S0S2-4343W kit with Amazon-FreeRTOS & AWS IoT/Lambda. It is a fairly straightforward app that first simulates a fault, collects chunks, estabilishes a secure connection to the AWS IoT MQTT broker, publishes the chunk and receives it back from AWS IoT/Lambda on a separate topic. 

The demo app is tested on a [CY8CKIT-064S0S2-4343W] evaluation board. The instructions below also assume same board. The app is tested to work with  ModusToolbox 2.3.

## Getting Started

- [awscli](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html) is required to perform some of the steps in this document.
- ModusToolbox setup instructions can be found [here](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_cypress_psoc64.html).
- You'll need an AWS account with IoT Core and an IAM user. How to setup an IAM user can be found [here](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_create.html).
- The board should be configured according to this [document](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html)

# Provisioning, certificates, keys & policies

Your device should be provisioned as per the [document](https://community.cypress.com/docs/DOC-20043), with one difference - use `policy_multi_CM0_CM4_jitp.json`, as this is the one actually used by the project to generate a firmware image. We'll hardcode the IoT Thing certificate and key into the app.

<TODO: Setup memfault>
<TODO: Hardcoding certs & policies, etc>

<!-- An Project Key will need to be baked into the demo app to enable it to
communicate with Memfault's web services. Go to https://app.memfault.com/,
navigate to the project you want to use and select 'Settings'. Copy the 'Project 
Key' and paste it into `apps/memfault_demo_app/memfault_demo_app.c`, 
replacing `<YOUR PROJECT KEY HERE>` with your Project Key. Save the file and 
rebuild, reflash the project and attach the debug console (see instruction above). -->

# Setting up AWS IoT & Lambda

In order to be able to use Lambda with IoT, the user has to [have permission to create roles and attach role policies](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_create_for-service.html). On top of that, we want two more actions to be available for the IAM user: `lambda:AddPermission` and `iam:CreatePolicy`. You can do it either using `awscli` or the IAM Management Console GUI - either way you'll have to attach the appropriate permissions policy to the IAM entity you're using. Next, we need to setup an [IoT trust role and policy](https://docs.aws.amazon.com/iot/latest/developerguide/iot-create-role.html). 

Once that's done, we'll be able to [setup a rule](https://docs.aws.amazon.com/iot/latest/developerguide/iot-repub-rule.html) that passes messages from our topic `device/+/data` to a lambda function. Keep in mind our rule will have two differences from the one in the link above:
1) Rule query statement - `SELECT topic(2) as device_id, chunk FROM 'device/+/data'`
2) Action - `Send a message to a Lambda function`

To setup the lambda, select `Create a new lambda function` -> `Author from scratch`. Type in a name, e.g. `republishChunk` and  use `Python 3.8` as the runtime. Once the function is created, two permissions are required to make it work:
1) Invoke the function: `aws lambda add-permission --function-name republishChunk --statement-id iot-events --action "lambda:InvokeFunction" --principal iot.amazonaws.com`
2) Publishing using the IoT MQTT Broker: `iot:Publish`

Once that's out of the way, we'll have to write the actual function for it to work properly. You'll find the demo function at `aws/republishChunk.py`.

### Building & flashing

First import the app into a ModusToolbox workspace, then either build the application using `Build application`, or do it directly from shell in the project directory by using `make build`. 

Analogously to flash the device either use `application program` from ModusToolbox, or `make program` from shell. In order for it to work, please make sure that board connection is in CMSIS-DAP mode.

### Serial output via UART

There should be two `/dev/ttyACM` devices available when the board is connected using the KitProg USB - one of them is the console. Connect to it using e.g. `screen`. Serial parameters are:
```
Baud rate: 115200
Data: 8 bit
Parity: None
Stop bits: 1
Flow control: None
```

# Troubleshooting

## TLS error

If you get a TLS error when trying to estabilish a connection to the IoT MQTT Broker saying that your certificate is incorrect, you might need to use a different IoT endpoint - try replacing the default address with: `aws iot describe-endpoint --endpoint-type iot:Data-ATS`.
