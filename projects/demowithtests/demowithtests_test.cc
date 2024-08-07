#include "demowithtests.h"
#include "tinytest.h"
#include <cstdio>
#include <stdio.h>

static void test_foo() { ASSERT("fib(10) != 55", fibonacci_number(10) == 55); }
static void test_bar() { ASSERT("oh no 1 != 2", 1 == 2); }

static void setup() {
  // No setup required
}

int main(void) {
  setup();

  RUN(test_foo);
  RUN(test_bar);

  return TEST_REPORT();
}