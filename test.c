#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <x86intrin.h>

int main(int argc, char** argv) {
  char* needle = argv[1];
  int nlen = strlen(needle);
  char* haystack = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCHELLO23456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
  int hlen = strlen(haystack);
  __m128i needle16 = _mm_loadu_si128((const __m128i *)needle);
  int off = 0;
  int pos = 0;
  char* buf = haystack;
  while (off < hlen) {
    __m128i haystack16 = _mm_loadu_si128((const __m128i *)buf);
    int r = _mm_cmpestri(needle16, nlen, haystack16, 16, _SIDD_CMP_EQUAL_ORDERED);
    if (r < (16 - nlen)) {
      pos = off + r;
      break;
    }
    off += 16 - nlen;
    buf += 16 - nlen;
  }
  if (pos > 0) {
    char* str = calloc(1, nlen);
    memcpy(str, haystack + pos, nlen);
    fprintf(stderr, "pos: %i, %s\n", pos, str);
  }
  return 0;
}