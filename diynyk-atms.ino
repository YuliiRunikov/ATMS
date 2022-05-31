#include <lvgl.h>

#include <TFT_eSPI.h>

#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#define LVGL_TICK_PERIOD 60

#define EMMISIVITY 0.95
#define INTERPOLATE false

#define C_BLUE 0x0000ff
#define C_RED 0xff0000
#define C_GREEN 0x00ff00
#define C_WHITE 0xffffff
#define C_BLACK 0x000000
#define C_LTGREY 0xc8c8c8
#define C_DKGREY 0x505050
#define C_GREY 0x7f7f7f
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define CANVAS_HEIGHT 24
#define CANVAS_WIDTH 32
// start with some initial colors
float minTemp = 20.0;
float maxTemp = 40.0;

byte red, green, blue;

float a, b, c, d;
int x, y, i;

// array for the 32 x 24 measured tempValues
static float tempValues[CANVAS_WIDTH * CANVAS_HEIGHT];
static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

long getColor(float val)
{
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed simple linear equations
    again a 5-6-5 color display will not need accurate temp to R G B color calculation
    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html
  */

  red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

  if ((val > minTemp) & (val < a))
  {
    green = constrain(255.0 / (a - minTemp) * val - (255.0 * minTemp) / (a - minTemp), 0, 255);
  }
  else if ((val >= a) & (val <= c))
  {
    green = 255;
  }
  else if (val > c)
  {
    green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
  }
  else if ((val > d) | (val < a))
  {
    green = 0;
  }

  if (val <= b)
  {
    blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
  }
  else if ((val > b) & (val <= d))
  {
    blue = 0;
  }
  else if (val > d)
  {
    blue = constrain(240.0 / (maxTemp - d) * val - (d * 240.0) / (maxTemp - d), 0, 240);
  }

  long color = 0x000000;
  color = (red << 16 | green << 8 | blue);
  // use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
  return color;
}

const byte MLX90640_address = 0x33; // Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8                  // Default shift for MLX90640 in open air
paramsMLX90640 mlx90640;

// Ticker tick; /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

lv_obj_t *slider_label;
int screenWidth = 320;
int screenHeight = 240;

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

/**/
void slider_event_cb(lv_obj_t *slider, lv_event_t event)
{

  printEvent("Slider", event);

  if (event == LV_EVENT_VALUE_CHANGED)
  {
    static char buf[4]; /* max 3 bytes  for number plus 1 null terminating byte */
    snprintf(buf, 4, "%u", lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf); /*Refresh the text*/
  }
}

void printEvent(String Event, lv_event_t event)
{

  Serial.print(Event);
  printf(" ");

  switch (event)
  {
  case LV_EVENT_PRESSED:
    printf("Pressed\n");
    break;

  case LV_EVENT_SHORT_CLICKED:
    printf("Short clicked\n");
    break;

  case LV_EVENT_CLICKED:
    printf("Clicked\n");
    break;

  case LV_EVENT_LONG_PRESSED:
    printf("Long press\n");
    break;

  case LV_EVENT_LONG_PRESSED_REPEAT:
    printf("Long press repeat\n");
    break;

  case LV_EVENT_RELEASED:
    printf("Released\n");
    break;
  }
}

void lv_example_canvas_2(void)
{
  /*Create a button to better see the transparency*/
  //  lv_btn_create(lv_scr_act());

  /*Create a buffer for the canvas*/
  // static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

  /*Create a canvas and initialize its palette*/
  lv_obj_t *canvas = lv_canvas_create(lv_scr_act(), NULL);
  lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);

  /*Create colors with the indices of the palette*/
  lv_color_t cqq = lv_color_hex(0xEEEEEE);
  lv_color_t c0 = lv_color_hex(0xCCCCCC);
  lv_canvas_fill_bg(canvas, /*lv_palette_lighten(0x9e9e9e00, 3)*/ cqq, LV_OPA_COVER);

  /*Create hole on the canvas*/
  uint32_t x;
  uint32_t y;
  for (y = 10; y < 30; y++)
  {
    for (x = 5; x < 20; x++)
    {
      lv_canvas_set_px(canvas, x, y, c0);
    }
  }
}

void setup()
{

  // Connect thermal sensor.
  Wire.begin();
  Wire.setClock(400000); // Increase I2C clock speed to 400kHz
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring.");
  }
  else
  {
    Serial.println("MLX90640 online!");
  }
  // Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");
  // Set refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x05); // Set rate to 8Hz effective - Works at 800kHz
  // Once EEPROM has been read at 400kHz we can increase
  Wire.setClock(800000);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  ledcSetup(10, 5000 /*freq*/, 10 /*resolution*/);
  ledcAttachPin(32, 10);
  analogReadResolution(10);
  ledcWrite(10, 768);

  Serial.begin(115200); /* prepare for possible serial debug */

  lv_init();

#if USE_LV_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin(); /* TFT init */
  tft.setRotation(3);

  uint16_t calData[5] = {275, 3620, 264, 3532, 1};
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);          /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;   /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);      /*Finally register the driver*/

  // Set the theme..
  lv_theme_t *th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, LV_THEME_DEFAULT_FONT_SMALL, LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);
  lv_theme_set_act(th);

  lv_obj_t *scr = lv_cont_create(NULL, NULL);
  lv_disp_load_scr(scr);

  // lv_obj_t * tv = lv_tabview_create(scr, NULL);
  // lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));

  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Arduino! (V7.0)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -50);

  lv_example_canvas_2();

  /* Create a slider in the center of the display */
  lv_obj_t *slider = lv_slider_create(lv_scr_act(), NULL);
  lv_obj_set_width(slider, screenWidth - 50); /*Set the width*/
  lv_obj_set_height(slider, 50);
  lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the center of the parent (screen)*/
  lv_obj_set_event_cb(slider, slider_event_cb);      /*Assign an event function*/

  /* Create a label below the slider */
  slider_label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(slider_label, "0");
  lv_obj_set_auto_realign(slider, true);
  lv_obj_align(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

/**/

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite();                                                                            /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++)
  {
    for (int x = area->x1; x <= area->x2; x++)
    {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite();            /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

bool my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY, 600);

  if (!touched)
  {
    return false;
  }

  if (touchX > screenWidth || touchY > screenHeight)
  {
    Serial.println("Y or y outside of expected parameters..");
    Serial.print("y:");
    Serial.print(touchX);
    Serial.print(" x:");
    Serial.print(touchY);
  }
  else
  {

    data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    /*Save the state and save the pressed coordinate*/
    // if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);

    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x");
    Serial.println(touchX);

    Serial.print("Data y");
    Serial.println(touchY);
  }

  return false; /*Return `false` because we are not buffering and no more data to read*/
}

void setAbcd()
{
  a = minTemp + (maxTemp - minTemp) * 0.2121;
  b = minTemp + (maxTemp - minTemp) * 0.3182;
  c = minTemp + (maxTemp - minTemp) * 0.4242;
  d = minTemp + (maxTemp - minTemp) * 0.8182;
}

void readTempValues()
{
  for (byte x = 0; x < 2; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; // Reflected temperature based on the sensor ambient temperature

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
  }
}

void setTempScale()
{
  minTemp = 255;
  maxTemp = 0;

  for (i = 0; i < 768; i++)
  {
    minTemp = min(minTemp, tempValues[i]);
    maxTemp = max(maxTemp, tempValues[i]);
  }

  setAbcd();
}

void drawPicture()
{
  for (int i = 0; i < 768; i++)
  {
    cbuf[i].full = getColor(tempValues[i]);
  }
}

void loop()
{
  readTempValues();
  setTempScale();
  drawPicture();
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}
