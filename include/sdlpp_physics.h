#ifndef H_SDLPP_PHYSICS
#define H_SDLPP_PHYSICS

#include <sdlpp_common.h>

namespace SDLPP {

class BitRow
{
  enum { BITS=8*sizeof(unsigned) };
  typedef std::vector<unsigned> base_vec;
  base_vec m_Data;
  int      m_Size;

  unsigned get_word(int i) const { if (i<0 || i>=int(m_Data.size())) return 0; return m_Data[i]; }
  unsigned get_word_at(int offset) const
  {
    int w=offset/BITS;
    int o=offset-(w*BITS);
    unsigned word=(get_word(w)>>o);
    if (o>0)
    {
      unsigned next=get_word(w+1);
      next<<=(BITS-o);
      word|=next;
    }
    return word;
  }

  int get_first_one(unsigned u) const
  {
    for(int i=0;i<BITS && u;++i,u>>=1)
      if ((u&1)!=0) return i;
    return 0;
  }
public:
  BitRow(int size) 
    : m_Data((size+BITS-1)/BITS,0), 
      m_Size(size)
  {}
  void set(int i, bool state=true)
  {
    if (i<0 || i>=m_Size) return;
    int wi=i/BITS;
    int oi=i-(wi*BITS);
    unsigned& word=m_Data[wi];
    unsigned value=(1<<oi);
    if (state) word|=value;
    else word&=~value;
  }

  unsigned get_bits(int offset, int len) const
  {
    unsigned word=get_word_at(offset);
    unsigned mask=-1;
    mask <<= len;
    mask = ~mask;
    return word&mask;
  }

  int test(const BitRow& br, int offset, int br_offset) const
  {
    if (offset<0) return br.test(*this,-offset,0);
    int len=Min(m_Size-offset,br.m_Size-br_offset);
    for(int i=0;i<len;i+=BITS)
    {
      unsigned w=get_word_at(i+offset);
      unsigned brw=br.get_word_at(i+br_offset);
      w&=brw;
      if (w) return i+get_first_one(w);
    }
    return -1;
  }
};

class CollisionModel2D
{
  typedef std::vector<BitRow>  Grid;
  Grid   m_Grid;
  iRect2 m_Rect;

  const BitRow& get_row(int y) const
  {
    static const BitRow empty_row(64);
    if (y<0 || y>=int(m_Grid.size())) return empty_row;
    return m_Grid[y];
  }

public:
  CollisionModel2D() {}
  CollisionModel2D(Bitmap image);
  bool empty() const { return m_Grid.empty(); }
  bool test(const CollisionModel2D& o, iVec2& offset);
  unsigned get_bits(const iVec2& offset, int len) const;
  void build(Bitmap image);
};

  
class RigidBody2D : public GameObject
{
  dVec2       m_Position;
  dVec2       m_Velocity;     // Linear
  dVec2       m_Acceleration;

  // 2D Angular motion & position is specified by a scalar angle
  // Angle is composed of: base angle + variable angle
  // since total angle of 0 has a vector of (0,-1), using a non-zero base angle
  // helps setting a default 'forward' direction for the object
  double      m_Angle;
  double      m_BaseAngle;
  double      m_AVelocity;    // Angular
  double      m_AAcceleration;

  double      m_Mass;
  xstring     m_Name;

  dVec2 get_collision_normal(CollisionModel2D& cm, const iVec2& offset);
public:
  RigidBody2D(double mass=0) 
    : m_Mass(mass)
    , m_Angle(0)
    , m_BaseAngle(0)
    , m_AVelocity(0)
    , m_AAcceleration(0) 
  {}

  virtual void               interact(RigidBody2D* o, int dt);
  virtual iRect2             get_rect() const = 0;
  virtual bool               is_collidable() const { return true; }
  virtual CollisionModel2D&  get_col_model() = 0;
  virtual xstring            get_flag(const xstring& flag) = 0;
  virtual const xstring&     get(const xstring& property) const = 0;
  virtual void               handle_collision(RigidBody2D* o, const dVec2& normal) {}
  virtual bool               advance(int dt)
  {
    double DT=dt*0.001;
    m_AVelocity+=m_AAcceleration*DT;
    m_Angle+=m_AVelocity*DT;
    m_Angle=fmod(m_Angle,2.0*PI);
    m_Velocity+=m_Acceleration*DT;
    m_Position+=m_Velocity*DT;
    return true;
  }

  double get_mass() const { return m_Mass; }
  void   set_mass(double m) { m_Mass=m; }
  iVec2 get_position() const { return iVec2(int(m_Position.x),int(m_Position.y)); }
  dVec2 get_position(int) const { return m_Position; }
  const dVec2& get_velocity() const { return m_Velocity; }
  const dVec2& get_acceleration() const { return m_Acceleration; }
  
  void  offset_position(const dVec2& dp)
  {
    m_Position+=dp;
  }
  
  void  set_position(const iVec2& p) 
  { 
    m_Position=dVec2(p.x,p.y); 
  }
  
  void  set_position(const dVec2& p) 
  { 
    m_Position=p;
  }

  void  set_velocity(const dVec2& v) 
  { 
    m_Velocity=v; 
  }

  void  set_velocity(const dVec2& direction, double speed)
  { 
    set_velocity(direction.normalized()*speed); 
  }
  void  set_acceleration(const dVec2& a) { m_Acceleration=a; }
  void  set_acceleration(const dVec2& direction, double magnitude) 
  { 
    set_acceleration(direction.normalized()*magnitude); 
  }
  void  set_acceleration(double magnitude)
  {
    dVec2 dir(magnitude*sin(m_BaseAngle+m_Angle),-magnitude*cos(m_BaseAngle+m_Angle));
    set_acceleration(dir);
  }
  
  void set_angular_velocity(double v) { m_AVelocity=v; }
  void set_angular_acceleration(double a) { m_AAcceleration=a; }
  void set_angle(double a) { m_Angle=a; }
  void set_base_angle(double a) { m_BaseAngle=a; }
  double get_base_angle() const { return m_BaseAngle; }
  double get_angle() const { return m_Angle; }
  double get_angular_velocity() const { return m_AVelocity; }
  double get_angular_acceleration() const { return m_AAcceleration; }

  void set_name(const xstring& name) { m_Name=name; }
  const xstring& get_name() const { return m_Name; }
};

class FrameTimer
{
  int m_LastTicks;
public:
  FrameTimer() : m_LastTicks(SDL_GetTicks()) {}
  int calc_dt()
  {
    int cur=SDL_GetTicks();
    int dt=cur-m_LastTicks;
    m_LastTicks=cur;
    return dt;
  }
};


} // namespace SDLPP

#endif // H_SDLPP_PHYSICS

