#include <pebble.h>
#include "compass-rose.h"

static Window *s_main_window;

static BitmapLayer *s_bm_layer;
static GBitmap *s_bitmap;

static Layer *s_canvas_layer;

static Time s_last_time, s_anim_time;
static bool s_animating;


static void tick_handler(struct tm *tick_time, TimeUnits changed)
{
    s_last_time.days = tick_time->tm_mday;
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.minutes = tick_time->tm_min;
    s_last_time.seconds = tick_time->tm_sec;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;

    layer_mark_dirty(s_canvas_layer);
}

static void animation_started(Animation *anim, void *context)
{
    s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context)
{
    s_animating = false;
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void
animate(int duration, int delay, AnimationImplementation *implementation,
        bool handlers)
{
    Animation *anim = animation_create();
    if (anim)
    {
        animation_set_duration(anim, duration);
        animation_set_delay(anim, delay);
        animation_set_curve(anim, AnimationCurveEaseInOut);
        animation_set_implementation(anim, implementation);
        if (handlers)
        {
            animation_set_handlers(anim, (AnimationHandlers) {
                    .started = animation_started,
                        .stopped = animation_stopped
                        }, NULL);
        }
        animation_schedule(anim);
    }
}

static GPoint
make_hand_point(int quantity, int intervals, int len, GPoint center)
{
    GPoint temp;
    temp.x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * quantity / intervals)
                       * (int32_t)len / TRIG_MAX_RATIO) + center.x;
    temp.y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * quantity / intervals)
                       * (int32_t)len / TRIG_MAX_RATIO) + center.y;
    return temp;
}

static int hours_to_minutes(int hours_out_of_12)
{
    return (int)(float)(((float) hours_out_of_12 / 12.0F) * 60.0F);
}

static void draw_proc(Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    Time mode_time = s_animating ? s_anim_time : s_last_time;

    int len_min = HAND_LENGTH_MIN;
    int len_hour = HAND_LENGTH_HOUR;

    // Plot hand ends
    GPoint minute_hand_long = make_hand_point(mode_time.minutes, 60, len_min,
                                              center);
    // Plot shorter overlaid hands
    len_min -= (MARGIN + 2);
    GPoint minute_hand_short = make_hand_point(mode_time.minutes, 60, len_min,
                                               center);

    float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
    float hour_angle;
    if (s_animating)
    {
        // Hours out of 60 for smoothness
        hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
    } else
    {
        hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
    }
    hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

    // Hour is more accurate
    GPoint hour_hand_long = (GPoint) {
        .x = (int16_t)(sin_lookup(hour_angle)
                       * (int32_t)len_hour / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(hour_angle)
                       * (int32_t)len_hour / TRIG_MAX_RATIO) + center.y,
    };

    // Shorter hour overlay
    len_hour -= (MARGIN + 2);
    GPoint hour_hand_short = (GPoint) {
        .x = (int16_t)(sin_lookup(hour_angle)
                       * (int32_t)len_hour / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(hour_angle)
                       * (int32_t)len_hour / TRIG_MAX_RATIO) + center.y,
    };

    // Draw hands
    graphics_context_set_stroke_color(ctx, GColorBlack);
    
    for(int y = 0; y < THICKNESS; y++) {
        for(int x = 0; x < THICKNESS; x++) {
            graphics_draw_line(ctx, GPoint(center.x + x, center.y + y),
                               GPoint(minute_hand_short.x + x,
                                      minute_hand_short.y + y));
            graphics_draw_line(ctx, GPoint(center.x + x, center.y + y),
                               GPoint(hour_hand_short.x + x,
                                      hour_hand_short.y + y));
        }
    }
    // Center
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(center.x + 1, center.y + 1), 5);
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

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, draw_proc);
    layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window)
{
    bitmap_layer_destroy(s_bm_layer);
    gbitmap_destroy(s_bitmap);

    layer_destroy(s_canvas_layer);
}

static int anim_percentage(AnimationProgress dist_normalized, int max)
{
    return (int)(float)(((float) dist_normalized
                         / (float) ANIMATION_NORMALIZED_MAX) * (float) max);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized)
{
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;

    s_anim_time.hours = anim_percentage(dist_normalized,
                                        hours_to_minutes(s_last_time.hours));
    s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);
    s_anim_time.seconds = anim_percentage(dist_normalized, s_last_time.seconds);

    layer_mark_dirty(s_canvas_layer);
}

static void init()
{
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
    });
    window_set_background_color(s_main_window, GColorChromeYellow);
    window_stack_push(s_main_window, true);

    time_t t = time(NULL);
    struct tm *tm_now = localtime(&t);
    s_last_time.hours = tm_now->tm_hour;
    s_last_time.minutes = tm_now->tm_min;
    s_last_time.seconds = tm_now->tm_sec;

    // Begin smooth animation
    static AnimationImplementation hands_impl = {
        .update = hands_update
    };
    animate(ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
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
