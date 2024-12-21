#ifndef _H_FINGERPRINT_READER_DEFS_H_
#define _H_FINGERPRINT_READER_DEFS_H_

#include <cstdint>
#include <cstring>

#define FINGERPRINT_OK                    ( 0x00 )    // Command execution is complete
#define FINGERPRINT_PACKET_RECV_ERR       ( 0x01 )    // Error when receiving data package
#define FINGERPRINT_NO_FINGER             ( 0x02 )    // No finger on the sensor
#define FINGERPRINT_IMAGE_FAIL            ( 0x03 )    // Failed to enroll the finger
#define FINGERPRINT_IMAGE_MESS            ( 0x06 )    // Failed to generate character file due to overly disorderly fingerprint image
#define FINGERPRINT_FEATURE_FAIL          ( 0x07 )    // Failed to generate character file due to the lack of character point or small fingerprint image
#define FINGERPRINT_NO_MATCH              ( 0x08 )    // Finger doesn't match
#define FINGERPRINT_NOT_FOUND             ( 0x09 )    // Failed to find matching finger
#define FINGERPRINT_ENROLL_MISMATCH       ( 0x0A )    // Failed to combine the character files
#define FINGERPRINT_BAD_LOCATION          ( 0x0B )    // Addressed PageID is beyond the finger library
#define FINGERPRINT_DB_RANGE_FAIL         ( 0x0C )    // Error when reading template from library or invalid template
#define FINGERPRINT_UPLOAD_FEATURE_FAIL   ( 0x0D )    // Error when uploading template
#define FINGERPRINT_PACKET_RESP_FAIL      ( 0x0E )    // Module failed to receive the following data packages
#define FINGERPRINT_UPLOAD_FAIL           ( 0x0F )    // Error when uploading image
#define FINGERPRINT_DELETE_FAIL           ( 0x10 )    // Failed to delete the template
#define FINGERPRINT_DBCLEAR_FAIL          ( 0x11 )    // Failed to clear finger library
#define FINGERPRINT_PASS_FAIL             ( 0x13 )    // Find whether the fingerprint passed or failed
#define FINGERPRINT_INVALID_IMAGE         ( 0x15 )    // Failed to generate image because of lack of valid primary image
#define FINGERPRINT_FLASH_ERR             ( 0x18 )    // Error when writing flash
#define FINGERPRINT_INVALID_REG           ( 0x1A )    // Invalid register number
#define FINGERPRINT_ADDR_CODE             ( 0x20 )    // Address code
#define FINGERPRINT_PASS_VERIFY           ( 0x21 )    // Verify the fingerprint passed
#define FINGERPRINT_START_CODE            ( 0xEF01 )  // Fixed value of EF01H; High byte transferred first

#define FINGERPRINT_CMD_PACKET            ( 0x1 )     // Command packet
#define FINGERPRINT_DATA_PACKET           ( 0x2 )     // Data packet, must follow command packet or acknowledge packet
#define FINGERPRINT_ACK_PACKET            ( 0x7 )     // Acknowledge packet
#define FINGERPRINT_END_DATA_PACKET       ( 0x8 )     // End of data packet

#define FINGERPRINT_TIMEOUT               ( 0xFF )    // Timeout was reached
#define FINGERPRINT_BAD_PACKET            ( 0xFE )    // Bad packet was sent

#define FINGERPRINT_GET_IMAGE             ( 0x01 )    // Collect finger image
#define FINGERPRINT_IMAGE_2TZ             ( 0x02 )    // Generate character file from image
#define FINGERPRINT_SEARCH                ( 0x04 )    // Search for fingerprint in slot
#define FINGERPRINT_REG_MODEL             ( 0x05 )    // Combine character files and generate template
#define FINGERPRINT_STORE                 ( 0x06 )    // Store template
#define FINGERPRINT_LOAD                  ( 0x07 )    // Read/load template
#define FINGERPRINT_UPLOAD                ( 0x08 )    // Upload template
#define FINGERPRINT_DELETE                ( 0x0C )    // Delete templates
#define FINGERPRINT_EMPTY                 ( 0x0D )    // Empty library
#define FINGERPRINT_READ_SYS_PARAMS       ( 0x0F )    // Read system parameters
#define FINGERPRINT_SET_PASSWORD          ( 0x12 )    // Sets passwords
#define FINGERPRINT_VERIFY_PASSWORD       ( 0x13 )    // Verifies the password
#define FINGERPRINT_HI_SPEED_SEARCH       ( 0x1B )    // Asks the sensor to search for a matching fingerprint template to the last model generated
#define FINGERPRINT_TEMPLATE_COUNT        ( 0x1D )    // Read finger template numbers
#define FINGERPRINT_SLEEP_MODE            ( 0x33 )    // Set sensor into dormancy mode
#define FINGERPRINT_AURA_LED_CONFIG       ( 0x35 )    // Aura LED control
#define FINGERPRINT_LED_STATE_ON          ( 0x50 )    // Turn on the onboard LED
#define FINGERPRINT_LED_STATE_OFF         ( 0x51 )    // Turn off the onboard LED

#define FINGERPRINT_CMD_CANCEL            ( 0x30 )    // Cancel command

#define FINGERPRINT_LED_BREATHING         ( 0x01 )    // Breathing light
#define FINGERPRINT_LED_FLASHING          ( 0x02 )    // Flashing light
#define FINGERPRINT_LED_ON                ( 0x03 )    // Always on
#define FINGERPRINT_LED_OFF               ( 0x04 )    // Always off
#define FINGERPRINT_LED_GRADUAL_ON        ( 0x05 )    // Gradually on
#define FINGERPRINT_LED_GRADUAL_OFF       ( 0x06 )    // Gradually off
#define FINGERPRINT_LED_RED               ( 0x01 )    // Red LED
#define FINGERPRINT_LED_BLUE              ( 0x02 )    // Blue LED
#define FINGERPRINT_LED_PURPLE            ( 0x03 )    // Purple LED

#define FINGERPRINT_REG_ADDR_ERROR        ( 0x1A )    // Shows register address error
#define FINGERPRINT_WRITE_REG             ( 0x0E )    // Write system register instruction

#define FINGERPRINT_BAUD_REG_ADDR         ( 0x4 )     // BAUDRATE register address
#define FINGERPRINT_BAUDRATE_9600         ( 0x1 )     // UART baud 9600
#define FINGERPRINT_BAUDRATE_19200        ( 0x2 )     // UART baud 19200
#define FINGERPRINT_BAUDRATE_28800        ( 0x3 )     // UART baud 28800
#define FINGERPRINT_BAUDRATE_38400        ( 0x4 )     // UART baud 38400
#define FINGERPRINT_BAUDRATE_48000        ( 0x5 )     // UART baud 48000
#define FINGERPRINT_BAUDRATE_57600        ( 0x6 )     // UART baud 57600
#define FINGERPRINT_BAUDRATE_67200        ( 0x7 )     // UART baud 67200
#define FINGERPRINT_BAUDRATE_76800        ( 0x8 )     // UART baud 76800
#define FINGERPRINT_BAUDRATE_86400        ( 0x9 )     // UART baud 86400
#define FINGERPRINT_BAUDRATE_96000        ( 0xA )     // UART baud 96000
#define FINGERPRINT_BAUDRATE_105600       ( 0xB )     // UART baud 105600
#define FINGERPRINT_BAUDRATE_115200       ( 0xC )     // UART baud 115200
  
#define FINGERPRINT_SECURITY_REG_ADDR     ( 0x5 )     // Security level register address
#define FINGERPRINT_SECURITY_LEVEL_1      ( 0x1 )     // Security level 1
#define FINGERPRINT_SECURITY_LEVEL_2      ( 0x2 )     // Security level 2
#define FINGERPRINT_SECURITY_LEVEL_3      ( 0x3 )     // Security level 3
#define FINGERPRINT_SECURITY_LEVEL_4      ( 0x4 )     // Security level 4
#define FINGERPRINT_SECURITY_LEVEL_5      ( 0x5 )     // Security level 5
  
#define FINGERPRINT_PACKET_REG_ADDR       ( 0x6 )     // Packet size register address
#define FINGERPRINT_PACKET_SIZE_32        ( 0x0 )     // Packet size is 32 Byte
#define FINGERPRINT_PACKET_SIZE_64        ( 0x1 )     // Packet size is 64 Byte
#define FINGERPRINT_PACKET_SIZE_128       ( 0x2 )     // Packet size is 128 Byte
#define FINGERPRINT_PACKET_SIZE_256       ( 0x3 )     // Packet size is 256 Byte

constexpr uint16_t FINGERPRINT_DEFAULT_PACKET_SIZE = 64;

#endif