#include <string>
#include <sstream>
using namespace std;

#define NUM_HASHED_FRAMES 5
#define NUM_HASHES_PER_FRAME 5


int main(int argc, char** argv) {
  /*
  if (argc < NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME * 2 + 1) {
    printf("USAGE: fp1h1 fp2h2... fp2h1 fp2h2... [-v]\n");
    return 1;
  }
  
  // parse the string hashes
  unsigned long long fingerprintA[NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME] = {};
  unsigned long long fingerprintB[NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME] = {};
  for (int i = 0; i < NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME; ++i) {
    fingerprintA[i] = stoull(argv[1 + i]);
    fingerprintB[i] = stoull(argv[1 + NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME + i]);
  }
  
  bool verbose = false;
  int verboseFlagArgIndex = NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME * 2 + 1;
  if (argc > verboseFlagArgIndex && string(argv[verboseFlagArgIndex]).compare("-v") == 0) {
    verbose = true;
  }
  
  // calculate the hamming distance for each hash
  int dists[NUM_HASHED_FRAMES] = {};
  for (int i = 0; i < NUM_HASHED_FRAMES; ++i) {
    int dist = 64;
    
    for (int j = 0; j < NUM_HASHES_PER_FRAME && dist != 0; ++j) {
      for (int k = 0; k < NUM_HASHES_PER_FRAME && dist != 0; ++k) {
        dist = min(dist, __builtin_popcountll(fingerprintA[i * NUM_HASHES_PER_FRAME + j]^fingerprintB[i * NUM_HASHES_PER_FRAME + k]));
      }
    }
    
    dists[i] = dist;
  }
  
  // add all the distances for the total distance
  int dist = 0;
  for (int i = 0; i < NUM_HASHED_FRAMES; ++i) {
    dist += dists[i];
  }
  
  // turn the distance (0 to 64*NUM_HASHED_FRAMES) into a simularity score (1 to 0)
  // each distance has a range of 0-64 (64 being the number of bits in a 64bit int/long long)
  float sim = 1 - ((float)dist / (64 * NUM_HASHED_FRAMES)); 
  
  if (verbose) {
    for (int i = 0; i < NUM_HASHED_FRAMES; ++i) {
      printf("%i ", dists[i]);
    }
    
    printf("- %i - ", dist);
  }
  
  printf("%f\n", sim);
  */
}