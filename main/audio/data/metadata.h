#ifndef _H_AUDIO_DATA_METADATA_H_
#define _H_AUDIO_DATA_METADATA_H_

enum class AudioName
{
    Opened, // 열렸습니다
    Closed, // 닫혔습니다
    Enrolled, // 동록되었습니다
    EnrollmentFailed, // 등록에 실패했습니다
    RepeatAgain, // 다시 입력해주세요
    Beep,
    Siren,
};

const void* get_audio_data(AudioName name);
size_t get_audio_len(AudioName name);
size_t get_audio_bit_depth(AudioName name);
size_t get_audio_sample_rate(AudioName name);

#endif