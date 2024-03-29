/*
Gopher game development kit license

Copyright (C) 2020-2021 Popov Evgeniy Alekseyevich

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Third�party code license

Pixel packing algorithm bases on code from SVGALib. SVGALib is public domain.
SVGALib homepage: http://www.svgalib.org/
*/

#include "gophergdk.h"

const int SOUND_CHANNELS=1;
const unsigned char GAMEPAD_PRESS=1;
const unsigned char GAMEPAD_RELEASE=0;
const size_t BUTTON_AMOUNT=14;

namespace OSS_BACKEND
{
 volatile int sound_device=-1;
 volatile size_t sound_buffer_length=0;
 volatile bool run_stream=true;
 volatile bool do_play=false;
}

namespace GOPHERGDK
{

void* oss_play_sound(void *buffer)
{
 while (OSS_BACKEND::run_stream)
 {
  if (OSS_BACKEND::do_play)
  {
   write(OSS_BACKEND::sound_device,buffer,OSS_BACKEND::sound_buffer_length);
   OSS_BACKEND::do_play=false;
  }

 }
 if (buffer!=NULL) free(buffer);
 if (OSS_BACKEND::sound_device!=-1) close(OSS_BACKEND::sound_device);
 return NULL;
}

void Halt(const char *message)
{
 puts(message);
 exit(EXIT_FAILURE);
}

Frame::Frame()
{
 frame_width=0;
 frame_height=0;
 pixels=0;
 length=0;
 buffer=NULL;
 shadow=NULL;
}

Frame::~Frame()
{
 if (buffer!=NULL)
 {
  delete[] buffer;
  buffer=NULL;
 }
 if (shadow!=NULL)
 {
  delete[] shadow;
  shadow=NULL;
 }

}

void Frame::calculate_buffer_length()
{
 pixels=static_cast<size_t>(frame_width)*static_cast<size_t>(frame_height);
 length=pixels*sizeof(unsigned short int);
}

unsigned int short *Frame::get_memory(const char *error)
{
 unsigned short int *target;
 target=NULL;
 try
 {
  target=new unsigned short int[pixels];
 }
 catch (...)
 {
  Halt(error);
 }
 return target;
}

void Frame::clear_buffer(unsigned short int *target)
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  target[index]=0;
 }

}

unsigned short int *Frame::create_buffer(const char *error)
{
 unsigned short int *target;
 target=NULL;
 this->calculate_buffer_length();
 target=this->get_memory(error);
 this->clear_buffer(target);
 return target;
}

size_t Frame::get_offset(const unsigned long int x,const unsigned long int y,const unsigned long int target_width)
{
 return static_cast<size_t>(x)+static_cast<size_t>(y)*static_cast<size_t>(target_width);
}

size_t Frame::get_offset(const unsigned long int x,const unsigned long int y) const
{
 return static_cast<size_t>(x)+static_cast<size_t>(y)*static_cast<size_t>(frame_width);
}

void Frame::set_size(const unsigned long int surface_width,const unsigned long int surface_height)
{
 frame_width=surface_width;
 frame_height=surface_height;
}

void Frame::create_buffers()
{
 buffer=this->create_buffer("Can't allocate memory for render buffer");
 shadow=this->create_buffer("Can't allocate memory for shadow buffer");
}

size_t Frame::get_length() const
{
 return length;
}

unsigned short int *Frame::get_buffer()
{
 return buffer;
}

size_t Frame::get_pixels() const
{
 return pixels;
}

unsigned long int Frame::get_frame_width() const
{
 return frame_width;
}

unsigned long int Frame::get_frame_height() const
{
 return frame_height;
}

bool Frame::draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned short int red,const unsigned short int green,const unsigned short int blue)
{
 bool result;
 size_t offset;
 result=false;
 offset=static_cast<size_t>(x)+static_cast<size_t>(y)*static_cast<size_t>(frame_width);
 if (offset<pixels)
 {
  buffer[offset]=(blue >> 3) +((green >> 2) << 5)+((red >> 3) << 11); // This code bases on code from SVGALib
  result=true;
 }
 return result;
}

void Frame::clear_screen()
{
 this->clear_buffer(buffer);
}

void Frame::save()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  shadow[index]=buffer[index];
 }

}

void Frame::restore()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  buffer[index]=shadow[index];
 }

}

void Frame::restore(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int target_x,target_y,stop_x,stop_y;
 size_t position;
 stop_x=x+width;
 stop_y=y+height;
 if ((x<frame_width)&&(y<frame_height))
 {
  if ((stop_x<=frame_width)&&(stop_y<=frame_height))
  {
   for (target_x=x;target_x<stop_x;++target_x)
   {
    for (target_y=y;target_y<stop_y;++target_y)
    {
     position=this->get_offset(target_x,target_y);
     buffer[position]=shadow[position];
    }

   }

  }

 }

}

Plane::Plane()
{
 target=NULL;
 plane=NULL;
 target_width=0;
 target_height=0;
 x_ratio=0;
 y_ratio=0;
}

Plane::~Plane()
{

}

void Plane::create_plane(const unsigned long int width,const unsigned long int height,const unsigned long int surface_width,const unsigned long int surface_height,unsigned short int *surface_buffer)
{
 this->set_size(width,height);
 this->create_buffers();
 plane=this->get_buffer();
 target=surface_buffer;
 target_width=surface_width;
 target_height=surface_height;
 x_ratio=static_cast<float>(width)/static_cast<float>(surface_width);
 y_ratio=static_cast<float>(height)/static_cast<float>(surface_height);
}

void Plane::transfer()
{
 unsigned long int x,y,width,steps;
 size_t index,location,position;
 width=this->get_frame_width();
 x=0;
 y=0;
 steps=target_width*target_height;
 for (index=0;index<steps;++index)
 {
  location=this->get_offset(x,y,target_width);
  position=this->get_offset((x_ratio*static_cast<float>(x)),(y_ratio*static_cast<float>(y)),width);
  target[location]=plane[position];
  ++x;
  if (x==target_width)
  {
   x=0;
   ++y;
  }

 }

}

Plane* Plane::get_handle()
{
 return this;
}

Timer::Timer()
{
 interval=0;
 start=time(NULL);
}

Timer::~Timer()
{

}

void Timer::set_timer(const double seconds)
{
 interval=seconds;
 start=time(NULL);
}

bool Timer::check_timer()
{
 bool result;
 result=false;
 if (difftime(time(NULL),start)>=interval)
 {
  result=true;
  start=time(NULL);
 }
 return result;
}

FPS::FPS()
{
 start=time(NULL);
 current=0;
 fps=0;
}

FPS::~FPS()
{

}

void FPS::update_counter()
{
 ++current;
 if (difftime(time(NULL),start)>=1)
 {
  fps=current;
  current=0;
  start=time(NULL);
 }

}

unsigned long int FPS::get_fps() const
{
 return fps;
}

Render::Render()
{
 start=0;
 device=open("/dev/fb0",O_RDWR);
 if (device==-1)
 {
  Halt("Can't get access to frame buffer");
 }
 memset(&setting,0,sizeof(FBIOGET_VSCREENINFO));
 memset(&configuration,0,sizeof(FBIOGET_FSCREENINFO));
}

Render::~Render()
{
 if (device!=-1) close(device);
}

void Render::read_base_configuration()
{
 if (ioctl(device,FBIOGET_VSCREENINFO,&setting)==-1)
 {
  Halt("Can't read framebuffer setting");
 }

}

void Render::read_advanced_configuration()
{
 if (ioctl(device,FBIOGET_FSCREENINFO,&configuration)==-1)
 {
  Halt("Can't read framebuffer setting");
 }

}

void Render::read_configuration()
{
 this->read_base_configuration();
 this->read_advanced_configuration();
}

void Render::get_start_offset()
{
 start=setting.xoffset*(setting.bits_per_pixel/CHAR_BIT)+setting.yoffset*configuration.line_length;
}

void Render::prepare_render()
{
 this->read_configuration();
 this->get_start_offset();
}

void Render::refresh()
{
 lseek(device,start,SEEK_SET);
 write(device,this->get_buffer(),this->get_length());
}

unsigned long int Render::get_width() const
{
 return setting.xres;
}

unsigned long int Render::get_height() const
{
 return setting.yres;
}

unsigned long int Render::get_color() const
{
 return setting.bits_per_pixel;
}

Screen::Screen()
{

}

Screen::~Screen()
{

}

void Screen::initialize()
{
 this->prepare_render();
 this->set_size(this->get_width(),this->get_height());
 this->create_buffers();
}

void Screen::update()
{
 this->refresh();
 this->update_counter();
}

Screen* Screen::get_handle()
{
 return this;
}

Gamepad::Gamepad()
{
 current=NULL;
 preversion=NULL;
 device=-1;
 length=sizeof(input_event);
 memset(&input,0,length);
}

Gamepad::~Gamepad()
{
 if (device!=-1) close(device);
 if (current!=NULL) delete[] current;
 if (preversion!=NULL) delete[] preversion;
}

unsigned char *Gamepad::get_memory(const char *message)
{
 unsigned char *buffer;
 buffer=NULL;
 try
 {
  buffer=new unsigned char[BUTTON_AMOUNT];
 }
 catch (...)
 {
  Halt(message);
 }
 return buffer;
}

void Gamepad::clear_buffer(unsigned char *target)
{
 size_t index;
 for (index=0;index<BUTTON_AMOUNT;++index)
 {
  target[index]=GAMEPAD_RELEASE;
 }

}

unsigned char *Gamepad::create_buffer(const char *message)
{
 unsigned char *buffer;
 buffer=this->get_memory(message);
 this->clear_buffer(buffer);
 return buffer;
}

void Gamepad::create_buffers()
{
 current=this->create_buffer("Can't allocate memory for primary input buffer");
 preversion=this->create_buffer("Can't allocate memory for secondary input buffer");
}

void Gamepad::open_device()
{
 device=open("/dev/event0",O_RDONLY|O_NONBLOCK|O_NOCTTY);
 if (device==-1)
 {
  Halt("Can't get access to gamepad");
 }

}

unsigned char Gamepad::get_state() const
{
 unsigned char state;
 state=GAMEPAD_RELEASE;
 if (input.value!=GAMEPAD_RELEASE)
 {
  state=GAMEPAD_PRESS;
 }
 return state;
}

GAMEPAD_BUTTONS Gamepad::get_button() const
{
 GAMEPAD_BUTTONS button;
 button=BUTTON_UP;
 switch (input.code)
 {
  case KEY_UP:
  button=BUTTON_UP;
  break;
  case KEY_DOWN:
  button=BUTTON_DOWN;
  break;
  case KEY_LEFT:
  button=BUTTON_LEFT;
  break;
  case KEY_RIGHT:
  button=BUTTON_RIGHT;
  break;
  case KEY_LEFTCTRL:
  button=BUTTON_A;
  break;
  case KEY_LEFTALT:
  button=BUTTON_B;
  break;
  case KEY_2:
  button=BUTTON_C;
  break;
  case KEY_SPACE:
  button=BUTTON_X;
  break;
  case KEY_LEFTSHIFT:
  button=BUTTON_Y;
  break;
  case KEY_1:
  button=BUTTON_Z;
  break;
  case KEY_BACKSPACE:
  button=BUTTON_R;
  break;
  case KEY_TAB:
  button=BUTTON_L;
  break;
  case KEY_ENTER:
  button=BUTTON_START;
  break;
  case KEY_ESC:
  button=BUTTON_MENU;
  break;
 }
 return button;
}

bool Gamepad::check_state(const GAMEPAD_BUTTONS button,const unsigned char state)
{
 bool result;
 result=false;
 if (current[button]==state)
 {
  if (preversion[button]!=state) result=true;
 }
 preversion[button]=current[button];
 return result;
}

void Gamepad::initialize()
{
 this->open_device();
 this->create_buffers();
}

void Gamepad::update()
{
 while (read(device,&input,length)>0)
 {
  if (input.type==EV_KEY)
  {
   current[this->get_button()]=this->get_state();
  }

 }

}

bool Gamepad::check_hold(const GAMEPAD_BUTTONS button)
{
 preversion[button]=current[button];
 return current[button]!=GAMEPAD_RELEASE;
}

bool Gamepad::check_press(const GAMEPAD_BUTTONS button)
{
 return this->check_state(button,GAMEPAD_PRESS);
}

bool Gamepad::check_release(const GAMEPAD_BUTTONS button)
{
 return this->check_state(button,GAMEPAD_RELEASE);
}

Battery::Battery()
{
 device=NULL;
 minimum=3580;
 maximum=4176;
 delta=maximum-minimum;
 memset(buffer,0,5);
}

Battery::~Battery()
{

}

void Battery::open_device()
{
 device=fopen("/proc/jz/battery","rt");
 if (device==NULL)
 {
  Halt("Can't get access to battery");
 }

}

void Battery::close_device()
{
 if (device!=NULL) fclose(device);
}

float Battery::read_battery_level()
{
 fgets(buffer,5,device);
 return atof(buffer);
}

float Battery::calculate_level(const float current) const
{
 float level;
 level=current;
 if (level>maximum) level=maximum;
 if (level<minimum) level=minimum;
 return ((level-minimum)/delta)*100.0;
}

unsigned short int Battery::get_level()
{
 unsigned short int level;
 this->open_device();
 level=this->calculate_level(this->read_battery_level());
 this->close_device();
 return level;
}

Backlight::Backlight()
{
 device=NULL;
 minimum=10;
 maximum=90;
 current=minimum;
 memset(buffer,0,4);
}

Backlight::~Backlight()
{

}

void Backlight::open_device(const char *mode)
{
 device=fopen("/proc/jz/lcd_backlight",mode);
 if (device==NULL)
 {
  Halt("Can't get access to display backlight");
 }

}

void Backlight::open_read()
{
 this->open_device("rt");
}

void Backlight::open_write()
{
 this->open_device("wt");
}

void Backlight::close_device()
{
 if (device!=NULL) fclose(device);
}

void Backlight::read_value()
{
 memset(buffer,0,4);
 fgets(buffer,4,device);
 current=atoi(buffer);
}

void Backlight::write_value(const unsigned char value)
{
 memset(buffer,0,4);
 sprintf(buffer,"%hhu",value);
 rewind(device);
 fputs(buffer,device);
}

void Backlight::set_level(const unsigned char level)
{
 this->open_write();
 this->write_value(level);
 this->close_device();
}

unsigned char Backlight::correct_level(const unsigned char level) const
{
 unsigned char value;
 value=level;
 if (value>maximum) value=maximum;
 if (value<minimum) value=minimum;
 return value;
}

unsigned char Backlight::get_minimum() const
{
 return minimum;
}

unsigned char Backlight::get_maximum() const
{
 return maximum;
}

unsigned char Backlight::get_level()
{
 this->open_read();
 this->read_value();
 this->close_device();
 return current;
}

void Backlight::set_light(const unsigned char level)
{
 this->set_level(this->correct_level(level));
}

void Backlight::increase_level()
{
 if (this->get_level()<maximum)
 {
  current+=minimum;
  this->set_light(current);
 }

}

void Backlight::decrease_level()
{
 if (this->get_level()>minimum)
 {
  current-=minimum;
  this->set_light(current);
 }

}

void Backlight::turn_off()
{
 this->get_level();
 this->set_level(0);
}

void Backlight::turn_on()
{
 this->set_light(current);
}

Memory::Memory()
{
 memset(&information,0,sizeof(struct sysinfo));
}

Memory::~Memory()
{

}

void Memory::read_system_information()
{
 if (sysinfo(&information)==-1)
 {
  Halt("Can't read system information");
 }

}

unsigned long int Memory::get_total_memory()
{
 this->read_system_information();
 return information.totalram*information.mem_unit;
}

unsigned long int Memory::get_free_memory()
{
 this->read_system_information();
 return information.freeram*information.mem_unit;
}

Sound::Sound()
{
 internal=NULL;
 buffer_length=0;
 stream=0;
}

Sound::~Sound()
{
 OSS_BACKEND::run_stream=false;
}

void Sound::open_device()
{
 OSS_BACKEND::sound_device=open("/dev/dsp",O_WRONLY,S_IRWXU|S_IRWXG|S_IRWXO);
 if (OSS_BACKEND::sound_device==-1)
 {
  Halt("Can't get access to sound card");
 }

}

void Sound::set_format()
{
 int format;
 format=AFMT_S16_LE;
 if (ioctl(OSS_BACKEND::sound_device,SNDCTL_DSP_SETFMT,&format)==-1)
 {
  Halt("Can't set sound format");
 }

}

void Sound::set_channels()
{
 int channels;
 channels=SOUND_CHANNELS;
 if (ioctl(OSS_BACKEND::sound_device,SNDCTL_DSP_CHANNELS,&channels)==-1)
 {
  Halt("Can't set number of audio channels");
 }

}

void Sound::set_rate(const int rate)
{
 if (ioctl(OSS_BACKEND::sound_device,SNDCTL_DSP_SPEED,&rate)==-1)
 {
  Halt("Can't set sample rate");
 }

}

void Sound::get_buffer_length()
{
 audio_buf_info configuration;
 memset(&configuration,0,sizeof(audio_buf_info));
 if (ioctl(OSS_BACKEND::sound_device,SNDCTL_DSP_GETOSPACE,&configuration)==-1)
 {
  Halt("Can't read configuration of sound buffer");
 }
 buffer_length=static_cast<size_t>(configuration.fragstotal)*static_cast<size_t>(configuration.fragsize);
}

void Sound::configure_sound_card(const int rate)
{
 this->open_device();
 this->set_format();
 this->set_rate(rate);
 this->set_channels();
 this->get_buffer_length();
}

void Sound::start_stream()
{
 if (pthread_create(&stream,NULL,oss_play_sound,internal)!=0)
 {
  Halt("Can't start sound stream");
 }

}

void Sound::create_buffer()
{
 internal=static_cast<char*>(calloc(buffer_length,sizeof(char)));
 if (internal==NULL)
 {
  Halt("Can't allocate memory for sound buffer");
 }

}

void Sound::initialize(const int rate)
{
 this->configure_sound_card(rate);
 this->create_buffer();
 this->start_stream();
}

bool Sound::check_busy()
{
 return OSS_BACKEND::do_play;
}

size_t Sound::get_length() const
{
 return buffer_length;
}

size_t Sound::send(char *buffer,const size_t length)
{
 size_t amount;
 if (OSS_BACKEND::do_play)
 {
  amount=0;
 }
 else
 {
  amount=buffer_length;
  if (length<buffer_length) amount=length;
  memmove(internal,buffer,amount);
  OSS_BACKEND::sound_buffer_length=amount;
  OSS_BACKEND::do_play=true;
 }
 return amount;
}

Sound* Sound::get_handle()
{
 return this;
}

Mixer::Mixer()
{
 device=-1;
 minimum=15;
 maximum=255;
 current=minimum;
}

Mixer::~Mixer()
{
 if (device==-1) close(device);
}

void Mixer::open_device()
{
 device=open("/dev/mixer",O_RDWR);
 if (device==-1)
 {
  Halt("Can't get access to mixer");
 }

}

void Mixer::set_level(const int level)
{
 if (ioctl(device,SOUND_MIXER_WRITE_VOLUME,&level)==-1)
 {
  Halt("Can't set volume");
 }

}

int Mixer::correct_level(const int level) const
{
 int value;
 value=level;
 if (value<minimum) value=minimum;
 if (value>maximum) value=maximum;
 return value;
}

void Mixer::set_volume(const int level)
{
 current=(this->correct_level(level)<<8)+this->correct_level(level);
 this->set_level(current);
}

void Mixer::turn_on()
{
 this->set_volume(current);
}

void Mixer::turn_off()
{
 this->set_level(0);
}

void Mixer::initialize()
{
 this->open_device();
 this->set_level(current);
}

int Mixer::get_minimum() const
{
 return minimum;
}

int Mixer::get_maximum() const
{
 return maximum;
}

int Mixer::get_volume() const
{
 return current;
}

System::System()
{
 srand(UINT_MAX);
}

System::~System()
{

}

unsigned long int System::get_random(const unsigned long int number)
{
 return rand()%number;
}

void System::quit()
{
 exit(EXIT_SUCCESS);
}

void System::run(const char *command)
{
 system(command);
}

char* System::read_environment(const char *variable)
{
 return getenv(variable);
}

void System::enable_logging(const char *name)
{
 if (freopen(name,"wt",stdout)==NULL)
 {
  Halt("Can't create log file");
 }

}

Filesystem::Filesystem()
{

}

Filesystem::~Filesystem()
{

}

bool Filesystem::file_exist(const char *name)
{
 FILE *target;
 bool exist;
 exist=false;
 target=fopen(name,"rb");
 if (target!=NULL)
 {
  exist=true;
  fclose(target);
 }
 return exist;
}

bool Filesystem::delete_file(const char *name)
{
 return remove(name)==0;
}

Binary_File::Binary_File()
{
 target=NULL;
}

Binary_File::~Binary_File()
{
 if (target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::open_file(const char *name,const char *mode)
{
 target=fopen(name,mode);
 if (target==NULL)
 {
  Halt("Can't open the binary file");
 }

}

void Binary_File::close()
{
 if (target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::set_position(const long int offset)
{
 fseek(target,offset,SEEK_SET);
}

long int Binary_File::get_position()
{
 return ftell(target);
}

long int Binary_File::get_length()
{
 long int result;
 fseek(target,0,SEEK_END);
 result=ftell(target);
 rewind(target);
 return result;
}

bool Binary_File::check_error()
{
 return ferror(target)!=0;
}

Input_File::Input_File()
{

}

Input_File::~Input_File()
{

}

void Input_File::open(const char *name)
{
 this->close();
 this->open_file(name,"rb");
}

void Input_File::read(void *buffer,const size_t length)
{
 fread(buffer,sizeof(char),length,target);
}

Output_File::Output_File()
{

}

Output_File::~Output_File()
{

}

void Output_File::open(const char *name)
{
 this->close();
 this->open_file(name,"wb");
}

void Output_File::create_temp()
{
 this->close();
 target=tmpfile();
 if (target==NULL)
 {
  Halt("Can't create a temporary file");
 }

}

void Output_File::write(void *buffer,const size_t length)
{
 fwrite(buffer,sizeof(char),length,target);
}

void Output_File::flush()
{
 fflush(target);
}

Audio::Audio()
{
 memset(&head,0,44);
}

Audio::~Audio()
{

}

void Audio::read_head()
{
 target.read(&head,44);
}

void Audio::check_riff_signature()
{
 if (strncmp(head.riff_signature,"RIFF",4)!=0)
 {
  Halt("Incorrect riff signature");
 }

}

void Audio::check_wave_signature()
{
 if (strncmp(head.wave_signature,"WAVE",4)!=0)
 {
  Halt("Incorrect wave signature");
 }

}

void Audio::check_type() const
{
 if (head.type!=1)
 {
  Halt("Incorrect type of wave file");
 }

}

void Audio::check_bits() const
{
 if (head.bits!=16)
 {
  Halt("Incorrect amount of sound bits");
 }

}

void Audio::check_channels() const
{
 if ((head.channels==0)||(head.channels>2))
 {
  Halt("Incorrect number of audio channels");
 }

}

void Audio::check_wave()
{
 this->check_riff_signature();
 this->check_wave_signature();
 this->check_type();
 this->check_bits();
 this->check_channels();
}

Audio* Audio::get_handle()
{
 return this;
}

size_t Audio::get_total() const
{
 return static_cast<size_t>(head.date_length);
}

size_t Audio::get_block() const
{
 return static_cast<size_t>(head.block_length);
}

unsigned long int Audio::get_rate() const
{
 return head.rate;
}

unsigned short int Audio::get_channels() const
{
 return head.channels;
}

unsigned short int Audio::get_bits() const
{
 return head.bits;
}

void Audio::load_wave(const char *name)
{
 target.open(name);
 this->read_head();
 this->check_wave();
}

void Audio::read_data(void *buffer,const size_t length)
{
 target.read(buffer,length);
}

void Audio::go_start()
{
 target.set_position(44);
}

Player::Player()
{
 sound=NULL;
 target=NULL;
 buffer=NULL;
 index=0;
 length=0;
}

Player::~Player()
{
 if (buffer!=NULL) delete[] buffer;
}

void Player::configure_player(Audio *audio)
{
 index=0;
 target=audio;
 length=target->get_total();
}

void Player::clear_buffer()
{
 if (buffer!=NULL)
 {
  delete[] buffer;
  buffer=NULL;
 }

}

void Player::create_buffer()
{
 try
 {
  buffer=new char[sound->get_length()];
 }
 catch (...)
 {
  Halt("Can't allocate memory for audio buffer");
 }

}

void Player::rewind_audio()
{
 index=0;
 target->go_start();
}

bool Player::is_end() const
{
 bool end_audio;
 end_audio=false;
 if (index==length)
 {
  end_audio=true;
 }
 return end_audio;
}

void Player::load(Audio *audio)
{
 this->configure_player(audio);
 this->clear_buffer();
 this->create_buffer();
}

void Player::initialize(Sound *target)
{
 sound=target;
}

void Player::play()
{
 size_t block;
 size_t elapsed;
 block=sound->get_length();
 if (index<length)
 {
  if (sound->check_busy()==false)
  {
   elapsed=length-index;
   if (block>elapsed) block=elapsed;
   target->read_data(buffer,block);
   index+=sound->send(buffer,block);
  }

 }

}

void Player::loop()
{
 if (this->is_end())
 {
  this->rewind_audio();
 }
 else
 {
  this->play();
 }

}

Primitive::Primitive()
{
 surface=NULL;
 color.red=0;
 color.green=0;
 color.blue=0;
}

Primitive::~Primitive()
{

}

void Primitive::initialize(Screen *screen)
{
 surface=screen;
}

void Primitive::set_color(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 color.red=red;
 color.green=green;
 color.blue=blue;
}

void Primitive::draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2)
{
 unsigned long int delta_x,delta_y,index,steps;
 float x,y,shift_x,shift_y;
 if (x1>x2)
 {
  delta_x=x1-x2;
 }
 else
 {
  delta_x=x2-x1;
 }
 if (y1>y2)
 {
  delta_y=y1-y2;
 }
 else
 {
  delta_y=y2-y1;
 }
 steps=delta_x;
 if (steps<delta_y) steps=delta_y;
 x=x1;
 y=y1;
 shift_x=static_cast<float>(delta_x)/static_cast<float>(steps);
 shift_y=static_cast<float>(delta_y)/static_cast<float>(steps);
 for (index=steps;index>0;--index)
 {
  x+=shift_x;
  y+=shift_y;
  surface->draw_pixel(x,y,color.red,color.green,color.blue);
 }

}

void Primitive::draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 this->draw_line(x,y,stop_x,y);
 this->draw_line(x,stop_y,stop_x,stop_y);
 this->draw_line(x,y,x,stop_y);
 this->draw_line(stop_x,y,stop_x,stop_y);
}

void Primitive::draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int step_x,step_y,stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 for (step_x=x;step_x<stop_x;++step_x)
 {
  for (step_y=y;step_y<stop_y;++step_y)
  {
   surface->draw_pixel(step_x,step_y,color.red,color.green,color.blue);
  }

 }

}

Image::Image()
{
 width=0;
 height=0;
 data=NULL;
}

Image::~Image()
{
 if (data!=NULL)
 {
  delete[] data;
  data=NULL;
 }

}

unsigned char *Image::create_buffer(const size_t length)
{
 unsigned char *result;
 result=NULL;
 try
 {
  result=new unsigned char[length];
 }
 catch (...)
 {
  Halt("Can't allocate memory for image buffer");
 }
 return result;
}

void Image::clear_buffer()
{
 if (data!=NULL)
 {
  delete[] data;
  data=NULL;
 }

}

void Image::load_tga(const char *name)
{
 Input_File target;
 size_t index,position,amount,compressed_length,uncompressed_length;
 unsigned char *compressed;
 unsigned char *uncompressed;
 TGA_head head;
 TGA_map color_map;
 TGA_image image;
 this->clear_buffer();
 target.open(name);
 compressed_length=static_cast<size_t>(target.get_length()-18);
 target.read(&head,3);
 target.read(&color_map,5);
 target.read(&image,10);
 if ((head.color_map!=0)||(image.color!=24))
 {
  Halt("Invalid image format");
 }
 if (head.type!=2)
 {
  if (head.type!=10)
  {
   Halt("Invalid image format");
  }

 }
 index=0;
 position=0;
 width=image.width;
 height=image.height;
 uncompressed_length=this->get_length();
 uncompressed=this->create_buffer(uncompressed_length);
 if (head.type==2)
 {
  target.read(uncompressed,uncompressed_length);
 }
 if (head.type==10)
 {
  compressed=this->create_buffer(compressed_length);
  target.read(compressed,compressed_length);
  while(index<uncompressed_length)
  {
   if (compressed[position]<128)
   {
    amount=compressed[position]+1;
    amount*=3;
    memmove(uncompressed+index,compressed+(position+1),amount);
    index+=amount;
    position+=1+amount;
   }
   else
   {
    for (amount=compressed[position]-127;amount>0;--amount)
    {
     memmove(uncompressed+index,compressed+(position+1),3);
     index+=3;
    }
    position+=4;
   }

  }
  delete[] compressed;
 }
 target.close();
 data=uncompressed;
}

void Image::load_pcx(const char *name)
{
 Input_File target;
 unsigned long int x,y;
 size_t index,position,line,row,length,uncompressed_length;
 unsigned char repeat;
 unsigned char *original;
 unsigned char *uncompressed;
 PCX_head head;
 this->clear_buffer();
 target.open(name);
 length=static_cast<size_t>(target.get_length()-128);
 target.read(&head,128);
 if ((head.color*head.planes!=24)&&(head.compress!=1))
 {
  Halt("Incorrect image format");
 }
 width=head.max_x-head.min_x+1;
 height=head.max_y-head.min_y+1;
 row=static_cast<size_t>(width)*3;
 line=static_cast<size_t>(head.planes)*static_cast<size_t>(head.plane_length);
 uncompressed_length=row*height;
 index=0;
 position=0;
 original=this->create_buffer(length);
 uncompressed=this->create_buffer(uncompressed_length);
 target.read(original,length);
 target.close();
 while (index<length)
 {
  if (original[index]<192)
  {
   uncompressed[position]=original[index];
   ++position;
   ++index;
  }
  else
  {
   for (repeat=original[index]-192;repeat>0;--repeat)
   {
    uncompressed[position]=original[index+1];
    ++position;
   }
   index+=2;
  }

 }
 delete[] original;
 original=this->create_buffer(uncompressed_length);
 for (x=0;x<width;++x)
 {
  for (y=0;y<height;++y)
  {
   index=static_cast<size_t>(x)*3+static_cast<size_t>(y)*row;
   position=static_cast<size_t>(x)+static_cast<size_t>(y)*line;
   original[index]=uncompressed[position+2*static_cast<size_t>(head.plane_length)];
   original[index+1]=uncompressed[position+static_cast<size_t>(head.plane_length)];
   original[index+2]=uncompressed[position];
  }

 }
 delete[] uncompressed;
 data=original;
}

unsigned long int Image::get_width() const
{
 return width;
}

unsigned long int Image::get_height() const
{
 return height;
}

size_t Image::get_length() const
{
 return static_cast<size_t>(width)*static_cast<size_t>(height)*3;
}

unsigned char *Image::get_data()
{
 return data;
}

void Image::destroy_image()
{
 width=0;
 height=0;
 this->clear_buffer();
}

Surface::Surface()
{
 width=0;
 height=0;
 image=NULL;
 surface=NULL;
}

Surface::~Surface()
{
 surface=NULL;
 if (image!=NULL) free(image);
}

IMG_Pixel *Surface::create_buffer(const unsigned long int image_width,const unsigned long int image_height)
{
 IMG_Pixel *result;
 size_t length;
 length=static_cast<size_t>(image_width)*static_cast<size_t>(image_height);
 result=reinterpret_cast<IMG_Pixel*>(calloc(length,3));
 if (result==NULL)
 {
  Halt("Can't allocate memory for image buffer");
 }
 return result;
}

void Surface::save()
{
 surface->save();
}

void Surface::restore()
{
 surface->restore();
}

void Surface::clear_buffer()
{
 if (image!=NULL)
 {
  free(image);
  image=NULL;
 }

}

void Surface::set_size(const unsigned long int image_width,const unsigned long int image_height)
{
 width=image_width;
 height=image_height;
}

void Surface::set_buffer(IMG_Pixel *buffer)
{
 image=buffer;
}

size_t Surface::get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y,const unsigned long int target_width)
{
 return static_cast<size_t>(start)+static_cast<size_t>(x)+static_cast<size_t>(y)*static_cast<size_t>(target_width);
}

size_t Surface::get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y) const
{
 return static_cast<size_t>(start)+static_cast<size_t>(x)+static_cast<size_t>(y)*static_cast<size_t>(width);
}

void Surface::draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
}

bool Surface::compare_pixels(const size_t first,const size_t second) const
{
 bool result;
 result=false;
 if (image[first].red!=image[second].red)
 {
  result=true;
  goto finish;
 }
 if (image[first].green!=image[second].green)
 {
  result=true;
  goto finish;
 }
 if (image[first].blue!=image[second].blue)
 {
  result=true;
  goto finish;
 }
 finish: ;
 return result;
}

unsigned long int Surface::get_surface_width() const
{
 return surface->get_frame_width();
}

unsigned long int Surface::get_surface_height() const
{
 return surface->get_frame_height();
}

void Surface::initialize(Screen *screen)
{
 surface=screen;
}

size_t Surface::get_length() const
{
 return static_cast<size_t>(width)*static_cast<size_t>(height)*3;
}

IMG_Pixel *Surface::get_image()
{
 return image;
}

void Surface::load_image(Image &buffer)
{
 width=buffer.get_width();
 height=buffer.get_height();
 this->clear_buffer();
 image=this->create_buffer(width,height);
 memmove(image,buffer.get_data(),buffer.get_length());
}

unsigned long int Surface::get_image_width() const
{
 return width;
}

unsigned long int Surface::get_image_height() const
{
 return height;
}

void Surface::mirror_image(const MIRROR_TYPE kind)
{
 unsigned long int x,y,index;
 IMG_Pixel *mirrored_image;
 x=0;
 y=0;
 mirrored_image=this->create_buffer(width,height);
 if (kind==MIRROR_HORIZONTAL)
 {
  for (index=width*height;index>0;--index)
  {
   mirrored_image[this->get_offset(0,x,y)]=image[this->get_offset(0,(width-x-1),y)];
   ++x;
   if (x==width)
   {
    x=0;
    ++y;
   }

  }

 }
 if (kind==MIRROR_VERTICAL)
 {
  for (index=width*height;index>0;--index )
  {
   mirrored_image[this->get_offset(0,x,y)]=image[this->get_offset(0,x,(height-y-1))];
   ++x;
   if (x==width)
   {
    x=0;
    ++y;
   }

  }

 }
 free(image);
 image=mirrored_image;
}

void Surface::resize_image(const unsigned long int new_width,const unsigned long int new_height)
{
 float x_ratio,y_ratio;
 unsigned long int x,y,steps;
 size_t index,location,position;
 IMG_Pixel *scaled_image;
 x=0;
 y=0;
 steps=new_width*new_height;
 scaled_image=this->create_buffer(new_width,new_height);
 x_ratio=static_cast<float>(width)/static_cast<float>(new_width);
 y_ratio=static_cast<float>(height)/static_cast<float>(new_height);
 for (index=0;index<steps;++index)
 {
  location=this->get_offset(0,x,y,new_width);
  position=this->get_offset(0,(x_ratio*static_cast<float>(x)),(y_ratio*static_cast<float>(y)),width);
  scaled_image[location]=image[position];
  ++x;
  if (x==new_width)
  {
   x=0;
   ++y;
  }

 }
 free(image);
 image=scaled_image;
 width=new_width;
 height=new_height;
}

void Surface::horizontal_mirror()
{
 this->mirror_image(MIRROR_HORIZONTAL);
}

void Surface::vertical_mirror()
{
 this->mirror_image(MIRROR_VERTICAL);
}

Animation::Animation()
{
 start=0;
 frame=1;
 frames=1;
}

Animation::~Animation()
{

}

void Animation::set_frame(const unsigned long int target)
{
 if (target>0)
 {
  if (target<=frames) frame=target;
 }

}

void Animation::increase_frame()
{
 ++frame;
 if (frame>frames)
 {
  frame=1;
 }

}

void Animation::set_frames(const unsigned long int amount)
{
 if (amount>1) frames=amount;
}

unsigned long int Animation::get_frames() const
{
 return frames;
}

unsigned long int Animation::get_frame() const
{
 return frame;
}

Background::Background()
{
 background_width=0;
 background_height=0;
 maximum_width=0;
 maximum_height=0;
 current=0;
 current_kind=NORMAL_BACKGROUND;
}

Background::~Background()
{

}

void Background::get_maximum_width()
{
 maximum_width=background_width;
 if (maximum_width>this->get_surface_width())
 {
  maximum_width=this->get_surface_width();
 }

}

void Background::get_maximum_height()
{
 maximum_height=background_height;
 if (maximum_height>this->get_surface_height())
 {
  maximum_height=this->get_surface_height();
 }

}

void Background::slow_draw_background()
{
 unsigned long int x,y,index;
 x=0;
 y=0;
 for (index=maximum_width*maximum_height;index>0;--index)
 {
  this->draw_image_pixel(this->get_offset(start,x,y),x,y);
  ++x;
  if (x==maximum_width)
  {
   x=0;
   ++y;
  }

 }

}

void Background::configure_background()
{
 switch(current_kind)
 {
  case NORMAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_BACKGROUND:
  background_width=this->get_image_width()/this->get_frames();
  background_height=this->get_image_height();
  start=(this->get_frame()-1)*background_width;
  break;
  case VERTICAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height()/this->get_frames();
  start=(this->get_frame()-1)*background_width*background_height;
  break;
 }

}

unsigned long int Background::get_width() const
{
 return background_width;
}

unsigned long int Background::get_height() const
{
 return background_height;
}

void Background::set_kind(const BACKGROUND_TYPE kind)
{
 current_kind=kind;
 this->configure_background();
 this->get_maximum_width();
 this->get_maximum_height();
}

void Background::set_setting(const BACKGROUND_TYPE kind,const unsigned long int frames)
{
 if (kind!=NORMAL_BACKGROUND) this->set_frames(frames);
 this->set_kind(kind);
}

void Background::set_target(const unsigned long int target)
{
 this->set_frame(target);
 this->set_kind(current_kind);
}

void Background::step()
{
 this->increase_frame();
 this->set_kind(current_kind);
}

void Background::draw_background()
{
 if (current!=this->get_frame())
 {
  this->slow_draw_background();
  this->save();
  current=this->get_frame();
 }
 else
 {
  this->restore();
 }

}

Sprite::Sprite()
{
 transparent=true;
 current_x=0;
 current_y=0;
 sprite_width=0;
 sprite_height=0;
 current_kind=SINGLE_SPRITE;
}

Sprite::~Sprite()
{

}

void Sprite::draw_transparent_sprite()
{
 unsigned long int x,y,index;
 x=0;
 y=0;
 for (index=sprite_width*sprite_height;index>0;--index)
 {
  if (this->compare_pixels(0,this->get_offset(start,x,y))==true)
  {
   this->draw_image_pixel(this->get_offset(start,x,y),x+current_x,y+current_y);
  }
  ++x;
  if (x==sprite_width)
  {
   x=0;
   ++y;
  }

 }

}

void Sprite::draw_normal_sprite()
{
 unsigned long int x,y,index;
 x=0;
 y=0;
 for (index=sprite_width*sprite_height;index>0;--index)
 {
  this->draw_image_pixel(this->get_offset(start,x,y),x+current_x,y+current_y);
  ++x;
  if (x==sprite_width)
  {
   x=0;
   ++y;
  }

 }

}

void Sprite::load_sprite(Image &buffer,const SPRITE_TYPE kind,const unsigned long int frames)
{
 this->load_image(buffer);
 if (kind!=SINGLE_SPRITE) this->set_frames(frames);
 this->set_kind(kind);
}

void Sprite::set_transparent(const bool enabled)
{
 transparent=enabled;
}

bool Sprite::get_transparent() const
{
 return transparent;
}

void Sprite::set_x(const unsigned long int x)
{
 current_x=x;
}

void Sprite::set_y(const unsigned long int y)
{
 current_y=y;
}

void Sprite::increase_x()
{
 ++current_x;
}

void Sprite::decrease_x()
{
 --current_x;
}

void Sprite::increase_y()
{
 ++current_y;
}

void Sprite::decrease_y()
{
 --current_y;
}

void Sprite::increase_x(const unsigned long int increment)
{
 current_x+=increment;
}

void Sprite::decrease_x(const unsigned long int decrement)
{
 current_x-=decrement;
}

void Sprite::increase_y(const unsigned long int increment)
{
 current_y+=increment;
}

void Sprite::decrease_y(const unsigned long int decrement)
{
 current_y-=decrement;
}

unsigned long int Sprite::get_x() const
{
 return current_x;
}

unsigned long int Sprite::get_y() const
{
 return current_y;
}

unsigned long int Sprite::get_width() const
{
 return sprite_width;
}

unsigned long int Sprite::get_height() const
{
 return sprite_height;
}

Sprite* Sprite::get_handle()
{
 return this;
}

Collision_Box Sprite::get_box() const
{
 Collision_Box target;
 target.x=current_x;
 target.y=current_y;
 target.width=sprite_width;
 target.height=sprite_height;
 return target;
}

void Sprite::set_kind(const SPRITE_TYPE kind)
{
 switch(kind)
 {
  case SINGLE_SPRITE:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_STRIP:
  sprite_width=this->get_image_width()/this->get_frames();
  sprite_height=this->get_image_height();
  start=(this->get_frame()-1)*sprite_width;
  break;
  case VERTICAL_STRIP:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height()/this->get_frames();
  start=(this->get_frame()-1)*sprite_width*sprite_height;
  break;
 }
 current_kind=kind;
}

SPRITE_TYPE Sprite::get_kind() const
{
 return current_kind;
}

void Sprite::set_target(const unsigned long int target)
{
 this->set_frame(target);
 this->set_kind(current_kind);
}

void Sprite::step()
{
 this->increase_frame();
 this->set_kind(current_kind);
}

void Sprite::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void Sprite::clone(Sprite &target)
{
 this->set_size(target.get_image_width(),target.get_image_height());
 this->set_frames(target.get_frames());
 this->set_kind(target.get_kind());
 this->set_transparent(target.get_transparent());
 this->set_buffer(this->create_buffer(target.get_image_width(),target.get_image_width()));
 memmove(this->get_image(),target.get_image(),target.get_length());
}

void Sprite::draw_sprite()
{
 if (transparent==true)
 {
  this->draw_transparent_sprite();
 }
 else
 {
  this->draw_normal_sprite();
 }

}

void Sprite::draw_sprite(const unsigned long int x,const unsigned long int y)
{
 this->set_position(x,y);
 this->draw_sprite();
}

void Sprite::draw_sprite(const bool transparency)
{
 this->set_transparent(transparency);
 this->draw_sprite();
}

void Sprite::draw_sprite(const bool transparency,const unsigned long int x,const unsigned long int y)
{
 this->set_transparent(transparency);
 this->draw_sprite(x,y);
}

Tileset::Tileset()
{
 offset=0;
 rows=0;
 columns=0;
 tile_width=0;
 tile_height=0;
}

Tileset::~Tileset()
{

}

unsigned long int Tileset::get_tile_width() const
{
 return tile_width;
}

unsigned long int Tileset::get_tile_height() const
{
 return tile_height;
}

unsigned long int Tileset::get_rows() const
{
 return rows;
}

unsigned long int Tileset::get_columns() const
{
 return columns;
}

void Tileset::select_tile(const unsigned long int row,const unsigned long int column)
{
 if ((row<rows)&&(column<columns))
 {
  offset=this->get_offset(0,row*tile_width,column*tile_height);
 }

}

void Tileset::draw_tile(const unsigned long int x,const unsigned long int y)
{
 unsigned long int tile_x,tile_y,index;
 tile_x=0;
 tile_y=0;
 for (index=tile_width*tile_height;index>0;--index)
 {
  this->draw_image_pixel(offset+this->get_offset(0,tile_x,tile_y),x+tile_x,y+tile_y);
  ++tile_x;
  if (tile_x==tile_width)
  {
   tile_x=0;
   ++tile_y;
  }

 }

}

void Tileset::draw_tile(const unsigned long int row,const unsigned long int column,const unsigned long int x,const unsigned long int y)
{
 this->select_tile(row,column);
 this->draw_tile(x,y);
}

void Tileset::load_tileset(Image &buffer,const unsigned long int row_amount,const unsigned long int column_amount)
{
 if ((row_amount>0)&&(column_amount>0))
 {
  this->load_image(buffer);
  rows=row_amount;
  columns=column_amount;
  tile_width=this->get_image_width()/rows;
  tile_height=this->get_image_height()/columns;
 }

}

Text::Text()
{
 current_x=0;
 current_y=0;
 font=NULL;
}

Text::~Text()
{

}

void Text::increase_position()
{
 font->increase_x(font->get_width());
}

void Text::restore_position()
{
 font->set_position(current_x,current_y);
}

void Text::set_position(const unsigned long int x,const unsigned long int y)
{
 font->set_position(x,y);
 current_x=x;
 current_y=y;
}

void Text::load_font(Sprite *target)
{
 font=target;
 font->set_frames(128);
 font->set_kind(HORIZONTAL_STRIP);
}

void Text::draw_character(const char target)
{
 font->set_target(static_cast<unsigned char>(target)+1);
 font->draw_sprite();
}

void Text::draw_text(const char *text)
{
 size_t index,length;
 length=strlen(text);
 this->restore_position();
 for (index=0;index<length;++index)
 {
  if (text[index]<32) continue;
  this->draw_character(text[index]);
  this->increase_position();
 }

}

void Text::draw_character(const unsigned long int x,const unsigned long int y,const char target)
{
 this->set_position(x,y);
 this->draw_character(target);
}

void Text::draw_text(const unsigned long int x,const unsigned long int y,const char *text)
{
 this->set_position(x,y);
 this->draw_text(text);
}

Collision::Collision()
{
 first.x=0;
 first.y=0;
 first.width=0;
 first.height=0;
 second=first;
}

Collision::~Collision()
{

}

void Collision::set_target(const Collision_Box &first_target,const Collision_Box &second_target)
{
 first=first_target;
 second=second_target;
}

bool Collision::check_horizontal_collision() const
{
 bool result;
 result=false;
 if ((first.x+first.width)>=second.x)
 {
  if (first.x<=(second.x+second.width)) result=true;
 }
 return result;
}

bool Collision::check_vertical_collision() const
{
 bool result;
 result=false;
 if ((first.y+first.height)>=second.y)
 {
  if (first.y<=(second.y+second.height)) result=true;
 }
 return result;
}

bool Collision::check_collision() const
{
 return this->check_horizontal_collision() || this->check_vertical_collision();
}

bool Collision::check_horizontal_collision(const Collision_Box &first_target,const Collision_Box &second_target)
{
 this->set_target(first_target,second_target);
 return this->check_horizontal_collision();
}

bool Collision::check_vertical_collision(const Collision_Box &first_target,const Collision_Box &second_target)
{
 this->set_target(first_target,second_target);
 return this->check_vertical_collision();
}

bool Collision::check_collision(const Collision_Box &first_target,const Collision_Box &second_target)
{
 this->set_target(first_target,second_target);
 return this->check_collision();
}

Collision_Box Collision::generate_box(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height) const
{
 Collision_Box result;
 result.x=x;
 result.y=y;
 result.width=width;
 result.height=height;
 return result;
}

}