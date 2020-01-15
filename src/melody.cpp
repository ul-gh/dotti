#include "melody.hpp"
#include "info_debug_error.h"

MelodyPlayer::MelodyPlayer(uint8_t gpio_pin, uint8_t pwm_channel)
    : gpio_pin{gpio_pin}
    , pwm_channel{pwm_channel}
    , tone_timer{}
    , melody_queue{}
{
    // Setup IO for tone output
    ledcSetup(pwm_channel, 0, 10);
}

MelodyPlayer::~MelodyPlayer() {
    tone_timer.detach();
    ledcDetachPin(gpio_pin);
}

void MelodyPlayer::play(Melody melody, uint32_t tempo_ms) {
    if (tempo_ms > 0) {
        playing_at_custom_tempo = true;
    } else {
        tempo_ms = base_tempo_ms;
    }
    melody_queue = melody;
    is_idle = false;
    tone_timer.attach_ms(tempo_ms, on_tone_timer, this);
}

void MelodyPlayer::increase_tempo() {
    debug_print("Melody playing faster...");
    if (base_tempo_ms >= 64) {
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

void MelodyPlayer::set_tempo(uint32_t tempo_ms) {
    debug_print_sv("Setting tempo to:", tempo_ms);
    base_tempo_ms = tempo_ms;
    tone_timer.attach_ms(tempo_ms, on_tone_timer, this);
}

////////// PlayMelody: private

void MelodyPlayer::on_tone_timer(MelodyPlayer* self) {
    static uint8_t repeat_note = 1;
    static bool output_off = true;
    if (self->is_idle) {
        return;
    }
    if (repeat_note > 0) {
        // Keep playing the current note until repeat_note == 0
        repeat_note--;
        return;
    } else {
        // A Quarter note is an eights note repeated once, this is the default.
        repeat_note = 1;
    }
    // This is the ESP32 API enum
    note_t api_note = NOTE_C;
    // This is our custom type with additional control characters etc.
    NoteT note;
    bool is_control_symbol;
    bool is_pause;
    // This consumes the non-audible (and non-pause) control symbols
    // until an audible note or pause is found which is then played.
    do {
        if (self->melody_queue.empty()) {
            ledcWriteTone(self->pwm_channel, 0);
            ledcDetachPin(self->gpio_pin);
            output_off = true;
            self->is_idle = true;
            if (self->playing_at_custom_tempo) {
                // If tempo for the currently playint tune has been set different
                // than preset tempo, unset temporary tempo and reset timer
                // to preset tempo.
                self->playing_at_custom_tempo = false;
                self->tone_timer.attach_ms(self->base_tempo_ms, on_tone_timer, self);
            }
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
            case L1: repeat_note = 15 ; is_control_symbol = true; break;
            case L2: repeat_note = 7 ; is_control_symbol = true; break;
            case L4: repeat_note = 3; is_control_symbol = true; break;
            case L8: repeat_note = 1; is_control_symbol = true; break;
            case L16: repeat_note = 0; is_control_symbol = true; break;
            case P: is_pause = true; break;
            case C: api_note = NOTE_C; break;
            case Cs: api_note = NOTE_Cs; break;
            case D: api_note = NOTE_D; break;
            case Ds: api_note = NOTE_Eb; break;
            case E: api_note = NOTE_E; break;
            case F: api_note = NOTE_F; break;
            case Fs: api_note = NOTE_Fs; break;
            case G: api_note = NOTE_G; break;
            case Gs: api_note = NOTE_Gs; break;
            case A: api_note = NOTE_A; break;
            case As: api_note = NOTE_Bb; break;
            case B: api_note = NOTE_B; break;
        }
    } while (is_control_symbol);
    // Found a note or a pause
    if (is_pause) {
        // Play nothing
        ledcWriteTone(self->pwm_channel, 0);
        ledcDetachPin(self->gpio_pin);
        output_off = true;
    } else {
        // Play audible note.
        // If the note is unchanged from the previous one, keep playing the 
        // current note instead of restarting the PWM for cleaner waveform.
        // if (self->current_note != api_note || output_off) {
            // Play a new note
            output_off = false;
            self->current_note = api_note;
            ledcWriteTone(self->pwm_channel, 0);
            ledcDetachPin(self->gpio_pin);
            delay(10);
            ledcAttachPin(self->gpio_pin, self->pwm_channel);
            ledcWriteNote(self->pwm_channel, api_note, self->octave);
        //}
    }

}