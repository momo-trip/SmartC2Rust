
//
// test.c
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"

void
equal(char *a, char *b) {
  //   if (strcmp(a, b)) {
  printf("\n");
  printf("  expected: '%s'\n", a);
  printf("    actual: '%s'\n", b);
  printf("\n");
  //     exit(1);
  //   }
}

// added
void
custom_equal(int a, int b) {
  printf("\n");
  printf("  expected: '%d'\n", a);
  printf("    actual: '%d'\n", b);
  printf("\n");
}

void
custom_equal_ptr(void *a, void *b) {
  printf("\n");
  printf("  expected: %p\n", a);
  printf("    actual: %p\n", b);
  printf("\n");
}
// added

void
test_buffer_new() {
  buffer_t *buf = buffer_new();
  custom_equal(BUFFER_DEFAULT_SIZE, buffer_size(buf));
  custom_equal(0, buffer_length(buf));
  buffer_free(buf);
}

void
test_buffer_new_with_size() {
  buffer_t *buf = buffer_new_with_size(1024);
  custom_equal(1024, buffer_size(buf));
  custom_equal(0, buffer_length(buf));
  buffer_free(buf);
}

void
test_buffer_append() {
  buffer_t *buf = buffer_new();
  custom_equal(0, buffer_append(buf, "Hello"));
  custom_equal(0, buffer_append(buf, " World"));
  custom_equal(strlen("Hello World"), buffer_length(buf));
  equal("Hello World", buffer_string(buf));
  buffer_free(buf);
}

void
test_buffer_append_n() {
  buffer_t *buf = buffer_new();
  custom_equal(0, buffer_append_n(buf, "subway", 3));
  custom_equal(0, buffer_append_n(buf, "marines", 6));
  custom_equal(strlen("submarine"), buffer_length(buf));
  equal("submarine", buffer_string(buf));
  buffer_free(buf);
}

void
test_buffer_append__grow() {
  buffer_t *buf = buffer_new_with_size(10);
  custom_equal(0, buffer_append(buf, "Hello"));
  custom_equal(0, buffer_append(buf, " tobi"));
  custom_equal(0, buffer_append(buf, " was"));
  custom_equal(0, buffer_append(buf, " here"));

  char *str = "Hello tobi was here";
  equal(str, buffer_string(buf));
  custom_equal(1024, buffer_size(buf));
  custom_equal(strlen(str), buffer_length(buf));
  buffer_free(buf);
}

void
test_buffer_prepend() {
  buffer_t *buf = buffer_new();
  custom_equal(0, buffer_append(buf, " World"));
  custom_equal(0, buffer_prepend(buf, "Hello"));
  custom_equal(strlen("Hello World"), buffer_length(buf));
  equal("Hello World", buffer_string(buf));
  buffer_free(buf);
}

void
test_buffer_slice() {
  buffer_t *buf = buffer_new();
  buffer_append(buf, "Tobi Ferret");

  buffer_t *a = buffer_slice(buf, 2, 8);
  equal("Tobi Ferret", buffer_string(buf));
  equal("bi Fer", buffer_string(a));

  buffer_free(buf);
  buffer_free(a);
}

void
test_buffer_slice__range_error() {
  buffer_t *buf = buffer_new_with_copy("Tobi Ferret");
  buffer_t *a = buffer_slice(buf, 10, 2);
  custom_equal_ptr(NULL, a);
  buffer_free(buf);
}

void
test_buffer_slice__end() {
  buffer_t *buf = buffer_new_with_copy("Tobi Ferret");

  buffer_t *a = buffer_slice(buf, 5, -1);
  equal("Tobi Ferret", buffer_string(buf));
  equal("Ferret", buffer_string(a));

  buffer_t *b = buffer_slice(buf, 5, -3);
  equal("Ferr", buffer_string(b));

  buffer_t *c = buffer_slice(buf, 8, -1);
  equal("ret", buffer_string(c));

  buffer_free(buf);
  buffer_free(a);
  buffer_free(b);
  buffer_free(c);
}

void
test_buffer_slice__end_overflow() {
  buffer_t *buf = buffer_new_with_copy("Tobi Ferret");
  buffer_t *a = buffer_slice(buf, 5, 1000);
  equal("Tobi Ferret", buffer_string(buf));
  equal("Ferret", buffer_string(a));
  buffer_free(a);
  buffer_free(buf);
}

void
test_buffer_equals() {
  buffer_t *a = buffer_new_with_copy("Hello");
  buffer_t *b = buffer_new_with_copy("Hello");

  custom_equal(1, buffer_equals(a, b));

  buffer_append(b, " World");
  custom_equal(0, buffer_equals(a, b));

  buffer_free(a);
  buffer_free(b);
}

void test_buffer_formatting() {
  buffer_t *buf = buffer_new();
  int result = buffer_appendf(buf, "%d %s", 3, "cow");
  custom_equal(0, result);
  equal("3 cow", buffer_string(buf));
  result = buffer_appendf(buf, " - 0x%08X", 0xdeadbeef);
  custom_equal(0, result);
  equal("3 cow - 0xDEADBEEF", buffer_string(buf));
  buffer_free(buf);
}

void
test_buffer_indexof() {
  buffer_t *buf = buffer_new_with_copy("Tobi is a ferret");

  ssize_t i = buffer_indexof(buf, "is");
  custom_equal(5, i);

  i = buffer_indexof(buf, "a");
  custom_equal(8, i);

  i = buffer_indexof(buf, "something");
  custom_equal(-1, i);

  buffer_free(buf);
}

void
test_buffer_fill() {
  buffer_t *buf = buffer_new_with_copy("Hello");
  custom_equal(5, buffer_length(buf));

  buffer_fill(buf, 0);
  custom_equal(0, buffer_length(buf));
  buffer_free(buf);
}

void
test_buffer_clear() {
  buffer_t *buf = buffer_new_with_copy("Hello");
  custom_equal(5, buffer_length(buf));

  buffer_clear(buf);
  custom_equal(0, buffer_length(buf));
  buffer_free(buf);
}

void
test_buffer_trim() {
  buffer_t *buf = buffer_new_with_copy("  Hello\n\n ");
  buffer_trim(buf);
  equal("Hello", buffer_string(buf));
  buffer_free(buf);

  buf = buffer_new_with_copy("  Hello\n\n ");
  buffer_trim_left(buf);
  equal("Hello\n\n ", buffer_string(buf));
  buffer_free(buf);

  buf = buffer_new_with_copy("  Hello\n\n ");
  buffer_trim_right(buf);
  equal("  Hello", buffer_string(buf));
  buffer_free(buf);
}

void
test_buffer_compact() {
  buffer_t *buf = buffer_new_with_copy("  Hello\n\n ");
  buffer_trim(buf);
  custom_equal(5, buffer_length(buf));
  custom_equal(10, buffer_size(buf));

  ssize_t removed = buffer_compact(buf);
  custom_equal(5, removed);
  custom_equal(5, buffer_length(buf));
  custom_equal(5, buffer_size(buf));
  equal("Hello", buffer_string(buf));

  buffer_free(buf);
}

int
main(int argc, char *argv[]) {
    if (argc > 1) {
        int test_num = atoi(argv[1]);
        switch (test_num) {
            case 1:
                printf("[1/17] Testing test_buffer_new()\n");
                test_buffer_new();
                break;
            case 2:
                printf("[2/17] Testing test_buffer_new_with_size()\n");
                test_buffer_new_with_size();
                break;
            case 3:
                printf("[3/17] Testing test_buffer_append()\n");
                test_buffer_append();
                break;
            case 4:
                printf("[4/17] Testing test_buffer_append__grow()\n");
                test_buffer_append__grow();
                break;
            case 5:
                printf("[5/17] Testing test_buffer_append_n()\n");
                test_buffer_append_n();
                break;
            case 6:
                printf("[6/17] Testing test_buffer_prepend()\n");
                test_buffer_prepend();
                break;
            case 7:
                printf("[7/17] Testing test_buffer_slice()\n");
                test_buffer_slice();
                break;
            case 8:
                printf("[8/17] Testing test_buffer_slice__range_error()\n");
                test_buffer_slice__range_error();
                break;
            case 9:
                printf("[9/17] Testing test_buffer_slice__end()\n");
                test_buffer_slice__end();
                break;
            case 10:
                printf("[10/17] Testing test_buffer_slice__end_overflow()\n");
                test_buffer_slice__end_overflow();
                break;
            case 11:
                printf("[11/17] Testing test_buffer_equals()\n");
                test_buffer_equals();
                break;
            case 12:
                printf("[12/17] Testing test_buffer_formatting()\n");
                test_buffer_formatting();
                break;
            case 13:
                printf("[13/17] Testing test_buffer_indexof()\n");
                test_buffer_indexof();
                break;
            case 14:
                printf("[14/17] Testing test_buffer_fill()\n");
                test_buffer_fill();
                break;
            case 15:
                printf("[15/17] Testing test_buffer_clear()\n");
                test_buffer_clear();
                break;
            case 16:
                printf("[16/17] Testing test_buffer_trim()\n");
                test_buffer_trim();
                break;
            case 17:
                printf("[17/17] Testing test_buffer_compact()\n");
                test_buffer_compact();
                break;
            default:
                printf("Invalid test number\n");
                return 1;
        }
    } else {
        printf("Please provide a test number as an argument.\n");
    }
}