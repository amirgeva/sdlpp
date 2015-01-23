#ifndef H_SDLPP_COMMON
#define H_SDLPP_COMMON

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <set>
#include <iterator>
#include <algorithm>
#include <deque>
#include <sstream>

#include <vec2d.h>
#include <random.h>
#include <xstring.h>
#include <SDL.h>

#define THROW(x) {\
  std::ostringstream os; os << __FILE__ << ":" << __LINE__ << " - " << x;\
  display_message(os.str());\
  throw xstring(os.str()); }

namespace SDLPP {

  const double infinity = 1e300;
  typedef std::vector<int> int_vec;
  typedef std::vector<char> char_vec;
  typedef std::vector<Uint8> uint8_vec;
  typedef std::vector<xstring> str_vec;
  typedef std::map<int,int> int_map;

  inline SDL_Rect R(const iRect2& r)
  {
    SDL_Rect res = { r.tl.x, r.tl.y, r.br.x - r.tl.x, r.br.y - r.tl.y };
    return res;
  }

  template<class T>
  inline xstring str(T t)
  {
    return xstring(t);
  }

  template<>
  inline xstring str(const char* s)
  {
	  if (!s) return "";
	  return s;
  }

  template<class T>
  inline T udiff(const T& a, const T& b)
  {
    return (a>b?a-b:b-a);
  }

  inline bool is_zero(double value, double eps=0.001)
  {
    return udiff(value,0.0)<eps;
  }

  inline int irand(int limit)
  {
    return int(urnd(limit));
  }

  inline double frand(double limit)
  {
    return urnd(limit);
  }

  template<class T>
  class ObjectCreator
  {
  public:
    ObjectCreator(int n) { for(int i=0;i<n;++i) new T; }
  };

  class Singleton
  {
  public:
    Singleton();
    virtual ~Singleton() {}
    virtual void shutdown() = 0;
  };

  class SingletonManager
  {
  public:
    static SingletonManager* instance()
    {
      static std::auto_ptr<SingletonManager> ptr(new SingletonManager);
      return ptr.get();
    }

    void register_singleton(Singleton* b)
    {
      m_Singletons.push_back(b);
    }

    void shutdown();
  private:
    friend class std::auto_ptr<SingletonManager>;
    SingletonManager() {}
    ~SingletonManager() { shutdown(); }
    SingletonManager(const SingletonManager&) {}

    std::list<Singleton*> m_Singletons;
  };

  /*
  class RefCount
  {
    int* m_Counter;
  public:
    RefCount() : m_Counter(new int(1)) {}
    RefCount(const RefCount& rhs) : m_Counter(rhs.m_Counter) { addref(); }
    RefCount& operator= (const RefCount& rhs) 
    { 
      subref(); 
      m_Counter=rhs.m_Counter; 
      addref(); 
      return *this; 
    }
    virtual ~RefCount() { subref(true); }

  protected:
    virtual void destroy() = 0;
    bool is_last_copy() const { return getref()==1; }
    int  getref() const { return *m_Counter; }
    void addref()
    {
      ++*m_Counter;
    }
    void subref(bool final=false)
    {
      if (--*m_Counter == 0) 
      {
        if (!final) destroy();
        delete m_Counter;
        m_Counter=0;
      }
    }
  };
  */

  template<class T>
  class Loader
  {
  public:
    virtual ~Loader() {}
    virtual bool load(const xstring& name, T&) = 0;
  };

  /** Generic cache model.  Retrieve objects by name (xstring)
      object loader is always custom and is set by user.
      Caches are singletons.
  */
  template<class T>
  class Cache : public Singleton
  {
  public:
    typedef Cache<T> self;

    typedef std::shared_ptr<Loader<T>> loader_ptr;
    void set_loader(loader_ptr l) { m_Loader = l; }
    
    virtual void shutdown()
    {
      clear();
    }

    bool is_loaded(const xstring& name) const
    {
      return m_Objects.count(name) > 0;
    }

    T& get(const xstring& name)
    {
      auto it = m_Objects.find(name);
      if (it == m_Objects.end())
      {
        T& obj = m_Objects[name];
        if (!m_Loader->load(name, obj))
          THROW("Cannot load '" << name << "'");
        return obj;
      }
      return it->second;
    }

    T& insert(const xstring& name, const T& obj)
    {
      return m_Objects[name] = obj;
    }

    void unload(const xstring& name)
    {
      m_Objects.erase(name);
    }

    void clear()
    {
      m_Objects.clear();
    }
  protected:
    friend struct std::default_delete<self>;
    Cache() {}
    ~Cache() {}
    Cache(const self&) {}
    self& operator= (const self&) { return *this; }
  private:
    loader_ptr                     m_Loader;
    std::unordered_map<xstring, T> m_Objects;
  };
  
  
  template<class T>
  class Accumulator
  {
    typedef std::vector<T> Seq;
    Seq m_Data;
    int m_Valid;
    int m_Position;
  public:
    typedef typename Seq::const_iterator const_iterator;
    const_iterator begin() const { return m_Data.begin(); }
    const_iterator end()   const { return m_Data.ends(); }

    Accumulator(int size) : m_Data(size), m_Valid(0), m_Position(0) {}
    
    void reset() 
    { 
      m_Valid=0; 
      m_Position=0;
    }

    void flush()
    {
      if (m_Valid<size())
      {
        typename Seq::iterator it=m_Data.begin();
        std::advance(it,m_Valid);
        std::fill(it,m_Data.end(),0);
      }
    }

    void start_sequence()
    {
      m_Position=0;
    }
    
    template<class II>
    int acc_data(II b, II e)
    {
      typename Seq::iterator oi=m_Data.begin();
      std::advance(oi,m_Position);
      int n=0;
      for(;b!=e && m_Position<m_Valid;++b,++m_Position,++oi,++n)
        *oi += *b;
      for(;b!=e && m_Valid<size();++b,++m_Position,++m_Valid,++oi,++n)
        *oi=*b;
      return n;
    }

    int size() const { return m_Data.size(); }
    void resize(int new_size) { m_Data.resize(new_size); }
  };


  template<class T>
  class BufferReader : public std::iterator<std::random_access_iterator_tag,T>
  {
    typedef BufferReader<T> self;

    T*   m_Buffer;
    int  m_Length;
    int  m_Position;
    bool m_Loop;
  public:
    BufferReader(T* buffer, int length, int pos, bool loop) 
      : m_Buffer(buffer), m_Length(length), m_Position(pos), m_Loop(loop) {}

    const T& operator* () { return m_Buffer[m_Position]; }
    self& operator++ () { ++m_Position; if (m_Position==m_Length && m_Loop) m_Position=0; return *this; }
    self  operator++(int) { self br(m_Buffer,m_Length,m_Position,m_Loop); ++*this; return br; }
    bool  operator==(const self& rhs) const { return m_Buffer==rhs.m_Buffer && m_Length==rhs.m_Length && m_Position==rhs.m_Position && m_Loop==rhs.m_Loop; }
    bool  operator!=(const self& rhs) const { return !(*this==rhs); }

    self begin()   const { return self(m_Buffer,m_Length,0,m_Loop); }
    self current() const { return self(m_Buffer,m_Length,m_Position,m_Loop); }
    self end()     const { return self(m_Buffer,m_Length,m_Loop?m_Length+1:m_Length,m_Loop); }

    void advance(int n)
    {
      m_Position+=n;
      while (m_Position>=m_Length && m_Loop) m_Position-=m_Length;
    }

    template<class OI>
    int copy(OI o, int n)
    {
      if (!m_Loop)
      {
        int act=Min(m_Length-m_Position,n);
        std::copy(m_Buffer+m_Position,m_Buffer+m_Position+act,o);
        m_Position+=act;
        return act;
      }
      else
      {
        int total=0;
        while (total<n)
        {
          int act=Min(m_Length-m_Position,n-total);
          std::copy(m_Buffer+m_Position,m_Buffer+m_Position+act,o);
          std::advance(o,act);
          m_Position+=act;
          total+=act;
          if (m_Position>=m_Length) m_Position=0;
        }
        return n;
      }
    }

    int get_position() const { return m_Position; }
  };
  

  class GameView
  {
    iRect2 m_ScreenView;
    iVec2  m_Offset;
  public:
	  GameView(int w=0, int h=0)
      : m_ScreenView(iVec2(0,0),iVec2(w,h)),
        m_Offset(0,0)
	  {
	  }

    void set_screen_view(const iRect2& r) { m_ScreenView=r; }
    void set_offset(const iVec2& v)       { m_Offset=v; }
    void delta_offset(const iVec2& delta) { m_Offset+=delta; }

    void set_center(const iVec2& v)
    {
      iVec2 c=m_ScreenView.get_size();
      c/=2;
      set_offset(v-c);
    }

    const iVec2& get_2D_offset() const
    {
      return m_Offset;
    }

    const iRect2& get_2D_view() const
    { 
      return m_ScreenView;
    }
  };

  class GameObject
  {
  public:
    virtual ~GameObject() {}
    virtual bool is_volatile() const { return false; }

    virtual void notify(const xstring& msg) {}

    /** Logically change the object state to reflect the passage of dt milliseconds
        No drawing is done at this stage.
        Returns false to indicate object is to be removed.
    */
    virtual bool advance(int dt) = 0;
    virtual void render(GameView& view) = 0;
  };

} // namespace SDLPP

#endif // H_SDLPP_COMMON


