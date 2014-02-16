
#include <TinyWireM.h>
#include <Adafruit_TinyLSM303.h>
#include <Adafruit_NeoPixel.h>

#define MOVE_THRESHOLD 6000
#define PIXEL_OUTPUT 1           
#define ANALOG_INPUT 2           
#define NUM_PIXELS 19            
#define BRIGHTNESS 30            

int32_t storedVector = 0,
         newVector = 0;
uint32_t color = 0, 
         color_timer = 0,  
         action_step_timer = 0,
         last_shacke = 0,
         sparkel_timer = 0;
         
uint16_t color_idx = 0, 
         curr_color_interval = 1000,
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
  pixels.setBrightness(BRIGHTNESS);
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
  if (abs(newVector - storedVector) > MOVE_THRESHOLD) {
    curr_color_granularity = 20;
    nextColor();
    last_shacke = millis(); 
    nr_shackes++;
  };
  
  if((millis() - last_shacke) >  party_interval){
    nr_shackes = 0;
  } 
  
  storedVector = newVector;
  
  if(nr_shackes > 8){
    sparkel();
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

void sparkel(){
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
  
  pixels.setBrightness(BRIGHTNESS);

}

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



