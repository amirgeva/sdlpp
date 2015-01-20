#ifndef H_2D_VECTOR
#define H_2D_VECTOR

#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>
#include <sstream>
#include <xstring.h>

namespace SDLPP {

const double PI = 3.1415926535897932384626433832795;
const double epsilon = 0.0001;

template<class T>
inline T Min(const T& a, const T& b)
{
  return (a<b?a:b);
}

template<class T>
inline T Max(const T& a, const T& b)
{
  return (a>b?a:b);
}


template<class T>
struct Vec2
{
  typedef Vec2<T> self;

  T x,y;

  Vec2(const Vec2<T>& v) : x(v.x), y(v.y) {}
  Vec2(const T t) : x(t), y(t) {}
  Vec2(const T _x=0, const T _y=0) : x(_x), y(_y) {}
  Vec2(const T* t) : x(t[0]), y(t[1]) {}

  template<class U>
  explicit Vec2(const Vec2<U>& u) : x(T(u.x)), y(T(u.y)) {}

#ifdef _WINDEF_
  Vec2(const POINT& p) : x(p.x), y(p.y) {}
  operator POINT() const { POINT p = {x, y}; return p; }
#endif

  const T& operator[] (const int i) const { return (&x)[i]; }
  T& operator[] (const int i)             { return (&x)[i]; }

  self& operator+=(const self& v)  { x += v.x; y += v.y; return *this; }
  self& operator-=(const self& v)  { x -= v.x; y -= v.y; return *this; }
  self& operator*=(const self& v)  { x *= v.x; y *= v.y; return *this; }
  self& operator*=(const T& s)     { x *= s;  y *= s;  return *this; }
  self& operator/=(const self& v)  { x /= v.x; y /= v.y; return *this; }
  self& operator/=(const T& s)     { x /= s;  y /= s;  return *this; }

  const self& operator+() const    { return *this; }
  self operator-() const           { return self(-x, -y); }

  T square_magnitude() const       { return x*x + y*y; }
  double magnitude() const         { return sqrt(double(square_magnitude())); }

  self normalized() const          { return (*this / magnitude()); }
  self& normalize()                { return *this /= magnitude(); }

  static const self Zero() { static self z(0,0); return z; }
};

template<class T>
class Rect2
{
public:
  typedef Rect2<T> self;
  typedef Vec2<T> point;
  Rect2() : tl(-1,-1), br(-2,-2) {}
  Rect2(const point& topleft, const point& bottomright) : tl(topleft), br(bottomright) {}
  Rect2(const T x0, const T y0, const T x1, const T y1) : tl(x0,y0), br(x1,y1) {}

#ifdef _WINDEF_
  Rect2(const RECT& r) : tl(r.left, r.top), br(r.right, r.bottom) {}
  RECT  toRECT() const    { RECT r = {tl.x, tl.y, br.x, br.y}; return r; }
#endif

  T get_width()  const   { return (br.x - tl.x); }
  T get_height() const   { return (br.y - tl.y); }

  point get_size() const { return point(get_width(), get_height()); }

  self& operator+=(const point& v)  { tl += v; br += v; return *this; }
  self& operator-=(const point& v)  { tl -= v; br -= v; return *this; }

  self translate(const point& v) const { return self(tl + v, br + v); }

  self& expand(const point& v) { tl-=v; br+=v; return *this; }

/*
  friend self operator+(const self&, const point&);
  friend self operator-(const self&, const point&);

  friend bool operator==(const self&, const self&);
  friend bool operator!=(const self&, const self&);
*/

  self& normalize() // makes sure tl,br follow intuitive position.
  {
    if (tl.x>br.x) Swap(tl.x,br.x);
    if (tl.y>br.y) Swap(tl.y,br.y);
    return *this;
  }
// intersection
  //friend self operator*(const self&, self&); 
  self& operator*=(const self&);

  bool inside(const point& v) const
  {
    return (v.x >= tl.x && v.x <= br.x && v.y >= tl.y && v.y <= br.y);
  }

  bool is_null() const   { return tl.x >= br.x || tl.y >= br.y; }
  bool is_valid() const  { return !is_null(); }

  bool contains(int x, int y) const
  {
    return (tl.x<=x && tl.y<=y && br.x>x && br.y>y);
  }

  bool contains(const point& pos) const
  {
    return contains(pos.x,pos.y);
  }

  self& unite(const Vec2<int>& p)
  {
    tl.x=Min(tl.x,p.x);
    tl.y=Min(tl.y,p.y);
    br.x=Max(br.x,p.x+1);
    br.y=Max(br.y,p.y+1);
    return *this;
  }

  self& unite(const self& r)
  {
    tl.x=Min(tl.x,r.tl.x);
    tl.y=Min(tl.y,r.tl.y);
    br.x=Max(br.x,r.br.x);
    br.y=Max(br.y,r.br.y);
    return *this;
  }

  self& intersect(const self& r)
  {
    tl.x=Max(tl.x,r.tl.x);
    tl.y=Max(tl.y,r.tl.y);
    br.x=Min(br.x,r.br.x);
    br.y=Min(br.y,r.br.y);
    return *this;
  }

  bool overlapping(const self& r) const
  {
    return (tl.x < r.br.x && tl.y < r.br.y && br.x > r.tl.x && br.y > r.tl.y);
  }

  self overlap(const self& r) const
  {
    return self(Maximize(tl,r.tl),Minimize(br,r.br));
  }

  point tl,br;
};

template<class T>
std::ostream& operator<< (std::ostream& os, const Vec2<T>& v)
{
  return os << v.x << ',' << v.y;
}

template<class T>
inline Vec2<T> operator+(const Vec2<T>& u, const Vec2<T>& v)
{
  return Vec2<T>(u.x+v.x, u.y+v.y);
}

template<class T>
inline Vec2<T> operator-(const Vec2<T>& u, const Vec2<T>& v)
{
  return Vec2<T>(u.x-v.x, u.y-v.y); 
}

template<class T>
inline Vec2<T> operator/(const Vec2<T>& u, const Vec2<T>& v)
{
  return Vec2<T>(u.x/v.x, u.y/v.y); 
}

template<class T>
inline Vec2<T> operator*(const Vec2<T>& v, const T& s)
{
  return Vec2<T>(v.x*s, v.y*s); 
}

template<class T>
inline Vec2<T> operator*(const int s, const Vec2<T>& v)
{
  return Vec2<T>(s*v.x, s*v.y);
}

template<class T>
inline Vec2<T> operator*(const T& s, const Vec2<T>& v)
{
  return Vec2<T>((int)(s*v.x), (int)(s*v.y));
}

template<class T>
inline Vec2<T> operator/(const Vec2<T>& v, const T& s)
{
  return Vec2<T>(v.x/s, v.y/s); 
}

template<class T>
inline bool operator==(const Vec2<T>& u, const Vec2<T>& v)
{
  return (u.x == v.x && u.y == v.y);
}

template<class T>
inline bool operator!= (const Vec2<T>& u, const Vec2<T>& v)
{
  return (!(u==v));
}

template<class T>
inline bool operator<(const Vec2<T>& u, const Vec2<T>& v)
{
  if (u.y==v.y) return u.x<v.x;
  return u.y<v.y;
}

template<class T>
inline bool operator<=(const Vec2<T>& u, const Vec2<T>& v)
{
  return (u<v || u==v);
}

template<class T>
inline bool operator>(const Vec2<T>& u, const Vec2<T>& v)
{
  return (!(u<=v));
}

template<class T>
inline bool operator>=(const Vec2<T>& u, const Vec2<T>& v)
{
  return (!(u<v));
}

template<class T>
inline T operator*(const Vec2<T>& u, const Vec2<T>& v)
{
  return u.x*v.x + u.y*v.y; 
}

template<class T>
inline Vec2<T> Minimize(const Vec2<T>& u, const Vec2<T>& v)
{
  return Vec2<T>(Min(u.x,v.x),Min(u.y,v.y));
}

template<class T>
inline Vec2<T> Maximize(const Vec2<T>& u, const Vec2<T>& v)
{
  return Vec2<T>(Max(u.x,v.x),Max(u.y,v.y));
}


template<class T>
inline Rect2<T> operator+(const Rect2<T>& r, const Vec2<T>& v)
{
  return (Rect2<T>(r.tl + v, r.br + v));
}

template<class T>
inline Rect2<T> operator-(const Rect2<T>& r, const Vec2<T>& v)
{
  return (Rect2<T>(r.tl - v, r.br - v));
}

template<class T>
inline bool operator==(const Rect2<T>& q, const Rect2<T>& r)
{
  return q.tl == r.tl && q.br == r.br;
}

template<class T>
inline bool operator!=(const Rect2<T>& q, const Rect2<T>& r)
{
  return !(q == r);
}

template<class T>
inline Rect2<T> operator*(const Rect2<T>& q, const Rect2<T>& r)
{
  Rect2<T> t(Maximize(q.tl, r.tl), Minimize(q.br, r.br));

  if (t.tl > t.br)
    return Rect2<T>();  // no intersection

  return t;
}

template<class T>
inline Rect2<T>& Rect2<T>::operator*=(const Rect2<T>& r)
{
  return *this = *this * r; 
}

typedef Vec2<int> iVec2;
typedef Vec2<double> dVec2;
typedef Rect2<int> iRect2;

struct convert_char : public std::unary_function<char,char>
{
  char m_From;
  char m_To;
  convert_char(char from, char to) : m_From(from), m_To(to) {}
  char operator() (char c) const { return (c==m_From?m_To:c); }
};

inline iRect2 parse_rect(xstring s)
{
  std::transform(s.begin(),s.end(),s.begin(),convert_char(',',' '));
  std::istringstream is(s);
  iRect2 r;
  is >> r.tl.x >> r.tl.y >> r.br.x >> r.br.y;
  return r;
}

template<class T>
inline std::ostream& operator<< (std::ostream& os, const Rect2<T>& rect)
{
  return os << rect.tl.x << ',' << rect.tl.y << ','
            << rect.br.x << ',' << rect.br.y;
}

template<class T>
class Array2D
{
  typedef std::vector<T> array;
  array m_Array;
  iVec2 m_Size;
public:
  Array2D() 
    : m_Size(0,0)
  {}

  Array2D(int width, int height)
    : m_Array(width*height),
      m_Size(width,height)
  {}

  Array2D(int width, int height, const T& def) 
    : m_Array(width*height,def),
      m_Size(width,height)
  {}

  void resize(int width, int height, const T& def=0) 
  {
    m_Array.resize(width*height,def);
    m_Size=iVec2(width,height);
  }

  const iVec2& size() const { return m_Size; }
  int get_width() const  { return size().x; }
  int get_height() const { return size().y; }

  T& operator() (int x, int y)
  {
    if (x<0 || x>=m_Size.x || y<0 || y>=m_Size.y) throw(xstring("Invalid coords"));
    return m_Array[y*m_Size.x+x];
  }

  const T& operator() (int x, int y) const
  {
    if (x<0 || x>=m_Size.x || y<0 || y>=m_Size.y) throw(xstring("Invalid coords"));
    return m_Array[y*m_Size.x+x];
  }

  typedef typename array::iterator iterator;
  typedef typename array::const_iterator const_iterator;
  iterator begin() { return m_Array.begin(); }
  iterator end()   { return m_Array.end(); }
  const_iterator begin() const { return m_Array.begin(); }
  const_iterator end()   const { return m_Array.end(); }
};

} // namespace SDLPP 

#endif // H_2D_VECTOR

