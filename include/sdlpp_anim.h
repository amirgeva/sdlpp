#ifndef H_SDLPP_ANIMATION
#define H_SDLPP_ANIMATION

#include <sdlpp_physics.h>
#include <properties.h>

namespace SDLPP {

//////////////////////////////////////////////////
//
// 2D Animation functions.
//
//
//
//
//////////////////////////////////////////////////

class Sprite
{
  class Frame
  {
    Bitmap           image;
    int              duration;
    CollisionModel2D col_model;
  public:
    Bitmap                  get_bitmap() { return image; }
    void                    set_bitmap(Bitmap bmp) { image=bmp; }
    int                     get_duration() const { return duration; }
    void                    set_duration(int d) { duration=d; }

    CollisionModel2D& get_col_model()
    { 
      if (col_model.empty()) col_model.build(image);
      return col_model;
    }
  };

  typedef std::vector<Frame> frame_vec;
  struct Sequence
  {
    Sequence(const xstring& nm="", double base_v=0) 
      : total_duration(0), 
        name(nm),
        base_velocity(base_v)
    {}
    frame_vec   frames;
    int         total_duration;
    xstring     name;
    double      base_velocity;
  };
  typedef std::vector<Sequence> sequence_vec;
  typedef std::map<xstring,xstring> flags_map;

  sequence_vec m_Sequences;
  flags_map    m_Flags;
public:
  Sprite();
  virtual ~Sprite();
  Sprite(const Sprite& rhs);
  Sprite& operator= (const Sprite& rhs);
  
  int     get_sequences_count() const;
  xstring get_sequence_name(int seq) const;
  int     get_sequence_frame_count(int seq) const;

  int  add_animation_sequence(const xstring& name, double base_velocity);
  int  add_animation_frame(int seq, Bitmap bmp, int duration);
  void clear();

  void set_flag(const xstring& flag, const xstring& value);
  const xstring& get_flag(const xstring& flag) const;

  int advance_sequence(int seq, int& last_t, int dt, const dVec2& velocity);
  Bitmap            get_bitmap(int seq, int frame);
  CollisionModel2D& get_col_model(int seq, int frame);
};

class SpriteLoader : public Loader<Sprite>
{
public:
  virtual bool load(const xstring& name, Sprite& s) override;
};

class SpriteCache : public Cache<Sprite>
{
public:
  static SpriteCache* instance()
  {
    static std::unique_ptr<SpriteCache> ptr(new SpriteCache);
    return ptr.get();
  }

  static SpriteLoader& get_loader()
  {
    static SpriteLoader sl;
    return sl;
  }

private:
  friend struct std::default_delete<SpriteCache>;
  SpriteCache()  
  {
    set_loader(loader_ptr(new SpriteLoader));
  }
  ~SpriteCache() {}
  SpriteCache(const SpriteCache&) {}
};

inline Sprite& sprite(const xstring& name) 
{ 
  return SpriteCache::instance()->get(name); 
}

class AnimatedSprite : public RigidBody2D
{
  Sprite&          m_Sprite;
  int              m_ActiveSequence;
  int              m_DT;
  int              m_CurrentFrame;
  Bitmap           m_CurrentImage;
  bool             m_Volatile;
  PropertyBag      m_Properties;

  void init();
public:
  AnimatedSprite(Sprite& spr);
  AnimatedSprite(const xstring& spr_xml);

  int  get_sequences_count() const;
  xstring get_sequence_name(int seq) const;

  void set_active_sequence(int seq);
  int  get_active_sequence() const { return m_ActiveSequence; }
  int  get_sequence_frame_count(int seq) const;
  int  get_current_frame() const { return m_CurrentFrame; }
  void set_current_frame(int frame); 

  Bitmap get_current_image() { return m_CurrentImage; }

  // GameObject overrides
  virtual bool        advance(int dt);
  virtual void        render(GameView& gv);
  virtual bool        is_volatile() const { return m_Volatile; }
  virtual bool        is_static_sprite() const { return false; }
  // RigidBody2D overrides
  virtual iRect2             get_rect() const;

  iVec2 get_center_position() const
  {
    return get_position()+get_rect().get_size()/2;
  }

  virtual CollisionModel2D&  get_col_model() 
  { 
    int frame=(m_CurrentFrame<0?0:m_CurrentFrame);
    return m_Sprite.get_col_model(m_ActiveSequence,frame); 
  }

  void set_flag(const xstring& flag, const xstring& value) { m_Sprite.set_flag(flag,value); }
  virtual xstring        get_flag(const xstring& flag) { return m_Sprite.get_flag(flag); }

  void set(const xstring& name, const xstring& value) { m_Properties.set(name,value); }
  const xstring& get(const xstring& name) const { return m_Properties.get(name); }

};

class TileLayer : public Array2D<Sprite>, public GameObject
{
  typedef Array2D<Sprite> base;
public:
  TileLayer() {}
  TileLayer(int width, int height) : base(width,height) {}
  void render(GameView& view);
  bool advance(int dt);
};


class AnimationManager : public GameObject, public Singleton
{
public:
  static AnimationManager* instance()
  {
    static std::auto_ptr<AnimationManager> ptr(new AnimationManager);
    return ptr.get();
  }

  virtual void shutdown()
  {
    clear();
  }

  void start_animation_scene()
  {
    m_Scene=true;
  }

  void add_animation_object(RigidBody2D* obj)
  {
    if (!m_Scene) THROW("Animation Scene not started.");
    m_Objects.push_back(obj);
    if (obj->is_collidable()) m_CollidableObjects.push_back(obj);
  }

  virtual bool advance(int dt);
  virtual void render(GameView& view)
  {
    obj_list::iterator b=m_Objects.begin(),e=m_Objects.end();
    for(;b!=e;++b)
    {
      GameObject* obj=*b;
      obj->render(view);
    }
  }

  void clear();
private:
  void check_for_collisions(int dt);

  friend class std::auto_ptr<AnimationManager>;
  AnimationManager() : m_Scene(false) {}
  ~AnimationManager() {}
  AnimationManager(const AnimationManager&) {}

  typedef std::list<RigidBody2D*> obj_list;
  typedef obj_list::iterator iterator;
  obj_list m_Objects;
  obj_list m_CollidableObjects;
  bool     m_Scene;
public:
  typedef obj_list::const_iterator const_iterator;
  const_iterator begin() const { return m_Objects.begin(); }
  const_iterator end()   const { return m_Objects.end(); }
};

class AnimationScene
{
public:
  AnimationScene() { AnimationManager::instance()->start_animation_scene(); }
  ~AnimationScene() { AnimationManager::instance()->clear(); }
};

#define ANIMATION_SCENE AnimationScene l_##__LINE__

inline void add_animation_object(RigidBody2D* obj)
{
  AnimationManager::instance()->add_animation_object(obj);
}

inline void advance(int dt) 
{ 
  AnimationManager::instance()->advance(dt); 
}

inline void render(GameView& view) 
{ 
  AnimationManager::instance()->render(view); 
}


} // namespace SDLPP


#endif // H_SDLPP_ANIMATION


