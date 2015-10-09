#include <pebble.h>

static Window *s_main_window;
static Layer *offscreen_layer;
static Layer *sun_layer;
static BitmapLayer *night_layer;
static Layer *ring_layer;
static GBitmap *stars;

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










static void bitmap_make_transparent(GBitmap *bitmap, GBitmap *mask) {
  GRect bounds = gbitmap_get_bounds(mask);
  for (int y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; y++) {
    GBitmapDataRowInfo row_info = gbitmap_get_data_row_info(bitmap, y);
    GBitmapDataRowInfo row_info_mask = gbitmap_get_data_row_info(mask, y);
    for (int x = row_info_mask.min_x; x < row_info_mask.max_x; x++) {
      GColor *pixel = (GColor*)&row_info.data[x];
      GColor *pixel_mask = (GColor*)&row_info_mask.data[x];
      if (pixel_mask->r != 0x0) {
        pixel->a = 0x3;
      } else {
        pixel->a = 0x0;
      }
    }
  }
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

static void sun_layer_update(Layer *layer, GContext *ctx) {
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

static void offscreen_layer_update(Layer* layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw the night slice
  const GRect entire_screen = GRect(0, 0, 180, 180);
  int hour_rise = 7;
  int minute_rise = 13;
  int hour_set = 18;
  int minute_set = 40;

  int diff_rise = ((hour_rise * 60) + minute_rise) * 360 / 1440;
  int diff_set = ((hour_set * 60) + minute_set) * 360 / 1440;

  int degree_rise = (diff_rise + 180) % 360;
  int degree_set = (diff_set + 180) % 360;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(
    ctx, entire_screen, 
    GOvalScaleModeFillCircle,
    90,
    DEG_TO_TRIGANGLE(degree_set),
    DEG_TO_TRIGANGLE(degree_rise)
  );

  // Capture the graphics context framebuffer
  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);

  bitmap_make_transparent(stars, framebuffer);

  // Release the framebuffer now that we are free to modify it
  graphics_release_frame_buffer(ctx, framebuffer);
}

static void ring_layer_update(Layer *layer, GContext *ctx) {
  const GRect entire_screen = GRect(0, 0, 180, 180);
  draw_circle(ctx, entire_screen, GColorWhite, 20, 360);
  graphics_context_set_stroke_color(ctx, GColorOxfordBlue);
  graphics_context_set_stroke_width(ctx, 10);

  const GRect time_orbit = GRect(10, 10, 175, 175);
}










static void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack);

  offscreen_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(offscreen_layer, offscreen_layer_update);

  sun_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(sun_layer, sun_layer_update);

  stars = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STARS);
  night_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
  bitmap_layer_set_bitmap(night_layer, stars);
  bitmap_layer_set_compositing_mode(night_layer, GCompOpSet);

  ring_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(ring_layer, ring_layer_update);

  layer_add_child(window_get_root_layer(window), offscreen_layer);
  layer_add_child(window_get_root_layer(window), sun_layer);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(night_layer));
  layer_add_child(window_get_root_layer(window), ring_layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(offscreen_layer);
  layer_destroy(sun_layer);
  bitmap_layer_destroy(night_layer);
  layer_destroy(ring_layer);
  gbitmap_destroy(stars);
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