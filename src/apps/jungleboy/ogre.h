#ifndef H_OGRE
#define H_OGRE

class Ogre : public AnimatedSprite
{
  bool m_Right;
  bool m_OnGround;
public:
  Ogre()
    : AnimatedSprite("rsc/ogre.xml"),
      m_Right(true),
      m_OnGround(true)
  {
    add_animation_object(this);
    set_active_sequence(0);
    set_acceleration(dVec2(0,200));
  }

  virtual void handle_collision(RigidBody2D* o, const dVec2& normal)
  {
    if (o->get("Boy")=="YES")
    {
      JungleBoy* jb=static_cast<JungleBoy*>(o);
      if (jb->is_vulnerable())
      {
        jb->die();
        set_active_sequence(m_Right?3:2);
      }
    }
    if (o->get("Floor")=="YES")
    {
      iRect2 r1=get_rect();
      iRect2 r2=o->get_rect();
      iRect2 res=r1.overlap(r2);
      if (res.get_height()<10 && get_velocity().y>0 && res.br.y==r1.br.y) 
      {
        offset_position(dVec2(0,-res.get_height()));
        set_velocity(dVec2(get_velocity().x,0));
        m_OnGround=true;
      }
      xstring edge_type=o->get("Edge");
      if (!edge_type.empty())
      {
        if (edge_type=="Right" && m_Right) m_Right=false;
        else
        if (edge_type=="Left" && !m_Right) m_Right=true;
      }
    }
  }

  virtual bool advance(int dt)
  {
    AnimatedSprite::advance(dt);
    if (get_active_sequence()>1)
    {
      if (get_current_frame()<2) return true;
    }
    if (m_Right)
    {
      set_active_sequence(1);
      set_velocity(dVec2(30,get_velocity().y));
    }
    else
    {
      set_active_sequence(0);
      set_velocity(dVec2(-30,get_velocity().y));
    }
    return true;
  }
};

#endif // H_OGRE


