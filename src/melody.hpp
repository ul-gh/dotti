#ifndef MELODY_PLAYER_HPP__
#define MELODY_PLAYER_HPP__

#include <deque>
#include <esp32-hal-ledc.h>
#include <Ticker.h>

// A single musical note [C, D, E, F, G, A, B],
// plus halve-tones [Cs, Ds, Fs, Gs, As],
// plus pause sympol [P],
// plus control symbols for octave shifting up and down [O_UP, O_DOWN],
// plus control prefixes for the notes making them a
// full, half, quarter or eiths note or pause [L1, L2, L4, L8, L16].
// Default tone length is 1/8.
enum NoteT {C,  D, E, F,  G,  A, B, P, O_UP, O_DOWN, L1, L2, L4, L8, L16,
            Cs, Ds,   Fs, Gs, As};

// Musical melody comprised of notes
using Melody = std::deque<NoteT>;

class MelodyPlayer {
public:
    MelodyPlayer(uint8_t gpio_pin, uint8_t pwm_channel);
    virtual ~MelodyPlayer();

    // tempo_ms: Sets tempo only for the currently playing tune.
    void play(Melody melody, uint32_t tempo_ms=0);

    void increase_tempo();
    void decrease_tempo();
    // tempo_ms: Duration of a sixteenths note in milliseconds
    void set_tempo(uint32_t tempo_ms);

private:
    const uint8_t gpio_pin;
    const uint8_t pwm_channel;

    Ticker tone_timer;

    Melody melody_queue;

    // This is the ESP32 API enum type
    note_t current_note = NOTE_C;

    bool is_idle = true;
    // base_tempo_ms: Duration of a sixteenths note in milliseconds
    uint32_t base_tempo_ms = 64;
    // Is set to true by play() if tempo is changed for a single tune
    bool playing_at_custom_tempo = false;
    uint8_t octave = 4;

    static void on_tone_timer(MelodyPlayer* self);
};





#endif