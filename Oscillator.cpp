#define _USE_MATH_DEFINES

#include "Oscillator.h"
#include <math.h>

double Oscillator::getEnvelope(double timeNow) {
		double currentAmplitude = 0.0;
		double envelopeTimeWindow = timeNow - this->onTime;

		if (this->onKey) {

			if (this->isEnveloping) {

				if (envelopeTimeWindow <= this->attackTime)
				{
					currentAmplitude = (envelopeTimeWindow / this->attackTime) * this->maxLevel;
				}

				if (envelopeTimeWindow > this->attackTime && envelopeTimeWindow <= (this->attackTime + this->decayTime)) {
					currentAmplitude = (this->sustainLevel - this->maxLevel) + this->maxLevel * ((envelopeTimeWindow - this->attackTime) / this->decayTime);
				}

				if (envelopeTimeWindow > (this->attackTime + this->decayTime)) {
					currentAmplitude = this->sustainLevel;
				}
			}
			else {
				return this->amplitude;
			}
		}
		else {
			if (this->isEnveloping) {
				currentAmplitude = ((timeNow - this->offTime) / this->releaseTime) * (0.0 - this->sustainLevel) + this->sustainLevel;

				if (currentAmplitude < 0.01) {
					currentAmplitude = 0.0;
				}
			}
			else {
				return 0.0;
			}
		}

		this->amplitude = currentAmplitude;
		return currentAmplitude;
	}

	void Oscillator::setEnvelope(bool isEnveloping, double maxLevel = 0.0, double A_time = 0.1, double D_time = 0.1, double S_level = 0.0, double R_time = 0.1) {
		this->isEnveloping = isEnveloping;
		if (isEnveloping) {
			this->maxLevel = maxLevel;
			this->attackTime = A_time;
			this->decayTime = D_time;
			this->sustainLevel = S_level;
			this->releaseTime = R_time;
		}
	}

	bool Oscillator::isEnvelopeEnabled() {
		return this->isEnveloping;
	}

	void Oscillator::setAmplitude(double amp) {
		this->amplitude = amp;
	}

	double Oscillator::getAmplitude() {
		return this->amplitude;
	}

	void Oscillator::On(double time) {
		this->onTime = time;
		this->onKey = true;
	}

	void Oscillator::Off(double time) {
		this->offTime = time;
		this->onKey = false;
	}

	void Oscillator::setFrequency(double hz) {
		this->frequency = hz;
	}
	
	double Oscillator::getFrequency() {
		return this->frequency;
	}

	double Oscillator::oscillate(double time, double frequency, int osc) {
		switch (osc)
		{
		default:
			return 0.0;
			break;
		case 1:
			return SineWave(frequency, time) * this->getEnvelope(time);
			break;
		case 2:
			return TriangleWave(frequency, time) * this->getEnvelope(time);
			break;
		case 3:
			return SquareWave(frequency, time) * this->getEnvelope(time);
			break;
		case 4:
			return PinkNoise(time) * this->getEnvelope(time);
			break;
		case 5:
			return SawtoothWave(frequency, time) * this->getEnvelope(time);
			break;
		}
	}