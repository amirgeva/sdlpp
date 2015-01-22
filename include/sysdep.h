#ifndef sysdep_h__
#define sysdep_h__

namespace SDLPP
{

  unsigned get_tick_count();
  typedef void(*function)();
  function get_function(const char* module, const char* name);
  void display_message(const xstring& msg);

} // namespace SDLPP


#ifdef WIN32
#include <windows.h>
#define MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
#if defined(LINUX) || defined(__ANDROID__)
#define MAIN int main(int argc, char* argv[])
#endif


#endif // sysdep_h__
