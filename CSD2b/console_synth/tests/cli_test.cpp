
#include <catch2/catch_all.hpp>
#include <console_synth/console_interface/console_interface.h>


TEST_CASE ("change tempo command")
{
    auto handler = ChangeTempoCommandHandler();

    CHECK (handler.canHandleCommand ("tempo 100"));
    CHECK (! handler.canHandleCommand ("temp 50"));
    CHECK (! handler.canHandleCommand (" tempo 50"));
}
