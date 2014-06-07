#include "pebble.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static Window *window;
static Layer *path_layer;
static Window *home_window;
static TextLayer *text_layer;
static Window *finished_window;
static TextLayer *finished_text_layer;
static TextLayer *finished_text_layer2;
static TextLayer *finished_text_layer3;

static BitmapLayer *image_layer;
static GBitmap *image;

// This defines graphics path information to be loaded as a path later
static const GPathInfo RIGHT_ARROW_PATH_POINTS = {
    // This is the amount of points
    7,
    // A path can be concave, but it should not twist on itself
    // The points should be defined in clockwise order due to the rendering
    // implementation. Counter-clockwise will work in older firmwares, but
    // it is not officially supported
    (GPoint []) {
        {0,12},
        {12,0},
        {0,-12},
        {0,-5},
        {-12,-5},
        {-12,5},
        {0,5}
    }
};
// This defines graphics path information to be loaded as a path later
static const GPathInfo LEFT_ARROW_PATH_POINTS = {
    // This is the amount of points
    7,
    // A path can be concave, but it should not twist on itself
    // The points should be defined in clockwise order due to the rendering
    // implementation. Counter-clockwise will work in older firmwares, but
    // it is not officially supported
    (GPoint []) {
        {0,-12},
        {-12,0},
        {0,12},
        {0,5},
        {12,5},
        {12,-5},
        {0,-5}
    }
};
// This defines graphics path information to be loaded as a path later
static const GPathInfo UP_ARROW_PATH_POINTS = {
    // This is the amount of points
    7,
    // A path can be concave, but it should not twist on itself
    // The points should be defined in clockwise order due to the rendering
    // implementation. Counter-clockwise will work in older firmwares, but
    // it is not officially supported
    (GPoint []) {
        {0,-12},
        {-12,0},
        {-5,0},
        {-5,12},
        {5,12},
        {5,0},
        {12,0}
    }
};
// This defines graphics path information to be loaded as a path later
static const GPathInfo DOWN_ARROW_PATH_POINTS = {
    // This is the amount of points
    7,
    // A path can be concave, but it should not twist on itself
    // The points should be defined in clockwise order due to the rendering
    // implementation. Counter-clockwise will work in older firmwares, but
    // it is not officially supported
    (GPoint []) {
        {0,12},
        {12,0},
        {5,0},
        {5,-12},
        {-5,-12},
        {-5,0},
        {-12,0}
    }
};

static GPath *right_arrow_path;
static GPath *left_arrow_path;
static GPath *up_arrow_path;
static GPath *down_arrow_path;

static GPath *right_arrow_path2;
static GPath *left_arrow_path2;
static GPath *up_arrow_path2;
static GPath *down_arrow_path2;

static GPath *right_arrow_bottom;
static GPath *left_arrow_bottom;
static GPath *up_arrow_bottom;
static GPath *down_arrow_bottom;

static int speed = 5; // pixels per 1/10 second
static int beat_offset = 1000; // time until next arrow comes out
int score = 0; // Score to display
int multiplier = 1; // multiplier for score

#define NUM_GRAPHIC_PATHS 4
static int PATH_WIDTH = 36;
static GPath *graphic_paths[NUM_GRAPHIC_PATHS];
static GPath *graphic_paths2[NUM_GRAPHIC_PATHS];
static GPath *current_path = NULL;
static GPath *current_path2 = NULL;
static int current_path_index = 0;
static int current_path_index2 = 0;
static int path_height = 0;
static int path_height2 = -72;

static AppTimer *timer;
static AppTimer *gametimer;
char c[8];
char status[20];

static int path_height_add(int pxl) {
    return path_height = (path_height + pxl);
}
static int path_height_add2(int pxl) {
    return path_height2 = (path_height2 + pxl);
}
int get_int_len (int value){
    int l=1;
    while(value>9){ l++; value/=10; }
    return l;
}
static void gametimer_callback(void *data) {
    window_stack_pop(true);
}
static void timer_callback(void *data) {
    path_height_add(speed);
    path_height_add2(speed);
    layer_mark_dirty(path_layer);
    timer = app_timer_register(50 /* milliseconds */, timer_callback, NULL);
}
static void evaluate_click(struct GPath * arrow_path){
    if(current_path == arrow_path){
        int current_path_height = path_height;
        
        if(current_path_height>112 && current_path_height<116){
            char new_status[9] = "PERFECT!";
            for(int i=0;i<9;i++){
                status[i] = new_status[i];
            }
            score += (10 * multiplier);
            multiplier += 1;
        }
        else if(current_path_height>108 && current_path_height<125){
            char new_status[9] = "GOOD!";
            for(int i=0;i<9;i++){
                status[i] = new_status[i];
            }
            score += (5 * multiplier);
            multiplier += 1;
        }
        else if (current_path_height < 108){
            char new_status[7] = "EARLY!";
            for(int i=0;i<7;i++){
                status[i] = new_status[i];
            }
            multiplier = 0;
        }
        else if (current_path_height > 125){
            char new_status[6] = "LATE!";
            for(int i=0;i<6;i++){
                status[i] = new_status[i];
            }
            multiplier = 0;
        }
    }
    else if(current_path2 == arrow_path){
        int current_path_height2 = path_height2;
        
        if(current_path_height2>112 && current_path_height2<116){
            char new_status[9] = "PERFECT!";
            for(int i=0;i<9;i++){
                status[i] = new_status[i];
            }
            score += (10 * multiplier);
            multiplier += 1;
        }
        else if(current_path_height2>108 && current_path_height2<125){
            char new_status[9] = "GOOD!";
            for(int i=0;i<9;i++){
                status[i] = new_status[i];
            }
            score += (5 * multiplier);
            multiplier += 1;
        }
        else if (current_path_height2 < 108){
            char new_status[7] = "EARLY!";
            for(int i=0;i<7;i++){
                status[i] = new_status[i];
            }
            multiplier = 0;
        }
        else if (current_path_height2 > 125){
            char new_status[6] = "LATE!";
            for(int i=0;i<6;i++){
                status[i] = new_status[i];
            }
            multiplier = 0;
        }
    }
    else {
        char new_status[7] = "WRONG!";
        for(int i=0;i<7;i++){
            status[i] = new_status[i];
        }
        multiplier = 0;
    }

}
// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *me, GContext *ctx) {
  (void)me;
    // check if arrow leaves bottom of screen
    if(path_height > 168){
        path_height = 0;
        srand(time(NULL));
        current_path_index = rand() % 4;
        current_path = graphic_paths[current_path_index];
        memset(&status[0], 0, sizeof(status));
    }
    if(path_height2 > 168){
        path_height2 = 0;
        srand(time(NULL));
        current_path_index2 = rand() % 4;
        current_path2 = graphic_paths2[current_path_index2];
        memset(&status[0], 0, sizeof(status));
    }

    if (current_path_index == 0) {
        gpath_move_to(current_path,GPoint(PATH_WIDTH*1-4, path_height));
    }
    else if (current_path_index == 1){
        gpath_move_to(current_path,GPoint(PATH_WIDTH*2-13, path_height));
    }
    else if (current_path_index == 2){
        gpath_move_to(current_path,GPoint(PATH_WIDTH*3-23, path_height));
    }
    else if (current_path_index == 3){
        gpath_move_to(current_path,GPoint(PATH_WIDTH*4-32, path_height));
    }
    
    
    if (current_path_index2 == 0) {
        gpath_move_to(current_path2,GPoint(PATH_WIDTH*1-4, path_height2));
    }
    else if (current_path_index2 == 1){
        gpath_move_to(current_path2,GPoint(PATH_WIDTH*2-13, path_height2));
    }
    else if (current_path_index2 == 2){
        gpath_move_to(current_path2,GPoint(PATH_WIDTH*3-23, path_height2));
    }
    else if (current_path_index2 == 3){
        gpath_move_to(current_path2,GPoint(PATH_WIDTH*4-32, path_height2));
    }

    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, current_path);
    gpath_draw_filled(ctx, current_path2);
    // draw bottom arrows
    graphics_context_set_stroke_color(ctx, GColorWhite);
    gpath_draw_outline(ctx, left_arrow_bottom);
    gpath_draw_outline(ctx, down_arrow_bottom);
    gpath_draw_outline(ctx, up_arrow_bottom);
    gpath_draw_outline(ctx, right_arrow_bottom);
    gpath_move_to(left_arrow_bottom, GPoint(PATH_WIDTH*1-4,115));
    gpath_move_to(down_arrow_bottom, GPoint(PATH_WIDTH*2-13,115));
    gpath_move_to(up_arrow_bottom, GPoint(PATH_WIDTH*3-23,115));
    gpath_move_to(right_arrow_bottom, GPoint(PATH_WIDTH*4-32,115));

    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx,
                       status,
                       fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                       GRect(0, 130, 144, 25),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
    
    int nDigits = get_int_len(score);
    int temp_score = score;
    for(int i=0;i<nDigits;i++){
        int index = nDigits - i - 1;
        c[index] = (char)(((int)'0')+(temp_score % 10));
        temp_score /= 10;
    }
    graphics_draw_text(ctx,
                       c,
                       fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                       GRect(0, 135, 144, 25),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentRight,
                       NULL);
}


static void down_handler(ClickRecognizerRef recognizer, void *context) {
    // Show the outline of the path when select is held down
    layer_mark_dirty(path_layer);
    if(path_height>path_height2){
        evaluate_click(down_arrow_path);
    }
    else {
        evaluate_click(down_arrow_path2);
    }
}

static void up_handler(ClickRecognizerRef recognizer, void *context) {
    // Show the outline of the path when select is held down
    layer_mark_dirty(path_layer);
    if(path_height>path_height2){
        evaluate_click(up_arrow_path);
    }
    else {
        evaluate_click(up_arrow_path2);
    }
}

static void select_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the outline of the path when select is held down
  layer_mark_dirty(path_layer);
    if(path_height>path_height2){
        evaluate_click(right_arrow_path);
    }
    else {
        evaluate_click(right_arrow_path2);
    }
}

static void back_handler(ClickRecognizerRef recognizer, void *context) {
    // Show the outline of the path when select is held down
    layer_mark_dirty(path_layer);
    if(path_height>path_height2){
        evaluate_click(left_arrow_path);
    }
    else {
        evaluate_click(left_arrow_path2);
    }
}

static void config_provider(void *context) {
//    window_set_click_context(BUTTON_ID_BACK, override_ctx);
   window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
   window_single_click_subscribe(BUTTON_ID_UP, up_handler);
   window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
   window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
}


static void home_up(ClickRecognizerRef recognizer, void *context){
    
}
static void home_down(ClickRecognizerRef recognizer, void *context){
    
}
static void home_select(ClickRecognizerRef recognizer, void *context){
    window_stack_pop(true);
    //    window_stack_push(window, true);
    gametimer = app_timer_register(30000 /* milliseconds */, gametimer_callback, NULL);
    app_timer_cancel(timer);
    timer = app_timer_register(50 /* milliseconds */, timer_callback, NULL);
}
static void home_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, home_up);
    window_single_click_subscribe(BUTTON_ID_DOWN, home_down);
    window_single_click_subscribe(BUTTON_ID_SELECT, home_select);
}


static void finished_up(ClickRecognizerRef recognizer, void *context){
    
}
static void finished_down(ClickRecognizerRef recognizer, void *context){
    
}
static void finished_select(ClickRecognizerRef recognizer, void *context){
    window_stack_pop(true);
    window_stack_push(finished_window, true);
    window_stack_push(window,true);
    window_stack_push(home_window, true);
    score = 0;
    memset(&c[0], 0, sizeof(c));
    path_height = -72;
    path_height2 = -144;
    multiplier = 0;
}
static void finished_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, finished_up);
    window_single_click_subscribe(BUTTON_ID_DOWN, finished_down);
    window_single_click_subscribe(BUTTON_ID_SELECT, finished_select);
}





static void init() {
    
    
    finished_window = window_create();
    window_stack_push(finished_window, true /* Animated */);
    window_set_background_color(finished_window, GColorWhite);
    
    window_set_click_config_provider(finished_window, finished_config_provider);
    
    Layer *finished_window_layer = window_get_root_layer(finished_window);
    GRect finished_bounds = layer_get_frame(finished_window_layer);
    GRect finished_bounds_half1 = GRect(0,20,finished_bounds.size.w,finished_bounds.size.h/4);
    finished_text_layer = text_layer_create(finished_bounds_half1);
    text_layer_set_text(finished_text_layer, "FINISHED GAME");
    text_layer_set_font(finished_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(finished_text_layer, GTextAlignmentCenter);
    layer_add_child(finished_window_layer, text_layer_get_layer(finished_text_layer));
    
    GRect finished_bounds_half2 = GRect(0,84,finished_bounds.size.w,finished_bounds.size.h/8);
    finished_text_layer2 = text_layer_create(finished_bounds_half2);
    text_layer_set_text(finished_text_layer2, "YOUR SCORE");
    text_layer_set_font(finished_text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(finished_text_layer2, GTextAlignmentCenter);
    layer_add_child(finished_window_layer, text_layer_get_layer(finished_text_layer2));
    
    GRect finished_bounds_half3 = GRect(0,105,finished_bounds.size.w,finished_bounds.size.h/4);
    finished_text_layer3 = text_layer_create(finished_bounds_half3);
    text_layer_set_text(finished_text_layer3, c);
    text_layer_set_font(finished_text_layer3, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(finished_text_layer3, GTextAlignmentCenter);
    layer_add_child(finished_window_layer, text_layer_get_layer(finished_text_layer3));
    
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  window_set_click_config_provider(window, config_provider);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  path_layer = layer_create(bounds);
  layer_set_update_proc(path_layer, path_layer_update_callback);
  layer_add_child(window_layer, path_layer);

    // Pass the corresponding GPathInfo to initialize a GPath
    right_arrow_path = gpath_create(&RIGHT_ARROW_PATH_POINTS);
    left_arrow_path = gpath_create(&LEFT_ARROW_PATH_POINTS);
    up_arrow_path = gpath_create(&UP_ARROW_PATH_POINTS);
    down_arrow_path = gpath_create(&DOWN_ARROW_PATH_POINTS);
    // Pass the corresponding GPathInfo to initialize a GPath
    right_arrow_path2 = gpath_create(&RIGHT_ARROW_PATH_POINTS);
    left_arrow_path2 = gpath_create(&LEFT_ARROW_PATH_POINTS);
    up_arrow_path2 = gpath_create(&UP_ARROW_PATH_POINTS);
    down_arrow_path2 = gpath_create(&DOWN_ARROW_PATH_POINTS);
    // Pass the corresponding GPathInfo to initialize a GPath
    right_arrow_bottom = gpath_create(&RIGHT_ARROW_PATH_POINTS);
    left_arrow_bottom = gpath_create(&LEFT_ARROW_PATH_POINTS);
    up_arrow_bottom = gpath_create(&UP_ARROW_PATH_POINTS);
    down_arrow_bottom = gpath_create(&DOWN_ARROW_PATH_POINTS);
    
  graphic_paths[3] = right_arrow_path;
  graphic_paths[0] = left_arrow_path;
  graphic_paths[2] = up_arrow_path;
  graphic_paths[1] = down_arrow_path;
    
  current_path = graphic_paths[0];
    
    graphic_paths2[3] = right_arrow_path2;
    graphic_paths2[0] = left_arrow_path2;
    graphic_paths2[2] = up_arrow_path2;
    graphic_paths2[1] = down_arrow_path2;
    
    current_path2 = graphic_paths2[0];
    
    home_window = window_create();
    window_stack_push(home_window, true /* Animated */);
    window_set_background_color(home_window, GColorWhite);
    
    window_set_click_config_provider(home_window, home_config_provider);
    
    Layer *home_window_layer = window_get_root_layer(home_window);
    GRect home_bounds = layer_get_frame(home_window_layer);
    GRect home_bounds_top = GRect(0,15,home_bounds.size.w,home_bounds.size.h * 7/8);
    GRect home_bounds_bottom = GRect(0,0,home_bounds.size.w,home_bounds.size.h / 8);
    // This needs to be deinited on app exit which is when the event loop ends
    image = gbitmap_create_with_resource(RESOURCE_ID_DDR_BG);
    // The bitmap layer holds the image for display
    image_layer = bitmap_layer_create(home_bounds_top);
    bitmap_layer_set_bitmap(image_layer, image);
    bitmap_layer_set_alignment(image_layer, GAlignCenter);
    layer_add_child(home_window_layer, bitmap_layer_get_layer(image_layer));

    
    text_layer = text_layer_create(home_bounds_bottom);
    text_layer_set_text(text_layer, "TAP TO BEGIN");
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(home_window_layer, text_layer_get_layer(text_layer));
}

static void deinit() {
    gbitmap_destroy(image);
    bitmap_layer_destroy(image_layer);
    
    gpath_destroy(right_arrow_path);
    gpath_destroy(left_arrow_path);
    gpath_destroy(up_arrow_path);
    gpath_destroy(down_arrow_path);

    gpath_destroy(right_arrow_path2);
    gpath_destroy(left_arrow_path2);
    gpath_destroy(up_arrow_path2);
    gpath_destroy(down_arrow_path2);
    
  layer_destroy(path_layer);
  window_destroy(window);
    
}





int main(void) {
  init();
  light_enable(true);
  // A timer can be canceled with `app_timer_cancel()`
  app_event_loop();

  deinit();
}
