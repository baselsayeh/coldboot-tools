// AESFix 1.0.1 (2008-07-18)
// By Nadia Heninger and J. Alex Halderman

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

using namespace std;

#ifdef __FreeBSD__
#include <err.h>
#else
#define err(x,y) { perror(y); exit(x); }
#endif

#include "aes.h"
#include "errvect.h"

#define ROUND_SIZE 16 // Bytes in an AES round
#define ROUNDS 11     // Rounds in an AES-128 key schedule
#define SCHED_LEN (ROUND_SIZE * ROUNDS) // Bytes in an AES-128 key schedule
#define SLICES 4      // Number of slices used in decoding
#define SLICE_SIZE 7  // Bytes per slice

// AES-128 key schedule
union Sched {
  uint8_t byte[ROUNDS*ROUND_SIZE];
  union {
    union {
      uint8_t byte[4];
      uint32_t all;
    } word[ROUND_SIZE/4];
  } round[ROUNDS];
  union {
    uint8_t byte[4];
    uint32_t all;
  } word[ROUNDS*ROUND_SIZE/4];
};

// Key schedule slice used for decoding
union Slice {
  uint8_t byte[SLICE_SIZE];
  uint32_t word;
  uint64_t qword;
};

typedef std::vector<Slice> SliceVector;


// Tests whether codeword could have decayed to vector via
// unidirectional bit decay of 1->0 (i.e. that all the bits in vector
// that are 1 are also 1 in codeword) and returns the opposite
inline bool InvalidDecay(uint64_t vector, uint64_t codeword) {
  return codeword ^ (vector | codeword);
}
inline bool InvalidDecay(uint32_t vector, uint32_t codeword) {
  return codeword ^ (vector | codeword);
}
inline bool InvalidDecay(uint8_t vector, uint8_t codeword) {
  return codeword ^ (vector | codeword);
}

// Prints a key schedule
void PrintSched(Sched &sched) {
  for (int r=0; r < ROUNDS; r++) {
    for (int w=0; w < ROUND_SIZE/4; w++) {
      for (int b=0; b < 4; b++)
	printf("%02X", sched.round[r].word[w].byte[b]);
      printf(" ");
    }
    printf("\n");
  }
  printf("\n");
}

// Places the necessary bytes back in the key schedule in slice-position s
// (it wouldn't hurt to do all the bytes, but this is faster)
void inline SmushSched(Sched &sched, Slice &slice, int s)
{
  sched.word[3].byte[(s+1)&3] = slice.byte[3];
  sched.word[4].byte[s] = slice.byte[4];
  sched.word[5].byte[s] = slice.byte[5];
  sched.word[6].byte[s] = slice.byte[6];
}

// Places bytes of round 0 back in the key schedule in slice-position s
void SmushKey(Sched &sched, Slice &slice, int s)
{
  sched.word[0].byte[s] = slice.byte[0];
  sched.word[1].byte[s] = slice.byte[1];
  sched.word[2].byte[s] = slice.byte[2];
  sched.word[3].byte[(s+1)&3] = slice.byte[3];
}

// Expands candidate key schedule and checks whether the original key
// schedule could have been formed from it by unidirectional bit decay
inline bool TestDecoding(Sched &original, Sched &candidate) {
  candidate.round[1].word[3].all = candidate.round[1].word[2].all ^ candidate.round[0].word[3].all;
  if (InvalidDecay(original.round[1].word[3].all, candidate.round[1].word[3].all))
    return false;

  for (int r = 2; r < ROUNDS; r++) {
    candidate.round[r].word[0].byte[0] = 
      sbox[candidate.round[r-1].word[3].byte[1]] ^ candidate.round[r-1].word[0].byte[0] ^ rcon[r];
    candidate.round[r].word[0].byte[1] = sbox[candidate.round[r-1].word[3].byte[2]] ^ candidate.round[r-1].word[0].byte[1];
    candidate.round[r].word[0].byte[2] = sbox[candidate.round[r-1].word[3].byte[3]] ^ candidate.round[r-1].word[0].byte[2];
    candidate.round[r].word[0].byte[3] = sbox[candidate.round[r-1].word[3].byte[0]] ^ candidate.round[r-1].word[0].byte[3];
    if (InvalidDecay(original.round[r].word[0].all, candidate.round[r].word[0].all))
      return false;
    
    for (int w = 1;  w < ROUND_SIZE/4; w++) {
      candidate.round[r].word[w].all = candidate.round[r].word[w-1].all ^ candidate.round[r-1].word[w].all;
      if (InvalidDecay(original.round[r].word[w].all, candidate.round[r].word[w].all))
	return false;
    }
  }

  return true;
}

// Expands slices a and b to form a single byte,
// round[1].word[3].byte[s], and returns true if it could not have
// decayed into the corresponding byte of the original key schedule.
// (This test is a performance optimization, and may be skipped.)
inline bool TestDecodeByte(Sched &original, int s, Slice &a, Slice &b) {
	return InvalidDecay(original.round[1].word[3].byte[s], a.byte[6] ^ b.byte[3]);
}

// Tests all combinations of current decodings that have the slice
// cand in slice-position s; prints the corrected key schedule if
// a combination passes the test
void CombineDecodings(Sched &original, 
		      SliceVector decodings[SLICES], 
		      Slice &cand, int s) {
	int a, b, c;
	switch (s) {
	  case 0: a = 3; b = 2; c = 1; break;
	  case 1: a = 0; b = 3; c = 2; break;
	  case 2: a = 1; b = 0; c = 3; break;
	  default: a = 2; b = 1; c = 0;
	}

	Sched sched;
	SmushSched(sched, cand, s);
	SmushKey(sched, cand, s);
	
	SliceVector &A = decodings[a];
	SliceVector &B = decodings[b];
	SliceVector &C = decodings[c];

	for (int i=A.size()-1; i >= 0; i--) {	
		if (TestDecodeByte(original, s, cand, A[i])) continue;
		SmushSched(sched, A[i], (s+3)&3);
		for (int j=B.size()-1; j >= 0; j--) {			
			if (TestDecodeByte(original, (s+3)&3, A[i], B[j])) continue;
			SmushSched(sched, B[j], (s+6)&3);
			for (int k=C.size()-1; k >= 0; k--) {
				if (TestDecodeByte(original, (s+6)&3, B[j], C[k])) continue;
				if (TestDecodeByte(original, (s+9)&3, C[k], cand)) continue;
				SmushSched(sched, C[k], (s+9)&3);
				if (TestDecoding(original, sched)) {
					SmushKey(sched, A[i], a);
					SmushKey(sched, B[j], b);
					SmushKey(sched, C[k], c);
					fprintf(stderr, "\ncorrected key schedule:\n");
					PrintSched(sched);
					exit(1);
				}
			}
		}
	}
}

// Tests all combinations of current decodings with at least one
// element from newDecodings
void TryNewDecodings(Sched &original, 
		     SliceVector oldDecodings[4],
		     SliceVector newDecodings[4]) {
  long long unsigned int p1=1, p2=1;
  for (int i=0; i < SLICES; i++) {
    p1 *= oldDecodings[i].size() + newDecodings[i].size();
    p2 *= oldDecodings[i].size();
  }
  fprintf(stderr, "%llu possibilities\n", p1-p2);

  for (int s=0; s < SLICES; s++) {
    for (unsigned int i=0; i < newDecodings[s].size(); i++) {
      CombineDecodings(original, oldDecodings, newDecodings[s][i], s);
      oldDecodings[s].push_back(newDecodings[s][i]);
    }
  }
}

// Expands the first 4 bytes of slice n into 7 bytes
void SliceExpand(Slice &s, int n)
{
  s.byte[4] = sbox[s.byte[3]] ^ s.byte[0];
  if (n == 0)
    s.byte[4] ^= rcon[1];
  s.byte[5] = s.byte[1] ^ s.byte[4];
  s.byte[6] = s.byte[2] ^ s.byte[5];    
}

// Generates all legal decodings for slices with a given weight
void DecodeSlices(Slice slice[SLICES], int weight,
		  SliceVector decodings[4])
{ 
  fprintf(stderr, "decoding for weight %d ... ", weight);

  for (int i=0; i < SLICES; i++) {
    ErrorVectorUnique ev(weight, 32, slice[i].byte);
    while (!ev.Done()) {
      Slice code = slice[i];
      ev.Apply(code.byte);
      SliceExpand(code,i);
      if (!InvalidDecay(slice[i].qword, code.qword))
	decodings[i].push_back(code);
      ev.Next();      
    }
  }

  fprintf(stderr, "%lu new slices ... ",
	  decodings[0].size() + decodings[1].size() + 
	  decodings[2].size() + decodings[3].size());
}

// Slices a key schedule into 4 groups of 7 bytes, each of which is
// uniquely defined by its first 4 bytes
void SliceSched(Sched &sched, Slice slices[4])
{
  for (int w=0; w < SLICE_SIZE; w++) {
    slices[0].byte[w] = sched.word[w].byte[0];
    slices[1].byte[w] = sched.word[w].byte[1];
    slices[2].byte[w] = sched.word[w].byte[2];
    slices[3].byte[w] = sched.word[w].byte[3];
  }

  slices[0].byte[3] = sched.word[3].byte[1];
  slices[1].byte[3] = sched.word[3].byte[2];
  slices[2].byte[3] = sched.word[3].byte[3];
  slices[3].byte[3] = sched.word[3].byte[0];
}

// Returns a decoded byte from a file of hex values, ignoring whitespace
int GetHexByte(FILE *f) {
  for (;;) {
    char a[3];
    if ((a[0] = fgetc(f)) == EOF)
      break;
    if ((a[0] >= '0' && a[0] <= '9') || 
	(a[0] >= 'a' && a[0] <= 'f') || 
	(a[0] >= 'A' && a[0] <= 'F')) {
      if ((a[1] = fgetc(f)) == EOF)
	break;
      a[2] = '\0';
      return strtol(a,NULL,16);
    }
  }
  return EOF;
}

// Reads hex-encoded bytes of a key schedule from filename
// into sched (does not return on error)
void ReadSched(char *filename, Sched &sched) {
  FILE *f = fopen(filename, "r");
  if (!f)
    err(1, "key schedule open failed");

  for (int i=0; i < SCHED_LEN; i++) {
    int c = GetHexByte(f);
    if (c == EOF) {
      fprintf(stderr, "error reading key schedule\n");
      exit(1);
    }
    sched.byte[i] = c;
  }

  fclose(f);
}

void Usage() {
  fprintf(stderr, "usage: aesfix SCHEDULE-FILE\n"
	  "Corrects bit errors in an AES key schedule "
	  "read from the specified hex-encoded file.\n\n");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    Usage();
    exit(1);
  }

  Sched sched;
  ReadSched(argv[1], sched);

  Slice slices[SLICES];
  SliceSched(sched, slices);

  SliceVector oldDecodings[SLICES];
  for (int weight=0; weight < 32; weight++) {
    SliceVector newDecodings[SLICES];
    DecodeSlices(slices, weight, newDecodings);
    TryNewDecodings(sched, oldDecodings, newDecodings);
  }

  return 0;
}
