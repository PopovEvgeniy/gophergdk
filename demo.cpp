#include "gophergdk.h"

int main(void)
{
 long int x,y,screen_width,screen_height;
 char perfomance[8];
 GOPHERGDK::Backlight light;
 GOPHERGDK::Screen screen;
 GOPHERGDK::Gamepad gamepad;
 GOPHERGDK::Sound sound;
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
 sound.initialize();
 player.initialize(sound.get_handle());
 audio.load_wave("space.wav",player);
 memset(perfomance,0,8);
 light.set_level(light.get_minimum());
 while(1)
 {
  screen.update();
  gamepad.update();
  if (player.play()==false) player.rewind_audio();
  if (gamepad.check_press(BUTTON_START)==true) break;
  if (gamepad.check_press(BUTTON_A)==true) ship.mirror_image(MIRROR_HORIZONTAL);
  if (gamepad.check_press(BUTTON_B)==true) ship.mirror_image(MIRROR_VERTICAL);
  if (gamepad.check_hold(BUTTON_X)==true) light.increase_level();
  if (gamepad.check_hold(BUTTON_Y)==true) light.decrease_level();
  if (gamepad.check_hold(BUTTON_UP)==true) y-=4;
  if (gamepad.check_hold(BUTTON_DOWN)==true) y+=4;
  if (gamepad.check_hold(BUTTON_LEFT)==true) x-=4;
  if (gamepad.check_hold(BUTTON_RIGHT)==true) x+=4;
  if ((x<=0)||(x>=screen_width)) x=screen_width/2;
  if ((y<=0)||(y>=screen_height)) y=screen_height/2;
  sprintf(perfomance,"%ld",screen.get_fps());
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