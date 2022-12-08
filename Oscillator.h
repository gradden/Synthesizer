#pragma once
#define _USE_MATH_DEFINES

#include <math.h>
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

	double whiteNoiseMinFreq;
	double whiteNoiseMaxFreq;

	bool onKey;
	bool playWithVelocity;
	short velocity;

	double SineWave(double frequency, double time) {
		return sin(frequency * 2.0 * M_PI * time);
	}

	double TriangleWave(double frequency, double time) {
		return (asin(SineWave(frequency, time)));
	}

	double SquareWave(double frequency, double time) {
		return SineWave(frequency, time) > 0.0 ? 1.0 : -1.0;
	}

	double WhiteNoise(double time, double min, double max) {
		double r = (double)rand() / RAND_MAX;

		return SineWave((min + r * (max-min)), time);
	}

	double SawtoothWave(double frequency, double time, double density) {
		double mix = 0.0;

		for (double i = 1.0; i < density; i++) {
			mix += (sin(i * 2.0 * M_PI * frequency * time)) / i;
		}

		return mix;
	}

	double customWave(double frequency, double time) {
		double mix = 0.0;

		mix = this->SawtoothWave(frequency, time, 100.0) + this->SawtoothWave((frequency+1.0), time, 100.0);

		return mix;
	}

public:
	Oscillator();


	double getEnvelope(double timeNow, bool on);
	void setEnvelope(bool isEnveloping, double maxLevel, double A_time, double D_time, double S_level, double R_time);
	bool isEnvelopeEnabled();

	void setAmplitude(double amp);
	double getAmplitude();

	void On(double time);
	void Off(double time);

	void setVelocityPressure(short value);
	void setVelocity(bool velocity);

	void setFrequency(double hz);
	double getFrequency();

	void setWhiteNoiseFreqScale(double& min, double& max);
	double getWhiteNoiseMinFreq();
	double getWhiteNoiseMaxFreq();

	double oscillate(double time, double frequency, int osc, double density);
};
