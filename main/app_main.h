#ifndef _H_SLS_MAIN_H_
#define _H_SLS_MAIN_H_

enum class DoorStatus
{
    None,
    Closed,
    Opened,
};

enum class SystemStatus
{
    None,
    EmergencyMode,
    PasswordChangeMode,
    PasswordChanged,
    FingerprintEnrollmentMode,
    RequestFingerprintEnrollmentMode,
};

enum class TelemetryMessageStatus : uint16_t
{
    Opened = 1,
    Closed,
    PasswordMismatch,
    FingerprintMismatch,
    LockdownCausePasswordMismatch,
    LockdownCauseFingerprintMismatch,
    PasswordChanged,
    StartFingerprintEnrollment,
    FingerprintEnrolled,
    FingerprintEnrollmentFailed,
    NotEnoughBattery,
    SystemBooted,
};

#define DESC_MAX_LEN    ( 32U )

typedef struct TelemetryPayload_s
{
    TelemetryMessageStatus status;
    char desc[DESC_MAX_LEN] = { 0 };
} TelemetryPayload_t;

static void read_password();
static void write_password();

#endif