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
			printf("wMsg=MIM_OPEN\n");
			break;
		case MIM_CLOSE:
			printf("wMsg=MIM_CLOSE\n");
			break;
		case MIM_DATA:
			//sprintf("0x%08X 0x%02X 0x%02X 0x%02X\r\n", dwParam2, dwParam1 & 0x000000FF, (dwParam1 >> 8) & 0x000000FF, (dwParam1 >> 16) & 0x000000FF);
			//break;
			//cout << "NOTE: " << note << endl;
			//cout << "ON / OFF :" << keyPressed << endl;
			this->midiMsgKeyboardParser(dwParam1);
			//printf("wMsg=MIM_DATA, dwParam1=%08x, dwParam2=%08x\n", dwParam1, dwParam2);
			break;
		case MIM_LONGDATA:
			printf("wMsg=MIM_LONGDATA\n");
			break;
		case MIM_ERROR:
			printf("wMsg=MIM_ERROR\n");
			break;
		case MIM_LONGERROR:
			printf("wMsg=MIM_LONGERROR\n");
			break;
		case MIM_MOREDATA:
			printf("wMsg=MIM_MOREDATA\n");
			break;
		default:
			printf("wMsg = unknown\n");
			break;
		}
		return;
	}

	void listMidiDevices() {
		unsigned int numDevs = midiInGetNumDevs();
		cout << numDevs << " MIDI devices connected:" << endl;

		MIDIINCAPS inputCapabilities;
		for (unsigned int i = 0; i < numDevs; i++) {
			midiInGetDevCaps(i, &inputCapabilities, sizeof(inputCapabilities));
			cout << "[" << i << "] " << inputCapabilities.szPname << endl;
		}

		int portID;
		cout << "Enter the port which you want to connect to: ";
		cin >> portID;
		cout << "Trying to connect with the device on port " << portID << "..." << endl;

		LPHMIDIIN device = new HMIDIIN[numDevs];
		int flag = midiInOpen(&device[portID], portID, (DWORD_PTR)midiInputCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);

		if (flag != MMSYSERR_NOERROR) {
			cout << "Error opening MIDI port." << endl;
		}
		else {
			cout << "You are now connected to port " << portID << "!" << endl;
			midiInStart(device[portID]);
		}
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
		if (msgBin.size() == 16) {
			string midiKeyPressed = msgBin.substr(8, 4);
			midiKeyPressed == "1000" ? this->pressed = false : this->pressed = true;

			string midiNote = msgBin.substr(0, 8);
			this->midiNote = stoi(midiNote, nullptr, 2);

			string midiChannel = msgBin.substr(12, 4);
			this->midiChannel = stoi(midiChannel, nullptr, 2);

			cout << "NOTE:" << this->midiNote << endl;
			cout << "KeyPressed:" << this->pressed << endl;
			cout << "Channel:" << this->midiChannel << endl;
		}
	}
};