#include <pebble.h>

static Window *s_main_window;
static Layer *layer;
int second;
int minute;
int hour;

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  second = tick_time->tm_sec;
  minute = tick_time->tm_min;
  hour = tick_time->tm_hour;
  
  layer_mark_dirty(layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void draw_circle(GContext *ctx, GRect rect, GColor color, int r, int deg) {
  graphics_context_set_fill_color(ctx, color);
  
  graphics_fill_radial(
    ctx, rect, 
    GOvalScaleModeFillCircle,
    r,
    DEG_TO_TRIGANGLE(0),
    deg % 360 == 0 ? TRIG_MAX_ANGLE : DEG_TO_TRIGANGLE(deg)
  );  
}

static void rect_layer_update_callback(Layer *layer, GContext *ctx) {
  const GRect rect = GRect(0, 0, 180, 180);
  const GRect minute_rect = GRect(60, 60, 60, 60);
  const GRect hour_rect = GRect(75, 75, 30, 30);

  draw_circle(ctx, rect, GColorBlack, 90, 360);
  
  GColor colors[6] = {
    GColorRed,
    GColorOrange,
    GColorRajah,
    GColorMediumAquamarine,
    GColorTiffanyBlue,
    GColorMidnightGreen
  };
  
  int i;
  for(i = 0; i < 6; i++) {
    draw_circle(ctx, rect, colors[i], (6 - i) * 10, (second + 1) * 6);
  }
  
  draw_circle(ctx, minute_rect, GColorMidnightGreen, 30, 360);
  draw_circle(ctx, minute_rect, GColorTiffanyBlue, 30, minute * 6);
  draw_circle(ctx, minute_rect, GColorLightGray, 2, 360);
  
  draw_circle(ctx, hour_rect, GColorTiffanyBlue, 15, 360);
  draw_circle(ctx, hour_rect, GColorMidnightGreen, 15, hour % 12 * 30);
  draw_circle(ctx, hour_rect, GColorBlack, 2, 360);
}

static void main_window_load(Window *window) {
  layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(layer, rect_layer_update_callback);
  layer_add_child(window_get_root_layer(window), layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}