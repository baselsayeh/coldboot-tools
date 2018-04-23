// RSAKeyFinder 1.0 (2008-07-18)
// By Nadia Heninger and J. Alex Halderman

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string>
#include <iostream>

#ifdef __FreeBSD__
#include <err.h>
#else
#define err(x,y) { perror(y); exit(x); }
#endif

using namespace std;

// Baby BER parser, just good enough for RSA keys.
//
// This is not robust to errors in the memory image, but if we added
// some entropy testing and intelligent guessing, it could be made to be.
//
// Parses a single field of the key, beginning at start.  Each field
// consists of a type, a length, and a value.  Puts the type of field
// into type, the number of bytes into len, and returns a pointer to
// the beginning of the value.
unsigned char* ParseNext(unsigned char* start, unsigned int &type, 
			 unsigned int &len) {
  unsigned char *val = NULL;
  type = start[0]; 
  len  = 0;
  if ((start[1] & 0x80) == 0) {
    len = start[1];
    val = &start[2];
  } else {
    int lensize = start[1] & 0x7F;
    for (int i=0; i < lensize; i++)
      len = (len << 8) | start[2+i];    
    val = &start[2+lensize];
  }
  return val;
}

// Sets output to a string displaying len bytes from buffer
void OutputBytes(unsigned char* buffer, int len, string& output) {
  for (int i=0; i<len; i++) {
    char tmp[4];
    snprintf(tmp, sizeof(tmp), "%02x ",buffer[i]);
    output += tmp;
    if ((i < len-1) && i % 16 == 15) output += "\n";
  }
  output += "\n";
}

// Field names in private and public keys
string private_fields[10] = {"version", "modulus", "publicExponent",
			     "privateExponent","prime1","prime2",
			     "exponent1", "exponent2","coefficient", ""};
string public_fields[3] = {"modulus", "publicExponent", ""};

// Sets output to a string listing the fields from a BER-encoded
// record, beginning at start, with the fields named according to the
// array of strings (returns true iff a valid encoding was found)
bool PrintFields(unsigned char* start, string* fields, string &output) {
  unsigned int len=0, type;
  output = "";
  start = ParseNext(start,type,len); // skip sequence field
  for (; *fields != ""; fields++) {
    start = ParseNext(start,type,len);
    if (start == NULL || len == 0 || len > 1000)
      return false;
    output +=  *fields + " = \n";
    OutputBytes(start,len,output);
    start += len;
  }

  return true;
}

// Returns a pointer to the beginning of a BER-encoded key by working
// backwards from the given memory map offset, looking for the
// sequence identifier (this is not completely safe)
unsigned char *FindKeyStart(unsigned char *map, int offset) {
  for (int k = offset; k >= 0 && k > offset-20; k--)
    if (map[k] == 0x30)
      return &map[k];
  return NULL;
}

// Finds and prints private (or private and public) keys in the memory
// map by searching for given target pattern
void FindKeys(unsigned char *image, int isize, unsigned char *target, int target_size, bool find_public) {
  for (int i = 0; i < isize - target_size; i++) {
    if (memcmp(&image[i], target, target_size))
      continue;   

    unsigned char *key = FindKeyStart(image, i);
    if (!key)
      continue;

    string output;
    if (PrintFields(key,private_fields,output)) {
      printf("FOUND PRIVATE KEY AT %x\n", (unsigned int)(key-image));
      cout << output << "\n";
    }  else if (find_public &&
		PrintFields(key,public_fields,output)) {
      printf("FOUND PUBLIC KEY AT %x\n", (unsigned int)(key-image));
      cout << output << "\n";
    }
  }
}

// Memory maps filename and return a pointer on success, setting len
// to the length of the file (does not return on error)
unsigned char *MapFile(char *filename, unsigned int &len) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0)
    err(1, "image open failed");

  struct stat st;
  if (fstat(fd, &st) != 0)
    err(1, "image fstat failed");

  unsigned char *map;
  map = (unsigned char*)mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
    err(1, "image mmap failed");

  len = st.st_size;
  return map;
}

// Returns a decoded byte from a file of hex values, ignoring whitespace
int GetHexByte(int fd) {
  for (;;) {
    char a[3];
    if (read(fd, &a[0], 1) < 1)
      break;
    if ((a[0] >= '0' && a[0] <= '9') || (a[0] >= 'a' && a[0] <= 'f')) {
      if (read(fd, &a[1], 1) < 1)
	break;
      a[2] = '\0';
      return strtol(a,NULL,16);
    }
  }
  return -1;
}

// Reads hexadecimal bytes from filename and returns a byte array
// containing these values, setting len to the number of bytes (does not
// return on error)
unsigned char *ReadModulus(char *filename, unsigned int &len) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0)
    err(1, "modulus open failed");

  struct stat st;
  if (fstat(fd, &st) != 0)
    err(1, "modulus fstat failed");

  unsigned char *modulus = new unsigned char[st.st_size];
  for (len=0; ;len++) {
    int c = GetHexByte(fd);
    if (c == -1)
      break;
    modulus[len] = c;
  }
  
  close(fd);
  return modulus;
}

void Usage() {
  fprintf(stderr, "USAGE: rsakeyfind MEMORY-IMAGE [MODULUS-FILE]\n"
	  "Locates BER-encoded RSA private keys in MEMORY-IMAGE.\n"
	  "If MODULUS-FILE is specified, it will locate private and public keys"
	  "matching the hex-encoded modulus read from this file.\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    Usage();
    exit(1);
  }

  unsigned int ilen;
  unsigned char *image = MapFile(argv[1], ilen); 
  if (argc == 3) {
    // method 1: searching for modulus
    unsigned int mlen;
    unsigned char *modulus = ReadModulus(argv[2], mlen);
    FindKeys(image, ilen, modulus, mlen, true);
  } else {
    // method 2: searching for versionmarker
    unsigned char versionmarker[4] = {0x02, 0x01, 0x00, 0x02};
    FindKeys(image, ilen, versionmarker, sizeof(versionmarker), false);
  }

  return 0;
}
