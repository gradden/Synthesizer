#include <iostream>
using namespace std;

#include "SoundMaker.h"
#include "Oscillator.h"

int main()
{
	loadSoundMaker();

	sound->SetUserFunction(wrapper);

	while (1) {
		
	}

	return 0;
}
