
// Written by Wouter Ensink

#include <console_synth/console_interface/console_interface.h>
#include <console_synth/engine.h>
#include <console_synth/utility/scoped_message_thread_enabler.h>

int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;

    auto engine = Engine {};
    auto consoleInterface = ConsoleInterface { engine };

    consoleInterface.run();

    return 0;
}
