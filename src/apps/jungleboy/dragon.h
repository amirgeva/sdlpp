#ifndef H_DRAGON
#define H_DRAGON

class Dragon : public AnimatedSprite
{
public:
  Dragon() : AnimatedSprite("rsc/dragon.xml")
  {
    add_animation_object(this);
    int r=irand(2);
    int y=irand(15)*32;
    if (r==0)
    {
      set_position(iVec2(-50,y));
      set_velocity(dVec2(100,0));
      set_active_sequence(1);
    }
    else
    {
      set_position(iVec2(650,y));
      set_velocity(dVec2(-100,0));
      set_active_sequence(0);
    }
  }

  virtual void handle_collision(RigidBody2D* o, const dVec2& normal)
  {
    if (o->get("Boy")=="YES")
    {
      JungleBoy* jb=static_cast<JungleBoy*>(o);
      if (jb->is_vulnerable())
      {
        jb->die();
      }
    }
  }

  virtual bool advance(int dt)
  {
    AnimatedSprite::advance(dt);
    double x=get_position().x;
    double sx=get_velocity().x;
    if (x<-100 && sx<0) return false;
    if (x>700  && sx>0) return false;
    return true;
  }
};

#endif // H_DRAGON