
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "src/rgba.h"

/*
 * Test named colors.
 */


void show_ptr(short ok) {
  printf("Value: %d\n", ok);
  printf("Parse status: %s\n", ok ? "OK" : "FAILED");
  assert(ok);
}

void show(int expected, int actual) {
  printf("Expected: %d, Actual: %d", expected, actual);
  if (expected == actual) {
      printf(" - PASS\n");
  } else {
      printf(" - FAIL\n");
  }
  assert(expected == actual);
}


void
test_named() {
  short ok;
  int32_t val = rgba_from_string("olive", &ok);
  show_ptr(ok);
  show(0x808000FF, val);
}

/*
 * Test rgb()
 */

void
test_rgb() {
  short ok;
  int32_t val = rgba_from_string("rgb(255, 30   , 0)", &ok);
  show_ptr(ok);
  show(0xff1e00ff, val);

  val = rgba_from_string("rgb(0,0,0)", &ok);
  show_ptr(ok);
  show(0x000000ff, val);
}

/*
 * Test rgba()
 */

void
test_rgba() {
  short ok;
  int32_t val = rgba_from_string("rgba(255, 30   , 0, .5)", &ok);
  show_ptr(ok);
  show(0xff1e007f, val);

  val = rgba_from_string("rgba(0,0,0, 1)", &ok);
  show_ptr(ok);
  show(0x000000ff, val);
}

/*
 * Test #rgb and #rrggbb
 */

void
test_hex() {
  short ok;
  int32_t val = rgba_from_string("#ff1e00", &ok);
  show_ptr(ok);
  show(0xff1e00ff, val);

  val = rgba_from_string("#ffffff", &ok);
  show_ptr(ok);
  show(0xffffffff, val);

  val = rgba_from_string("#ffcc00", &ok);
  show_ptr(ok);
  show(0xffcc00ff, val);

  val = rgba_from_string("#fco", &ok);
  show_ptr(ok);
  show(0xffcc00ff, val);
}

/*
 * Test to string.
 */

void
test_to_string() {
  char buf[256];
  rgba_t color = rgba_new(0xffcc00ff);
  rgba_to_string(color, buf, 256);
  show(0, strcmp("#ffcc00", buf));

  color = rgba_new(0xffcc0050);
  rgba_to_string(color, buf, 256);
  show(0, strcmp("rgba(255, 204, 0, 0.31)", buf));
}

/*
 * Run tests.
 */

int
main(int argc, char **argv){
    if (argc != 2) {
        printf("Error: Invalid test number\n");
      return 1;
    }
    
    int test_num = atoi(argv[1]);
    
    switch (test_num) {
      case 1:
        printf("Running test 1: Named colors\n");
        test_named();
        break;
      case 2:
        printf("Running test 2: RGB format\n");
        test_rgb();
        break;
      case 3:
        printf("Running test 3: RGBA format\n");
        test_rgba();
        break;
      case 4:
        printf("Running test 4: Hex format\n");
        test_hex();
        break;
      case 5:
        printf("Running test 5: To string conversion\n");
        test_to_string();
        break;
      default:
        printf("Error: Invalid test number %d\n", test_num);
    }
    return 0;
  }
