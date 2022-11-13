#pragma once

class Note {
public:
	int noteId;
	double freq;
	bool active;
	double amplitude;

	Note() {
		this->noteId = 100;
		this->freq = 0.0;
		this->active = false;
		this->amplitude = 0.0;
	}
};
