/* Minimal test harness: CHECK(cond), CHECK_INT(a,b), CHECK_NEAR(a,b,eps); finish with `return test_report();` */
#ifndef TEST_H_
#define TEST_H_
#include <math.h>
#include <stdio.h>

static int g_checks, g_failures;

#define CHECK(cond)                                                          \
  do {                                                                       \
    g_checks++;                                                              \
    if (!(cond)) {                                                           \
      g_failures++;                                                          \
      printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);               \
    }                                                                        \
  } while (0)

#define CHECK_INT(a, b)                                                      \
  do {                                                                       \
    long va_ = (long)(a), vb_ = (long)(b);                                   \
    g_checks++;                                                              \
    if (va_ != vb_) {                                                        \
      g_failures++;                                                          \
      printf("  FAIL %s:%d: %s == %s (got %ld, expected %ld)\n", __FILE__,   \
             __LINE__, #a, #b, va_, vb_);                                    \
    }                                                                        \
  } while (0)

#define CHECK_NEAR(a, b, eps)                                                \
  do {                                                                       \
    double va_ = (a), vb_ = (b);                                             \
    g_checks++;                                                              \
    if (fabs(va_ - vb_) > (eps)) {                                           \
      g_failures++;                                                          \
      printf("  FAIL %s:%d: %s ~ %s (got %f, expected %f)\n", __FILE__,      \
             __LINE__, #a, #b, va_, vb_);                                    \
    }                                                                        \
  } while (0)

#define RUN(fn)            \
  do {                     \
    printf("  %s\n", #fn); \
    fn();                  \
  } while (0)

static int test_report(void) {
  printf("%d checks, %d failures\n", g_checks, g_failures);
  return g_failures ? 1 : 0;
}
#endif
