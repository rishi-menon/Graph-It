#pragma once
#include "DebugFinal.h"
#include <cmath>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>
// #include <gtx/norm.hpp>
// #include <gtx/compatibility.hpp>

template <typename T>
T Min(T a, T b) {
    return (a < b) ? a : b;
}
template <typename T>
T Max(T a, T b) {
    return (a > b) ? a : b;
}

inline double Lerp(double min, double max, double t) {
    return min + (max-min)*t;
}

inline double Percent(double min, double max, double val) {
    return (val - min) / (max - min);
}

inline double Remap(double val, double min, double max, double newMin, double newMax) {
    return Lerp(newMin, newMax, Percent(min, max, val));
}

inline double Random(double min, double max) {
    return glm::linearRand(min, max);
}

template <typename T>
inline double Mag(const T& v) {
    return glm::sqrt( glm::dot(v, v) );
}

template <typename T>
inline double SqMag(const T& v) {
    return glm::dot(v, v);
}