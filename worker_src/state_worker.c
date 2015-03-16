#include <pebble_worker.h>

#define WORKER_DUMMY 0
#define WORKER_WRIST 1
#define WORKER_BT 2

#define PERSIST_KEY 99

static void tap_handler(AccelAxisType axis, int32_t direction) {
    persist_write_int(PERSIST_KEY, WORKER_WRIST);
    worker_launch_app();
}

static void bt_handler(bool connected) {
    persist_write_int(PERSIST_KEY, WORKER_BT);
    worker_launch_app();
}

static void worker_init() {
    accel_tap_service_subscribe(tap_handler);
    bluetooth_connection_service_subscribe(bt_handler);
}

static void worker_deinit() {
    accel_tap_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
}

int main(void) {
    worker_init();
    worker_event_loop();
    worker_deinit();
}
