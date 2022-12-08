#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <math.h>

class Filter {
private:
    double* filterCoeff;
    double* buffer;
    double cutoffFrequency;
    int taps;
    int sampleRate;

    void generateLowPassCoeff() {
        if (this->cutoffFrequency < 0.0 || this->cutoffFrequency > this->sampleRate) {
            this->cutoffFrequency = this->sampleRate / 2.0;
        }

        double cutoff = this->cutoffFrequency / this->sampleRate;
        int a = this->taps / 2;
        for (int i = -a; i < a; i++) {
            if (i == 0) {
                filterCoeff[a] = 2 * cutoff;
            }
            else {
                filterCoeff[i + a] = sin(2 * (M_PI)*cutoff * i) / (i * (M_PI));
            }
        }
    }

public:
    Filter(int sampleRate, int taps = 50) {
        filterCoeff = new double[taps];
        buffer = new double[taps];
        memset(buffer, 0.0, taps * sizeof(double));
        this->taps = taps;
        this->sampleRate = sampleRate;
        this->cutoffFrequency = 120.0;
    }

    ~Filter() {
        delete[] filterCoeff;
        delete[] buffer;
    }

    void setCutoff(double& cutoffFrequency) {
        this->cutoffFrequency = cutoffFrequency;
        this->generateLowPassCoeff();
    }

    double filter(const double& inputSample) {
        memmove(&this->buffer[1], &this->buffer[0], (this->taps - 1) * sizeof(*this->buffer));
        this->buffer[0] = inputSample;

        double outputSample = 0.0;
        for (unsigned int i = 0; i < this->taps; i++) {
            outputSample += this->buffer[i] * this->filterCoeff[i];
        }

        return outputSample;
    }
};
