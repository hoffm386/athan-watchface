#include <pebble.h>

static Window *s_main_window;
static Layer *sun_layer;
static Layer *night_layer;
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
  const GRect entire_screen = GRect(0, 0, 180, 180);
  const GRect sun_rect = GRect(65, 65, 50, 50);


  draw_circle(ctx, entire_screen, GColorCeleste, 90, 360);
  draw_circle(ctx, sun_rect, GColorOrange, 25, 360);


  graphics_context_set_stroke_color(ctx, GColorOrange);
  graphics_context_set_stroke_width(ctx, 1);

  int i;
  for (i = 6; i < 360; i += 12) {
    const GPoint in = gpoint_from_polar(
      sun_rect,
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(i)
    );
    const GPoint out = gpoint_from_polar(
      entire_screen,
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(i)
    );
    graphics_draw_line(ctx, out, in);
  }

  graphics_context_set_stroke_color(ctx, GColorChromeYellow);
  graphics_context_set_stroke_width(ctx, 2);

  for (i = 0; i < 360; i += 12) {
    const GPoint in = gpoint_from_polar(
      sun_rect,
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(i)
    );
    const GPoint out = gpoint_from_polar(
      entire_screen,
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(i)
    );
    graphics_draw_line(ctx, out, in);
  }
}

static void night_layer_update_callback(Layer *layer, GContext *ctx) {
  const GRect entire_screen = GRect(0, 0, 180, 180);

  int hour_rise = 7;
  int minute_rise = 13;
  int hour_set = 18;
  int minute_set = 40;

  int diff_rise = ((hour_rise * 60) + minute_rise) * 360 / 1440;
  int diff_set = ((hour_set * 60) + minute_set) * 360 / 1440;

  int degree_rise = (diff_rise + 180) % 360;
  int degree_set = (diff_set + 180) % 360;

  graphics_context_set_fill_color(ctx, GColorOxfordBlue);
  graphics_fill_radial(
    ctx, entire_screen, 
    GOvalScaleModeFillCircle,
    90,
    DEG_TO_TRIGANGLE(degree_set),
    DEG_TO_TRIGANGLE(degree_rise)
  );
}










static void main_window_load(Window *window) {
  sun_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(sun_layer, sun_layer_update_callback);

  night_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(night_layer, night_layer_update_callback);

  layer_add_child(window_get_root_layer(window), sun_layer);
  layer_add_child(window_get_root_layer(window), night_layer);
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

  APP_LOG(APP_LOG_LEVEL_INFO, "Logging a thing from c");
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