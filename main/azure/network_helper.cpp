extern "C"
{
#include <azure_iot_hub_client.h>
#include <azure_iot_provisioning_client.h>
#include <backoff_algorithm.h>
#include <transport_tls_socket.h>
#include <transport_abstraction.h>
}

#include "certs.h"
#include "config.h"
#include "network_helper.h"

#define RAND32()    ( rand() / RAND_MAX )

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    void * pParams;
};

static const char* TAG = "AzureNetworkHelper";
static uint8_t shared_mqtt_msg_buf[MQTT_MESSAGE_BUF_SIZE] = { 0 };

uint8_t* get_shared_mqtt_msg_buf(uint32_t* buf_size)
{
    *buf_size = sizeof(shared_mqtt_msg_buf);
    return shared_mqtt_msg_buf;
}

/**
* @brief Connect to server with backoff retries.
*/
uint32_t ConnectToServerWithBackoffRetries( const char * pcHostName,
                                                      uint32_t port,
                                                      NetworkCredentials_t * pxNetworkCredentials,
                                                      NetworkContext_t * pxNetworkContext )
{
    TlsTransportStatus_t xNetworkStatus;
    BackoffAlgorithmStatus_t xBackoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t xReconnectParams;
    uint16_t usNextRetryBackOff = 0U;

    /* Initialize reconnect attempts and interval. */
    BackoffAlgorithm_InitializeParams( &xReconnectParams,
                                       AZURE_IOT_RETRY_BACKOFF_BASE_MS,
                                       AZURE_IOT_RETRY_MAX_BACKOFF_DELAY_MS,
                                       AZURE_IOT_RETRY_MAX_ATTEMPTS );

    /* Attempt to connect to IoT Hub. If connection fails, retry after
     * a timeout. Timeout value will exponentially increase till maximum
     * attempts are reached.
     */
    do
    {
        LogInfo( ( "Creating a TLS connection to %s:%lu.\r\n", pcHostName, port ) );
        /* Attempt to create a mutually authenticated TLS connection. */
        xNetworkStatus = TLS_Socket_Connect( pxNetworkContext,
                                             pcHostName, port,
                                             pxNetworkCredentials,
                                             AZURE_IOT_TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                             AZURE_IOT_TRANSPORT_SEND_RECV_TIMEOUT_MS );

        if( xNetworkStatus != eTLSTransportSuccess )
        {
            /* Generate a random number and calculate backoff value (in milliseconds) for
             * the next connection retry.
             * Note: It is recommended to seed the random number generator with a device-specific
             * entropy source so that possibility of multiple devices retrying failed network operations
             * at similar intervals can be avoided. */
            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &xReconnectParams, RAND32(), &usNextRetryBackOff );

            if( xBackoffAlgStatus == BackoffAlgorithmRetriesExhausted )
            {
                LogError( ( "Connection to the IoT Hub failed, all attempts exhausted." ) );
            }
            else if( xBackoffAlgStatus == BackoffAlgorithmSuccess )
            {
                LogWarn( ( "Connection to the IoT Hub failed [%d]. "
                           "Retrying connection with backoff and jitter [%d]ms.",
                           xNetworkStatus, usNextRetryBackOff ) );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
        }
    } while( ( xNetworkStatus != eTLSTransportSuccess ) && ( xBackoffAlgStatus == BackoffAlgorithmSuccess ) );

    return xNetworkStatus == eTLSTransportSuccess ? 0 : 1;
}
/*-----------------------------------------------------------*/

/**
* @brief Setup transport credentials.
*/
uint8_t SetupNetworkCredentials( NetworkCredentials_t* pxNetworkCredentials )
{
    pxNetworkCredentials->xDisableSni = pdFALSE;

    /* Set the credentials for establishing a TLS connection. */
    pxNetworkCredentials->pucRootCa = ( const unsigned char * ) AZURE_IOT_ROOT_CA_PEM;
    pxNetworkCredentials->xRootCaSize = sizeof( AZURE_IOT_ROOT_CA_PEM );

    return 0;
}
/*-----------------------------------------------------------*/