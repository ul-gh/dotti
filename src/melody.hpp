#ifndef MELODY_PLAYER_HPP__
#define MELODY_PLAYER_HPP__

#include <deque>
#include <esp32-hal-ledc.h>
#include <Ticker.h>


// A single musical note [C, D, E, F, G, A, B],
// plus pause sympol [P],
// plus control symbols for octave shifting up and down [O_UP, O_DOWN],
// plus control prefixes for the notes making them a
// full, half, quarter or eiths note or pause [X1, X2, X4, X8]
using NoteT = enum {C, D, E, F, G, A, B, P, O_UP, O_DOWN, L1, L2, L4, L8};

// Musical melody comprised of notes
using Melody = std::deque<NoteT>;

class MelodyPlayer {
public:
    Ticker tone_timer;

    // base_tempo: Duration of an eighths note in milliseconds
    MelodyPlayer(uint8_t gpio_pin, uint8_t pwm_channel,
                 uint32_t base_tempo_ms=128);
    virtual ~MelodyPlayer();

    void play(Melody melody, uint8_t octave=4);

    void increase_tempo();
    void decrease_tempo();

private:
    Melody melody_queue;
    note_t current_note = NOTE_C;

    const uint8_t gpio_pin;
    const uint8_t pwm_channel;

    bool is_idle = true;
    uint32_t base_tempo_ms;
    uint8_t octave = 4;

    static void on_tone_timer(MelodyPlayer* self);
};





#endif