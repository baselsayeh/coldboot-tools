#include <stdint.h>
#include "errvect.h"

ErrorVector::ErrorVector(int w, int s) : weight(w), size(s) {
  pos = new int[weight];
  for (int i=0; i < weight; i++)
    pos[i] = i;
  done = weight > size;
}

ErrorVector::~ErrorVector() {
  if (pos)
    delete [] pos;
}

bool ErrorVector::Next() {
  if (done)
    return false;
  if (!weight) {
    done = true;
    return false;
  }
  if (pos[weight-1] != size-1) {
    pos[weight-1]++;
  } else {
    int mobile = -1;
    for (int i=weight-2; i >= 0; i--)
      if (pos[i]+1 != pos[i+1]) {
	mobile = i;
	break;
      }
    if (mobile < 0) {
      done = true;
      return false;
    }
    pos[mobile]++;
    for (int i=mobile+1; i < weight; i++)
      pos[i] = pos[i-1]+1;
  }
  return true;
}

void ErrorVector::Apply(uint8_t *vector) {
  for (int i=0; i < weight; i++) {
    int p = pos[i];
    vector[p >> 3] |= (1 << (p & 0x7));
  }
}

ErrorVectorUnique::ErrorVectorUnique(int w, int s, uint8_t *v) 
  : ErrorVector(w,s) {
  zero = new int[s];
  int z=0;
  for (int i=0; i < s; i++)
    if (IsZero(v,i))
      zero[z++] = i;
  size = z;
  done = weight > size;
}

ErrorVectorUnique::~ErrorVectorUnique() {
  if (zero)
    delete [] zero;
}

// Is the initialization vector 0 at bit-position p?
bool ErrorVectorUnique::IsZero(uint8_t *v, int p) {
  return (p < size) && !(v[p >> 3] & (1 << (p & 0x7)));
}

void ErrorVectorUnique::Apply(uint8_t *vector) {
    for (int i=0; i < weight; i++) {
      int p = zero[pos[i]];
      vector[p >> 3] |= (1 << (p & 0x7));
    }
  }
