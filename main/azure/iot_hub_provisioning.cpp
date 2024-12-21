#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

extern "C"
{
#include <azure_iot_hub_client.h>
#include <azure_iot_provisioning_client.h>
#include <azure_sample_crypto.h>

#include <backoff_algorithm.h>
#include <transport_abstraction.h>
#include <transport_tls_socket.h>
}

#include "config.h"
#include "defs.h"
#include "network_helper.h"
#include "helper/system.h"
#include "dev_provisioning.h"
#include "iot_hub_action.h"
#include "iot_hub_provisioning.h"

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    void * pParams;
};

static const char* TAG = "AzureIotHubProvisioning";

static EventGroupHandle_t status_event_handle = nullptr;
static TaskHandle_t azure_iot_hub_prv_task_handle = nullptr;
static AzureIoTResult error_code = eAzureIoTSuccess;

static NetworkContext_t network_context = { 0 };
static NetworkCredentials_t network_credentials = { 0 };
static TlsTransportParams_t tls_transport_params = { 0 };

static AzureIoTHubClient_t azure_iot_hub_client;
static AzureIoTTransportInterface_t transport_interface;
static bool session_present = false;

bool is_iot_hub_provisioned()
{
    return status_event_handle && xEventGroupGetBits(status_event_handle) & EVENT_BITS_IHP_SUCCESS;
}

AzureIoTHubClient_t* get_iot_hub_client()
{
    return &azure_iot_hub_client;
}

static void task_provision_iot_hub(void* _NO_USED_)
{
    if (!is_dev_provisioned())
    {
        // TODO;
        vTaskDelete(NULL);
        return;
    }

    AzureIoTHubClientOptions_t hub_options = { 0 };

    uint32_t mqtt_msg_buf_size = 0;

    const char* iot_hub_hostname = get_iot_hub_hostname();
    const char* iot_hub_device_id = get_iot_hub_dev_id();

    /* Set the pParams member of the network context with desired transport. */
    network_context.pParams = &tls_transport_params;

    /* Initialize the network credentials. */
    SetupNetworkCredentials(&network_credentials);

    CHECK_ERROR_AND_DEL_TASK(
        ConnectToServerWithBackoffRetries( iot_hub_hostname, 
                                                AZURE_IOT_HUB_ENDPOINT_PORT, 
                                                &network_credentials, 
                                                &network_context) != 0,
        "Failed to connect to the server");
    ESP_LOGI(TAG, "Connected to Azure IoT Hub EndPoint: %s:%d", iot_hub_hostname, AZURE_IOT_HUB_ENDPOINT_PORT);

    /* Fill in Transport Interface send and receive function pointers. */
    transport_interface.pxNetworkContext = &network_context;
    transport_interface.xSend = TLS_Socket_Send;
    transport_interface.xRecv = TLS_Socket_Recv;

    /* Initialize Azure IoT Middleware.  */
    AZURE_CHECK_ERROR_AND_DEL_TASK(AzureIoT_Init(), "Failed to initialize Azure IoT Middleware");
    ESP_LOGI(TAG, "Azure IoT Middleware initialized");

        /* Init IoT Hub option */
    error_code = AzureIoTHubClient_OptionsInit( &hub_options );
    configASSERT( error_code == eAzureIoTSuccess );

    hub_options.pucModuleID = (const uint8_t*)AZURE_IOT_HUB_MODULE_ID;
    hub_options.ulModuleIDLength = strlen(AZURE_IOT_HUB_MODULE_ID);

    hub_options.pucModelID = (const uint8_t*)AZURE_IOT_HUB_MODEL_ID;
    hub_options.ulModelIDLength = strlen(AZURE_IOT_HUB_MODEL_ID);

    hub_options.xTelemetryCallback = iot_hub_tel_callback;
    
    error_code = AzureIoTHubClient_Init(  &azure_iot_hub_client,
                                            (const uint8_t*)iot_hub_hostname, strlen(iot_hub_hostname),
                                            (const uint8_t*)iot_hub_device_id, strlen(iot_hub_device_id),
                                            &hub_options,
                                            get_shared_mqtt_msg_buf(&mqtt_msg_buf_size), mqtt_msg_buf_size,
                                            get_time,
                                            &transport_interface );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to initialize iot hub client");
    ESP_LOGI(TAG, "Azure IoT Hub Client initialized");
    
    error_code = AzureIoTHubClient_SetSymmetricKey(   &azure_iot_hub_client,
                                                        (const uint8_t*)AZURE_IOT_DPS_SYMMETRIC_KEY,
                                                        strlen(AZURE_IOT_DPS_SYMMETRIC_KEY),
                                                        Crypto_HMAC );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to set symmetric key");
    configASSERT( error_code == eAzureIoTSuccess );

    error_code = AzureIoTHubClient_Connect(   &azure_iot_hub_client, 
                                                false, &session_present, 
                                                AZURE_IOT_HUB_CONNACK_RECV_TIMEOUT_MS   );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to connect to azure iot hub");
    ESP_LOGI(TAG, "Connected to Azure IoT Hub EndPoint");

    xEventGroupSetBits(status_event_handle, EVENT_BITS_IHP_SUCCESS);
    vTaskDelete(NULL);
}

void reconnect_iot_hub()
{
    error_code = AzureIoTHubClient_Connect(   &azure_iot_hub_client, 
                                                false, &session_present, 
                                                AZURE_IOT_HUB_CONNACK_RECV_TIMEOUT_MS   );
    AZURE_CHECK_ERROR_AND_RETN(error_code, "Failed to connect to azure iot hub");
    ESP_LOGI(TAG, "Connected to Azure IoT Hub EndPoint");
}

void exec_iot_hub_provisioning()
{
    if (!status_event_handle)
        status_event_handle = xEventGroupCreate();

    if (azure_iot_hub_prv_task_handle == nullptr || eTaskGetState(azure_iot_hub_prv_task_handle) == eDeleted)
        xTaskCreate(task_provision_iot_hub, "azure-prv-dev", FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, &azure_iot_hub_prv_task_handle);
}