

void lv_example_canvas_2(void)
{
    /*Create a button to better see the transparency*/
  //  lv_btn_create(lv_scr_act());

    /*Create a buffer for the canvas*/
    //static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

    /*Create a canvas and initialize its palette*/
    lv_obj_t * canvas = lv_canvas_create(lv_scr_act(), NULL);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    
    /*Create colors with the indices of the palette*/
  

      lv_canvas_fill_bg(canvas, lv_palette_lighten(0x9e9e9e00, 3), LV_OPA_COVER);


    /*Create hole on the canvas*/
    uint32_t x;
    uint32_t y;
    for(y = 10; y < 30; y++) {
        for(x = 5; x < 20; x++) {
            lv_canvas_set_px(canvas, x, y, c0);
        }
    }

}


void setup() {

    // Connect thermal sensor.
  Wire.begin();
  Wire.setClock(400000); // Increase I2C clock speed to 400kHz
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring.");
  }
  else {
    Serial.println("MLX90640 online!");
  }
  // Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) Serial.println("Parameter extraction failed");
  // Set refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x05); // Set rate to 8Hz effective - Works at 800kHz
  // Once EEPROM has been read at 400kHz we can increase
  Wire.setClock(800000);


  
  
pinMode(LED, OUTPUT);
digitalWrite(LED, HIGH);

  ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  ledcAttachPin(32, 10);
  analogReadResolution(10);
  ledcWrite(10,768);

  Serial.begin(115200); /* prepare for possible serial debug */

  lv_init();

  #if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
  #endif

  tft.begin(); /* TFT init */
  tft.setRotation(3);

  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
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
  lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

  //Set the theme..
  lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);     
  lv_theme_set_act(th);

  lv_obj_t * scr = lv_cont_create(NULL, NULL);
  lv_disp_load_scr(scr);

  //lv_obj_t * tv = lv_tabview_create(scr, NULL);
  //lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));

  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Arduino! (V7.0)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -50);

  lv_example_canvas_2();

  /* Create a slider in the center of the display */
  lv_obj_t * slider = lv_slider_create(lv_scr_act(), NULL);
  lv_obj_set_width(slider, screenWidth-50);                        /*Set the width*/
  lv_obj_set_height(slider, 50);
  lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);    /*Align to the center of the parent (screen)*/
  lv_obj_set_event_cb(slider, slider_event_cb);         /*Assign an event function*/

  /* Create a label below the slider */
  slider_label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(slider_label, "0");
  lv_obj_set_auto_realign(slider, true);
  lv_obj_align(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

}
