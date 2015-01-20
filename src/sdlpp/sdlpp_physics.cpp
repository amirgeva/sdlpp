#include <sdlpp.h>

namespace SDLPP {

CollisionModel2D::CollisionModel2D(Bitmap image)
{
  build(image);
}

void CollisionModel2D::build(Bitmap image)
{
  m_Grid.clear();
  m_Grid.resize(image.get_height(),BitRow(image.get_width()));
  m_Rect=iRect2(0,0,image.get_width(),image.get_height());
  Uint32 ck=image.get_colorkey();
  int w=image.get_width(),h=image.get_height();
  //SDL_Surface* s=image.get_surface();
  for(int y=0;y<h;++y)
  {
    //Uint32* row=(Uint32*)((char*)s->pixels+y*s->pitch);
    const Uint32* row = image.get_row(y);
    BitRow& br=m_Grid[y];
    for(int x=0;x<w;++x)
    {
      if (row[x]!=ck) br.set(x);
    }
  }
}

unsigned CollisionModel2D::get_bits(const iVec2& offset, int len) const
{
  if (offset.y<0 || offset.y>=int(m_Grid.size())) return 0;
  const BitRow& br=m_Grid[offset.y];
  return br.get_bits(offset.x,len);
}

bool CollisionModel2D::test(const CollisionModel2D& o, iVec2& offset)
{
  iRect2 orect=o.m_Rect;
  orect+=offset;
  iRect2 overlap=m_Rect.overlap(orect);
  int h=overlap.get_height();
  int oy=overlap.tl.y-offset.y;
  for(int y=0;y<h;++y)
  {
    const BitRow& br=get_row(y+overlap.tl.y);
    const BitRow& obr=o.get_row(oy+y);
    int hit=br.test(obr,offset.x,0);
    if (hit>=0)
    {
      offset=iVec2(offset.x+hit,offset.y+y);
      return true;
    }
  }
  return false;
}

dVec2 RigidBody2D::get_collision_normal(CollisionModel2D& cm, const iVec2& offset)
{
  unsigned w[] = { cm.get_bits(offset-iVec2(1,1),3),
                   cm.get_bits(offset-iVec2(1,0),3),
                   cm.get_bits(offset-iVec2(1,-1),3) };
  dVec2 cg;
  int n=0;
  for(int y=0;y<3;++y)
  {
    unsigned wy=w[y];
    for(int x=0;x<3;++x)
    {
      if ((wy&(1<<x))!=0) 
      {
        cg+=dVec2(x,y);
        ++n;
      }
    }
  }
  cg*=(1.0/n);
  dVec2 center(1,1);
  center-=cg;
  if (center.magnitude()<epsilon) return dVec2(0,-1);
  return center.normalized();
}

void RigidBody2D::interact(RigidBody2D* o, int dt)
{
  RigidBody2D* rb1=this;
  RigidBody2D* rb2=o;
  if (rb1->get_mass()<rb2->get_mass()) std::swap(rb1,rb2);

  iRect2 r=rb1->get_rect();
  iRect2 r2=rb2->get_rect();
  if (!r.overlapping(r2)) return;

  CollisionModel2D& cm=rb1->get_col_model();
  CollisionModel2D& ocm=rb2->get_col_model();
  iVec2 offset=r2.tl-r.tl;
  if (cm.test(ocm,offset))
  {
    dVec2 dir=rb1->get_collision_normal(cm,offset);
    rb1->handle_collision(rb2,-dir);
    rb2->handle_collision(rb1,dir);
    /*
    dVec2 dir=rb1->get_collision_normal(cm,offset);
    SoundClip("boing.wav").play();

    // Collision response test code.  Will be upgraded to more realistic physics
    //dVec2 total_momentum=get_velocity()*get_mass() + o->get_velocity()*o->get_mass();
    //double momentum_mag=total_momentum.magnitude();
    //double new_mon2=momentum_mag/o->get_mass();


    dVec2 v=rb2->get_velocity();
    dVec2 nv=(v*dir)*dir;
    v-=2*nv;
    rb2->offset_position(-rb2->get_velocity()*(dt*0.001));
    rb2->set_velocity(v);
    */
  }
}


} // namespace SDLPP


