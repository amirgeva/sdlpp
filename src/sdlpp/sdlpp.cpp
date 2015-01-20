#include <sdlpp.h>
#include <sysdep.h>

#ifdef _DEBUG
std::ofstream flog("test.log");
void LOG(const xstring& s) { flog << s << std::endl; }
#endif

namespace SDLPP {

static ResourceFile  g_FileSytemResourceFile(0);
static ResourceFile* g_DefaultResourceFile=&g_FileSytemResourceFile;

void set_default_resource_file(ResourceFile* rf)
{
  g_DefaultResourceFile=rf;
}

ResourceFile* get_default_resource_file()
{
  return g_DefaultResourceFile;
}

int calculate_dt()
{
  static Uint32 last_tick=0;
  Uint32 cur=SDL_GetTicks();
  int dt=20;
  if (last_tick!=0) dt=cur-last_tick;
  last_tick=cur;
  return dt;
}


 
xstring hexstr(unsigned n)
{
  static const xstring hex="0123456789ABCDEF";
  xstring res;
  for(int i=0;i<8;++i)
  {
    res+=hex.substr(n&15,1);
    n>>=4;
  }
  std::reverse(res.begin(),res.end());
  return res;
}

void SingletonManager::shutdown()
{
  std::list<Singleton*>::iterator b=m_Singletons.begin(),e=m_Singletons.end();
  for(;b!=e;++b)
  {
    Singleton* s=*b;
    s->shutdown();
  }
  m_Singletons.clear();
}

Application::Application()
{
  int rc=SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER);
  if (rc<0) THROW(SDL_GetError());
  SDL_JoystickEventState(SDL_ENABLE);
  //SDL_EnableUNICODE(1);
}

Application::~Application()
{
  SingletonManager::instance()->shutdown();
  SDL_Quit();
}

void Application::init_graphics(int width, int height, bool full_screen)
{
  Graphics::instance()->initialize(width,height,full_screen);
}

void Application::flip_graphics()
{
  Graphics::instance()->flip();
}

void Application::init_audio(int freq, bool stereo)
{
  SoundManager::instance()->initialize(freq,stereo);
  SoundManager::instance()->pause(false);
}











} // namespace SDLPP
