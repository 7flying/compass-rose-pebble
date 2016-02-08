#include <pebble.h>

static Window *s_main_window;

static BitmapLayer *s_bm_layer;
static GBitmap *s_bitmap;


static void tick_handler(struct tm *tick_time, TimeUnits changed)
{
    
}

static void main_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COMPASS_ROSE);
    
    GPoint center = grect_center_point(&bounds);
    GSize image_size = gbitmap_get_bounds(s_bitmap).size;

    GRect image_frame = GRect(center.x, center.y, image_size.w, image_size.h);
    image_frame.origin.x -= image_size.w / 2;
    image_frame.origin.y -= image_size.h / 2;

    s_bm_layer = bitmap_layer_create(image_frame);
    bitmap_layer_set_bitmap(s_bm_layer, s_bitmap);
    bitmap_layer_set_compositing_mode(s_bm_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bm_layer));
}

static void main_window_unload(Window *window)
{
    bitmap_layer_destroy(s_bm_layer);
    gbitmap_destroy(s_bitmap);
}

static void init()
{
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
                                   .load = main_window_load,
                                   .unload = main_window_unload
    });
    //window_set_background_color(s_main_window, GColorCadetBlue);
    // window_set_background_color(s_main_window, GColorIcterine);
    window_set_background_color(s_main_window, GColorChromeYellow);
    window_stack_push(s_main_window, true);
}

static void deinit()
{
    window_destroy(s_main_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
