#ifndef H_BOY
#define H_BOY

#include <sdlpp_sound.h>
#include "pickup_base.h"
#include "common.h"

extern bool g_only_keyboard;
void print_food();
typedef std::pair<effect_ptr,Bitmap> epair;
typedef std::list<epair> effect_list;

class JungleBoy : public AnimatedSprite
{
  bool m_LastRight;
  int  m_OnGround;
  bool m_Dying;
  sound_clip_ptr m_EatSound;
  sound_clip_ptr m_AaahhSound;
  sound_clip_ptr m_DeathSound;
  int       m_DeathDuration;
  effect_list m_Effects;
  int       m_EffectPress;
  int       m_Shield;
  bool      m_ShieldBlink;
  Joystick* m_Joystick;

  void activate_effect();
public:
  JungleBoy()
    : AnimatedSprite("rsc/boy.xml"),
      m_EatSound(new SoundClip("rsc/eat2.wav")),
      m_AaahhSound(new SoundClip("rsc/aaahh.wav")),
      m_DeathSound(new SoundClip("rsc/death.wav")),
      m_LastRight(true),
      m_OnGround(0),
      m_Dying(false),
      m_DeathDuration(0),
      m_EffectPress(0),
      m_Shield(0),
      m_ShieldBlink(true),
      m_Joystick(0)
  {
    set("Boy","YES");
    set_acceleration(dVec2(0,200)); // Gravity
    if (!g_only_keyboard && EventManager::instance()->get_joystick_count()>0)
    {
      std::cout << "Using joystick\n";
      m_Joystick=&EventManager::instance()->get_joystick(0);
    }
  }

  void reset()
  {
    set_velocity(dVec2(0,0));
    add_animation_object(this);
    set_active_sequence(1);  // Standing
    m_DeathDuration=0;
    m_Dying=false;
    m_LastRight=true;
    m_OnGround=0;
    m_EffectPress=0;
  }

  int get_death_duration() const { return m_DeathDuration; }

  void take_pickup(effect_ptr effect, Bitmap image)
  {
    play(m_EatSound);
    m_Effects.push_back(std::make_pair(effect,image));
  }

  void eat_food()
  {
    g_score+=10;
    play(m_EatSound);
    if (--g_food_left==0) g_next_screen=true;
    print_food();
  }

  void add_shield(int n)
  {
    m_Shield+=n;
  }

  void die()
  {
    if (!m_Dying)
    {
      play(m_DeathSound);
      set_active_sequence(4);
      m_Dying=true;
      m_DeathDuration=0;
    }
  }

  bool is_vulnerable() const { return m_Shield==0; }

  virtual void handle_collision(RigidBody2D* o, const dVec2& normal)
  {
    if (o->get("Floor")=="YES")
    {
      iRect2 r1=get_rect();
      iRect2 r2=o->get_rect();
      iRect2 res=r1.overlap(r2);
      if (res.get_height()<10 && get_velocity().y>0 && res.br.y==r1.br.y) 
      {
        offset_position(dVec2(0,-res.get_height()));
        set_velocity(dVec2(get_velocity().x,0));
        m_OnGround=15;
      }
    }
    else
    {
      o->handle_collision(this,-normal);
    }
  }

  virtual void render(GameView& gv)
  {
    if (m_Shield==0 || !m_ShieldBlink)
      AnimatedSprite::render(gv);
    if (!m_Effects.empty())
    {
      Font& f=game_font();
      int x=300;
      effect_list::iterator b=m_Effects.begin(),e=m_Effects.end();
      for(int i=1;b!=e;++b,++i)
      {
        std::ostringstream os; os << i;
        Bitmap bmp=b->second;
        x+=f.draw(x,5,os.str(),MapRGB(255,0,255)).x;
        bmp.draw(x,5);
        x+=bmp.get_width()+5;
      }
    }
  }

  bool is_joy(double x, double y)
  {
    if (!m_Joystick) return false;
    double X=m_Joystick->axes[0];
    double Y=m_Joystick->axes[1];
    return ((x!=0 && udiff(x,X)<0.5) || (y!=0 && udiff(y,Y)<0.5));
  }

  virtual bool advance(int dt)
  {
    --m_OnGround;
    AnimatedSprite::advance(dt);
    if (m_Joystick) m_Joystick->update();
    if (m_Shield>0)
    {
      m_Shield=Max(0,m_Shield-dt);
      m_ShieldBlink=!m_ShieldBlink;
    }
    if (get_position().y>450 && !m_Dying)
    {
      play(m_AaahhSound);
      m_Dying=true;
    }
    if (m_Dying)
    {
      m_DeathDuration+=dt;
      if (get_current_frame()==11) set_position(iVec2(100,500));
      else return true;
    }
    if (m_EffectPress>0)
    {
      if (!is_pressed(SDL_Keycode(SDLK_0+m_EffectPress)))
        activate_effect();
    }
    else
    {
      for(int i=0;i<int(m_Effects.size());++i)
        if (is_pressed(SDL_Keycode(SDLK_1+i))) m_EffectPress=i+1;
    }
    int act_seq=get_active_sequence();
    if ((is_pressed(SDLK_UP) || is_pressed(SDLK_KP_8) || is_joy(0,-1)) && m_OnGround>0)
    {
      set_velocity(dVec2(get_velocity().x,-200));
      set_acceleration(dVec2(0,get_acceleration().y));
      m_OnGround=0;
    }
    if ((is_pressed(SDLK_DOWN) || is_pressed(SDLK_KP_2) || is_joy(0,1)) && m_OnGround>0 && (act_seq==1 || act_seq==3 || act_seq>4))
    {
      if (act_seq<4)
        set_active_sequence(act_seq==1?5:6);
    }
    else
    if ((is_pressed(SDLK_RIGHT) || is_pressed(SDLK_KP_6) || is_joy(1,0)) && m_OnGround>0)
    {
      m_LastRight=true;
      set_active_sequence(0);
      dVec2 v=get_velocity();
      if (v.x<200) 
        set_acceleration(dVec2(300,get_acceleration().y));
      else
        set_acceleration(dVec2(0,get_acceleration().y));
    }
    else
    if ((is_pressed(SDLK_LEFT) || is_pressed(SDLK_KP_4) || is_joy(-1,0)) && m_OnGround>0)
    {
      m_LastRight=false;
      set_active_sequence(2);
      dVec2 v=get_velocity();
      if (v.x>-200) 
        set_acceleration(dVec2(-300,get_acceleration().y));
      else
        set_acceleration(dVec2(0,get_acceleration().y));
    }
    else
    {
      dVec2 v=get_velocity();
      if (fabs(v.x)>1)
      {
        if (m_OnGround>0)
        {
          double ax=100;
          dVec2 a=get_acceleration();
          a.x=(v.x>0 ? -ax : ax);
          set_acceleration(a);
        }
        else
        {
          dVec2 a=get_acceleration();
          a.x=(v.x>0 ? -20 : 20);
          set_acceleration(a);
        }
      }
      else
      {
        set_velocity(dVec2(0,v.y));
        set_acceleration(dVec2(0,get_acceleration().y));
        set_active_sequence(m_LastRight?1:3);
      }
    }
    return true;
  }
};

#endif // H_BOY


