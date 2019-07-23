#ifndef RNG_H
#define RNG_H

#include "lcg.cpp"
#include "mersenne_twister.cpp"
#include "xor_shift.cpp"
#include <cmath>
#include <cstdint>
#include <functional>

#pragma once

namespace rng {

template<typename StateType = uint64_t,
         typename OutputType = uint32_t,
         typename DriverType = typename std::function<OutputType(StateType*)>>
class Random
{
  StateType _state;
  DriverType _rng;

public:
  Random(StateType state)
    : _state(state)
  {
    _rng = lcg_xor_rot;
  };
  Random(DriverType rng)
    : _state(static_cast<uint64_t>(1))
    , _rng(rng){};
  Random(StateType state, DriverType rng)
    : _state(state)
    , _rng(rng){};

  StateType state() { return _state; };
  StateType state() const { return _state; };
  void state(StateType state) { _state = state; };

  OutputType generate()
  {
    OutputType t = _rng(&_state);
    return t;
  };

  OutputType operator()() { return this->generate(); };

  OutputType rand2m(uint8_t m)
  {
    return this->generate() >> (sizeof(OutputType) - (m + 1));
  };

  OutputType bounded_rand(OutputType range)
  {
    OutputType t = (-range) % range;
    while (true) {
      OutputType r = this->generate();
      if (r >= t)
        return r % range;
    }
  };

  OutputType randrange(OutputType a, OutputType b)
  {
    OutputType range = b - a;
    return bounded_rand(range) + a;
  };

  double unit(uint64_t interval = 0x7FFFFFF)
  {
    double i = static_cast<double>(interval);
    return this->generate() / i;
  };
};
}
#endif // RNG_H