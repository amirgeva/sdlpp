#ifndef H_FOOD
#define H_FOOD

class Food : public AnimatedSprite
{
  static const char* random_food()
  {
    static const char* names[] = {
      "rsc/food/apple.xml",
      "rsc/food/bagel.xml",
      "rsc/food/banana.xml",
      "rsc/food/berry.xml",
      "rsc/food/carrot.xml",
      "rsc/food/grapes.xml",
      "rsc/food/orange.xml",
      "rsc/food/pear.xml",
      "rsc/food/tberry.xml",
      "rsc/food/watermelon.xml"
    };
    static int n = sizeof(names)/sizeof(const char*);
    return names[irand(n)];
  }

  bool m_Eaten;
public:
  Food()
    : AnimatedSprite(random_food()),
      m_Eaten(false)
  {
    add_animation_object(this);
  }

  virtual void handle_collision(RigidBody2D* o, const dVec2& normal)
  {
    if (o->get("Boy")=="YES" && !m_Eaten)
    {
      JungleBoy* jb=static_cast<JungleBoy*>(o);
      jb->eat_food();
      m_Eaten=true;
    }
  }

  virtual bool advance(int dt)
  {
    if (!AnimatedSprite::advance(dt)) return false;
    return (!m_Eaten);
  }
};

#endif // H_FOOD

