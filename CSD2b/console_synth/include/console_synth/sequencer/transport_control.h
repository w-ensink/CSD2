
// Written by Wouter Ensink

#pragma once

#include <vector>
#include <algorithm>

enum struct PlayState
{
    playing,
    stopped,
    recording,
    exporting
};


struct TransportControl
{
    struct Listener
    {
        virtual void playbackStarted() {}
        virtual void playbackStopped() {}
    };

    void addListener (Listener* listener)
    {
        listeners.push_back (listener);
    }

    bool removeListener (Listener* listenerToRemove)
    {
        return std::remove_if (listeners.begin(),
                               listeners.end(),
                               [listenerToRemove] (auto* l) { return l == listenerToRemove; })
               != listeners.end();
    }

    void startPlayback()
    {
        playState = PlayState::playing;
        std::for_each (listeners.begin(), listeners.end(), [] (auto* l) { l->playbackStarted(); });
    }

    void stopPlayback()
    {
        playState = PlayState::stopped;
        std::for_each (listeners.begin(), listeners.end(), [] (auto* l) { l->playbackStopped(); });
    }

    [[nodiscard]] bool isPlaying() const noexcept { return playState == PlayState::playing; }
    [[nodiscard]] bool isStopped() const noexcept { return playState == PlayState::stopped; }
    [[nodiscard]] bool isRecording() const noexcept { return playState == PlayState::recording; }
    [[nodiscard]] bool isExporting() const noexcept { return playState == PlayState::exporting; }

    [[nodiscard]] PlayState getPlayState() const noexcept { return playState; }

private:
    PlayState playState = PlayState::stopped;
    std::vector<Listener*> listeners;
};