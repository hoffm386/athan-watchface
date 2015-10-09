#include <pebble.h>

static Window *s_main_window;
static Layer *sun_layer;
static Layer *night_layer;
static Layer *ring_layer;
int second;
int minute;
int hour;

enum {
  KEY_SUNRISE = 0,
  KEY_SUNSET = 1,
  KEY_FAJR = 2,
  KEY_DHUHR = 3,
  KEY_ASR = 4,
  KEY_MAGHRIB = 5,
  KEY_ISHA = 6
};

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  second = tick_time->tm_sec;
  minute = tick_time->tm_min;
  hour = tick_time->tm_hour;
  
  //layer_mark_dirty(sun_layer);
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
  const GRect sun_rect = GRect(70, 70, 40, 40);


  draw_circle(ctx, entire_screen, GColorCeleste, 90, 360);
  draw_circle(ctx, sun_rect, GColorOrange, 20, 360);


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

static void ring_layer_update_callback(Layer *layer, GContext *ctx) {
  const GRect entire_screen = GRect(0, 0, 180, 180);
  draw_circle(ctx, entire_screen, GColorWhite, 25, 360);
}










static void main_window_load(Window *window) {
  sun_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(sun_layer, sun_layer_update_callback);

  night_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(night_layer, night_layer_update_callback);

  ring_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(ring_layer, ring_layer_update_callback);

  layer_add_child(window_get_root_layer(window), sun_layer);
  layer_add_child(window_get_root_layer(window), night_layer);
  layer_add_child(window_get_root_layer(window), ring_layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(sun_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_FAJR:
      APP_LOG(APP_LOG_LEVEL_INFO, "C code received Fahr %s", t->value->cstring);
      break;
    case KEY_DHUHR:
      APP_LOG(APP_LOG_LEVEL_INFO, "C code received Dhuhr %s", t->value->cstring);
      break;
    case KEY_ASR:
      APP_LOG(APP_LOG_LEVEL_INFO, "C code received Asr %s", t->value->cstring);
      break;
    case KEY_MAGHRIB:
      APP_LOG(APP_LOG_LEVEL_INFO, "C code received Maghrib %s", t->value->cstring);
      break;
    case KEY_ISHA:
      APP_LOG(APP_LOG_LEVEL_INFO, "C code received Isha %s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  
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