#ifndef H_PICKUP
#define H_PICKUP


#include "pickup_base.h"

class PickupEffectCreator
{
public:
  virtual ~PickupEffectCreator() {}
  virtual effect_ptr create() const = 0;
};

class PickupEffectFactory : public Singleton
{
public:
  /** Returns a pointer to the factory object */
  static PickupEffectFactory* instance(bool destroy=false)
  {
    static PickupEffectFactory* ptr=0;
    if (!ptr && !destroy) ptr=new PickupEffectFactory;
    else
    if (ptr && destroy) { delete ptr; ptr=0; }
    return ptr;
  }
  virtual void shutdown() { instance(true); }

  void register_creator(const xstring& name, PickupEffectCreator* c)
  {
    m_Creators[name]=c;
  }

  effect_ptr create(const xstring& name)
  {
    crt_map::iterator it=m_Creators.find(name);
    if (it==m_Creators.end()) return 0;
    return it->second->create();
  }

private:
  PickupEffectFactory() {}
  ~PickupEffectFactory() {}
  PickupEffectFactory(const PickupEffectFactory&) {}

  typedef std::map<xstring,PickupEffectCreator*> crt_map;
  crt_map m_Creators;
};

#define REGISTER_EFFECT(x) class x##_Creator : public PickupEffectCreator { public: \
x##_Creator() { PickupEffectFactory::instance()->register_creator(#x,this); }\
effect_ptr create() const { return effect_ptr(new x); } } g_##x##_Creator


class Pickup : public AnimatedSprite
{
  effect_ptr    m_Effect;
  int           m_Type;
  bool          m_Taken;
public:
  Pickup()
  : AnimatedSprite("rsc/pickup.xml")
  , m_Taken(false)
  {
    add_animation_object(this);
    int n=get_sequences_count();
    m_Type=irand(n);
    set_active_sequence(m_Type);
    m_Effect=PickupEffectFactory::instance()->create(get_sequence_name(m_Type));
  }

  virtual void handle_collision(RigidBody2D* o, const dVec2& normal)
  {
    if (o->get("Boy")=="YES" && !m_Taken)
    {
      JungleBoy* jb=static_cast<JungleBoy*>(o);
      jb->take_pickup(m_Effect,get_current_image());
      m_Taken=true;
    }
  }

  virtual bool advance(int dt)
  {
    if (!AnimatedSprite::advance(dt)) return false;
    return !m_Taken;
  }
};

#endif // H_PICKUP

