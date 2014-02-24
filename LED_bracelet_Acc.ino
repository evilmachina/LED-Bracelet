
#include <TinyWireM.h>
#include <Adafruit_TinyLSM303.h>
#include <Adafruit_NeoPixel.h>

#define MOVE_THRESHOLD 6000
#define PIXEL_OUTPUT 1           
#define ANALOG_INPUT 2           
#define NUM_PIXELS 19            
byte brightness = 180; 

PROGMEM byte exp_map[256]={
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,
  3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,5,5,5,
  5,5,5,5,5,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,
  9,9,10,10,10,10,10,11,11,11,11,12,12,12,13,13,13,13,
  14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,
  20,20,21,21,22,22,23,23,24,24,25,26,26,27,27,28,29,
  29,30,31,31,32,33,33,34,35,36,36,37,38,39,40,41,42,
  42,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,
  62,63,64,66,67,69,70,72,74,75,77,79,80,82,84,86,88,
  90,91,94,96,98,100,102,104,107,109,111,114,116,119,
  122,124,127,130,133,136,139,142,145,148,151,155,158,
  161,165,169,172,176,180,184,188,192,196,201,205,210,
  214,219,224,229,234,239,244,250,255
};

int32_t storedVector = 0,
         newVector = 0;
uint32_t color = 0, 
         color_timer = 0,  
         action_step_timer = 0,
         last_shacke = 0,
         sparkel_timer = 0,
         brightness_decay_timer = 0;
         
uint16_t color_idx = 0, 
         curr_color_interval = 1000,
         curr_brightness_decay_interval = 30,
         party_interval = 1000,
         sparkel_interval = 5000;
uint8_t  spectrum_part = 0, 
         curr_action = 0,
         curr_color_granularity = 1,
         nr_shackes = 0;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_OUTPUT);
Adafruit_TinyLSM303 lsm;

void setup() 
{
    
  pixels.begin();
  pixels.setBrightness(brightness);
  nextColor();
  pixels.show();
  
  if (!lsm.begin())
  {
    setColor(pixels.Color(255, 0, 0));
    while (1);
  }
}



void loop() 
{
  lsm.read();
  newVector = lsm.accelData.x*lsm.accelData.x;
  newVector += lsm.accelData.y*lsm.accelData.y;
  newVector += lsm.accelData.z*lsm.accelData.z;
  newVector = sqrt(newVector);
  curr_color_granularity = 1;
  
  if(brightness > 180 && (millis() - brightness_decay_timer) > curr_brightness_decay_interval){
    brightness--;
    brightness_decay_timer = millis();
  }
  
  pixels.setBrightness(exp_map[brightness]);
  
  if (abs(newVector - storedVector) > MOVE_THRESHOLD) {
    curr_color_granularity = 20;
    nextColor();
    last_shacke = millis(); 
    if(brightness < 255) brightness += 10;
    nr_shackes++;
  };
  
  if((millis() - last_shacke) >  party_interval){
    nr_shackes = 0;
  } 
  
  storedVector = newVector;
  
  if(nr_shackes > 8){
    //sparkel();
    
     nr_shackes = 0;
  }
  
  if ((millis() - color_timer) > curr_color_interval)
  {
    nextColor();
    color_timer = millis();
  }
  
  setColor(color);    
}


int idx = 0;

/*void sparkel(){
  sparkel_timer = millis();
  pixels.setBrightness(100);
  while((millis() - sparkel_timer) < sparkel_interval){
    pixels.setPixelColor(idx,0);
        // pick a new pixel
    idx = random (NUM_PIXELS);
        // set new pixel to the current color
    pixels.setPixelColor(idx,color);
    pixels.show();
  }
  
  pixels.setBrightness(brightness);

}*/

void nextColor ()
{
  switch (spectrum_part)
  {
    case 0 :  // spectral wipe from red to blue
    {
      color = Adafruit_NeoPixel::Color(255-color_idx,color_idx,0);
      color_idx += curr_color_granularity;
      if (color_idx > 255) 
      {
          spectrum_part = 1;
          color_idx = 0;
      }
      break;
    }
    case 1 :  // spectral wipe from blue to green
    {
      color = Adafruit_NeoPixel::Color(0,255-color_idx,color_idx);
      color_idx += curr_color_granularity;
      if (color_idx > 255) 
      {
          spectrum_part = 2;
          color_idx = 0;
      }
      break;
    }
    case 2 :  // spectral wipe from green to red
    {
      color = Adafruit_NeoPixel::Color(color_idx,0,255-color_idx);
      color_idx += curr_color_granularity;
      if (color_idx > 255) 
      {
          spectrum_part = 0;
          color_idx = 0;
      }
      break;
    }
    
  }
}

void setColor(uint32_t color){
  for (int i = 0; i < NUM_PIXELS; i++) pixels.setPixelColor(i,color);
  pixels.show();
}



