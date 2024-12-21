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

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    void * pParams;
};

static const char* TAG = "AzureDeviceProvisioning";

static EventGroupHandle_t status_event_handle = nullptr;
static TaskHandle_t azure_dev_prv_task_handle = nullptr;
static AzureIoTResult error_code = eAzureIoTSuccess;
static uint8_t iot_hub_hostname[AZURE_IOT_HUB_HOSTNAME_BUF_MAX_LEN] = { 0 };
static uint8_t iot_hub_dev_id[AZURE_IOT_HUB_DEV_ID_BUF_MAX_LEN] = { 0 };

bool is_dev_provisioned()
{
    return status_event_handle && xEventGroupGetBits(status_event_handle) & EVENT_BITS_DPS_SUCCESS;
}

const char* get_iot_hub_hostname()
{
    return reinterpret_cast<const char*>(iot_hub_hostname);
}

const char* get_iot_hub_dev_id()
{
    return reinterpret_cast<const char*>(iot_hub_dev_id);
}

static void task_provision_dev(void* _NO_USED_)
{
    NetworkCredentials_t network_credentials = { 0 };
    NetworkContext_t network_context = { 0 };
    TlsTransportParams_t tls_transport_params = { 0 };

    AzureIoTTransportInterface_t transport_interface;
    AzureIoTProvisioningClient_t azure_iot_provisioning_client;

    uint32_t mqtt_msg_buf_size = 0;
    uint32_t received_iot_hub_hostname_len = AZURE_IOT_HUB_HOSTNAME_BUF_MAX_LEN;
    uint32_t received_iot_hub_dev_id_len = AZURE_IOT_HUB_DEV_ID_BUF_MAX_LEN;

    /* Set the pParams member of the network context with desired transport. */
    network_context.pParams = &tls_transport_params;

    /* Initialize the network credentials. */
    SetupNetworkCredentials(&network_credentials);
    
    CHECK_ERROR_AND_DEL_TASK(
        ConnectToServerWithBackoffRetries( AZURE_IOT_DPS_ENDPOINT_HOSTNAME, 
                                                AZURE_IOT_DPS_ENDPOINT_PORT, 
                                                &network_credentials, 
                                                &network_context) != 0,
        "Failed to connect to the server");
    ESP_LOGI(TAG, "Connected to Azure IoT Provisioning EndPoint: %s:%d", AZURE_IOT_DPS_ENDPOINT_HOSTNAME, AZURE_IOT_DPS_ENDPOINT_PORT);

    /* Fill in Transport Interface send and receive function pointers. */
    transport_interface.pxNetworkContext = &network_context;
    transport_interface.xSend = TLS_Socket_Send;
    transport_interface.xRecv = TLS_Socket_Recv;

    /* Initialize Azure IoT Middleware.  */
    AZURE_CHECK_ERROR_AND_DEL_TASK(AzureIoT_Init(), "Failed to initialize Azure IoT Middleware");
    ESP_LOGI(TAG, "Azure IoT Middleware initialized");

    error_code = AzureIoTProvisioningClient_Init( &azure_iot_provisioning_client,
                                                    (const uint8_t*)AZURE_IOT_DPS_ENDPOINT_HOSTNAME,
                                                    strlen(AZURE_IOT_DPS_ENDPOINT_HOSTNAME),
                                                    (const uint8_t*)AZURE_IOT_DPS_ID_SCOPE,
                                                    strlen(AZURE_IOT_DPS_ID_SCOPE),
                                                    (const uint8_t*)AZURE_IOT_DPS_REG_ID,
                                                    strlen(AZURE_IOT_DPS_REG_ID),
                                                    NULL,
                                                    get_shared_mqtt_msg_buf(&mqtt_msg_buf_size), mqtt_msg_buf_size,
                                                    get_time,
                                                    &transport_interface );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to Azure IoT Provisioning Client");
    ESP_LOGI(TAG, "Azure IoT Provisioning Client initialized");

    error_code = AzureIoTProvisioningClient_SetSymmetricKey(  &azure_iot_provisioning_client, 
                                                                (const uint8_t*)AZURE_IOT_DPS_SYMMETRIC_KEY, strlen(AZURE_IOT_DPS_SYMMETRIC_KEY),
                                                                Crypto_HMAC );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to set symmetric key");

    error_code = AzureIoTProvisioningClient_SetRegistrationPayload(   &azure_iot_provisioning_client,
                                                                        (const uint8_t*)AZURE_IOT_DPS_MODEL_ID_PAYLOAD,
                                                                        strlen(AZURE_IOT_DPS_MODEL_ID_PAYLOAD)  );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to Azure IoT Provisioning Client set registration payload");
    ESP_LOGI(TAG, "Azure IoT Provisioning Client set registration payload");

    do
    {
        error_code = AzureIoTProvisioningClient_Register( &azure_iot_provisioning_client, 
                                                            AZURE_IOT_DPS_REG_TIMEOUT_MS );
    } while(error_code == eAzureIoTErrorPending );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to register Azure IoT Provisioning Client");
    ESP_LOGI(TAG, "Azure IoT Provisioning Client registered");

    error_code = AzureIoTProvisioningClient_GetDeviceAndHub(  &azure_iot_provisioning_client, 
                                                                iot_hub_hostname, &received_iot_hub_hostname_len,
                                                                iot_hub_dev_id, &received_iot_hub_dev_id_len );
    AZURE_CHECK_ERROR_AND_DEL_TASK(error_code, "Failed to get dev and hub");
    ESP_LOGI(TAG, "Azure IoT Provisioning Client got device and hub: %s:%s", iot_hub_hostname, iot_hub_dev_id);

    AzureIoTProvisioningClient_Deinit(&azure_iot_provisioning_client);
    ESP_LOGI(TAG, "Azure IoT Provisioning Client deinitialized");

    /* Deinitialize Azure IoT Middleware.  */
    AzureIoT_Deinit();
    ESP_LOGI(TAG, "Azure IoT Middleware deinitialized");

    /* Close the network connection. */
    TLS_Socket_Disconnect(&network_context);
    ESP_LOGI(TAG, "Disconnected from Azure IoT Provisioning EndPoint");

    xEventGroupSetBits(status_event_handle, EVENT_BITS_DPS_SUCCESS);
    vTaskDelete(NULL);
}

void exec_dev_provisioning()
{
    if (!status_event_handle)
        status_event_handle = xEventGroupCreate();

    if (azure_dev_prv_task_handle == nullptr || eTaskGetState(azure_dev_prv_task_handle) == eDeleted)
        xTaskCreate(task_provision_dev, "azure-prv-dev", FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, &azure_dev_prv_task_handle);
}