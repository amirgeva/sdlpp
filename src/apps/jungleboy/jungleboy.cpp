#include <sdlpp.h>
#include <sdlpp_io.h>

using namespace SDLPP;

extern bool g_next_screen;
extern bool g_game_over;
extern int  g_food_left;
extern int  g_lives;

#include "boy.h"
#include "ogre.h"
#include "cloud.h"
#include "food.h"
#include "pickup.h"
#include "platforms.h"

void JungleBoy::activate_effect()
{
  effect_list::iterator it=m_Effects.begin();
  if (m_EffectPress>1) std::advance(it,m_EffectPress-1);
  effect_ptr e=it->first;
  e->apply(this);
  m_Effects.erase(it);
  m_EffectPress=0;
}

