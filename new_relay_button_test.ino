/* Using LVGL with Arduino + SquareLine Studio UI
 * ESP32‑S3 + Waveshare 2‑inch LCD (ST7789)
 */

#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "bsp_cst816.h"
#include "ui.h"     // <-- SquareLine Studio UI

// -------------------- PIN DEFINITIONS --------------------
#define LCD_SCLK 39
#define LCD_MOSI 38
#define LCD_MISO 40
#define LCD_DC   42
#define LCD_RST  -1
#define LCD_CS   45
#define LCD_BL    1

#define TP_SDA   48
#define TP_SCL   47

#define RELAY_PIN 15
#define RELAY_PIN_2 4 // <-- New Relay Pin

#define LCD_ROTATION 0
#define LCD_H_RES 240
#define LCD_V_RES 320

#define LEDC_FREQ 5000
#define LEDC_TIMER_10_BIT 10

// -------------------- DISPLAY SETUP --------------------
Arduino_DataBus *bus = new Arduino_ESP32SPI(
  LCD_DC, LCD_CS,
  LCD_SCLK, LCD_MOSI, LCD_MISO
);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, LCD_ROTATION, true,
  LCD_H_RES, LCD_V_RES
);

// -------------------- LVGL BUFFERS --------------------
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;

lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_disp_drv_t disp_drv;

// -------------------- LVGL FLUSH --------------------
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  // LVGL draws into full framebuffer, so nothing to flush here
  lv_disp_flush_ready(disp);
}

// -------------------- TOUCH --------------------
void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  uint16_t x, y;

  bsp_touch_read();
  if (bsp_touch_get_coordinates(&x, &y)) {
    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// -------------------- RELAY CALLBACK --------------------
extern "C" void relay_toggle_cb(lv_event_t * e) {
  static bool relay_state = false;
  relay_state = !relay_state;
  digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
  Serial.print("Touch Button Tapped! Relay is now: ");
  Serial.println(relay_state ? "ON" : "OFF");
}
extern "C" void relay_toggle_cb_2(lv_event_t * e) {
  static bool relay_state_2 = false;
  relay_state_2 = !relay_state_2;
  digitalWrite(RELAY_PIN_2, relay_state_2 ? HIGH : LOW);
  Serial.print("Button 2 Tapped! Relay 2 is now: ");
  Serial.println(relay_state_2 ? "ON" : "OFF");
}
// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting LVGL + SquareLine UI");

  // Init relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Set initial state to OFF
  pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_2, LOW); // Set initial state to OFF
  // Init display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() FAILED!");
  }
  gfx->fillScreen(BLACK);

  // Backlight
  ledcAttach(LCD_BL, LEDC_FREQ, LEDC_TIMER_10_BIT);
  ledcWrite(LCD_BL, (1 << LEDC_TIMER_10_BIT) * 0.8);

  // Init touch
  Wire.begin(TP_SDA, TP_SCL);
  bsp_touch_init(&Wire, gfx->getRotation(), gfx->width(), gfx->height());

  // Init LVGL
  lv_init();

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  bufSize = screenWidth * screenHeight;

  // Allocate full framebuffer
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(
    bufSize * sizeof(lv_color_t),
    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
  );

  if (!disp_draw_buf) {
    Serial.println("Framebuffer alloc failed!");
    while (1);
  }

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

  // Register display driver
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.direct_mode = true;
  lv_disp_drv_register(&disp_drv);

  // Register touch driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // -------------------- LOAD SQUARELINE UI --------------------
  ui_init();

  Serial.println("Setup complete");
}

// -------------------- LOOP --------------------
void loop() {
  lv_timer_handler();

  // Animate the gauge
  static uint32_t last_update = 0;
  static int current_temp = 0;
  static int step = 2; // Increase this to make it move faster

  if (millis() - last_update > 30) { // Updates every 30ms for smooth animation
    last_update = millis();

    current_temp += step;
    
    // Reverse direction at the limits
    if (current_temp >= 100) {
      current_temp = 100;
      step = -step; 
    } else if (current_temp <= 0) {
      current_temp = 0;
      step = -step; 
    }

    lv_arc_set_value(ui_uiTempGauge, current_temp);
    lv_label_set_text_fmt(ui_uiTempValue, "%d", current_temp);
  }

  // Push full framebuffer to screen
#if LV_COLOR_16_SWAP
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#else
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif

  delay(5);
}