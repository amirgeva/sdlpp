#ifndef H_PICKUP_BASE
#define H_PICKUP_BASE

#include <memory>


class PickupEffect
{
public:
  virtual ~PickupEffect() {}
  virtual void apply(class JungleBoy* boy) = 0;
};

typedef std::shared_ptr<PickupEffect> effect_ptr;

#endif // H_PICKUP_BASE
