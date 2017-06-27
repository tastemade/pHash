#include "stdio.h"
#include "pHash.h"
#include "CImg.h"
#include "cimgffmpeg.h"
#include <string>
#include <stdexcept>


int main(int argc, char** argv) {
  unsigned long long fingerprintA[3] = {
    stoull(argv[1]), stoull(argv[2]), stoull(argv[3])
  };
  unsigned long long fingerprintB[3] = {
    stoull(argv[4]), stoull(argv[5]), stoull(argv[6])
  };
  
  int dist1A = __builtin_popcountll(fingerprintA[0]^fingerprintB[0]);
  int dist2A = __builtin_popcountll(fingerprintA[1]^fingerprintB[1]);
  int dist3A = __builtin_popcountll(fingerprintA[2]^fingerprintB[2]);
  
  int dist1B = ph_hamming_distance(fingerprintA[0], fingerprintB[0]);
  int dist2B = ph_hamming_distance(fingerprintA[1], fingerprintB[1]);
  int dist3B = ph_hamming_distance(fingerprintA[2], fingerprintB[2]);
  
  float distA = (dist1A + dist2A + dist3A) / 3.0f;
  float distB = (dist1B + dist2B + dist3B) / 3.0f;
  
  printf("%i %i %i = %f\n", dist1A, dist2A, dist3A, distA);
  printf("Check:\n");
  printf("%i %i %i = %f\n", dist1B, dist2B, dist3B, distB);
}