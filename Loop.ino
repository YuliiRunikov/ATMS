void loop() {
  
 readTempValues();
  setTempScale();
  drawPicture();  
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}
