#include "gophergdk.h"

int main()
{
 char level[6];
 GOPHERGDK::Screen screen;
 GOPHERGDK::Gamepad gamepad;
 GOPHERGDK::Image image;
 GOPHERGDK::Sprite font;
 GOPHERGDK::Battery battery;
 GOPHERGDK::Text text;
 memset(level,0,6);
 gamepad.initialize();
 screen.initialize();
 font.initialize(screen.get_handle());
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 text.set_position(screen.get_width()/2,screen.get_height()/2);
 while(1)
 {
  screen.update();
  gamepad.update();
  if(gamepad.check_hold(BUTTON_START)==true) break;
  sprintf(level,"%hu",battery.get_level());
  text.draw_text(level);
 }
 return 0;
}