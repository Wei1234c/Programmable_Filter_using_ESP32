//
// Created by wei on 11/29/18.
//

#ifndef FILTERS_CCDE_FILTER_H
#define FILTERS_CCDE_FILTER_H

#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846/* pi */


class CircularBuffer {
private:
    double *buffer = NULL;
    int idx_new = 0;
    int idx_0 = 0;

public:
    int size;

    CircularBuffer(const int size = 0) : size(size) {
        buffer = new double[size]{0.0};
    }

    void new_value(double value) {
        buffer[idx_new] = value;
        idx_0 = idx_new;
        idx_new = (idx_new - 1 + size) % size;
    }

    double &operator[](int index) {
        index = (idx_0 + index) % size;
        return buffer[index];
    }
};


class CCDE_Filter {

private:
    double *b = NULL;
    double *a = NULL;
    bool is_fir = true;

    void _init_buffers(int b_size, int a_size) {
        buffer_x = new CircularBuffer(b_size);
        if (!is_fir) {
            buffer_y = new CircularBuffer(a_size - 1);
        }
    }

    double *_divide(double x[], int size, double dnom) {
        for (int i = 0; i < size; ++i) {
            x[i] = x[i] / dnom;
        }
        return x;
    }

    double _dot(CircularBuffer *buff, double coeffs[]) {
        double sum = 0;
        for (int i = 0; i < (*buff).size; ++i) {
            sum += (*buff)[i] * coeffs[i];
        }
        return sum;
    }

    void _updated_buffer(CircularBuffer *buff, double value) {
        buff->new_value(value);
    }

    double _poly_evalue(CircularBuffer *buff, double coeff[]) {
        return _dot(buff, coeff);
    }

public:

    double *filtered_y = NULL;
    CircularBuffer *buffer_x = NULL;
    CircularBuffer *buffer_y = NULL;


    void prepare(double b[], int b_size, double a[], int a_size) {
        this->b = _divide(b, b_size, a[0]);
        this->a = _divide(a, a_size, a[0]);
        is_fir = (a_size == 1);
        _init_buffers(b_size, a_size);
    }

    CCDE_Filter() {
        prepare(new double[1]{1.0}, 1, new double[1]{1.0}, 1);
    }

    ~CCDE_Filter() {
        delete[] filtered_y;
        delete buffer_x;
        delete buffer_y;
    }

    CCDE_Filter(double b[], int size_b, double a[], int size_a) {
        prepare(b, size_b, a, size_a);
    }


    void new_input(double value) {
        _updated_buffer(buffer_x, value);
    }

    virtual double new_output(double input_value) {
        double _x;
        double _y;

        new_input(input_value);
        _x = _poly_evalue(buffer_x, b);
        _y = _x;

        if (!is_fir) {
            _y = _x - _poly_evalue(buffer_y, &a[1]);
            _updated_buffer(buffer_y, _y);
        }
        return _y;
    }

    double *filter(double x[], int size) {
        filtered_y = new double[size];
        for (int i = 0; i < size; ++i) {
            filtered_y[i] = new_output(x[i]);
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

        double ca[1] = {a + b + c};

        prepare(cb, 3 + 2 * N, ca, 1);
    }
};


class NaturalEcho : public CCDE_Filter {

public:
    NaturalEcho(double lamda = 0.6, double alpha = 0.7, double delay = 0.02, int Fs = 44100) {
        double cb[2] = {1, -lamda};

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
        buffer_x = new CircularBuffer(int(delay * 2 + 1));
    }

    double new_output(double input_value) {
        new_input(input_value);
        n += 1;
        int idx = int(floor(delay * (1 + cos(omega * n))));
        return input_value + (*buffer_x)[idx];
    }
};


class MovingAverage : public CCDE_Filter {

public:
    MovingAverage(int size = 10) {
        double *cb = new double[size];
        for (int i = 0; i < size; ++i) {
            cb[i] = 1;
        }
        double ca[1] = {float(size)};

        prepare(cb, size, ca, 1);
    }
};


class LeakyIntegrator : public CCDE_Filter {

public:
    LeakyIntegrator(double lamda = 0.6) {
        double cb[1] = {1 - lamda};
        double ca[2] = {1, -lamda};

        prepare(cb, 1, ca, 2);
    }
};


class CrossTalkCanceller {
private:
    int _h_i_size = 16;
    double *_h_ipsilateral = new double[16]{1.0, 0.0, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 0.05853095, 0.44806808, 0.8575164,
                                            0.};

    int _h_c_size = 16;
    double *_h_contralateral = new double[16]{0.0, 0.0, 0.0, 0.0, 0.0,
                                              0.0, -0.2419317, -0.9260218, 0.0, 0.0,
                                              0.0, 0.0, 0.0, 0.0, 0.0,
                                              0.};
    CCDE_Filter *filter_ll = NULL;
    CCDE_Filter *filter_rl = NULL;
    CCDE_Filter *filter_lr = NULL;
    CCDE_Filter *filter_rr = NULL;

    double l_left = 0;
    double r_left = 0;
    double left_out = 0;

    double r_right = 0;
    double l_right = 0;
    double right_out = 0;

    double output[2]{0, 0};

public:
    CrossTalkCanceller() {
        setup_filters(_h_ipsilateral, _h_i_size, _h_contralateral, _h_c_size);
    }

    CrossTalkCanceller(double h_ipsilateral[], int h_i_size, double h_contralateral[], int h_c_size) {
        setup_filters(h_ipsilateral, h_i_size, h_contralateral, h_c_size);
    }

    void setup_filters(double h_ipsilateral[], int h_i_size, double h_contralateral[], int h_c_size) {
        _h_i_size = h_i_size;
        _h_ipsilateral = h_ipsilateral;
        _h_c_size = h_c_size;
        _h_contralateral = h_contralateral;
        filter_ll = new CCDE_Filter(h_ipsilateral, h_i_size, new double[1]{1}, 1);
        filter_rl = new CCDE_Filter(h_contralateral, h_c_size, new double[1]{1}, 1);
        filter_lr = new CCDE_Filter(h_contralateral, h_c_size, new double[1]{1}, 1);
        filter_rr = new CCDE_Filter(h_ipsilateral, h_i_size, new double[1]{1}, 1);
    }

    double *new_output(double left, double right) {
        l_left = (*filter_ll).new_output(left);
        r_left = (*filter_rl).new_output(right);
        left_out = l_left + r_left;

        l_right = (*filter_lr).new_output(left);
        r_right = (*filter_rr).new_output(right);
        right_out = r_right + l_right;

        output[0] = left_out;
        output[1] = right_out;
        return output;
    }
};

#endif //FILTERS_CCDE_FILTER_H
