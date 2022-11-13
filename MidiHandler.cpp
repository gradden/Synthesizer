#include "MidiHandler.h"

void CALLBACK MidiHandler::midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	((MidiHandler*)dwInstance)->midiWrapper(hMidiIn, wMsg, dwParam1, dwParam2);
}

void MidiHandler::midiWrapper(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	switch (wMsg) {
	case MIM_OPEN:
		printf("wMsg=MIDI Message channel has been opened.\n");
		break;
	case MIM_CLOSE:
		printf("wMsg=MIDI Message channel has been closed.\n");
		break;
	case MIM_DATA:
		this->midiMsgKeyboardParser(dwParam1);
		break;
	case MIM_LONGDATA:
		printf("wMsg=MIDI Message longdata received.\n");
		break;
	case MIM_ERROR:
		printf("wMsg=MIDI Message error.\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIDI Message long error.\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIDI Message more data.\n");
		break;
	default:
		printf("wMsg = Unknown\n");
		break;
	}
	return;
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
	if (midiInOpen(&device[DeviceID], DeviceID, (DWORD_PTR)midiInputCallback, (DWORD_PTR)this, CALLBACK_FUNCTION) == S_OK) {
		cout << "Connected to " << DeviceID << ". MIDI device!" << endl;
		midiInStart(device[DeviceID]); 
	}
	else {
		cout << "MIDI connecting failed" << endl;
	}
}

