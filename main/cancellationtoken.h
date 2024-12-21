#ifndef _H_CANCELLATION_TOKEN_H_
#define _H_CANCELLATION_TOKEN_H_

#include "helper/system.h"

class CancellationToken
{
public:
    CancellationToken(bool canceled);
    ~CancellationToken();

    bool is_cancellation_requested();
private:
    bool _is_cancellation_requested = false;
    uint64_t _lifetime;
    int _milliseconds_delay = -1;

    friend class CancellationTokenSource;
};

#endif