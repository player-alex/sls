#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "helper/system.h"
#include "cancellationtokensource.h"

CancellationTokenSource::CancellationTokenSource() { }
CancellationTokenSource::~CancellationTokenSource() { }

void CancellationTokenSource::reset()
{
    for(auto& token: _tokens)
        token->_is_cancellation_requested = false;
}

void CancellationTokenSource::cancel()
{
    for(auto& token: _tokens)
        token->_is_cancellation_requested = true;
}

void CancellationTokenSource::cancel_after(int millisecondsDelay)
{
    for(auto& token: _tokens)
    {
        token->_milliseconds_delay = millisecondsDelay;
        token->_lifetime = get_time();
    }
}

void CancellationTokenSource::link_token(CancellationToken* token)
{
    _tokens.push_back(token);
}

CancellationToken* CancellationTokenSource::create_linked_token()
{
    _tokens.push_back(new CancellationToken(false));
    return _tokens.back();
}