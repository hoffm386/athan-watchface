#include <pebble.h>

static Window *s_main_window;
static Layer *sun_layer;
int second;
int minute;
int hour;

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  second = tick_time->tm_sec;
  minute = tick_time->tm_min;
  hour = tick_time->tm_hour;
  
  layer_mark_dirty(sun_layer);
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

static void sun_layer_update_callback(Layer *layer, GContext *ctx) {
  const GRect rect = GRect(70, 70, 40, 40);

  draw_circle(ctx, rect, GColorBlack, 20, 360);
}

static void main_window_load(Window *window) {
  sun_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(sun_layer, sun_layer_update_callback);
  layer_add_child(window_get_root_layer(window), sun_layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(sun_layer);
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