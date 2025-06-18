#include <zephyr/kernel.h>
#include <zmk/input.h>
#include <zmk/event_manager.h>
#include <zmk/events/sensor_event.h>
#include <zmk/behavior.h>
#include <zmk/layers.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define JIGGLE_THRESHOLD 15
#define JIGGLE_TIME_WINDOW 300 // milliseconds
#define JIGGLE_COUNT_REQUIRED 4
#define GESTURE_LAYER 5

static int64_t last_time = 0;
static int jiggle_count = 0;
static int last_direction = 0;

int gesture_jiggle_process(struct zmk_sensor_event *ev) {
    int x = ev->data.relative.x;

    int dir = (x > JIGGLE_THRESHOLD) - (x < -JIGGLE_THRESHOLD);
    if (dir != 0 && dir != last_direction) {
        int64_t now = k_uptime_get();

        if (jiggle_count == 0) {
            last_time = now;
        }

        if ((now - last_time) < JIGGLE_TIME_WINDOW) {
            jiggle_count++;
            last_direction = dir;

            if (jiggle_count >= JIGGLE_COUNT_REQUIRED) {
                LOG_INF("Jiggle gesture detected!");
                zmk_layer_activate(GESTURE_LAYER);
                jiggle_count = 0;
                return 0;
            }
        } else {
            jiggle_count = 1;
            last_time = now;
            last_direction = dir;
        }
    }

    return 0; // Let other processors continue
}

ZMK_INPUT_DECLARE_PROCESSOR(gesture_jiggle_process);
