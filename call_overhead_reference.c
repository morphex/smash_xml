void recurser(int *start, int *stop) {
  if (*start >= *stop) {
    return;
  }
  else {
    (*start)++;
    recurser(start, stop);
  }
}

void iterator(int *start, int *stop) {
  for (; *start < *stop; (*start)++) {
  }
  return;
}

int main() {
  int start = 0;
  int start2 = 0;
  int stop = 10;
  recurser(&start, &stop);
  iterator(&start2, &stop);
  printf("%i,%i\n", start, start2);
  return start + start2;
}
