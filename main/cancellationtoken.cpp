#include "helper/system.h"
#include "cancellationtoken.h"

CancellationToken::CancellationToken(bool canceled)
{
    _is_cancellation_requested = canceled;
    _lifetime = get_time();
}

CancellationToken::~CancellationToken() { }

bool CancellationToken::is_cancellation_requested()
{
    if (_milliseconds_delay >= 0)
        return get_time() >= _lifetime + _milliseconds_delay;

    return _is_cancellation_requested;
}