#pragma once
#pragma comment(lib, "winmm.lib")

#include <sstream>
#include <Windows.h>

class MidiHandler {
public:
	MidiHandler() {
		this->midiChannel = 0;
		this->pressed = false;
		this->midiNote = 0;
	}

	static void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
		((MidiHandler*)dwInstance)->midiWrapper(hMidiIn, wMsg, dwParam1, dwParam2);
	}

	void midiWrapper(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
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

	void initMidiDevice() {
		int MidiDevicesCount = midiInGetNumDevs();
		cout << MidiDevicesCount << " MIDI devices connected:" << endl;

		MIDIINCAPS MidiInCaps;
		for (unsigned int i = 0; i < MidiDevicesCount; i++) {
			midiInGetDevCaps(i, &MidiInCaps, sizeof(MIDIINCAPS));
			cout << i << " - " << MidiInCaps.szPname << endl;
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

	bool getKeyPressedState() {
		return this->pressed;
	}

	int getNote() {
		return this->midiNote;
	}

	int getChannel() {
		return this->midiChannel;
	}



private:
	bool pressed;
	int midiChannel;
	int midiNote;

	const char* hex_to_bin(char c) {
		switch (toupper(c))
		{
			case '0': return "0000";
			case '1': return "0001";
			case '2': return "0010";
			case '3': return "0011";
			case '4': return "0100";
			case '5': return "0101";
			case '6': return "0110";
			case '7': return "0111";
			case '8': return "1000";
			case '9': return "1001";
			case 'A': return "1010";
			case 'B': return "1011";
			case 'C': return "1100";
			case 'D': return "1101";
			case 'E': return "1110";
			case 'F': return "1111";
		}
	}

	std::string hex_str_to_bin_str(const std::string& hex)
	{
		std::string bin;
		for (unsigned i = 0; i != hex.length(); ++i)
			bin += hex_to_bin(hex[i]);
		return bin;
	}

	void midiMsgKeyboardParser(DWORD_PTR midiMsg)
	{
		std::ostringstream msgHex;
		msgHex << std::hex << (midiMsg & 0x0000FFFF);

		string msgBin = this->hex_str_to_bin_str(msgHex.str());

		if (msgBin.substr(8, 4) == "1001" || msgBin.substr(8, 4) == "1000") {
			string midiKeyPressed = msgBin.substr(8, 4);
			midiKeyPressed == "1000" ? this->pressed = false : this->pressed = true;

			string midiNote = msgBin.substr(0, 8);
			this->midiNote = stoi(midiNote, nullptr, 2);

			string midiChannel = msgBin.substr(12, 4);
			this->midiChannel = stoi(midiChannel, nullptr, 2);
		}
	}
};