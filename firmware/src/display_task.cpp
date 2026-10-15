#if SK_DISPLAY
#include "display_task.h"
#include "semaphore_guard.h"
#include "CST816D.h"
#include "views/view.h"
#include "views/styles.h"
#include "views/circle_menu_view.h"
#include "views/list_menu_view.h"
#include "views/dial_view.h"
#include "views/message_view.h"
#include "views/home_view.h"

static const uint8_t LEDC_CHANNEL_LCD_BACKLIGHT = 0;

TFT_eSPI tft_ = TFT_eSPI();

static lv_disp_draw_buf_t disp_buf;

static DisplayTask* display_task_;

#if SK_TOUCH
CST816D touch(PIN_SDA, PIN_SCL, PIN_TP_RST, PIN_TP_INT);

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

  bool touched;
  uint8_t gesture;
  uint16_t touchX, touchY;

  SemaphoreGuard lock(*display_task_->i2c_mutex_);
  touched = touch.getTouch(&touchX, &touchY, &gesture);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}
#endif

DisplayTask::DisplayTask(const uint8_t task_core) : Task{"Display", 8192, 1, task_core} {
  display_task_ = this;
  knob_state_queue_ = xQueueCreate(1, sizeof(PB_SmartKnobState));
  assert(knob_state_queue_ != NULL);

  mutex_ = xSemaphoreCreateMutex();
  assert(mutex_ != NULL);
}

DisplayTask::~DisplayTask() {
  vQueueDelete(knob_state_queue_);
  vSemaphoreDelete(mutex_);
}

void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft_.startWrite();
    tft_.setAddrWindow(area->x1, area->y1, w, h);
    tft_.setSwapBytes(true);
    tft_.pushPixels(&color_p->full, w * h);
    tft_.endWrite();

    lv_disp_flush_ready(disp);
}

void DisplayTask::clear_screen() {
  lv_obj_clean(lv_scr_act());
}

void DisplayTask::button_event_cb(lv_event_t * event)
{
    lv_obj_t * button = lv_event_get_target(event);
    lv_indev_wait_release(lv_indev_get_act());
    int button_index = (int) lv_obj_get_user_data(button);
    display_task_->publish(
      {
        .inputType = INPUT_WITH_DATA,
        .inputData = button_index
      }
    );
}

void gesture_event_cb(lv_event_t * e)
{
  lv_obj_t * screen = lv_event_get_current_target(e);
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
  switch(dir) {
    case LV_DIR_TOP:
      display_task_->publish(
        {
          .inputType = INPUT_BACK,
          .inputData = NULL
        }
      );
      break;
    case LV_DIR_BOTTOM:
      display_task_->publish(
        {
          .inputType = INPUT_FORWARD,
          .inputData = NULL
        }
      );
      break;
    case LV_DIR_RIGHT:
      display_task_->publish(
        {
          .inputType = INPUT_NEXT,
          .inputData = NULL
        }
      );
      break;
    case LV_DIR_LEFT:
      display_task_->publish(
        {
          .inputType = INPUT_PREV,
          .inputData = NULL
        }
      );
      break;
  }
}

void DisplayTask::run() {
    tft_.begin();
    tft_.invertDisplay(1);
    tft_.setRotation(SK_DISPLAY_ROTATION);
    tft_.fillScreen(TFT_BLACK);

    #if SK_TOUCH
    touch.begin();
    #endif

    ledcSetup(LEDC_CHANNEL_LCD_BACKLIGHT, 5000, 12);
    ledcAttachPin(PIN_LCD_BACKLIGHT, LEDC_CHANNEL_LCD_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, UINT16_MAX);

    lv_init();
    lv_color_t *buf = (lv_color_t*) heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf);
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    #if SK_TOUCH
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    #endif

    screen = lv_scr_act();
    lv_obj_add_event_cb(screen, gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);

    init_styles();

    CircleMenuView circleMenuView = CircleMenuView(screen, display_task_);
    ListMenuView listMenuView = ListMenuView(screen, display_task_);
    DialView dialView = DialView(screen, display_task_);
    MessageView messageView = MessageView(screen, display_task_);
    HomeView homeView = HomeView(screen, display_task_);

    View* current_view = &homeView;
    PB_SmartKnobConfig boot_config = {};
    boot_config.has_view_config = true;
    boot_config.view_config.view_type = VIEW_HOME_SCREEN;
    homeView.setupView(boot_config);
    lv_task_handler();

    while(1) {
        PB_SmartKnobState state;
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE) {
          continue;
        }

        state_ = state;

        lv_task_handler(); // Force LVGL to process any pending updates

        int32_t view_type = VIEW_HOME_SCREEN;
        if (state.config.has_view_config) {
            view_type = state.config.view_config.view_type;
        } else if (state.config.max_position < state.config.min_position) {
            // Unbounded rotation config used by the home page
            view_type = VIEW_HOME_SCREEN;
        }

        bool config_change = (latest_state_.config.detent_strength_unit != state.config.detent_strength_unit)
        || (latest_state_.config.endstop_strength_unit != state.config.endstop_strength_unit)
        || (latest_state_.config.min_position != state.config.min_position)
        || (latest_state_.config.max_position != state.config.max_position)
        || (latest_state_.config.view_config.view_type != state.config.view_config.view_type)
        || (latest_state_.config.has_view_config != state.config.has_view_config);

        latest_state_ = state;

        if (config_change) {
          clear_screen();
          log("clearing screen");
          if (view_type == VIEW_CIRCLE_MENU) {
            current_view = &circleMenuView;
            log("viewing circle menu");
          } else if (view_type == VIEW_LIST_MENU) {
            current_view = &listMenuView;
            log("viewing list menu");
          } else if (view_type == VIEW_DIAL) {
            current_view = &dialView;
            log("viewing dial menu");
          } else if (view_type == VIEW_MESSAGE) {
            current_view = &messageView;
            log("viewing message view");
          } else {
            current_view = &homeView;
            log("viewing home screen");
          }
          current_view->setupView(state.config);
        }

        current_view->updateView(state);

        lv_task_handler(); // Ensure UI updates are processed immediately

        {
          SemaphoreGuard lock(mutex_);
          ledcWrite(LEDC_CHANNEL_LCD_BACKLIGHT, brightness_);
        }
        delay(5);
    }
}

QueueHandle_t DisplayTask::getKnobStateQueue() {
  return knob_state_queue_;
}

void DisplayTask::setBrightness(uint16_t brightness) {
  SemaphoreGuard lock(mutex_);
  brightness_ = brightness >> (16 - SK_BACKLIGHT_BIT_DEPTH);

  
}

void DisplayTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void DisplayTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

void DisplayTask::setListener(QueueHandle_t queue) {
    listener_ = queue;
}

void DisplayTask::publish(userInput_t user_input) {
  xQueueOverwrite(listener_, &user_input);
}

void DisplayTask::setI2CMutex(SemaphoreHandle_t * mutex) {
    i2c_mutex_ = mutex;
}

#endif
