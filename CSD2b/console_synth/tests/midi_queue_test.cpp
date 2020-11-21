

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
    auto numbers = std::vector { 1, 2, 3, 4, 5, 6, 7 };
    auto messages = std::vector<juce::MidiMessage>();
    messages.reserve (numbers.size());

    for (auto num : numbers)
        messages.push_back (juce::MidiMessage::noteOn (1, num, (uint8_t) 127));

    auto queue = MidiMessageQueue (100);

    for (auto& m : messages)
        queue.addMessage (m);

    for (auto i = 0; i < numbers.size(); ++i)
        REQUIRE (messages[i].getNoteNumber() == numbers[i]);

    auto callbackNumber = 0;
    queue.handleAllPendingMessages ([&] (auto message) {
        REQUIRE (message.getNoteNumber() == messages[callbackNumber].getNoteNumber());
        ++callbackNumber;
    });
}


TEST_CASE ("multiple pushes, multiple times")
{
    auto noteNumbers = std::vector { 1, 2, 3, 4, 5, 6, 7 };
    auto messages = std::vector<juce::MidiMessage> (noteNumbers.size());

    for (auto num : noteNumbers)
        messages.push_back (juce::MidiMessage::noteOn (1, num, (uint8_t) 127));

    auto queue = MidiMessageQueue (100);


    // add messages to queue for first time and handle them
    {
        for (auto& m : messages)
            queue.addMessage (m);

        auto callbackNumber = 0;
        queue.handleAllPendingMessages ([&] (auto message) {
            REQUIRE (messages[callbackNumber].getNoteNumber() == message.getNoteNumber());
            ++callbackNumber;
        });

        // the number of handled messages must be equal to the number of messages added to the queue
        REQUIRE (callbackNumber == messages.size());
    }

    // assert the queue is now empty
    {
        auto callbackNumber = 0;
        queue.handleAllPendingMessages ([&callbackNumber] (auto m) {
            ++callbackNumber;
        });
        REQUIRE (callbackNumber == 0);
    }

    // make new pending messages and handle them too
    {
        for (auto& m : messages)
            queue.addMessage (m);

        auto callbackNumber = 0;
        queue.handleAllPendingMessages ([&] (auto message) {
            REQUIRE (messages[callbackNumber].getNoteNumber() == message.getNoteNumber());
            ++callbackNumber;
        });

        // the number of handled messages must be equal to the number of messages added to the queue
        REQUIRE (callbackNumber == messages.size());
    }
}