#include "gophergdk.h"

int main(void)
{
 long int x,y,screen_width,screen_height;
 char perfomance[8];
 GOPHERGDK::Backlight light;
 GOPHERGDK::Screen screen;
 GOPHERGDK::Gamepad gamepad;
 GOPHERGDK::Sound sound;
 GOPHERGDK::Mixer volume;
 GOPHERGDK::Audio audio;
 GOPHERGDK::Player player;
 GOPHERGDK::Timer timer;
 GOPHERGDK::Image image;
 GOPHERGDK::Background space;
 GOPHERGDK::Sprite ship,font;
 GOPHERGDK::Text text;
 screen.initialize();
 screen_width=screen.get_width();
 screen_height=screen.get_height();
 x=screen_width/2;
 y=screen_height/2;
 image.load_tga("space.tga");
 space.load_image(image);
 image.load_tga("ship.tga");
 ship.load_sprite(image,HORIZONTAL_STRIP,2);
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 gamepad.initialize();
 space.initialize(screen.get_handle());
 ship.initialize(screen.get_handle());
 font.initialize(screen.get_handle());
 space.resize_image(screen_width,screen_height);
 space.set_kind(NORMAL_BACKGROUND);
 screen.clear_screen();
 text.set_position(font.get_width(),font.get_height());
 timer.set_timer(1);
 sound.initialize(44100);
 volume.initialize();
 volume.set_volume(volume.get_maximum());
 player.initialize(sound.get_handle());
 audio.load_wave("space.wav");
 player.load(audio.get_handle());
 light.set_light(light.get_minimum());
 memset(perfomance,0,8);
 while(1)
 {
  screen.update();
  gamepad.update();
  player.loop();
  if (gamepad.check_press(BUTTON_START)==true) break;
  if (gamepad.check_press(BUTTON_R)==true) volume.turn_off();
  if (gamepad.check_press(BUTTON_L)==true) volume.turn_on();
  if (gamepad.check_press(BUTTON_A)==true) ship.mirror_image(MIRROR_HORIZONTAL);
  if (gamepad.check_press(BUTTON_B)==true) ship.mirror_image(MIRROR_VERTICAL);
  if (gamepad.check_press(BUTTON_X)==true) light.increase_level();
  if (gamepad.check_press(BUTTON_Y)==true) light.decrease_level();
  if (gamepad.check_press(BUTTON_C)==true) light.turn_off();
  if (gamepad.check_press(BUTTON_Z)==true) light.turn_on();
  if (gamepad.check_hold(BUTTON_UP)==true) y-=4;
  if (gamepad.check_hold(BUTTON_DOWN)==true) y+=4;
  if (gamepad.check_hold(BUTTON_LEFT)==true) x-=4;
  if (gamepad.check_hold(BUTTON_RIGHT)==true) x+=4;
  if ((x<=0)||(x>=screen_width)) x=screen_width/2;
  if ((y<=0)||(y>=screen_height)) y=screen_height/2;
  sprintf(perfomance,"%lu",screen.get_fps());
  space.draw_background();
  text.draw_text(perfomance);
  ship.set_position(x,y);
  ship.draw_sprite();
  if (timer.check_timer()==true)
  {
   ship.step();
  }

 }
 return 0;
}