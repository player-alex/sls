#include <unordered_map>

#include "audio_data_beep.h"
#include "audio_data_siren.h"
#include "audio_data_opened.h"
#include "audio_data_closed.h"
#include "audio_data_enrolled.h"
#include "audio_data_enrollment_failed.h"
#include "audio_data_repeat_again.h"
#include "metadata.h"

using namespace std;

static const unordered_map<AudioName, const void*> AUDIO_DATAS = {
    { AudioName::Opened, AUDIO_DATA_OPENED },
    { AudioName::Closed, AUDIO_DATA_CLOSED },
    { AudioName::Enrolled, AUDIO_DATA_ENROLLED },
    { AudioName::EnrollmentFailed, AUDIO_DATA_ENROLLMENT_FAILED },
    { AudioName::RepeatAgain, AUDIO_DATA_REPEAT_AGAIN },
    { AudioName::Beep, AUDIO_DATA_BEEP },
    { AudioName::Siren, AUDIO_DATA_SIREN },
};

static const unordered_map<AudioName, size_t> AUDIO_LENS = {
    { AudioName::Opened, AUDIO_DATA_OPENED_LEN },
    { AudioName::Closed, AUDIO_DATA_CLOSED_LEN },
    { AudioName::Enrolled, AUDIO_DATA_ENROLLED_LEN  },
    { AudioName::EnrollmentFailed, AUDIO_DATA_ENROLLMENT_FAILED_LEN },
    { AudioName::RepeatAgain, AUDIO_DATA_REPEAT_AGAIN_LEN  },
    { AudioName::Beep, AUDIO_DATA_BEEP_LEN  },
    { AudioName::Siren, AUDIO_DATA_SIREN_LEN  },
};

static const unordered_map<AudioName, size_t> AUDIO_BIT_DEPTHS = {
    { AudioName::Opened, AUDIO_DATA_OPENED_BIT_DEPTH },
    { AudioName::Closed, AUDIO_DATA_CLOSED_BIT_DEPTH },
    { AudioName::Enrolled, AUDIO_DATA_ENROLLED_BIT_DEPTH },  
    { AudioName::EnrollmentFailed, AUDIO_DATA_ENROLLMENT_FAILED_BIT_DEPTH },
    { AudioName::RepeatAgain, AUDIO_DATA_REPEAT_AGAIN_BIT_DEPTH },  
    { AudioName::Beep, AUDIO_DATA_BEEP_BIT_DEPTH },  
    { AudioName::Siren, AUDIO_DATA_SIREN_BIT_DEPTH },  
};

static const unordered_map<AudioName, size_t> AUDIO_SAMPLE_RATES = {
    { AudioName::Opened, AUDIO_DATA_OPENED_SAMPLE_RATE },
    { AudioName::Closed, AUDIO_DATA_CLOSED_SAMPLE_RATE },
    { AudioName::Enrolled, AUDIO_DATA_ENROLLED_SAMPLE_RATE },  
    { AudioName::EnrollmentFailed, AUDIO_DATA_ENROLLMENT_FAILED_SAMPLE_RATE },
    { AudioName::RepeatAgain, AUDIO_DATA_REPEAT_AGAIN_SAMPLE_RATE },  
    { AudioName::Beep, AUDIO_DATA_BEEP_SAMPLE_RATE },  
    { AudioName::Siren, AUDIO_DATA_SIREN_SAMPLE_RATE },  
};

const void* get_audio_data(AudioName name)
{
    return AUDIO_DATAS.at(name);
}

size_t get_audio_len(AudioName name)
{
    return AUDIO_LENS.at(name);
}

size_t get_audio_bit_depth(AudioName name)
{
    return AUDIO_BIT_DEPTHS.at(name);
}

size_t get_audio_sample_rate(AudioName name)
{
    return AUDIO_SAMPLE_RATES.at(name);
}