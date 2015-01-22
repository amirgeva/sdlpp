#ifndef H_SDLPP_INPUT
#define H_SDLPP_INPUT

#include <vector>
#include <sdlpp_common.h>
#include <SDL.h>
#include <vec2d.h>

namespace SDLPP {

class EventListener
{
public:
  virtual ~EventListener() {}
  /// Handle the incoming event.   Return true if event was handled and should not
  /// be passed to other handlers
  virtual bool handle_event(const SDL_Event&) = 0;
};

struct Joystick
{
public:
  std::vector<double> axes;
  std::vector<Uint8>  hats;
  std::vector<bool>   buttons;
  std::vector<iVec2>  balls;

  void update();

  /// Do not construct your own joystick.  
  /// Use the EventManager::get_joystick function instead
  Joystick(int i=-1);
  ~Joystick();
  void init(int i);
private:
  SDL_Joystick*       joystick;
};

class EventManager : public Singleton
{
public:
  static EventManager* instance()
  {
    static std::unique_ptr<EventManager> ptr(new EventManager);
    return ptr.get();
  }

  void   register_listener(SDL_EventType event_type, EventListener* listener);
  void   remove_listener(SDL_EventType event_type, EventListener* listener);
  void   poll();
  bool   is_pressed(SDL_Keycode key);
  void   set_key_state(SDL_Keycode key, int value) { m_KeysState[key & 1023] = value; }
  iVec2  get_mouse_position();
  bool   is_mouse_button_pressed(int button);
  bool   is_key_available() const { return !m_KeyQueue.empty(); }
  Uint16 get_key() { Uint16 k=m_KeyQueue.front(); m_KeyQueue.pop_front(); return k; }
  void   clear_keys() { m_KeyQueue.clear(); }

  int    get_joystick_count() const { return m_Joysticks.size(); }
  Joystick& get_joystick(int i)
  {
    static Joystick Null;
    if (i<0 || i>=get_joystick_count()) return Null;
    return m_Joysticks[i];
  }

  bool touching_rect(const iRect2& rect) const;

  void shutdown();
private:
  friend struct std::default_delete<EventManager>;
  EventManager() : m_KeysState(1024,0), m_MouseButtons(32,false)
  {
    init_joysticks();
  }
  ~EventManager() {}
  EventManager(const EventManager&) {}
  void init_joysticks();

  typedef std::multimap<SDL_EventType, EventListener*> listener_map;
  typedef std::vector<char> key_vec;
  iVec2                 m_MousePosition;
  key_vec               m_KeysState;
  listener_map          m_Listeners;
  std::vector<bool>     m_MouseButtons;
  std::vector<Joystick> m_Joysticks;
  std::map<int,iVec2>   m_Touches;
  std::list<Uint16>     m_KeyQueue;
};

#define LISTEN_FOR_EVENT(x) EventManager::instance()->register_listener(x,this)

class QuitListener : public EventListener
{
  bool m_Quit;
public:
  QuitListener() 
    : m_Quit(false)
  {
    EventManager::instance()->register_listener(SDL_QUIT,this);
  }
  virtual bool handle_event(const SDL_Event&) 
  { 
    m_Quit=true; 
    return false; 
  }
  bool quit() const { return m_Quit; }
};



inline void poll() { EventManager::instance()->poll(); }
inline bool is_pressed(SDL_Keycode key) { return EventManager::instance()->is_pressed(key); }
inline iVec2 get_mouse_position() { return EventManager::instance()->get_mouse_position(); }
inline bool  is_mouse_button_pressed(int button) { return EventManager::instance()->is_mouse_button_pressed(button); }
inline void clear_key(SDL_Keycode key) { while (is_pressed(key)) { poll(); SDL_Delay(100); } }
inline void clear_button(int button) { while (is_mouse_button_pressed(button)) { poll(); SDL_Delay(100); } }
} // namespace SDLPP


#endif // H_SDLPP_INPUT


