# Memfault for Cypress CY8CKIT-064S0S2-4343W kit with Amazon-FreeRTOS

This folder contains an integration example for the Cypress [CY8CKIT-064S0S2-4343W](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit) kit with Amazon-FreeRTOS & AWS IoT/Lambda. 

It is an app with a simple console interface that allows you to generate a fault, heartbeat or trace and once data is available a secure connection is estabilished with the AWS IoT MQTT broker, all available chunks are published, then passed from AWS IoT to Lambda using an IoT rule and eventually posted to Memfault. 

The demo app is tested on a [CY8CKIT-064S0S2-4343W](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit) evaluation board with ModusToolbox 2.3.

## Getting Started

- [awscli](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html) is required to perform some of the steps in this document.
- ModusToolbox setup instructions can be found [here](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_cypress_psoc64.html).
- You'll need an AWS account with an IAM user. How to setup an IAM user can be found [here](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_create.html).
- The board, IAM user and AWS IoT should be configured according to this [document](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html).
- Import the application into your ModusToolbox workspace from the `demo/mqtt_memfault_project` directory.

## Provisioning, certificates, keys & policies

In order to be able to do anything with your device it should be provisioned for secure boot as in the [document](https://community.cypress.com/docs/DOC-20043), with one difference - use `policy_multi_CM0_CM4_jitp.json`, as this is the one actually used by the project to generate a firmware image. If you'll specify the default tfm policy, the board won't be able to boot.

In `demos/include/aws_clientcredential.h`, you'll have to type in:
- `clientcredentialMQTT_BROKER_ENDPOINT` - MQTT broker endpoint retrieved from `awscli` in the [first steps in FreeRTOS document](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html).
- `clientcredentialIOT_THING_NAME` - AWS IoT Thing name.
- `clientcredentialWIFI_SSID` - Wifi SSID string.
- `clientcredentialWIFI_PASSWORD` - Wifi password.
- `clientcredentialWIFI_SECURITY` - Wifi security.

You'll also have to generate the header file with your AWS IoT Thing certificate & private key. To do that, you'll have to pass the files generated during Thing register process to `tools/certificate_configuration/CertificateConfigurator.html` and replace `demos/include/aws_clientcredential_keys.h` with the resulting file.

## Setting up AWS IoT & Lambda

In order to be able to use Lambda with IoT, the user has to [have permission to create roles and attach role policies](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_create_for-service.html). On top of that, we want two more actions to be available for the IAM user: `lambda:AddPermission` and `iam:CreatePolicy`. You can do it either using `awscli` or the IAM Management Console GUI - either way you'll have to attach the appropriate permissions policy to the IAM entity you're using. Next, we need to setup an [IoT trust role and policy](https://docs.aws.amazon.com/iot/latest/developerguide/iot-create-role.html). 

Once that's done, we'll be able to [setup a rule](https://docs.aws.amazon.com/iot/latest/developerguide/iot-repub-rule.html) that passes messages from our topic `device/+/data` to a lambda function. Follow the instructions in the link, but keep in mind our rule will have two differences from the one above:
1) Rule query statement - `SELECT encode(*, 'base64') AS chunk, topic(2) AS device_id FROM 'device/+/data'`
2) Action - `Send a message to a Lambda function`

To setup Lambda, select `Create a new lambda function` -> `Author from scratch`. Type in a name, e.g. `forwardChunk` and  use `Python 3.8` as the runtime. Once the function is created, invoke `aws lambda add-permission --function-name forwardChunk --statement-id iot-events --action "lambda:InvokeFunction" --principal iot.amazonaws.com` to give appropriate permissions to IoT.

Once that's out of the way, you'll find the demo function at `aws/forwardChunk.py` which you have to paste into the editor. Make sure to replace `<INSERT KEY HERE>` with your Memfault Project Key, which you can find in your [Memfault dashboard](https://app.memfault.com), before using it.

## Setting up the Memfault port

Add contents of the snippet below to `mqtt_memfault_project/Makefile`:
```
MEMFAULT_PORT_ROOT := <PORT ROOT>
MEMFAULT_SDK_ROOT := <MEMFAULT SDK ROOT>

MEMFAULT_COMPONENTS := core util panics metrics
include $(MEMFAULT_SDK_ROOT)/makefiles/MemfaultWorker.mk
```

Where: 
- `<PORT ROOT>` should be replaced with the path for `ports/cypress/CY8CKIT-064S0S2-4343W`, which is the directory containing the board-specific port files in the `memfault-firmware-sdk` root directory.
- `<MEMFAULT SDK ROOT>` should be replaced with the path to `memfault-firmware-sdk` root directory.

Update `SOURCES` and `INCLUDES` variables in `mqtt_memfault_project/Makefile` with:
```
SOURCES = \
	$(MEMFAULT_COMPONENTS_SRCS) \
	$(MEMFAULT_PORT_ROOT)/memfault_platform_storage.c \
	$(MEMFAULT_PORT_ROOT)/memfault_platform_port.c \
	$(MEMFAULT_PORT_ROOT)/memfault_test.c \
	$(MEMFAULT_SDK_ROOT)/ports/freertos/src/memfault_metrics_freertos.c \
	$(MEMFAULT_SDK_ROOT)/ports/freertos/src/memfault_core_freertos.c \
	$(MEMFAULT_SDK_ROOT)/ports/freertos/src/memfault_freertos_ram_regions.c \
	$(MEMFAULT_SDK_ROOT)/ports/freertos/src/memfault_panics_freertos.c

INCLUDES = \
	$(CY_AFR_ROOT)/vendors/cypress/MTB/libraries/wifi-host-driver/WiFi_Host_Driver/resources/nvram/TARGET_CY8CKIT_064B0S2_4343W \
	$(MEMFAULT_COMPONENTS_INC_FOLDERS) \
	$(MEMFAULT_SDK_ROOT)/ports/include \
	$(MEMFAULT_PORT_ROOT)
```

Port stores coredumps, which include stack, data & bss sections, in flash memory between `__MfltCoredumpsStart` and `__MfltCoredumpsEnd` symbols. Demo implementation provides a 512-byte aligned 128kB region at the end of CM4 flash memory region, just behind the CM4 application signature.

To uniquely identify your firmware type, version & device in your Memfault dashboard, please also make sure to edit `memfault_platform_port.c` and fill all the related fields in `memfault_platform_get_device_info`.

## Building, flashing & debugging

Build the application using `Build aws_demos Application`, or do it directly from shell in the project directory by using `make build`. 

Analogously to flash the device either use `aws_demos Program (KitProg3)` from ModusToolbox, or `make program` from shell. In order for it to work, please make sure that the KitProg connection is in CMSIS-DAP mode.

You can use the gdbserver/client integrated into ModusToolbox to debug the application by using `aws_demos Debug (KitProg3)` or `make debug` from shell.

## Application details

Main function is in `vendors/cypress/boards/CY8CKIT_064S0S2_4343W/aws_demos/application_code/main.c`. MQTT communication for the application is implemented in `mqtt_memfault_project/demo/memfault/mqtt_demo_memfault.c`. 

Demo overview:
- First the user is asked whether a fault, trace or heartbeat event should be generated or the application should continue.
- TCP connection to AWS IoT is estabilished.
- `CONNECT` to MQTT broker is sent.
- All data chunks if available are retrieved and published over MQTT on the `device/+/data` topic.
- AWS IoT retrieves the chunks and passes them to the Lambda function.
- Lambda function forwards the chunks to Memfault.

## Serial console via UART

There should be two `/dev/ttyACM` devices available when the board is connected using the KitProg USB - one of them is the console. Connect to it using e.g. `screen`. Serial parameters are:
```
Baud rate: 115200
Data: 8 bit
Parity: None
Stop bits: 1
Flow control: None
```

## Troubleshooting

### TLS error

If you get a TLS error when trying to estabilish a connection to the IoT MQTT Broker saying that your certificate/key is incorrect, you might need to use a different IoT endpoint - try replacing the default address with: `aws iot describe-endpoint --endpoint-type iot:Data-ATS`.

## Related documents

- [PSoC® 64 Secure MCU Secure Boot SDK User Guide](https://www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide)
- [Provisioning Guide for the Cypress CY8CKIT-064S0S2-4343W Kit](https://community.cypress.com/t5/Resource-Library/Provisioning-Guide-for-the-Cypress-CY8CKIT-064S0S2-4343W-Kit/ta-p/252469)
- [PSoC 64 Standard Secure – AWS Wi-Fi BT Pioneer Kit Guide](https://www.cypress.com/file/509676/download)
