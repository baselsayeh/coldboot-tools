// Iterates through vectors of a given size and Hamming weight
class ErrorVector {
protected:
  int weight;
  int size;
  int *pos;
  bool done;

public:
  ErrorVector(int w, int s);
  ~ErrorVector();

  // Advances to the next vector
  bool Next();

  // ORs vector with the current iteration
  void Apply(uint8_t *vector);

  // Have we iterated through all vectors?
  bool Done() { return done; }
};

// Iterates through vectors of a given size and Hamming weight
// with fixed 1s at positions containing 1s in the specified vector
// (we use this to iterate through unique vectors that could have
// decayed into a particular vector)
class ErrorVectorUnique : public ErrorVector {
  int *zero;
  bool IsZero(uint8_t *v, int p);

public:
  ErrorVectorUnique(int w, int s, uint8_t *v);
  ~ErrorVectorUnique();
  void Apply(uint8_t *vector);
};
