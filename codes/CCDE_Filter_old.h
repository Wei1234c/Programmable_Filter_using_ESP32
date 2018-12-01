//
// Created by wei on 11/29/18.
//

#ifndef FILTERS_CCDE_FILTER_H
#define FILTERS_CCDE_FILTER_H

#include <math.h> 


#define PI 3.14159265358979323846/* pi */


class CCDE_Filter {

private:
	double _b[1]{ 1.0 };
	double _a[1]{ 1.0 };
	double *b;
	double *a;
	int b_size = 1;
	int a_size = 1;
	int buffer_x_size = 1;
	int buffer_y_size = 1;
	bool is_fir = true;
	double _x = 0;
	double _y = 0;

	void _copy_array(double source[], double dest[], int size) {
		dest = new double[size];
		for (int i = 0; i < size; ++i) {
			dest[i] = source[i];
		}
	}


	double *_divide(double x[], int size, double dnom) {
		for (int i = 0; i < size; ++i) {
			x[i] /= dnom;
		}
		return x;
	}

	double _dot(double x[], double y[], int size) {
		double sum = 0;
		for (int i = 0; i < size; ++i) {
			sum += x[i] * y[i];
		}
		return sum;
	}


	void _updated_buffer(double buff[], int size, double value) {
		for (int i = 1; i < size; ++i) {
			buff[i] = buff[i - 1];
		}
		buff[0] = value;
	}

	double _poly_evalue(double buff[], int size, double coeff[]) {
		return _dot(buff, coeff, size);
	}

public:

	double *filtered_y;
	double *buffer_x;
	double *buffer_y;

	CCDE_Filter() {
		prepare(_b, 1, _a, 1);
	}

	~CCDE_Filter() {
		delete[] filtered_y;
		delete[] buffer_x;
		if (!is_fir) {
			delete[] buffer_y;
		}
	}

	CCDE_Filter(double b[], int size_b, double a[], int size_a) {
		prepare(b, size_b, a, size_a);
	}

	void prepare(double b[], int size_b, double a[], int size_a) {
		b_size = size_b;
		a_size = size_a;
		this->b = _divide(b, b_size, a[0]);
		this->a = _divide(a, a_size, a[0]);
		is_fir = (a_size == 1);
		init_buffers();
	}

	void init_buffers() {
		buffer_x_size = b_size;
		buffer_y_size = a_size - 1;
		buffer_x = new double[buffer_x_size];
		if (!is_fir) {
			buffer_y = new double[buffer_y_size];
		}
	}

	void new_input(double value) {
		_updated_buffer(buffer_x, buffer_x_size, value);
	}

	virtual double new_output(double input_value) {
		new_input(input_value);
		_x = _poly_evalue(buffer_x, buffer_x_size, b);
		_y = _x;

		if (!is_fir) {
			_y = _x - _poly_evalue(buffer_y, buffer_y_size, &a[1]);
			_updated_buffer(buffer_y, buffer_y_size, _y);
		}
		return _y;
	}

	double *filter(double x[], int size) {
		filtered_y = new double[size];
		double v;
		for (int i = 0; i < size; ++i) {
			v = new_output(x[i]);
			filtered_y[i] = v;
		}
		return filtered_y;
	}
};


class Distortion : public CCDE_Filter {
private:
	double _limit;
	double _G;

public:
	Distortion(double limit = 0.02, double G = 5) : CCDE_Filter() {
		_limit = abs(limit);
		_G = G;
	}

	double new_output(double input_value) {
		double output = input_value;
		if (output < -_limit) { output = -_limit; }
		if (output > _limit) { output = _limit; }
		return _G * output;
	}
};


class SimpleEcho : public CCDE_Filter {

public:
	SimpleEcho(double a = 1, double b = 0.75, double c = 0.5, double delay = 0.02, int Fs = 44100) {
		int N = int(Fs * delay);
		double *cb = new double[3 + 2 * N];
		cb[0 * N + 0] = a;
		cb[1 * N + 1] = b;
		cb[2 * N + 2] = c;
		for (int i = 1; i < N + 1; ++i) {
			cb[i] = 0;
		}
		for (int i = N + 2; i < 2 * N + 2; ++i) {
			cb[i] = 0;
		}

		double ca[1] = { a + b + c };

		prepare(cb, 3 + 2 * N, ca, 1);
	}
};


class NaturalEcho : public CCDE_Filter {

public:
	NaturalEcho(double lamda = 0.6, double alpha = 0.7, double delay = 0.02, int Fs = 44100) {
		double cb[2] = { 1, -lamda };

		int N = int(Fs * delay);
		double *ca = new double[N + 3];
		ca[0] = 1;
		ca[1] = -lamda;
		ca[N + 2] = -alpha * (1 - lamda);
		for (int i = 2; i < N + 2; ++i) {
			ca[i] = 0;
		}

		prepare(cb, 2, ca, N + 3);
	}
};


class Reverb : public CCDE_Filter {

public:
	Reverb(double alpha = 0.7, double delay = 0.02, int Fs = 44100) {
		int N = int(Fs * delay);
		double *cb = new double[N + 2];
		double *ca = new double[N + 2];
		cb[0] = -alpha;
		cb[N + 1] = 1;
		ca[0] = 1;
		ca[N + 1] = -alpha;
		for (int i = 1; i < N + 1; ++i) {
			cb[i] = 0;
			ca[i] = 0;
		}

		prepare(cb, N + 2, ca, N + 2);
	}
};


class Tremolo : public CCDE_Filter {
private:
	double omega = 0;
	double G = 0;
	double n = 0;

public:
	Tremolo(double freq = 5, double G = 2, int Fs = 44100) : CCDE_Filter() {
		omega = PI * freq / Fs;
		this->G = G;
		n = 0.0;
	}

	double new_output(double input_value) {
		n += 1;
		return (1 + cos(omega * n) / G) * input_value;
	}
};


class Flanger : public CCDE_Filter {
private:
	double omega = 0;
	double delay = 0;
	double G = 0;
	double n = 0;

public:
	Flanger(double freq = 0.1, double max_delay = 0.002, int Fs = 44100) : CCDE_Filter() {
		omega = PI * freq / Fs;
		delay = max_delay * Fs;
		n = 0.0;
		buffer_x = new double[int(delay * 2 + 1)];
	}

	double new_output(double input_value) {
		new_input(input_value);
		n += 1;
		int idx = int(floor(delay * (1 + cos(omega * n))));
		return input_value + buffer_x[idx];
	}
};


class MovingAverage : public CCDE_Filter {

public:
	MovingAverage(int size = 10) {
		double *cb = new double[size];
		for (int i = 1; i < size; ++i) {
			cb[i] = 1;
		}
		double ca[1] = { float(size) };

		prepare(cb, size, ca, 1);
	}
};


class LeakyIntegrator : public CCDE_Filter {

public:
	LeakyIntegrator(double lamda = 0.6) {
		double cb[1] = { 1 - lamda };
		double ca[2] = { 1, -lamda };

		prepare(cb, 1, ca, 2);
	}
};


class CircularBuffer {
private:
	double *buffer;
	unsigned idx_0;

public:
	unsigned size;

	CircularBuffer(const unsigned size) {
		this->size = size;
		buffer = new double[size] {0.0};
		idx_0 = 0;
	}

	void new_value(double value) {
		idx_0 = (idx_0 + size - 1) % size;
		buffer[idx_0] = value;
	}

	double &operator[](unsigned index) {
		index = (idx_0 + index) % size;
		return buffer[index];
	}
};


#endif //FILTERS_CCDE_FILTER_H