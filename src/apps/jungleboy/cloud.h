#ifndef H_CLOUD
#define H_CLOUD

class Cloud : public AnimatedSprite
{
  static xstring get_name()
  {
    int n=rand()%3+1;
    std::ostringstream os;
    os << "rsc/cloud" << n << ".xml";
    return os.str();
  }
public:
  Cloud()
    : AnimatedSprite(get_name())
  {
    set_position(iVec2(rand()%540,rand()%400));
    add_animation_object(this);
  }
  virtual bool is_collidable() const { return false; }
  virtual bool is_static_sprite() const { return true; }
};

#endif // H_CLOUD

