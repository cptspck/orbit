#include "simple_analog.h"

#include "pebble.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_num_buffer[4], s_day_buffer[6];

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_draw_filled(ctx, s_tick_paths[i]);
  }
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  //int16_t second_hand_length = 10;
  int16_t minute_hand_length = 24;
  int16_t hour_hand_length = 50;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  int32_t hour_angle = TRIG_MAX_ANGLE * t->tm_hour / 12;
  GPoint hour_hand = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  

  
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  GPoint minute_hand = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + hour_hand.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + hour_hand.y,
  };
  
  
  
 // int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  //GPoint second_hand = {
   // .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + minute_hand.x,
    //.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + minute_hand.y,
 // };

  // second hand
  //graphics_context_set_fill_color(ctx, GColorFromRGB(150, 150, 150));
  //graphics_fill_circle(ctx, second_hand, 3);

  // hour hand
  graphics_context_set_fill_color(ctx, GColorFromRGB(180, 0, 0));
  graphics_fill_circle(ctx, hour_hand, 8);

  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorFromRGB(255, 150, 0));
  graphics_fill_circle(ctx, center, 24);
	
	
	//Guide marks
	graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 225, 225));
	
	GPoint start1;
	GPoint end1;
	GPoint start2;
	GPoint end2;
	
  if(t->tm_min < 15){
		GPoint start1 = {.x = hour_hand.x + 8, .y = hour_hand.y,};
		GPoint end1 = {.x = hour_hand.x + 30, .y = hour_hand.y,};
		
    GPoint start2 = {.x = hour_hand.x, .y = hour_hand.y - 8,};
    GPoint end2 = {.x = hour_hand.x, .y = hour_hand.y - 30,};
		
			graphics_draw_line(ctx, start1, end1);
		graphics_draw_line(ctx, start2, end2);
	 } else if(t->tm_min < 30){
     GPoint start1 = {.x = hour_hand.x + 8, .y = hour_hand.y,};
     GPoint end1 = {.x = hour_hand.x + 30, .y = hour_hand.y,};

		 GPoint start2 = {.x = hour_hand.x, .y = hour_hand.y + 8,};
     GPoint end2 = {.x = hour_hand.x, .y = hour_hand.y + 30,};
		
			graphics_draw_line(ctx, start1, end1);
		graphics_draw_line(ctx, start2, end2);
      } else if(t->tm_min < 45){
     GPoint start1 = {.x = hour_hand.x - 8, .y = hour_hand.y,};
     GPoint end1 = {.x = hour_hand.x - 30, .y = hour_hand.y,};
    
     GPoint start2 = {.x = hour_hand.x, .y = hour_hand.y + 8,};
     GPoint end2 = {.x = hour_hand.x, .y = hour_hand.y + 30,};
		
			graphics_draw_line(ctx, start1, end1);
			graphics_draw_line(ctx, start2, end2);
   }  else {
     GPoint start1 = {.x = hour_hand.x - 8, .y = hour_hand.y,};
     GPoint end1 = {.x = hour_hand.x - 30, .y = hour_hand.y,};
    
     GPoint start2 = {.x = hour_hand.x, .y = hour_hand.y - 8,};
     GPoint end2 = {.x = hour_hand.x, .y = hour_hand.y - 30,};
		
			graphics_draw_line(ctx, start1, end1);
			graphics_draw_line(ctx, start2, end2);
   };
	graphics_draw_line(ctx, start1, end1);
	graphics_draw_line(ctx, start2, end2);
	
	//minute hand
	
	graphics_context_set_fill_color(ctx, GColorFromRGB(0, 125, 200));
  graphics_fill_circle(ctx, minute_hand, 5);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
  text_layer_set_text(s_num_label, s_num_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);

  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loaded background");

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loaded hands layer");

  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loaded date layer");

  s_day_label = text_layer_create(GRect(center.x-22, center.y - 10, 27, 20));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorFromRGB(255, 150, 0));
  text_layer_set_text_color(s_day_label, GColorBlack);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(GRect(center.x+5, center.y - 10, 18, 20));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorFromRGB(255, 150, 0));
  text_layer_set_text_color(s_num_label, GColorBlack);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);

  layer_destroy(s_hands_layer);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  s_day_buffer[0] = '\0';
  s_num_buffer[0] = '\0';

  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
}

static void deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }

  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
