#ifndef H_XML
#define H_XML

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <xstring.h>

/** Main class for an XML node / element 
    Each element provides access to:
	* Attributes via get/set methods
	* Children elements in a tree structures via add/remove/iterators
*/
class xml_element
{
  typedef xml_element& reference;
  typedef xml_element* pointer;
  typedef std::vector<xml_element*> child_vec;
  typedef std::map<xstring,xstring> attr_map;

  xstring    m_Type;       ///  <type attr="value" .... >content</type>
  child_vec  m_Children;
  attr_map   m_Attributes;
  xstring    m_Content;    ///  not supported yet

  /** Copying disabled to avoid hazardous shallow copies
	  Deep copies not implemented by default copy constructor / assignment
	  to avoid costly copies hidden from the user
	  Deep copy can be achieved instead via serialization
  */
  xml_element(const xml_element&) {}
  xml_element& operator= (const xml_element&) { return *this; }
public:
  xml_element(const xstring& type="") : m_Type(type) {}
  ~xml_element() 
  {
    for(iterator b=begin();b!=end();++b) delete *b;
  }

  void set_type(const xstring& type) { m_Type=type; }
  const xstring& get_type() const { return m_Type; }

  int  get_child_count() const { return int(m_Children.size()); }
  void add_child(pointer p) { m_Children.push_back(p); }
  
  xml_element* add_child(const xstring& type) 
  { 
    xml_element* child=new xml_element(type);
    add_child(child);
    return child;
  }

  /** Removes a child, given by its element pointer
      delete_child controls whether to delete the child's subtree   */
  void remove(pointer child, bool delete_child)
  {
    for(iterator it=begin();it!=end();++it)
    {
      xml_element* c=*it;
      if (c==child) { m_Children.erase(it); break; }
    }
    if (delete_child) delete child;
  }

  /** Finds first child with the given type */
  xml_element* find_child(const xstring& type)
  {
    for(iterator it=begin();it!=end();++it)
    {
      xml_element* c=*it;
      if (c->get_type()==type) return c;
    }
    return 0;
  }

  bool has_attribute(const xstring& name) const { return m_Attributes.count(name)>0; }
  void set_attribute(const xstring& name, const xstring& value) { m_Attributes[name]=value; }
  xstring get_attribute(const xstring& name) const
  {
    attr_map::const_iterator it=m_Attributes.find(name);
    if (it==m_Attributes.end()) return "";
    return it->second;
  }
  
  typedef child_vec::iterator iterator;
  typedef child_vec::const_iterator const_iterator;
  iterator begin() { return m_Children.begin(); }
  iterator end()   { return m_Children.end(); }
  const_iterator begin() const { return m_Children.begin(); }
  const_iterator end()   const { return m_Children.end(); }

  typedef attr_map::const_iterator attr_iterator;
  attr_iterator attr_begin() const { return m_Attributes.begin(); }
  attr_iterator attr_end()   const { return m_Attributes.end(); }

  void print(std::ostream& os=std::cout, int indent=0, bool packed=false) const
  {
    xstring spaces=packed?xstring(""):xstring(indent,' ');
    xstring eol=packed?xstring(""):xstring("\n");
    os << spaces << "<" << get_type();
    for(attr_iterator it=attr_begin();it!=attr_end();++it)
      os << " " << it->first << "=\"" << it->second << "\"";
    if (get_child_count()==0 && m_Content.empty()) { os << "/>" << eol; return; }
    os << ">" << eol;
    if (!m_Content.empty()) os << spaces << m_Content << eol;
    for(const_iterator ci=begin();ci!=end();++ci)
      (*ci)->print(os,indent+2,packed);
    os << spaces << "</" << get_type() << ">" << eol;
  }

  xstring print(bool packed)
  {
    std::ostringstream os;
    print(os,0,packed);
    return xstring(os.str());
  }
};

class xml_parser
{
public:
  xml_parser() : m_InQuotes(false), m_LineNumber(1) {}
private:
  enum Token { LTAG, RTAG, EQ, QUOTES, SLASH, IDENT, TEXT, QUESTION, XEOF };

  bool m_InQuotes;
  int  m_LineNumber;

  static bool is_white_space(char c)
  {
    return (c<=32);
  }

  static bool quotes_pred(char c)
  {
    return c=='"';
  }

  static bool not_alnum(char c)
  {
    return ((c<'A' || c>'Z') && (c<'a' || c>'z') && (c<'0' || c>'9') && c!='_');
  }

  static bool is_question(char c)
  {
    return (c=='?');
  }

  Token analyze(std::istream& is, xstring& token_text)
  {
    char ch=' ';
    while (!is.eof() && is_white_space(ch))
    {
      ch=is.get();
      if (ch=='\n') ++m_LineNumber;
    }
    token_text="";
    if (is.eof()) return XEOF;
    token_text=xstring(1,ch);
    if (ch=='"') 
    { 
      m_InQuotes=!m_InQuotes; 
      return QUOTES; 
    }
    if (m_InQuotes)
    {
      token_text+=read_until(is,quotes_pred);
      return TEXT;
    }
    if (ch=='<') { return LTAG; }
    if (ch=='>') { return RTAG; }
    if (ch=='=') { return EQ; }
    if (ch=='/') { return SLASH; }
    if (ch=='?') { return QUESTION; }
    token_text+=read_until(is,not_alnum);
    return IDENT;
  }

  template<class PRED>
  xstring read_until(std::istream& is, PRED p)
  {
    xstring res;
    while (!is.eof())
    {
      char ch=is.peek();
      if (p(ch)) return res;
      ch=is.get();
      res+=xstring(1,ch);
    }
    return res;
  }

#define SYNTAX_ERROR throw "Syntax Error"
#define EXPECT(t) { token=analyze(is,last); if (token!=t) SYNTAX_ERROR; }

  void parse_element(std::istream& is, xml_element* parent)
  {
    xstring last;
    Token  token;
    while (true)
    {
      token=analyze(is,last);
      if (token==XEOF) return;
      if (token==LTAG)
      {
        token=analyze(is,last);
        if (token==QUESTION)
        {
          read_until(is,is_question);
          EXPECT(QUESTION);
          EXPECT(RTAG);
          continue;
        }
        if (token==SLASH)
        {
          if (!parent) SYNTAX_ERROR; 
          EXPECT(IDENT);
          if (last != parent->get_type()) SYNTAX_ERROR;
          EXPECT(RTAG);
          return;
        }
        else
        if (token==IDENT)
        {
          xml_element* child=new xml_element;
          child->set_type(last);
          parent->add_child(child);
          while (true)
          {
            token=analyze(is,last);
            if (token==XEOF) return;
            if (token==IDENT)  // Attribute
            {
              xstring attr_value,attr_name=last;
              EXPECT(EQ);
              token=analyze(is,last);
              if (token==QUOTES)
              {
                token=analyze(is,last);
                if (token==TEXT)
                {
                  attr_value=last;
                  EXPECT(QUOTES);
                }
                else
                  if (token!=QUOTES) SYNTAX_ERROR;
              }
              else // Old format, quotes optional
              {
                attr_value=last;
              }
              child->set_attribute(attr_name,attr_value);
            }
            else
            if (token==SLASH)
            {
              EXPECT(RTAG);
              break;
            }
            else
            if (token==RTAG)
            {
              parse_element(is,child);
              break;
            }
          }
        }
      }
    }
  }

public:

  xml_element* parse(std::istream& is)
  {
    if (is.fail()) return 0;
    xml_element* root=new xml_element;
    try
    {
      parse_element(is,root);
      int n=root->get_child_count();
      if (n>1) throw "Error: Multiple root nodes";
      if (n==0) throw "Error: No root node";
      xml_element* new_root=*(root->begin());
      root->remove(new_root,false);
      delete root;
      root=new_root;
    } catch (const char* msg)
    {
      std::cerr << "Line " << m_LineNumber << " - " << msg << std::endl;
      delete root; root=0;
    }
    return root;
  }
};

inline xml_element* load_xml_from_file(const char* filename)
{
  std::ifstream fin(filename);
  if (fin.fail()) return 0;
  return xml_parser().parse(fin);
}

inline xml_element* load_xml_from_text(const xstring& text)
{
  std::istringstream is(text);
  return xml_parser().parse(is);
}

inline xstring get_xml_text(xml_element* root)
{
  std::ostringstream os;
  root->print(os,0,false);
  return xstring(os.str());
}

inline xstring get_xml_text(xml_element& root)
{
  return get_xml_text(&root);
}

inline xml_element* clone_element(xml_element* root)
{
  return load_xml_from_text(get_xml_text(root));
}

#endif // H_XML

