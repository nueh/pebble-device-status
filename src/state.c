#include <pebble.h>

#define WORKER_DUMMY 0
#define WORKER_WRIST 1
#define WORKER_BT 2

#define PERSIST_KEY 99

#define ALERT_TIME 1


static Window *window;
static Layer *layer;
const bool animated = false;
static AppTimer *app_timer;
static GBitmap *bmp_bt_state;

static void close_app() {
    window_stack_pop_all(animated);
}

GBitmap* bitmap_from_set(GBitmap **set, GSize size, int number) {
    GRect bounds;

    bounds.origin = GPoint(size.w*number, 0);
    bounds.size = size;
    return gbitmap_create_as_sub_bitmap(*set, bounds);
}

void draw_picture(GContext* ctx, GBitmap **sources,
                  GRect bounds, int number) {
    GBitmap *temp = bitmap_from_set(sources, bounds.size, number);
    graphics_draw_bitmap_in_rect(ctx,
                                temp,
                                bounds);
    gbitmap_destroy(temp);
}

static void battery_state(GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(9, 0, 14, 4), 0, 0);
    graphics_fill_rect(ctx, GRect(0, 4, 32, 60), 0, 0);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(3, 8, 26, 52), 0, 0);
    graphics_context_set_fill_color(ctx, GColorBlack);

    BatteryChargeState charge_state = battery_state_service_peek();
    int bat_percent = charge_state.charge_percent/10;
    if (bat_percent < 10) {
        graphics_fill_rect(ctx,
                           GRect(4, 7+(10-bat_percent)*5, 24, 5*bat_percent),
                                 0, 0);
    } else {
        graphics_fill_rect(ctx, GRect(3, 8, 26, 52), 0, 0);
    };
}

static void bluetooth_state(GContext *ctx) {
    vibes_long_pulse();
    draw_picture(ctx, &bmp_bt_state, GRect(0, 10, 32, 32),
                 (int)bluetooth_connection_service_peek());
}

static void update_layer(Layer *layer, GContext *ctx) {
    if (persist_exists(PERSIST_KEY)) {
        int worker_event = persist_read_int(PERSIST_KEY);
        switch (worker_event) {
            case WORKER_WRIST:
                battery_state(ctx);
                break;
            case WORKER_BT:
                bluetooth_state(ctx);
                break;
        };
    } else {
        close_app();
    };
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    layer = layer_create(GRect(56, 52, 32, 64));

    bmp_bt_state = gbitmap_create_with_resource(RESOURCE_ID_BT_STATE);

    layer_add_child(window_layer, layer);
    layer_set_update_proc(layer, update_layer);
}

static void window_unload(Window *window) {
    layer_destroy(layer);
    gbitmap_destroy(bmp_bt_state);
}

static void init(void) {
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    #ifdef PBL_PLATFORM_APLITE
    window_set_fullscreen(window, true);
    #endif
    window_stack_push(window, animated);

    AppLaunchReason app_reason = launch_reason();
    if (app_reason == APP_LAUNCH_WORKER) {
        app_timer = app_timer_register(ALERT_TIME*1000, close_app, NULL);
    } else {
        close_app();
    };
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    deinit();
}
