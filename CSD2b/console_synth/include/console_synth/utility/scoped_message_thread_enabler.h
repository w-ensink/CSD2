
// Written by Wouter Ensink

#pragma once

namespace details
{
struct ScopedMessageThreadEnabler
{
    ScopedMessageThreadEnabler();
    ~ScopedMessageThreadEnabler();
};

#define SCOPE_ENABLE_MESSAGE_THREAD \
    auto smt = ::details::ScopedMessageThreadEnabler {};

}  // namespace details
