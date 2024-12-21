#ifndef _H_AZURE_NETWORK_HELPER_H_
#define _H_AZURE_NETWORK_HELPER_H_

#define AZURE_IOT_RETRY_MAX_ATTEMPTS                    ( 5U )
#define AZURE_IOT_RETRY_MAX_BACKOFF_DELAY_MS            ( 5000U )
#define AZURE_IOT_RETRY_BACKOFF_BASE_MS                 ( 500U )

#define AZURE_IOT_TRANSPORT_SEND_RECV_TIMEOUT_MS        ( 2000U )
#define MQTT_MESSAGE_BUF_SIZE                           ( 5 * 1024U )

uint8_t* get_shared_mqtt_msg_buf(uint32_t* buf_size);

uint32_t ConnectToServerWithBackoffRetries( const char * pcHostName,
                                                      uint32_t port,
                                                      NetworkCredentials_t * pxNetworkCredentials,
                                                      NetworkContext_t * pxNetworkContext );

uint8_t SetupNetworkCredentials( NetworkCredentials_t* pxNetworkCredentials );

#endif