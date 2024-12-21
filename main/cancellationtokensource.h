#ifndef _H_CANCELLATION_TOKEN_SOURCE_H_
#define _H_CANCELLATION_TOKEN_SOURCE_H_

#include <vector>
#include <cancellationtoken.h>

using namespace std;

class CancellationTokenSource
{
public:
    CancellationTokenSource();
    ~CancellationTokenSource();

    void reset();
    void cancel();
    void cancel_after(int millisecondsDelay);
    void link_token(CancellationToken* token);
    CancellationToken* create_linked_token();
private:
    vector<CancellationToken*> _tokens;
};

#endif