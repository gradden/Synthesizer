#pragma once
#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <thread>
#include <atomic>
#include <condition_variable>

using namespace std;

template<class T>
class SoundMaker {
public:
	SoundMaker(wstring selectedSoundcard, unsigned int sampleRate = 44100, unsigned int channel = 1, unsigned int blocks = 8, unsigned int blockSamples = 512) {
		this->sampleRate = sampleRate;
		this->channel = channel;
		this->BlockCount = blocks;
		this->freeBlocks = blocks;
		this->BlockSamples = blockSamples;
		this->BlockCurrent = 0;
		this->soundcards = this->getSoundcards();
		this->MemoryUnit = new T[blocks * blockSamples];
		this->wavehdr = new WAVEHDR[blocks];
		auto iterator = find(this->soundcards.begin(), this->soundcards.end(), selectedSoundcard);

		if (this->wavehdr == nullptr || this->MemoryUnit == nullptr) {
			std::exception("Memory initialization is not null");
		}

		ZeroMemory(this->MemoryUnit, (sizeof(T) * this->BlockCount * this->BlockSamples));
		ZeroMemory(this->wavehdr, (sizeof(WAVEHDR) * this->BlockCount));

		if (iterator != this->soundcards.end()) {
			short soundcardID = distance(soundcards.begin(), iterator);
			if (!this->openWaveOut(soundcardID)) {
				std::exception("Failed to open soundcard");
			}
		}
		else {
			throw std::exception("Soundcard not found!");
		}

		for (unsigned int n = 0; n < blocks; n++) {
			this->wavehdr[n].dwBufferLength = sizeof(T) * this->BlockSamples;
			this->wavehdr[n].lpData = (LPSTR)(this->MemoryUnit + (n * this->BlockSamples));
		}

		this->SoundCardThread = thread(&SoundMaker::sendSound, this);

		unique_lock<mutex> lm(this->blockMutex);
		this->threadHandler.notify_one();
	}

	~SoundMaker() {
		terminate(this->SoundCardThread);
		delete[] this->MemoryUnit;
		delete[] this->wavehdr;
	}

	static vector<wstring> getSoundcards() {
		vector<wstring> devices;
		WAVEOUTCAPS waveoutcaps;

		for (int i = 0; i < waveOutGetNumDevs(); i++) {
			if (waveOutGetDevCaps(i, &waveoutcaps, sizeof(WAVEOUTCAPS)) == S_OK) {
				devices.push_back(waveoutcaps.szPname);
			}
		}

		return devices;
	}

	void setWave(double(wave)(double)) {
		this->soundWave = wave;
	}

	double getTime() {
		return this->time;
	}


private:
	HWAVEOUT hwaveout;
	thread SoundCardThread;
	T* MemoryUnit;
	WAVEHDR* wavehdr;
	unsigned int sampleRate;
	unsigned short channel;
	unsigned int BlockCount;
	unsigned int BlockSamples;
	unsigned int BlockCurrent;
	double time;
	double limitValue = 1.0;
	bool proc;
	double(*soundWave)(double);
	atomic<unsigned int> freeBlocks;
	mutex blockMutex;
	condition_variable threadHandler;
	vector<wstring> soundcards;

	bool openWaveOut(short soundcardId) {
		WAVEFORMATEX wfx{};
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec = this->sampleRate;
		wfx.nChannels = this->channel;
		wfx.wBitsPerSample = sizeof(T) * 8;
		wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;

		if (waveOutOpen(&this->hwaveout, soundcardId, &wfx, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK) {
			cout << "Failed to open wave out process";
			return false;
		}

		return true;
	}

	void sendSound() {
		this->time = 0.0;
		double timeStep = (1.0 / (double)this->sampleRate);
		double maxSample = (double)(T)pow(2, (sizeof(T) * 8) - 1) - 1;

		while (1) {
			if (this->freeBlocks == 0) {
				unique_lock<mutex> lm(this->blockMutex);
				this->threadHandler.wait(lm);
			}
			else {
				this->freeBlocks -= 1;

				if (this->wavehdr[this->BlockCurrent].dwFlags & WHDR_PREPARED)
					waveOutUnprepareHeader(this->hwaveout, &this->wavehdr[this->BlockCurrent], sizeof(WAVEHDR));

				T newSample = 0;
				int current = this->BlockCurrent * this->BlockSamples;

				for (unsigned int i = 0; i < this->BlockSamples; i++) {
					if (soundWave == nullptr) {
						newSample = (T)(0.0 * maxSample);
					}
					else {
						newSample = (T)(brickwallLimiter(this->soundWave(this->time)) * maxSample);
					}

					this->MemoryUnit[current + i] = newSample;
					this->time += timeStep;
				}

				waveOutPrepareHeader(this->hwaveout, &this->wavehdr[this->BlockCurrent], sizeof(WAVEHDR));
				waveOutWrite(this->hwaveout, &this->wavehdr[this->BlockCurrent], sizeof(WAVEHDR));
				this->BlockCurrent++;
				this->BlockCurrent %= this->BlockCount;
			}


		}
	}

	void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2) {
		if (uMsg == WOM_DONE) {
			this->freeBlocks++;
			unique_lock<mutex> lm(this->blockMutex);
			this->threadHandler.notify_one();
		}
	}

	static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
		((SoundMaker*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
	}

	double brickwallLimiter(double sample) {
		return (sample >= 0.0) ? fmin(sample, limitValue) : fmax(sample, -limitValue);
	}
	
};
