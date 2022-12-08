const short SINE_WAVE = 1;
const short TRIANGLE_WAVE = 2;
const short SQUARE_WAVE = 3;
const short WHITE_NOISE = 4;
const short SAWTOOTH_WAVE = 5;
const double SAWTOOTH_DENSITY = 100.0;

const short PLAYMODE_KEYBOARD = 1;
const short PLAYMODE_MIDI = 2;
const bool PLAY_WITH_VELOCITY = true;

const int DEFAULT_SAMPLE_RATE = 44100;
const short DEFAULT_OUTPUT_MODE = 2;
const short DEFAULT_BLOCK_COUNT = 8;
const int DEFAULT_BLOCK_SAMPLE = 512;

const double ADSR_MAX_VALUE = 0.1;
const double ADSR_ATTACK_TIME = 0.2;
const double ADSR_DECAY_TIME = 0.1;
const double ADSR_SUSTAIN_LEVEL = 0.1;
const double ADSR_RELEASE_TIME = 0.1;

const double BASE_KEYBOARD_FREQUENCY = 261.63;
const double BASE_MIDI_FREQUENCY = 8.1758;
const short MAX_KEYBOARD_BUTTON_COUNT = 13;

const int LPF_TAPS = 50;
const double LPF_CUTOFF_FREQUENCY = 150.0;

const int WHITE_NOISE_MIN_FREQ = 10;
const int WHITE_NOISE_MAX_FREQ = 20000;