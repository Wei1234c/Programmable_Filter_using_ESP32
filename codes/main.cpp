#include <iostream>
#include "CCDE_Filter.h"


int main() {
    int size = 11;
    CircularBuffer cb = CircularBuffer(10);
    for (int i = 0; i < size; ++i) {
        cb.new_value(i);
    }
    for (int i = 0; i < size; ++i) {
        std::cout << cb[i] << ", ";
    }
}

//
//
//int main() {
////    double b[]{2};
////    double a[]{2};
////    int size_b = sizeof(b) / sizeof(double);
////    int size_a = sizeof(a) / sizeof(double);
//
//    CCDE_Filter filter = CCDE_Filter();
//    //CCDE_Filter filter = CCDE_Filter(b, size_b, a, size_a);
//    //Distortion filter = Distortion(10, 5);
//    //SimpleEcho filter = SimpleEcho(1, 0.75, 0.5, 0.3, 44100);
////    NaturalEcho filter = NaturalEcho( 0.6,  0.7,  0.02, 44100);
////    LeakyIntegrator filter = LeakyIntegrator( 0.6 );
////    MovingAverage filter = MovingAverage(10);
////    LeakyIntegrator filter = LeakyIntegrator(10);
//
//    double x[] = {2, 3, 4, 6};
//    int size_x = sizeof(x) / sizeof(double);
//    double *y = filter.filter(x, size_x);
//    for (int i = 0; i < size_x; ++i) {
//        std::cout <<  y[i]  <<  ", ";
//    }
//}