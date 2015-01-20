#include <sdlpp.h>

namespace SDLPP {

void EventManager::register_listener(SDL_EventType event_type, EventListener* listener)
{
  m_Listeners.insert(std::make_pair(event_type,listener));
}

void EventManager::remove_listener(SDL_EventType event_type, EventListener* listener)
{
  std::pair<listener_map::iterator,listener_map::iterator> p;
  p=m_Listeners.equal_range(event_type);
  listener_map::iterator it;
  for(it=p.first;it!=p.second;++it)
  {
    if (it->second == listener) 
    {
      m_Listeners.erase(it);
      break;
    }
  }
}


void EventManager::poll()
{
  SDL_Event e;
  int rc=1;
  while (rc)
  {
    //SDL_JoystickUpdate();
    if ((rc=SDL_PollEvent(&e))!=0)
    {
      std::pair<listener_map::iterator,listener_map::iterator> p;
      p=m_Listeners.equal_range(SDL_EventType(e.type));
      bool handled=false;
      for(listener_map::iterator it=p.first;it!=p.second;++it)
      {
        EventListener* l=it->second;
        if (l->handle_event(e)) { handled=true; break; }
      }      
      if (handled) continue;
      switch (e.type)
      {
//         case SDL_ACTIVEEVENT:			/* Application loses/gains visibility */
//           break;
        case SDL_KEYDOWN:			/* Keys pressed */
          {
            const SDL_Keysym& key=e.key.keysym;
            set_key_state(key.sym, 1);
            //m_KeysState[key.sym]=1;
            m_KeyQueue.push_back(key.sym);
            //GUI::instance()->raise_event("Keyboard","Key");
          }
          break;
        case SDL_KEYUP:			/* Keys released */
          {
            const SDL_Keysym& key=e.key.keysym;
            set_key_state(key.sym, 0);
            //m_KeysState[key.sym]=0;
          }
          break;
        case SDL_MOUSEMOTION:			/* Mouse moved */
          m_MousePosition=iVec2(e.motion.x,e.motion.y);
          break;
        case SDL_MOUSEBUTTONDOWN:		/* Mouse button pressed */
          m_MouseButtons[e.button.button]=true;
          break;
        case SDL_MOUSEBUTTONUP:		/* Mouse button released */
          m_MouseButtons[e.button.button]=false;
          break;
        case SDL_JOYAXISMOTION:		/* Joystick axis motion */
          m_Joysticks[e.jaxis.which].axes[e.jaxis.axis]=double(e.jaxis.value)*(1.0/32768.0);
          break;
        case SDL_JOYBALLMOTION:		/* Joystick trackball motion */
          m_Joysticks[e.jball.which].balls[e.jball.ball]=iVec2(e.jball.xrel,e.jball.yrel);
          break;
        case SDL_JOYHATMOTION:		/* Joystick hat position change */
          m_Joysticks[e.jhat.which].hats[e.jhat.hat]=e.jhat.value;
          break;
        case SDL_JOYBUTTONDOWN:		/* Joystick button pressed */
          m_Joysticks[e.jbutton.which].buttons[e.jbutton.button]=true;
          break;
        case SDL_JOYBUTTONUP:			/* Joystick button released */
          m_Joysticks[e.jbutton.which].buttons[e.jbutton.button]=false;
          break;
        case SDL_QUIT:			/* User-requested quit */
          break;
        case SDL_SYSWMEVENT:			/* System specific event */
          break;
//         case SDL_EVENT_RESERVEDA:		/* Reserved for future use.. */
//           break;
//         case SDL_EVENT_RESERVEDB:		/* Reserved for future use.. */
//           break;
//         case SDL_VIDEORESIZE:			/* User resized video mode */
//           break;
//         case SDL_VIDEOEXPOSE:			/* Screen needs to be redrawn */
//           break;
      }
    }
  }

}

bool EventManager::is_pressed(SDL_Keycode key)
{
  return (m_KeysState[key&1023]!=0);
}

iVec2 EventManager::get_mouse_position()
{
  return m_MousePosition;
}

bool  EventManager::is_mouse_button_pressed(int button)
{
  return m_MouseButtons[button];
}

void EventManager::shutdown()
{
  m_Joysticks.clear();
  //gui_stub();
}

void EventManager::init_joysticks()
{
  int n=SDL_NumJoysticks();
  m_Joysticks.resize(n);
  for(int i=0;i<n;++i)
  {
    Joystick& j=m_Joysticks[i];
    j.init(i);
  }
}

Joystick::Joystick(int i)
  : joystick(0)
{ 
  if (i>=0) init(i);
}

Joystick::~Joystick()
{
  if (joystick) SDL_JoystickClose(joystick);
}

void Joystick::init(int i)
{
  joystick=SDL_JoystickOpen(i); 
  axes.resize(SDL_JoystickNumAxes(joystick));
  hats.resize(SDL_JoystickNumHats(joystick));
  buttons.resize(SDL_JoystickNumButtons(joystick),false);
  balls.resize(SDL_JoystickNumBalls(joystick));
}

void Joystick::update()
{
  SDL_JoystickUpdate();
}



} // namespace SDLPP

