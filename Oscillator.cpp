#define _USE_MATH_DEFINES

#include "Oscillator.h"
#include <math.h>
#include <iostream>

Oscillator::Oscillator()
{
	this->amplitude = 0.3;
	this->frequency = 440.0;
	this->osc_type = 1;
	this->isEnveloping = false;

	this->maxLevel = 0.3;
	this->attackTime = 0.1;
	this->decayTime = 0.0;
	this->sustainLevel = 0.3;
	this->releaseTime = 0.4;
	this->onTime = 0.0;
	this->offTime = 0.0;
	this->onKey = false;

	this->playWithVelocity = false;
	this->velocity = 127;

	this->whiteNoiseMinFreq = 20;
	this->whiteNoiseMaxFreq = 20000;
}

double Oscillator::getEnvelope(double timeNow, bool on) {
		double currentAmplitude = 0.0;
		double envelopeTimeWindow = timeNow - this->onTime;

		if (on) {
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
				currentAmplitude = this->amplitude;
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
				currentAmplitude = 0.0;
			}
		}

		if (this->playWithVelocity) {
			double velocitiedLevel = currentAmplitude / 127;
			currentAmplitude = velocitiedLevel * this->velocity;
		}

		return currentAmplitude;
	}

double Oscillator::getReleaseForNote(double timeNow, double amplitude) {
	double currentAmplitude = amplitude;
	if (this->isEnveloping) {
		currentAmplitude = ((timeNow - this->offTime) / this->releaseTime) * (0.0 - this->sustainLevel) + this->sustainLevel;

		if (currentAmplitude < 0.01) {
			currentAmplitude = 0.0;
		}
	}
	else {
		return 0.0;
	}
	return currentAmplitude;
}

void Oscillator::setEnvelope(bool isEnveloping, double maxLevel, double A_time, double D_time, double S_level, double R_time) {
	this->isEnveloping = isEnveloping;
	if (isEnveloping) {
		this->maxLevel = maxLevel;
		this->attackTime = A_time;
		this->decayTime = D_time;
		this->sustainLevel = S_level;
		this->releaseTime = R_time;
	}
}

void Oscillator::setVelocityPressure(short value) {
	this->velocity = value;
}

void Oscillator::setVelocity(bool velocity) {
	this->playWithVelocity = velocity;
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
		return SineWave(frequency, time);
		break;
	case 2:
		return TriangleWave(frequency, time);
		break;
	case 3:
		return SquareWave(frequency, time);
		break;
	case 4:
		return WhiteNoise(time, this->whiteNoiseMinFreq, this->whiteNoiseMaxFreq);
		break;
	case 5:
		return SawtoothWave(frequency, time);
		break;
	case 6:
		return customWave(frequency, time);
		break;
	}
	
}

void Oscillator::setWhiteNoiseFreqScale(double& min, double& max) {
	if (min >= 1.0 && max <= 40000.0) {
		this->whiteNoiseMinFreq = min;
		this->whiteNoiseMaxFreq = max;
	}
}

double Oscillator::getWhiteNoiseMinFreq() {
	return this->whiteNoiseMinFreq;
}

double Oscillator::getWhiteNoiseMaxFreq() {
	return this->whiteNoiseMaxFreq;
}