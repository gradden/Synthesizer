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

class SoundcardHandler {
public:
	SoundcardHandler(wstring selectedSoundcard, UINT sampleRate = 44100, UINT channel = 1, UINT blocks = 8, UINT blockSamples = 512) {
		this->sampleRate = sampleRate;
		this->channel = channel;
		this->blocks = blocks;
		this->freeBlocks = blocks;
		this->blockSamples = blockSamples;
		this->currentBlock = 0;
		this->time = 0.0;
		this->limitValue = 1.0;
		this->sampleSize = sizeof(short) * 8;
		this->maxSample = pow(2, this->sampleSize - 1) - 1;
		this->soundcards = this->getSoundcards();
		this->MemoryUnit = new short[blocks * blockSamples];
		this->wavehdr = new WAVEHDR[blocks];
		auto iterator = find(this->soundcards.begin(), this->soundcards.end(), selectedSoundcard);

		ZeroMemory(this->MemoryUnit, (sizeof(this->MemoryUnit)));
		ZeroMemory(this->wavehdr, (sizeof(WAVEHDR) * blocks));

		if (iterator != this->soundcards.end()) {
			short soundcardID = distance(soundcards.begin(), iterator);
			if (!this->openWaveOut(soundcardID)) {
				throw exception("Failed to open soundcard");
			}
		}
		else {
			throw exception("Soundcard not found!");
		}

		for (UINT n = 0; n < blocks; n++) {
			this->wavehdr[n].dwBufferLength = sizeof(short) * this->blockSamples;
			this->wavehdr[n].lpData = (LPSTR)(this->MemoryUnit + (n * this->blockSamples));
		}

		this->SoundCardThread = thread(&SoundcardHandler::sendSound, this);

		unique_lock<mutex> lm(this->blockMutex);
		this->threadHandler.notify_one();
	}

	~SoundcardHandler() {
		delete[] this->MemoryUnit;
		delete[] this->wavehdr;
	}

	static vector<wstring> getSoundcards() {
		vector<wstring> devices;
		WAVEOUTCAPS waveoutcaps;

		for (int i = 0; i < waveOutGetNumDevs(); i++) {
			if (waveOutGetDevCaps(i, &waveoutcaps, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR) {
				devices.push_back(waveoutcaps.szPname);
			}
			else {
				throw exception("Cannot get the list of soundcards");
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
	short* MemoryUnit;
	WAVEHDR* wavehdr;
	UINT sampleRate;
	USHORT channel;
	UINT blocks;
	UINT blockSamples;
	UINT currentBlock;
	UINT sampleSize;
	double time;
	double limitValue;
	double(*soundWave)(double);
	atomic<UINT> freeBlocks;
	mutex blockMutex;
	condition_variable threadHandler;
	vector<wstring> soundcards;
	double maxSample;

	bool openWaveOut(short soundcardId) {
		WAVEFORMATEX wfx{};
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.cbSize = 0;
		wfx.nChannels = this->channel;
		wfx.nSamplesPerSec = this->sampleRate;
		wfx.wBitsPerSample = this->sampleSize;
		wfx.nBlockAlign = (this->sampleSize / 8) * wfx.nChannels;
		wfx.nAvgBytesPerSec = this->sampleRate * wfx.nBlockAlign;

		if (waveOutOpen(&this->hwaveout, soundcardId, &wfx, (DWORD_PTR)waveOutWrapper, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
			return false;
		}

		return true;
	}

	void sendSound() {
		while (1) {
			if (this->freeBlocks == 0) {
				unique_lock<mutex> lm(this->blockMutex);
				this->threadHandler.wait(lm);
			}
			else {
				this->freeBlocks -= 1;

				if (this->wavehdr[this->currentBlock].dwFlags & WHDR_PREPARED) {
					waveOutUnprepareHeader(this->hwaveout, &this->wavehdr[this->currentBlock], sizeof(WAVEHDR));
				}

				this->fillAudioData();

				waveOutPrepareHeader(this->hwaveout, &this->wavehdr[this->currentBlock], sizeof(WAVEHDR));
				waveOutWrite(this->hwaveout, &this->wavehdr[this->currentBlock], sizeof(WAVEHDR));
				this->currentBlock = (this->currentBlock + 1) % this->blocks;
			}
		}
	}

	void waveOutCallback(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2) {
		if (uMsg == WOM_DONE) {
			this->freeBlocks++;
			unique_lock<mutex> lm(this->blockMutex);
			this->threadHandler.notify_one();
		}
	}

	static void CALLBACK waveOutWrapper(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
		((SoundcardHandler*)dwInstance)->waveOutCallback(hWaveOut, uMsg, dwParam1, dwParam2);
	}

	double brickwallLimiter(double sample) {
		return (sample >= 0.0) ? fmin(sample, this->limitValue) : fmax(sample, -this->limitValue);
	}

	void fillAudioData() {
		int current = this->currentBlock * this->blockSamples;

		for (UINT i = 0; i < this->blockSamples; i++) {
			this->MemoryUnit[current + i] = (soundWave == nullptr) ?
				(short)(0.0 * (double)maxSample) :
				(short)(brickwallLimiter(this->soundWave(this->time)) * (double)maxSample);
			this->time += (1.0 / (double)this->sampleRate);
		}
	}
	
};
