/*
#define volatileAccess(v) *((volatile typeof((v)) *) &(v))
*/

inline int volatileAccess(type, v) {
}*((volatile type *) &(v))

int main() {
  int i = 0;
  type mytype = typeof(i);
  return volatileAccess(i);
}
