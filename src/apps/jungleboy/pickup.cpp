#include <sdlpp.h>

using namespace SDLPP;

#include "boy.h"
#include "ogre.h"
#include "cloud.h"
#include "food.h"
#include "pickup.h"
#include "platforms.h"


class FlyingShoes : public PickupEffect
{
public:
  virtual void apply(JungleBoy* boy)
  {
    boy->set_velocity(dVec2(boy->get_velocity().x,-350));
  }
};

REGISTER_EFFECT(FlyingShoes);

class NextScreen : public PickupEffect
{
public:
  virtual void apply(JungleBoy* boy)
  {
    g_next_screen=true;
  }
};

REGISTER_EFFECT(NextScreen);


class ExtraLife : public PickupEffect
{
public:
  virtual void apply(JungleBoy* boy)
  {
    g_lives++;
  }
};

REGISTER_EFFECT(ExtraLife);


class Shield : public PickupEffect
{
public:
  virtual void apply(JungleBoy* boy)
  {
    boy->add_shield(4000);
  }
};

REGISTER_EFFECT(Shield);