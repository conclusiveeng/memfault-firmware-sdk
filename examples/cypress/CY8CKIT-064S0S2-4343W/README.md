# Memfault for Cypress CY8CKIT-064S0S2-4343W kit with Amazon-FreeRTOS

This folder contains an integration example for the Cypress [CY8CKIT-064S0S2-4343W](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit) kit with Amazon-FreeRTOS & AWS IoT/Lambda. It is a simple app that first simulates a fault, collects chunks, estabilishes a secure connection to the AWS IoT MQTT broker, publishes the chunk, passes it from AWS IoT to Lambda, which then posts it to Memfault. 

The demo app is tested on a [CY8CKIT-064S0S2-4343W](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit) evaluation board. The instructions below also assume same board. The app is tested to work with ModusToolbox 2.3.

## Getting Started

- [awscli](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html) is required to perform some of the steps in this document.
- ModusToolbox setup instructions can be found [here](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_cypress_psoc64.html).
- You'll need an AWS account with IoT Core and an IAM user. How to setup an IAM user can be found [here](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_create.html).
- The board should be configured according to this [document](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html)

# Provisioning, certificates, keys & policies

Your device should be provisioned as per the [document](https://community.cypress.com/docs/DOC-20043), with one difference - use `policy_multi_CM0_CM4_jitp.json`, as this is the one actually used by the project to generate a firmware image. If you'll use the default tfm policy, it won't work.

In `demos/include/aws_clientcredential.h`, you'll have to type in:
- `clientcredentialMQTT_BROKER_ENDPOINT` - MQTT broker endpoint retrieved from `awscli`
- `clientcredentialIOT_THING_NAME` - AWS IoT thing name
- `clientcredentialWIFI_SSID` - Wifi SSID string
- `clientcredentialWIFI_PASSWORD` - Wifi password
- `clientcredentialWIFI_SECURITY` - Wifi security

You'll also have to generate the header file with your AWS IoT Thing certificate & private key. To do that, you'll have to pass them to `tools/certificate_configuration/CertificateConfigurator.html` and replace `demos/include/aws_clientcredential_keys.h` with the resulting file.

# Setting up AWS IoT & Lambda

In order to be able to use Lambda with IoT, the user has to [have permission to create roles and attach role policies](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_create_for-service.html). On top of that, we want two more actions to be available for the IAM user: `lambda:AddPermission` and `iam:CreatePolicy`. You can do it either using `awscli` or the IAM Management Console GUI - either way you'll have to attach the appropriate permissions policy to the IAM entity you're using. Next, we need to setup an [IoT trust role and policy](https://docs.aws.amazon.com/iot/latest/developerguide/iot-create-role.html). 

Once that's done, we'll be able to [setup a rule](https://docs.aws.amazon.com/iot/latest/developerguide/iot-repub-rule.html) that passes messages from our topic `device/+/data` to a lambda function. Follow the instructions in the link, but keep in mind our rule will have two differences from the one above:
1) Rule query statement - `SELECT encode(*, 'base64') AS chunk, topic(2) AS device_id FROM 'device/+/data'`
2) Action - `Send a message to a Lambda function`

To setup the lambda, select `Create a new lambda function` -> `Author from scratch`. Type in a name, e.g. `republishChunk` and  use `Python 3.8` as the runtime. Once the function is created, invoke `aws lambda add-permission --function-name republishChunk --statement-id iot-events --action "lambda:InvokeFunction" --principal iot.amazonaws.com` to give appropriate permissions to IoT.

Once that's out of the way, you'll find the demo function at `aws/republishChunk.py` which you have to paste into the editor. Make sure to replace `<INSERT KEY HERE>` with your Memfault Project Key, which you can find in your [Memfault dashboard](https://app.memfault.com), before using it.

### Building & flashing

First import the app into a ModusToolbox workspace, then either build the application using `Build application`, or do it directly from shell in the project directory by using `make build`. 

Analogously to flash the device either use `application program` from ModusToolbox, or `make program` from shell. In order for it to work, please make sure that the board KitProg connection is in CMSIS-DAP mode.

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
