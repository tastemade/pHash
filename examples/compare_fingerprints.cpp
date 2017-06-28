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
  
  // calculate the hamming distance for each hash
  int dist1 = __builtin_popcountll(fingerprintA[0]^fingerprintB[0]);
  int dist2 = __builtin_popcountll(fingerprintA[1]^fingerprintB[1]);
  int dist3 = __builtin_popcountll(fingerprintA[2]^fingerprintB[2]);
  
  // add all the distances for the total distance
  int dist = dist1 + dist2 + dist3;
  
  // turn the distance (0 to 192) into a simularity score (1 to 0)
  // each distance has a range of 0-64 (64 being the number of bits in a 64bit int/long long)
  float sim = 1 - ((float)dist / (64*3)); 
  
  printf("%f\n", sim);
}