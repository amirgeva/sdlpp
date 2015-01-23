#ifndef H_XSTRING
#define H_XSTRING

template<class E, class T=std::char_traits<E>, class A=std::allocator<E> >
class basic_xstring : public std::basic_string<E,T,A>
{
  typedef E                           value_type;
  typedef A                           allocator_type;
  typedef std::basic_string<E,T,A>    parent;
  typedef basic_xstring<E,T,A>        self;
  typedef typename parent::size_type  size_type;
  using parent::npos;
public:
  basic_xstring(const value_type* ptr, size_type n, const allocator_type& al = allocator_type())
    : parent(ptr,n,al) {}
  basic_xstring(const value_type* ptr, const allocator_type& al = allocator_type())
    : parent(ptr,al) {}
  basic_xstring(const basic_xstring& s, size_type off = 0, size_type n = npos, const allocator_type& al = allocator_type())
    : parent(s,off,n,al) {}
  basic_xstring(size_type n, value_type c, const allocator_type& al = allocator_type())
    : parent(n,c,al) {}
  explicit basic_xstring(const allocator_type& al = allocator_type())
    : parent(al) {}
  basic_xstring(const parent& p) : parent(p) {}
  template<class U>
  explicit basic_xstring(const U& u)
  {
    std::basic_ostringstream<E,T,A> os;
    os << u;
    *this=os.str();
  }

  operator const value_type* () const { return parent::c_str(); }

  int as_int() const
  {
    return atoi(parent::c_str());
  }

  double as_double() const
  {
    return atof(parent::c_str());
  }

  void trim_left()
  {
    int p=parent::find_first_not_of(" \t\n\r");
    if (p>0) *this=parent::substr(p);
  }

  void trim_right()
  {
    int p=parent::find_last_not_of(" \t\n\r");
    if (p>=0)
      *this=parent::substr(0,p+1);
  }

  void trim()
  {
    trim_left();
    trim_right();
  }

  bool starts_with(const self& sub) const
  {
    if (parent::size() < sub.size()) return false;
    return parent::substr(0, sub.size()) == sub;
  }

  bool ends_with(const self& sub) const
  {
    if (parent::size() < sub.size()) return false;
    return parent::substr(parent::size() - sub.size()) == sub;
  }

  bool read_line(std::istream& is)
  {
    *this="";
    if (is.eof()) return false;
    char buffer[1024];
    while (true)
    {
      is.getline(buffer,1000);
      if (is.eof()) return !parent::empty();
      parent s=buffer;
      *this+=s;
      if (s.length()<1000) return !parent::empty();
    }
  }
};

template<class T>
class basic_string_tokenizer
{
  typedef basic_xstring<T> str;
  typedef std::vector<str> seq;
  seq      m_Tokens;
  unsigned m_Current;
public:
  basic_string_tokenizer(const str& s, const str& delim=" \t")
    : m_Current(0)
  {
    int p=-1;
    int len=s.length();
    while (true)
    {
      p=s.find_first_not_of(delim,p+1);
      if (p<0) break;
      int e=s.find_first_of(delim,p+1);
      if (e<0) e=len;
      m_Tokens.push_back(s.substr(p,e-p));
      p=e;
    }
  }

  unsigned size() const { return m_Tokens.size(); }
  bool     has_more_tokens() const { return m_Current<m_Tokens.size(); }
  str      get_next_token() { return m_Tokens[m_Current++]; }
};

typedef basic_xstring<char> xstring;
typedef basic_string_tokenizer<char> xstring_tokenizer;

inline void make_lower(xstring& s)
{
  xstring::iterator b=s.begin(),e=s.end();
  for(;b!=e;++b)
  {
    char& c=*b;
    if (c>='A' && c<='Z') c+=32;
  }
}

inline xstring hexstr(unsigned n)
{
  static const xstring hex="0123456789ABCDEF";
  xstring res;
  for(int i=0;i<8;++i)
  {
    res+=hex.substr(n&15,1);
    n>>=4;
  }
  std::reverse(res.begin(),res.end());
  return res;
}


namespace std 
{
  template<>
  struct hash<::xstring> : public hash < string > {};
}

#endif // H_XSTRING

