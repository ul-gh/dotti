#include "melody.hpp"
#include "info_debug_error.h"

MelodyPlayer::MelodyPlayer(uint8_t gpio_pin, uint8_t pwm_channel,
                           uint32_t base_tempo_ms)
    : tone_timer{}
    , melody_queue{}
    , gpio_pin{gpio_pin}
    , pwm_channel{pwm_channel}
    , base_tempo_ms{base_tempo_ms}
{
    // Setup IO for tone output
    ledcSetup(pwm_channel, 0, 10);
}

MelodyPlayer::~MelodyPlayer() {
    tone_timer.detach();
    ledcDetachPin(gpio_pin);
}

void MelodyPlayer::play(Melody melody, uint8_t octave) {
    octave = octave;
    melody_queue = melody;
    is_idle = false;
    tone_timer.attach_ms(base_tempo_ms, on_tone_timer, this);
}

void MelodyPlayer::increase_tempo() {
    debug_print("Melody playing faster...");
    if (base_tempo_ms >= 128) {
        base_tempo_ms /= 2;
    }
    tone_timer.attach_ms(base_tempo_ms, on_tone_timer, this);
}

void MelodyPlayer::decrease_tempo() {
    debug_print("Melody playing slower...");
    if (base_tempo_ms < 2048) {
        base_tempo_ms *= 2;
    }
    tone_timer.attach_ms(base_tempo_ms, on_tone_timer, this);
}

////////// PlayMelody: private

void MelodyPlayer::on_tone_timer(MelodyPlayer* self) {
    static uint8_t repeat_note = 0;
    static bool pwm_activated = false;
    if (self->is_idle) {
        return;
    }
    if (repeat_note > 0) {
        repeat_note--;
        return;
    }
    note_t api_note = NOTE_C;
    NoteT note;
    bool is_control_symbol;
    bool is_pause;
    do {
        if (self->melody_queue.empty()) {
            self->is_idle = true;
            ledcWriteTone(self->pwm_channel, 0);
            ledcDetachPin(self->gpio_pin);
            return;
        }
        // Queue is not empty:
        note = self->melody_queue.front();
        self->melody_queue.pop_front();
        is_control_symbol = false;
        is_pause = false;
        switch (note) {
            case O_DOWN: self->octave--; is_control_symbol = true; break;
            case O_UP: self->octave++; is_control_symbol = true; break;
            case L1: repeat_note = 7 ; is_control_symbol = true; break;
            case L2: repeat_note = 3 ; is_control_symbol = true; break;
            case L4: repeat_note = 1; is_control_symbol = true; break;
            case L8: repeat_note = 0; is_control_symbol = true; break;
            case P: is_pause = true; break;
            case C: api_note = NOTE_C; break;
            case D: api_note = NOTE_D; break;
            case E: api_note = NOTE_E; break;
            case F: api_note = NOTE_F; break;
            case G: api_note = NOTE_G; break;
            case A: api_note = NOTE_A; break;
            case B: api_note = NOTE_B; break;
        }
    } while (is_control_symbol);
    if (is_pause) {
        // Play nothing (pause)
        ledcWriteTone(self->pwm_channel, 0);
        ledcDetachPin(self->gpio_pin);
        pwm_activated = false;
    } else {
        if (self->current_note != api_note || !pwm_activated) {
            // Play a new note
            ledcAttachPin(self->gpio_pin, self->pwm_channel);
            ledcWriteNote(self->pwm_channel, api_note, self->octave);
            pwm_activated = true;
        }
    }

}