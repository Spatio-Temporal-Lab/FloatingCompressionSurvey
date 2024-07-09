#pragma once

#include <cstdint>

union DOUBLE {
  double d;
  uint64_t i;
};

union FLOAT {
  float f;
  uint32_t i;
};

// Utils
int getFAlpha(int alpha);
int *getAlphaAndBetaStar(double v, int lastBetaStar);
int *getAlphaAndBetaStar_32(float v, int lastBetaStar);
double roundUp(double v, int alpha);
float roundUp_32(float v, int alpha);
double get10iN(int i);
float get10iN_32(int i);
int getSP(double v);