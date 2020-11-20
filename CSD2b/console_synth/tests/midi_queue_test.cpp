

#include <catch2/catch_all.hpp>
#include <midi/midi_message_queue.h>


TEST_CASE ("single midi message")
{
    auto queue = MidiMessageQueue (100);
    auto message = juce::MidiMessage::noteOn (1, 60, (uint8_t) 100);
    queue.addMessage (message);

    auto success = false;

    REQUIRE (message.getNoteNumber() == 60);

    queue.handleAllPendingMessages ([&success] (auto message) {
        if (message.getNoteNumber() == 60)
            success = true;
    });

    REQUIRE (success);
}


TEST_CASE ("multiple pushes, single go")
{
    auto numbers = { 1, 2, 3, 4, 5, 6, 7 };
    std::vector<juce::MidiMessage> messages;
    for (auto num : numbers)
        messages.push_back (juce::MidiMessage::noteOn (1, num, (uint8_t) 127));

    auto queue = MidiMessageQueue (100);

    for (auto& m : messages)
        queue.addMessage (m);


}