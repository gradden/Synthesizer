#pragma once
#define _USE_MATH_DEFINES

#include<math.h>
#include <stdlib.h>

class Oscillator {
private:
	double amplitude;
	double frequency;
	int osc_type;
	bool isEnveloping;

	double onTime;
	double offTime;

	double maxLevel;
	double attackTime;
	double decayTime;
	double sustainLevel;
	double releaseTime;

	bool onKey;

	double SineWave(double frequency, double time) {
		return sin(frequency * 2.0 * M_PI * time);
	}

	double TriangleWave(double frequency, double time) {
		return (asin(SineWave(frequency, time)));
	}

	double SquareWave(double frequency, double time) {
		return SineWave(frequency, time) > 0.0 ? 1.0 : 0.0;
	}

	double PinkNoise(double time) {
		return SineWave((double)(rand() % 10000 + 100), time);
	}

	double SawtoothWave(double frequency, double time) {
		double mix = 0.0;

		for (double i = 1.0; i < 100.0; i++) {
			mix += (sin(i * 2.0 * M_PI * frequency * time)) / i;
		}

		return mix;
	}

public:
	Oscillator() {
		this->amplitude = 0.3;
		this->frequency = 440.0;
		this->osc_type = 1;
		this->isEnveloping = true;

		this->maxLevel = 0.3;
		this->attackTime = 0.1;
		this->decayTime = 0.0;
		this->sustainLevel = 0.3;
		this->releaseTime = 0.4;
		this->onTime = 0.0;
		this->offTime = 0.0;
		this->onKey = false;
	}

	double getEnvelope(double timeNow);
	void setEnvelope(bool isEnveloping, double maxLevel, double A_time, double D_time, double S_level, double R_time);
	bool isEnvelopeEnabled();

	void setAmplitude(double amp);
	double getAmplitude();

	void On(double time);
	void Off(double time);

	void setFrequency(double hz);
	double getFrequency();

	double oscillate(double time, double frequency, int osc);
};
