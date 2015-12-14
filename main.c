#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static int s_battery_level;
static Layer *s_battery_layer;
static GBitmap *s_heart_bitmap;
static BitmapLayer *s_bitmap_layer;


static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
 layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  //find width of bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * PBL_IF_ROUND_ELSE(150.0F, 125.0F));
  
  //draw background
  graphics_context_set_fill_color(ctx, GColorFolly);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
//get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(15, 9), PBL_IF_ROUND_ELSE(40, 35), PBL_IF_ROUND_ELSE(150, 125), 25));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  //create heart bitmap
  s_heart_bitmap = gbitmap_create_with_resource(PBL_IF_ROUND_ELSE(RESOURCE_ID_IMAGE_HEART_ROUND, RESOURCE_ID_IMAGE_HEART_RECT));
  s_bitmap_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(15, 9), PBL_IF_ROUND_ELSE(40, 35), PBL_IF_ROUND_ELSE(150, 125), 25));
  bitmap_layer_set_bitmap(s_bitmap_layer, s_heart_bitmap);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  
  
  //add heart bitmap as child to window
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));
  
  //create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
  GRect(-5, PBL_IF_ROUND_ELSE(26, 20), bounds.size.w, 100));
  
  //create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEFTONE_80));
  
  //set background color
  window_set_background_color(s_main_window, GColorPictonBlue);
  
  //improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  //add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //create GFont
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AZOFT_SANS_BOLD_ITALIC_20));
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(18, 0), 110, 144, 30));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Add to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

}

static void main_window_unload(Window *window) {
  //Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  
  //destroy bitmap
  gbitmap_destroy(s_heart_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);

  
  //unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);

}

static void init() {
//create main window element and assign to pointer
  s_main_window = window_create();
  
  //set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  //show the window on the watch, with animated = true
  window_stack_push(s_main_window, true);
  
  //make sure time displayed from start
  update_time();
  
  //register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());


}

static void deinit() {
//Destroy Window
  window_destroy(s_main_window);
  
  layer_destroy(s_battery_layer);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
