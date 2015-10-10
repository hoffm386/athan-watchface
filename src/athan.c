#include <pebble.h>

static Window *main_window;

static Layer *offscreen_layer;
static Layer *sun_layer;
static BitmapLayer *night_layer;
static TextLayer *time_layer;
static Layer *prayer_layer;
static Layer *ring_layer;

static GBitmap *stars;
static GBitmap *sun;
static GBitmap *moon;

int second;
int minute;
int hour;

int hour_rise;
int minute_rise;
int hour_set;
int minute_set;

enum {
  KEY_SUNRISE_HOUR = 0,
  KEY_SUNRISE_MINUTE = 1,
  KEY_SUNSET_HOUR = 2,
  KEY_SUNSET_MINUTE = 3,
  KEY_PRAYER_TIMES = 4
};

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  second = tick_time->tm_sec;
  minute = tick_time->tm_min;
  hour = tick_time->tm_hour;

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, buffer);
  layer_mark_dirty(ring_layer);
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

static int degreeify(int hour, int minute) {
  int diff = ((hour * 60) + minute) * 360 / 1440;
  int degree = (diff + 180) % 360;
  return degree;
}

static void sun_layer_update(Layer *layer, GContext *ctx) {
  const GRect entire_screen = GRect(0, 0, 180, 180);
  const GRect sun_outline_rect = GRect(70, 70, 40, 40);
  const GRect sun_rect = GRect(72, 72, 36, 36);

  draw_circle(ctx, entire_screen, GColorVividCerulean, 90, 360);

  graphics_context_set_stroke_color(ctx, GColorChromeYellow);
  graphics_context_set_stroke_width(ctx, 2);

  int i;
  for (i = 0; i < 360; i += 12) {
    const GPoint in = gpoint_from_polar(
      sun_outline_rect,
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

  draw_circle(ctx, sun_outline_rect, GColorWindsorTan, 20, 360);
  draw_circle(ctx, sun_rect, GColorOrange, 18, 360);
}

static void offscreen_layer_update(Layer* layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw the night slice
  const GRect entire_screen = GRect(0, 0, 180, 180);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(
    ctx, entire_screen, 
    GOvalScaleModeFillCircle,
    90,
    DEG_TO_TRIGANGLE(degreeify(hour_set, minute_set)),
    DEG_TO_TRIGANGLE(degreeify(hour_rise, minute_rise))
  );

  // Capture the graphics context framebuffer
  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);

  bitmap_make_transparent(stars, framebuffer);

  // Release the framebuffer now that we are free to modify it
  graphics_release_frame_buffer(ctx, framebuffer);
}

static void prayer_layer_update(Layer *layer, GContext *ctx) {
  const GRect time_orbit = GRect(10, 10, 160, 160);

  int degree_rise = degreeify(hour_rise, minute_rise);
  int degree_set = degreeify(hour_set, minute_set);

  int hour_fajr = 6;
  int minute_fajr = 0;
  int hour_dhuhr = 12;
  int minute_dhuhr = 56;
  int hour_asr = 16;
  int minute_asr = 11;
  int hour_maghrib = 18;
  int minute_maghrib = 46;
  int hour_isha = 19;
  int minute_isha = 53;

  int degrees[5] = {
    degreeify(hour_fajr, minute_fajr),
    degreeify(hour_dhuhr, minute_dhuhr),
    degreeify(hour_asr, minute_asr),
    degreeify(hour_maghrib, minute_maghrib),
    degreeify(hour_isha, minute_isha)
  };

  int i;
  for (i = 0; i < 5; i++) {
    graphics_context_set_fill_color(
      ctx,
      degrees[i] >= degree_set && degrees[i] <= degree_rise ? GColorOxfordBlue : GColorVividCerulean
    );

    const GRect space = grect_centered_from_polar(
      time_orbit,
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(degrees[i]),
      GSize(6, 6)
    );

    graphics_fill_radial(
      ctx, space, 
      GOvalScaleModeFillCircle,
      3,
      DEG_TO_TRIGANGLE(0),
      TRIG_MAX_ANGLE
    ); 
  }

  graphics_context_set_fill_color(ctx, GColorOxfordBlue);
  
   
}

static void ring_layer_update(Layer *layer, GContext *ctx) {
  const GRect entire_screen = GRect(0, 0, 180, 180);
  draw_circle(ctx, entire_screen, GColorWhite, 20, 360);
  graphics_context_set_stroke_color(ctx, GColorOxfordBlue);
  graphics_context_set_stroke_width(ctx, 10);

  const GRect time_orbit = GRect(10, 10, 160, 160);

  int degree_icon = degreeify(hour, minute);
  int degree_rise = degreeify(hour_rise, minute_rise);
  int degree_set = degreeify(hour_set, minute_set);
  
  GBitmap *icon = degree_icon >= degree_set && degree_icon <= degree_rise ? moon : sun;

  const GRect icon_space = grect_centered_from_polar(
    time_orbit,
    GOvalScaleModeFitCircle,
    DEG_TO_TRIGANGLE(degree_icon),
    GSize(18, 18)
  );

  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, icon, icon_space);
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

  prayer_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(prayer_layer, prayer_layer_update);

  sun = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUN);
  moon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOON);
  ring_layer = layer_create(GRect(0, 0, 180, 180));
  layer_set_update_proc(ring_layer, ring_layer_update);
  
  time_layer = text_layer_create(GRect(62, 130, 56, 25));
  text_layer_set_background_color(time_layer, GColorOxfordBlue);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), offscreen_layer);
  layer_add_child(window_get_root_layer(window), sun_layer);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(night_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  layer_add_child(window_get_root_layer(window), ring_layer);
  layer_add_child(window_get_root_layer(window), prayer_layer);
}
static void main_window_unload(Window *window) {
  layer_destroy(offscreen_layer);
  layer_destroy(sun_layer);
  bitmap_layer_destroy(night_layer);
  text_layer_destroy(time_layer);
  layer_destroy(prayer_layer);
  layer_destroy(ring_layer);
  gbitmap_destroy(stars);
  gbitmap_destroy(sun);
  gbitmap_destroy(moon);
}











static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_SUNRISE_HOUR:
        hour_rise = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO, "C code received sunrise hour %d", hour_rise);
        break;
      case KEY_SUNRISE_MINUTE:
        minute_rise = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO, "C code received sunrise minute %d", minute_rise);
        break;
      case KEY_SUNSET_HOUR:
        hour_set = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO, "C code received sunset hour %d", hour_set);
        break;
      case KEY_SUNSET_MINUTE:
        minute_set = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO, "C code received sunset hour %d", minute_set);
        break;
      case KEY_PRAYER_TIMES:
        APP_LOG(APP_LOG_LEVEL_INFO, "C code received prayer times %s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  layer_mark_dirty(sun_layer);
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
  main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

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
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}