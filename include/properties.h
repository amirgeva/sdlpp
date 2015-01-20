#ifndef properties_h__
#define properties_h__

#include <unordered_map>
#include <xstring.h>

class PropertyBag
{
  std::unordered_map<xstring, xstring>  m_Properties;
public:
  virtual ~PropertyBag() {}

  xstring& operator[] (const xstring& name)
  {
    return m_Properties[name];
  }

  template<class T>
  void set(const xstring& name, const T& value)
  {
    m_Properties[name] = xstring(value);
  }

  const xstring& get(const xstring& name) const
  {
    static const xstring none;
    auto it = m_Properties.find(name);
    if (it == m_Properties.end()) return none;
    return it->second;
  }

  int iget(const xstring& name) const
  {
    return get(name).as_int();
  }

  double dget(const xstring& name) const
  {
    return get(name).as_double();
  }
};



#endif // properties_h__
