#include <sdlpp.h>
#include <sysdep.h>

namespace SDLPP {


Sprite::Sprite()
{}

Sprite::~Sprite()
{}

Sprite::Sprite(const Sprite& rhs)
: m_Sequences(rhs.m_Sequences)
{}

Sprite& Sprite::operator= (const Sprite& rhs)
{
  m_Sequences=rhs.m_Sequences;
  return *this;
}

int Sprite::add_animation_sequence(const xstring& name, double base_velocity)
{
  int res=int(m_Sequences.size());
  m_Sequences.push_back(Sequence(name,base_velocity));
  return res;
}

int Sprite::add_animation_frame(int seq, Bitmap bmp, int duration)
{
  if (seq<0 || seq>=int(m_Sequences.size())) 
    THROW("Invalid sequence number");
  if (duration<=0)
    THROW("Invalid frame duration: "+xstring(duration));
  Sequence& sequence=m_Sequences[seq];
  frame_vec& fv=sequence.frames;
  int res=fv.size();
  fv.push_back(Frame());
  fv.back().set_bitmap(bmp);
  fv.back().set_duration(duration);
  sequence.total_duration+=duration;
  return res;
}

void Sprite::clear()
{
  m_Sequences.clear();
}

void Sprite::set_flag(const xstring& flag, const xstring& value)
{
  m_Flags[flag]=value;
}

const xstring& Sprite::get_flag(const xstring& flag) const
{
  static const xstring empty_flag="";
  flags_map::const_iterator it=m_Flags.find(flag);
  if (it==m_Flags.end()) return empty_flag;
  return it->second;
}

Bitmap Sprite::get_bitmap(int seq, int frame)
{
  if (seq<0 || seq>=int(m_Sequences.size())) 
    THROW("Invalid sequence number");
  Sequence& sequence=m_Sequences[seq];
  if (frame<0 || frame>=int(sequence.frames.size()))
    THROW("Invalid frame number");
  return sequence.frames[frame].get_bitmap();
}

CollisionModel2D& Sprite::get_col_model(int seq, int frame)
{
  if (seq<0 || seq>=int(m_Sequences.size())) 
    THROW("Invalid sequence number");
  Sequence& sequence=m_Sequences[seq];
  if (frame<0 || frame>=int(sequence.frames.size()))
    THROW("Invalid frame number");
  return sequence.frames[frame].get_col_model();
}

int  Sprite::get_sequences_count() const
{
  return m_Sequences.size();
}

xstring Sprite::get_sequence_name(int seq) const
{
  if (seq<0 || seq>=get_sequences_count()) return "";
  return m_Sequences[seq].name;
}

int Sprite::get_sequence_frame_count(int seq) const
{
  if (seq<0 || seq>=get_sequences_count()) return 0;
  return m_Sequences[seq].frames.size();
}

int Sprite::advance_sequence(int seq, int& last_t, int dt, const dVec2& velocity)
{
  //LOG("last_t="+num2str(last_t)+"  dt="+num2str(dt));
  if (seq<0 || seq>=int(m_Sequences.size())) 
    THROW("Invalid sequence number");
  Sequence& sequence=m_Sequences[seq];
  double axial_velocity=velocity.magnitude();
  {
    xstring dir=get_flag("AnimDir");
    if (dir=="X") axial_velocity=velocity.x;
    if (dir=="Y") axial_velocity=velocity.y;
  }
  double mult=1.0;
  if (sequence.base_velocity != 0.0  && axial_velocity>0.001) 
    mult=sequence.base_velocity / axial_velocity;
  frame_vec& fv=sequence.frames;
  if (fv.empty())
    THROW("Advancing empty sequence.");
  frame_vec::iterator b=fv.begin(),e=fv.end();
  int total_duration=0;
  for(;b!=e;++b)
  {
    //int total_duration=int(sequence.total_duration*mult);
    total_duration+=int(b->get_duration()*mult);
  }
  b=fv.begin();
  int t=0,endt=last_t+dt;
  while (endt>=total_duration) 
    endt-=total_duration;
  int i=0;
  while (true)
  {
    if (b==e) { b=fv.begin(); t=0; i=0; }
    Frame& f=*b;
    int duration=int(f.get_duration()*mult);
    if ((t+duration) > endt)
    {
      //LOG("Sprite frame: "+num2str(i));
      last_t=endt;
      return i;
    }
    t+=duration;
    ++b;
    ++i;
  }
  return -1;
}

bool SpriteLoader::load(const xstring& name, Sprite& s)
{
  //ResourceFile* rf = get_default_resource_file();
  xml_element* root=load_xml(name);
  if (!root) THROW("Resource not found: " << name);
  int ck=atoi(root->get_attribute("ColorKey").c_str());
  Uint32 color_key=Uint32(ck);
  xml_element::iterator seq_b=root->begin(),seq_e=root->end();
  for(;seq_b!=seq_e;++seq_b)
  {
    xml_element* sequence=*seq_b;
    if (sequence->get_type()!="Sequence") continue;
    xstring name=sequence->get_attribute("Name");
    double base_velocity=atof(sequence->get_attribute("BaseVelocity").c_str());
    int seq_id=s.add_animation_sequence(name,base_velocity);
    xml_element::iterator frb=sequence->begin(),fre=sequence->end();
    for(;frb!=fre;++frb)
    {
      xml_element* frame=*frb;
      if (frame->get_type()!="Frame") continue;
      xstring image_name=frame->get_attribute("Image");
      xstring rect_str=frame->get_attribute("Rect");
      Bitmap base_image=BitmapCache::instance()->load(image_name,color_key);
      Bitmap image;
      if (!rect_str.empty())
      {
        iRect2 r=parse_rect(rect_str);
        image = base_image.cut(r);
      }
      else
        image=base_image;
      //if (rf) image=Bitmap(*rf,image_name);
      //else image=Bitmap(image_name);
      //image.set_colorkey(color_key);
      int duration=atoi(frame->get_attribute("Duration").c_str());
      s.add_animation_frame(seq_id,image,duration);
    }
  }
  xml_element::iterator flb=root->begin(),fle=root->end();
  for(;flb!=fle;++flb)
  {
    xml_element* flag=*flb;
    if (flag->get_type()!="Flag") continue;
    xstring name=flag->get_attribute("Name");
    xstring value=flag->get_attribute("Value");
    s.set_flag(name,value);
  }
  return true;
}



AnimatedSprite::AnimatedSprite(Sprite& spr) 
  : m_Sprite(spr),
    m_ActiveSequence(0),
    m_CurrentFrame(-1),
    m_Volatile(false),
    m_DT(0)
{
  init();
}

AnimatedSprite::AnimatedSprite(const xstring& spr_xml)
  : m_Sprite(sprite(spr_xml)),
    m_ActiveSequence(0),
    m_CurrentFrame(-1),
    m_Volatile(false),
    m_DT(0)
{
  init();
}

void AnimatedSprite::init()
{
  xstring smass=get_flag("Mass");
  if (!smass.empty()) set_mass(atof(smass.c_str()));
  m_Volatile=(!get_flag("Volatile").empty());
  xstring spos=get_flag("Position");
  if (!spos.empty())
  {
    int p=spos.find(',');
    int x=atoi(spos.substr(0,p).c_str());
    int y=atoi(spos.substr(p+1).c_str());
    set_position(iVec2(x,y));
  }
}

int  AnimatedSprite::get_sequences_count() const
{
  return m_Sprite.get_sequences_count();
}

xstring AnimatedSprite::get_sequence_name(int seq) const
{
  return m_Sprite.get_sequence_name(seq);
}

void AnimatedSprite::set_active_sequence(int seq)
{
  if (m_ActiveSequence!=seq) m_CurrentFrame=-1;
  m_ActiveSequence=seq; 
}

int  AnimatedSprite::get_sequence_frame_count(int seq) const
{
  return m_Sprite.get_sequence_frame_count(seq);
}

void AnimatedSprite::set_current_frame(int frame)
{
  if (frame>=0 && frame<get_sequence_frame_count(get_active_sequence()))
  {
    m_CurrentFrame=frame;
    m_CurrentImage=m_Sprite.get_bitmap(m_ActiveSequence,m_CurrentFrame);
  }
}

bool AnimatedSprite::advance(int dt)
{
  if (is_static_sprite()) 
  {
    if (m_CurrentFrame<0)
    {
      m_CurrentFrame=m_Sprite.advance_sequence(m_ActiveSequence,m_DT,dt,get_velocity());
      m_CurrentImage=m_Sprite.get_bitmap(m_ActiveSequence,m_CurrentFrame);
    }
    return true;
  }
  if (!RigidBody2D::advance(dt)) return false;
  int cur_frame=m_Sprite.advance_sequence(m_ActiveSequence,m_DT,dt,get_velocity());
  if (m_CurrentFrame!=cur_frame)
  {
    m_CurrentFrame=cur_frame;
    m_CurrentImage=m_Sprite.get_bitmap(m_ActiveSequence,m_CurrentFrame);
  }    
  return true;
}

void AnimatedSprite::render(GameView& gv)
{
  const iRect2& view=gv.get_2D_view();
  iRect2 src=m_CurrentImage.get_rect();
  iVec2 p=get_position();
  p-=gv.get_2D_offset();
  iVec2 half_size=src.get_size();
  //half_size.x/=2; half_size.y/=2;
  //p-=half_size;
  iRect2 clip=src;
  clip+=p;
  clip.intersect(view);
  if (clip.is_valid())
  {
    src=clip;
    src-=p;
    p+=src.tl;
    m_CurrentImage.draw(src,p);
  }
}

iRect2 AnimatedSprite::get_rect() const
{
  iVec2 tl=get_position();
  iVec2 size=m_CurrentImage.get_size();
  iVec2 br=tl+size;
  return iRect2(tl,br);
}

inline bool y_pred(const RigidBody2D* a, const RigidBody2D* b)
{
  iRect2 ra=a->get_rect();
  iRect2 rb=b->get_rect();
  return ra.tl.y<rb.tl.y;
}

void AnimationManager::check_for_collisions(int dt)
{
  m_CollidableObjects.sort(y_pred);
  iterator it,b=m_CollidableObjects.begin(),e=m_CollidableObjects.end();
  for(;b!=e;++b)
  {
    RigidBody2D* o1=*b;
    int bottom_y=o1->get_rect().br.y;
    for(it=b;++it!=e;)
    {
      RigidBody2D* o2=*it;
      if (o2->get_rect().tl.y>=bottom_y) break;
      o1->interact(o2,dt);
    }
  }
}

void AnimationManager::clear()
{
  obj_list::iterator b=m_Objects.begin(),e=m_Objects.end();
  for(;b!=e;++b)
  {
    GameObject* obj=*b;
    if (obj->is_volatile()) delete obj;
  }
  m_Objects.clear();
  m_CollidableObjects.clear();
  m_Scene=false;
}

bool AnimationManager::advance(int DT)
{
  int dt=Min(100,DT);
  for(int i=0;i<DT;i+=dt)
  {
    dt=Min(dt,(DT-i));
    obj_list::iterator b=m_Objects.begin(),e=m_Objects.end();
    while(b!=e)
    {
      GameObject* obj=*b;
      if (!obj->advance(dt)) 
      {
        if (obj->is_volatile()) delete obj;
        obj_list::iterator ci=std::find(m_CollidableObjects.begin(),m_CollidableObjects.end(),obj);
        if (ci!=m_CollidableObjects.end()) m_CollidableObjects.erase(ci);
        b=m_Objects.erase(b);
      }
      else ++b;
    }
    check_for_collisions(dt);
  }
  return true;
}

void TileLayer::render(GameView& view)
{
  iRect2 window=view.get_2D_view();
  iVec2 tile_size=(*this)(0,0).get_bitmap(0,0).get_size();
  window.tl/=tile_size;
  window.br/=tile_size;
  for(int y=window.tl.y;y<=window.br.y;++y)
  {
    for(int x=window.tl.x;x<=window.br.x;++x)
    {
      Bitmap bmp=(*this)(x,y).get_bitmap(0,0);
      iVec2 p(x,y);
      p*=tile_size;
      p-=window.tl;
      bmp.draw(p);
    }
  }
}

bool TileLayer::advance(int dt)
{
  return true;
}



} // namespace SDLPP
