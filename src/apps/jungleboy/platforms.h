#ifndef H_PLATFORMS
#define H_PLATFORMS

class GrassFloor : public AnimatedSprite
{
public:
  enum Section { LEFT, CENTER, RIGHT };
  GrassFloor(Section which=CENTER)
    : AnimatedSprite("rsc/bgrass.xml")
  {
    int seq=rand()%6;
    if (which==LEFT)  seq=6;
    if (which==RIGHT) seq=7;
    set("Floor","YES");
    if (which==LEFT) set("Edge","Left");
    if (which==RIGHT) set("Edge","Right");
    set_active_sequence(seq);
    add_animation_object(this);
  }

  virtual bool is_static_sprite() const { return true; }
};

#endif // H_PLATFORMS

