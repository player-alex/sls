#ifndef _H_FINGERPRINT_READER_HELPER_H_
#define _H_FINGERPRINT_READER_HELPER_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

#include "cancellationtoken.h"
#include "reader.h"

class FingerprintReaderHelper
{
public:
    const static uint32_t DEFAULT_TASK_STACK_SIZE = 3072;
    const static TickType_t DEFAULT_TASK_TIMEOUT = 20000;
    
    const static EventBits_t EVENT_BITS_ENROLLMENT_RESERVED = 0x50;
    const static EventBits_t EVENT_BITS_ENROLLMENT_FAILED = 0x51;
    const static EventBits_t EVENT_BITS_ENROLLED  = 0xA0;
    const static EventBits_t EVENT_BITS_ENROLLING = 0xA1;
    const static EventBits_t EVENT_BITS_SEARCHED = 0xA2;

    enum class TaskType
    {
        GetImage,
        Enroll,
        Search,
    };

    const unordered_map<TaskType, const char*> TASK_NAMES = {
        { TaskType::GetImage, "fprh_get_img" },
        { TaskType::Enroll, "fprh_enr" },
        { TaskType::Search, "fprh_search" },
    };

    CancellationToken* cancellation_token = nullptr;

    FingerprintReaderHelper(FingerprintReader* reader = nullptr);
    ~FingerprintReaderHelper();

    EventBits_t get_last_enrollment_status() const { return _last_enrollment_status; }

    FingerprintReader* get_reader () const { return _reader; }
    void set_reader(FingerprintReader* reader) { _reader = reader; }

    void get_image();
    void enroll(uint16_t id);
    pair<uint16_t, uint16_t> search(bool fast_search, uint8_t slot = 1);

private:
    FingerprintReader* _reader;
    void _get_image(bool wait_to_removed = false, bool* owner_task_running = nullptr);
    
    EventBits_t _last_enrollment_status = EVENT_BITS_ENROLLMENT_RESERVED;
};

#endif