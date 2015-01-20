#ifndef H_SDLPP
#define H_SDLPP

#include <sdlpp_common.h>
#include <sdlpp_io.h>
#include <sdlpp_graphics.h>
#include <sdlpp_font.h>
#include <sdlpp_anim.h>
#include <sdlpp_input.h>
#include <sdlpp_sound.h>
#include <sysdep.h>

namespace SDLPP {

/** Main application class.  Create and hold one object for the entire program run time.
    Use the various init functions after construction to notify the system as to what 
    subsystems will be used and at what setting.
*/
class Application
{
public:
  Application();
  ~Application();

  /** Initialize the audio subsystem.
      Specify sampling frequency in freq
      Specify whether you need mono or stereo audio.
  */
  void init_audio(int freq, bool stereo);

  /** Initialize the graphics subsystem.
      Specify the size of the graphics screen and whether it should run in a window or 
      on the entire screen.
  */
  void init_graphics(int width, int height, bool full_screen);

  /** Flips the graphics back buffer to the front.
      Used in animation and any other smooth graphics movement. */
  void flip_graphics();
};

/** Called once per frame, calculates the time in ms for the frame
    Useful for the parameter to call advance() */
int  calculate_dt();

} // namespace SDLPP

// #include <sdlpp_sound.h>
// #include <sdlpp_anim.h>
// #include <sdlpp_input.h>

#ifdef _DEBUG
void LOG(const xstring& s);
#else
#define LOG(x) ;
#endif


#endif // H_SDLPP


