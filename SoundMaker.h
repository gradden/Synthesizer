#pragma once
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <Windows.h>

using namespace std;

template<class T>
class SoundMaker
{
private:
	HWAVEOUT hwaveout;
	//on_xxx = 'ObjectNormal_xxx' variable
	//op_xxx = 'ObjectPointer_xxx' variable
	unsigned int n_sampleRate;
	unsigned int n_channel;
	unsigned int n_BlockCount;
	unsigned int n_BlockSamples;
	bool b_proc;

	thread SoundCardThread;

	T* p_MemoryUnit;
	WAVEHDR *wavehdr;



public:
	SoundMaker(wstring device, unsigned int sampleRate = 44100, unsigned int channel = 1, unsigned int blocks = 8, unsigned int blockSamples = 512) {
		MakeHDR(device, sampleRate, channel, blocks, blockSamples);
	}

	~SoundMaker() {
		Destroy();
	}

	bool Destroy() {
		return false;
	}

	void Stop()
	{
		b_proc = false;
		SoundCardThread.join();
	}

	static vector<wstring> ListDevices() {

		int DeviceCount = waveOutGetNumDevs();
		vector<wstring> devices;
		WAVEOUTCAPS woc;

		for (int i = 0; i < DeviceCount; i++) {
			if (waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS)) == S_OK) {
				devices.push_back(woc.szPname);
			}
		}

		return devices;
	}

	bool MakeHDR(wstring device, unsigned int sampleRate, unsigned int channel, unsigned int blocks, unsigned int blockSamples) {
		b_proc = false;
		n_sampleRate = sampleRate;
		n_channel = channel;
		n_BlockCount = blocks;
		n_BlockSamples = blockSamples;
		nBlockFree = n_BlockCount;
		n_BlockCurrent = 0;
		p_MemoryUnit = nullptr;
		wavehdr = nullptr;

		userFunction = nullptr;

		vector<wstring> devices = ListDevices();
		auto d = find(devices.begin(), devices.end(), device);

		if (d != devices.end()) {

			WAVEFORMATEX wfx{};
			int deviceID = distance(devices.begin(), d);
			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.nSamplesPerSec = n_sampleRate;
			wfx.nChannels = n_channel;
			wfx.wBitsPerSample = sizeof(T) * 8;
			wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			wfx.cbSize = 0; //WAVE_FORMAT_PCM esetén 0 extra bitet tartalmaz a headers

			if (waveOutOpen(&hwaveout, deviceID, &wfx, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK) {
				cout << "Failed to open soundcard";
			}
		}

		p_MemoryUnit = new T[n_BlockCount * n_BlockSamples];
		if (p_MemoryUnit == nullptr) {
			Destroy();
		}
		ZeroMemory(p_MemoryUnit, sizeof(T) * n_BlockCount * n_BlockSamples);

		wavehdr = new WAVEHDR[n_BlockCount];
		if (wavehdr == nullptr) {
			Destroy();
		}
		ZeroMemory(wavehdr, sizeof(WAVEHDR) * n_BlockCount);

		for (unsigned int n = 0; n < n_BlockCount; n++)
		{
			wavehdr[n].dwBufferLength = n_BlockSamples * sizeof(T);
			wavehdr[n].lpData = (LPSTR)(p_MemoryUnit + (n * n_BlockSamples));
		}

		b_proc = true;

		SoundCardThread = thread(&SoundMaker::MainThread, this);

		unique_lock<mutex> lm(BlockNotZero);
		cvBlockNotZero.notify_one();

		//SoundCardThread.join();

		return true;

		
	}

	// Handler for soundcard request for more data
	void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
	{
		if (uMsg != WOM_DONE) return;

		nBlockFree++;
		unique_lock<mutex> lm(BlockNotZero);
		cvBlockNotZero.notify_one();
	}

	// Static wrapper for sound card handler
	static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
	{
		((SoundMaker*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
	}

	void SetUserFunction(double(func)(double))
	{
		userFunction = func;
	}

	double clip(double dSample, double dMax)
	{
		if (dSample >= 0.0)
			return fmin(dSample, dMax);
		else
			return fmax(dSample, -dMax);
	}

	double GetTime()
	{
		return m_dGlobalTime;
	}

	// Override to process current sample
	virtual double UserProcess(double dTime)
	{
		return 0.0;
	}


private:
	double(*userFunction)(double);
	atomic<unsigned int> nBlockFree;
	double m_dGlobalTime;
	mutex BlockNotZero;
	condition_variable cvBlockNotZero;
	unsigned int n_BlockCurrent;

	void MainThread() {

		m_dGlobalTime = 0.0;
		double dTimeStep = 1.0 / (double)n_sampleRate;

		// Goofy hack to get maximum integer for a type at run-time
		T nMaxSample = (T)pow(2, (sizeof(T) * 8) - 1) - 1;
		double dMaxSample = (double)nMaxSample;

		while (b_proc) {

			if (nBlockFree == 0) {
				unique_lock<mutex> lm(BlockNotZero);
				cvBlockNotZero.wait(lm);
			}

			nBlockFree--;

			if (wavehdr[n_BlockCurrent].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(hwaveout, &wavehdr[n_BlockCurrent], sizeof(WAVEHDR));

			T newSample = 0;
			int current = n_BlockCurrent * n_BlockSamples;

			for (unsigned int i = 0; i < n_BlockSamples; i++) {
				//cout << userFunction << endl;
				if (userFunction == nullptr) {
					newSample = (T)(clip(UserProcess(m_dGlobalTime), 1.0) * dMaxSample);
				}
				else {
					newSample = (T)(clip(userFunction(m_dGlobalTime), 1.0) * dMaxSample);
				}

				p_MemoryUnit[current + i] = newSample;
				m_dGlobalTime = m_dGlobalTime + dTimeStep;
			}

			waveOutPrepareHeader(hwaveout, &wavehdr[n_BlockCurrent], sizeof(WAVEHDR));
			waveOutWrite(hwaveout, &wavehdr[n_BlockCurrent], sizeof(WAVEHDR));
			n_BlockCurrent++;
			n_BlockCurrent %= n_BlockCount;
		}

	}
};

