int recurser(int start, int stop) {
  if (start >= stop) {
    return start;
  }
  else {
    return recurser(start+1, stop);
  }
}

int iterator(int start, int stop) {
  for (; start <= stop; start++) {
    start++;
  }
  return start;
}

int main() {
  int start = 0;
  int stop = 10;
  int recurser_result = recurser(start, stop);
  int iterator_result = iterator(start, stop);
  return recurser_result + iterator_result;
}
