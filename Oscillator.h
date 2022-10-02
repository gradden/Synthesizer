#pragma once
#define _USE_MATH_DEFINES

#include<math.h>

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
		return asin(SineWave(frequency, time));
	}

	double SquareWave(double frequency, double time) {
		return SineWave(frequency, time) > 0.0 ? 1.0 : 0.0;
	}

	double PinkNoise(double time) {
		return SineWave((double)(rand() % 10000 + 100), time);
	}

	double getEnvelope(double timeNow) {
		double currentAmplitude = 0.0;
		double envelopeTime = timeNow - this->onTime;


		if (this->onKey) {
			if (this->isEnveloping) {
				if (envelopeTime <= this->attackTime)
				{
					currentAmplitude = (envelopeTime / this->attackTime) * this->maxLevel;
				}

				if (envelopeTime > this->attackTime && envelopeTime <= (this->attackTime + this->decayTime)) {
					currentAmplitude = (this->sustainLevel - this->maxLevel) + this->maxLevel * ((envelopeTime - this->attackTime) / this->decayTime);
				}

				if (envelopeTime > (this->attackTime + this->decayTime)) {
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

		return currentAmplitude;
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

	void setEnvelope(bool isEnveloping, double maxLevel = 0.0, double A_time = 0.1, double D_time = 0.1, double S_level = 0.0, double R_time = 0.1) {
		this->isEnveloping = isEnveloping;
		if (isEnveloping) {
			this->maxLevel = maxLevel;
			this->attackTime = A_time;
			this->decayTime = D_time;
			this->sustainLevel = S_level;
			this->releaseTime = R_time;
		}
	}

	void setAmplitude(double amp) {
		this->amplitude = amp;
	}

	void On(double time) {
		this->onTime = time;
		this->onKey = true;
	}

	void Off(double time) {
		this->offTime = time;
		this->onKey = false;

	}

	void setFrequency(double hz) {
		this->frequency = hz;
	}
	
	double getFrequency() {
		return this->frequency;
	}

	double oscillate(double time, double frequency, int osc) {
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
		}
	}
};
