#pragma once
#pragma comment(lib, "winmm.lib")

#include <sstream>
#include <Windows.h>
#include <iostream>
using namespace std;

class MidiHandler {
public:
	MidiHandler() {
		this->midiChannel = 0;
		this->pressed = false;
		this->midiNote = 0;
	}

	static void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	void midiWrapper(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	void initMidiDevice();

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

	string charToString(WCHAR chars[]) {


		string s;
		for (int i = 0; i <= 32; i++) {
			s += chars[i];
		}
		return s;
	}

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

	string hex_str_to_bin_str(const string& hex) {
		std::string bin;
		for (unsigned i = 0; i != hex.length(); ++i)
			bin += hex_to_bin(hex[i]);
		return bin;
	}

	void midiMsgKeyboardParser(DWORD_PTR midiMsg) {
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