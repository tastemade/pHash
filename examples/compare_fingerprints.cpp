#include <string>
using namespace std;

#define NUM_HASHES 5


int main(int argc, char** argv) {
  if (argc < NUM_HASHES*2 + 1) {
    printf("USAGE: fp1h1 fp2h2... fp2h1 fp2h2... [-v]");
    return 1;
  }
  
  // parse the string hashes
  unsigned long long fingerprintA[NUM_HASHES] = {};
  unsigned long long fingerprintB[NUM_HASHES] = {};
  for (int i = 0; i < NUM_HASHES; ++i) {
    fingerprintA[i] = stoull(argv[1 + i]);
    fingerprintB[i] = stoull(argv[1 + NUM_HASHES + i]);
  }
  
  bool verbose = false;
  if (argc > NUM_HASHES*2 + 1 && string(argv[NUM_HASHES*2 + 1]).compare("-v") == 0) {
    verbose = true;
  }
  
  // calculate the hamming distance for each hash
  int dists[NUM_HASHES] = {};
  for (int i = 0; i < NUM_HASHES; ++i) {
    dists[i] = __builtin_popcountll(fingerprintA[i]^fingerprintB[i]);
  }
  
  // add all the distances for the total distance
  int dist = 0;
  for (int i = 0; i < NUM_HASHES; ++i) {
    dist += dists[i];
  }
  
  // turn the distance (0 to 64*NUM_HASHES) into a simularity score (1 to 0)
  // each distance has a range of 0-64 (64 being the number of bits in a 64bit int/long long)
  float sim = 1 - ((float)dist / (64 * NUM_HASHES)); 
  
  if (verbose) {
    for (int i = 0; i < NUM_HASHES; ++i) {
      printf("%i ", dists[i]);
    }
    
    printf("- %i - ", dist);
  }
  
  printf("%f\n", sim);
}