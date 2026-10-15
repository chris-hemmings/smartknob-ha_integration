#pragma once

#if SK_DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

#include "logger.h"
#include "proto_gen/smartknob.pb.h"
#include "task.h"
#include "lvgl.h"
#include "input_type.h"

#define DISP_BUF_SIZE (TFT_WIDTH * 220) // Larger buffer for LVGL allows for more stable FPS - if memory is a concern buffer size can be reduced at the cost of FPS 

class DisplayTask : public Task<DisplayTask> {
    friend class Task<DisplayTask>; // Allow base Task to invoke protected run()

    public:
        DisplayTask(const uint8_t task_core);
        ~DisplayTask();

        QueueHandle_t getKnobStateQueue();
        SemaphoreHandle_t * i2c_mutex_;

        void setListener(QueueHandle_t queue);
        void publish(userInput_t user_input);
        void flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
        void setBrightness(uint16_t brightness);
        void setLogger(Logger* logger);
        void setI2CMutex(SemaphoreHandle_t * mutex);
        void applyViewConfig(const PB_SmartKnobConfig& config);
        static void button_event_cb(lv_event_t * event);

    protected:
        void run();

    private:
        lv_obj_t * screen;
        
        QueueHandle_t knob_state_queue_;

        QueueHandle_t listener_;
        PB_SmartKnobState state_ = {};
        PB_SmartKnobState latest_state_;
        PB_SmartKnobConfig pending_view_config_ = {};
        bool pending_view_config_valid_ = false;
        SemaphoreHandle_t mutex_;
        uint16_t brightness_;
        Logger* logger_;
        void log(const char* msg);
        void clear_screen();
};

#else

class DisplayTask {};

#endif
