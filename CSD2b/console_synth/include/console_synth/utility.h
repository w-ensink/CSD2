
// Written by Wouter Ensink

#pragma once

namespace details
{
struct ScopedMessageThread
{
    ScopedMessageThread();
    ~ScopedMessageThread();
};

#define SCOPE_ENABLE_MESSAGE_THREAD \
    auto smt = ::details::ScopedMessageThread {};

}  // namespace details
