#include "MidiHandler.h"

void CALLBACK MidiHandler::midiBridgeCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	((MidiHandler*)dwInstance)->midiBridge(hMidiIn, wMsg, dwParam1, dwParam2);
}

void MidiHandler::midiBridge(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	if (wMsg != MIM_ERROR && wMsg != MIM_LONGERROR && wMsg == MIM_DATA) {
		this->midiMsgKeyboardParser(dwParam1);
	}
}

void MidiHandler::initMidiDevice() {
	int MidiDevicesCount = midiInGetNumDevs();
	cout << MidiDevicesCount << " MIDI devices connected:" << endl;

	MIDIINCAPS MidiInCaps;
	for (int i = 0; i < MidiDevicesCount; i++) {
		midiInGetDevCaps(i, &MidiInCaps, sizeof(MIDIINCAPS));
		cout << i << " - " << this->charToString(MidiInCaps.szPname) << endl;
	}

	int DeviceID;
	cout << "Choose MIDI Device:" && cin >> DeviceID;

	LPHMIDIIN device = new HMIDIIN[MidiDevicesCount];
	if (midiInOpen(&device[DeviceID], DeviceID, (DWORD_PTR)midiBridgeCallback, (DWORD_PTR)this, CALLBACK_FUNCTION) == S_OK) {
		cout << "Connected to " << DeviceID << ". MIDI device!" << endl;
		midiInStart(device[DeviceID]); 
	}
	else {
		cout << "MIDI connecting failed" << endl;
	}
}

