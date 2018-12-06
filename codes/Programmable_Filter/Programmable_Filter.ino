#include "CCDE_Filter.h"


CCDE_Filter filter = CCDE_Filter();
//Distortion filter(1000, 5);
//SimpleEcho filter(1, 0.75, 0.5,  0.02,  44100);
//NaturalEcho filter( 0.6, 0.7,   0.02,  44100); // not working.
//Reverb filter(0.7,  0.02, 44100);
//Tremolo filter(5, 2, 44100);
//Flanger filter( 0.1, 0.01, 44100);


int _h_i_size = 16;
double *_h_ipsilateral = new double[16]{ 1.0, 0.0, 0.0, 0.0, 0.0,
										 0.0, 0.0, 0.0, 0.0, 0.0,
										 0.0, 0.0, 0.05853095, 0.44806808, 0.8575164,
										 0. };

int _h_c_size = 16;
double *_h_contralateral = new double[16]{ 0.0, 0.0, 0.0, 0.0, 0.0,
										   0.0, -0.2419317, -0.9260218, 0.0, 0.0,
										   0.0, 0.0, 0.0, 0.0, 0.0,
										   0. };

//CrossTalkCanceller filter = CrossTalkCanceller();
//CrossTalkCanceller filter = CrossTalkCanceller(_h_ipsilateral, _h_i_size, _h_contralateral, _h_c_size);

void setup() {

	Serial.begin(115200);

	/*
    analogReadResolution(12);             // Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
    analogSetWidth(12);                   // Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
              //  9-bit gives an ADC range of 0-511
              // 10-bit gives an ADC range of 0-1023
              // 11-bit gives an ADC range of 0-2047
              // 12-bit gives an ADC range of 0-4095
    analogSetCycles(8);                   // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
    analogSetSamples(1);                  // Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
    analogSetClockDiv(1);                 // Set the divider for the ADC clock, default is 1, range is 1 - 255
    analogSetAttenuation(ADC_11db);       // Sets the input attenuation for ALL ADC inputs, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
    analogSetPinAttenuation(VP,ADC_11db); // Sets the input attenuation, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
              // ADC_0db provides no attenuation so IN/OUT = 1 / 1 an input of 3 volts remains at 3 volts before ADC measurement
              // ADC_2_5db provides an attenuation so that IN/OUT = 1 / 1.34 an input of 3 volts is reduced to 2.238 volts before ADC measurement
              // ADC_6db provides an attenuation so that IN/OUT = 1 / 2 an input of 3 volts is reduced to 1.500 volts before ADC measurement
              // ADC_11db provides an attenuation so that IN/OUT = 1 / 3.6 an input of 3 volts is reduced to 0.833 volts before ADC measurement
    adcAttachPin(VP);                     // Attach a pin to ADC (also clears any other analog mode that could be on), returns TRUE/FALSE result
    adcStart(VP);                         // Starts an ADC conversion on attached pin's bus
    adcBusy(VP);                          // Check if conversion on the pin's ADC bus is currently running, returns TRUE/FALSE result
    adcEnd(VP);                           // Get the result of the conversion (will wait if it have not finished), returns 16-bit integer result
    */
	//  adcAttachPin(36);
}

int mapping(double input_value) {
	int output_value = input_value / 4;
	if (output_value > 255) {
		output_value = 255;
	}
	if (output_value < 0) {
		output_value = 0;
	}
	return output_value;
}

double left = 0;
double right = 0;
double* output = NULL;

void loop() {
	left = analogRead(36);
	right = analogRead(39);

	dacWrite(25, mapping(filter.new_output(left)));
	dacWrite(26, mapping(filter.new_output(right)));

//  output = filter.new_output(left, right);
//  dacWrite(25, mapping(output[0]));
//  dacWrite(26, mapping(output[1]));
}
