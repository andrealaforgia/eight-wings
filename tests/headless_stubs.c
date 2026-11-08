/* animate.c also holds wrap_animate(), which refers to two window-size helpers of the engine's graphics layer.
   The game never calls it; these stubs let the tests link the engine's motion code without any window code. */
#include "graphics.h"

double wrap_x(const graphics_context_ptr graphics_context, double x) {
  (void)graphics_context;
  return x;
}

double wrap_y(const graphics_context_ptr graphics_context, double y) {
  (void)graphics_context;
  return y;
}
