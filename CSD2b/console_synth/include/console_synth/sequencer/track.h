
#pragma once

#include <console_synth/sequencer/event.h>


class Track
{
public:
    explicit Track (std::unique_ptr<Melody> mel) : melody { std::move (mel) } {}

    Track() : melody { std::make_unique<Melody>() } {}

    Track (const Track& other)
    {
        melody = std::make_unique<Melody> (*other.melody);
    }

    Track& operator= (const Track& other)
    {
        melody = std::make_unique<Melody> (*other.melody);
        return *this;
    }

    [[nodiscard]] Melody& getMelody() const { return *melody; }

    std::unique_ptr<Melody> melody;
};