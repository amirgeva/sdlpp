#include <sdlpp_common.h>
#include <sdlpp_io.h>
//#include <cstring>

namespace SDLPP {

struct ResourceHeader
{
  ResourceHeader() : size(0), name_length(0), reserved1(0), reserved2(0) {}
  int size;
  int name_length;
  int reserved1,reserved2;
};

ResourceFile::ResourceFile(const char* filename)
: m_Filename(filename?filename:"")
{
  if (m_Filename.empty()) return;
  std::ifstream f(filename,std::ios::in|std::ios::binary);
  if (f.fail()) 
    THROW ("File not found: "+xstring(filename));
  char name_buffer[1024];
  while (!f.eof())
  {
    ResourceHeader rh;
    f.read((char*)&rh,sizeof(ResourceHeader));
    if (f.eof()) break;
    if (rh.name_length>1000)
      THROW("Resource name too long (max 1000)");
    f.read(name_buffer,rh.name_length);
    name_buffer[rh.name_length]=0;
    Resource r;
    r.position=int(f.tellg());
    r.size=rh.size;
    f.seekg(r.size,std::ios::cur);
    m_Resources[name_buffer]=r;
  }
}

class block_streambuf : public std::streambuf
{
  typedef std::istream::pos_type pos_type;
  enum { BUFFER_SIZE=1024, PUTBACK=4 };
  typedef std::vector<char> char_vec;
  std::istream* m_Source;
  char_vec      m_Buffer;
  pos_type      m_Start,m_Current,m_Stop;
public:
  block_streambuf(std::istream* source, std::streamsize limit)
  : m_Source(source)
  , m_Buffer(BUFFER_SIZE+PUTBACK)
  , m_Start(source->tellg())
  , m_Stop(source->tellg()+limit)
  {
    m_Current=m_Start;
    char* end=&m_Buffer.front() + m_Buffer.size();
    setg(end,end,end);
  }
  ~block_streambuf()
  {
    delete m_Source;
  }
protected:
  int_type underflow()
  {
    if (gptr() < egptr()) return traits_type::to_int_type(*gptr());
    char* base=&m_Buffer.front();
    char* start=base;

    if (eback()==base)
    {
      std::memmove(base,egptr()-PUTBACK,PUTBACK);
      start+=PUTBACK;
    }
    std::streamsize to_read=m_Buffer.size()-(start-base);
    std::streamsize left=m_Stop-m_Current;
    if (left<to_read) to_read=left;
    if (to_read==0) return traits_type::eof();
    m_Source->read(start,to_read);
    std::streamsize n=m_Source->gcount();
    if (!n) return traits_type::eof();
    m_Current+=n;
    setg(base, start, start + n);
    return traits_type::to_int_type(*gptr());
  }
};

class owner_istream : public std::istream
{
public:
  owner_istream(block_streambuf* sb)
    : std::istream(sb)
  {}
  ~owner_istream()
  {
    delete rdbuf();
  }
};

size_t        ResourceFile::get_size(const xstring& resource_name)
{
  if (m_Filename.empty())
  {
    std::ifstream f(resource_name, std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    return size_t(f.tellg());
  }
  rsc_map::iterator it = m_Resources.find(resource_name);
  if (it == m_Resources.end()) return 0;
  Resource& r = it->second;
  return r.size;
}

istream_ptr ResourceFile::get(const xstring& resource_name)
{
  if (m_Filename.empty())
  {
    std::ifstream* f=new std::ifstream(resource_name.c_str(),std::ios::in|std::ios::binary);
    if (f->fail()) 
    {
      delete f;
      THROW ("File not found: "+resource_name);
    }
    return istream_ptr(f);
  }
  rsc_map::iterator it=m_Resources.find(resource_name);
  if (it==m_Resources.end()) return 0;
  Resource& r=it->second;
  std::ifstream* f=new std::ifstream(m_Filename.c_str(),std::ios::in|std::ios::binary);
  if (f->fail())
  {
    delete f;
    THROW ("File not found: "+m_Filename);
  }
  f->seekg(r.position);
  std::istream* is=new owner_istream(new block_streambuf(f,r.size));
  return istream_ptr(is);
}


ResourceFileWriter::ResourceFileWriter(const char* filename, bool append)
: m_File(filename,(append?std::ios::app:std::ios::out)|std::ios::binary)
{
  if (m_File.fail()) 
    THROW ("Cannot open file for writing: "+xstring(filename));
}

void ResourceFileWriter::add_resource(const char* filename)
{
  std::ifstream f(filename,std::ios::in|std::ios::binary);
  if (f.fail())
    THROW ("File not found: "+xstring(filename));
  ResourceHeader rh;
  rh.name_length=strlen(filename);
  f.seekg(0,std::ios::end);
  rh.size=int(f.tellg());
  f.seekg(0);
  m_File.write((const char*)&rh,sizeof(ResourceHeader));
  m_File.write(filename,rh.name_length);
  char buffer[65536];
  int total=0;
  while (!f.eof())
  {
    f.read(buffer,65536);
    int act=int(f.gcount());
    if (act==0) break;
    total+=act;
    m_File.write(buffer,act);
  }
  if (total != rh.size)
    THROW ("Mismatch in file size between seek and read: "+xstring(filename));
}

void ResourceFileWriter::add_resource(const char* name, const char* buffer, int length)
{
  ResourceHeader rh;
  rh.name_length=strlen(name);
  rh.size=length;
  m_File.write((const char*)&rh,sizeof(ResourceHeader));
  m_File.write(name,rh.name_length);
  m_File.write(buffer,length);
}

Sint64 SDLCALL istream_seek(struct SDL_RWops *context, Sint64 offset, int whence)
{
  std::istream* is=reinterpret_cast<std::istream*>(context->hidden.unknown.data1);
  std::ios_base::seek_dir dir=std::ios::beg;
  switch (whence)
  {
    case SEEK_SET: dir=std::ios::beg; break;
    case SEEK_CUR: dir=std::ios::cur; break;
    case SEEK_END: dir=std::ios::end; break;
  }
  is->seekg(offset,std::ios_base::seekdir(dir));
  return int(is->tellg());
}

size_t SDLCALL istream_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
  std::istream* is=reinterpret_cast<std::istream*>(context->hidden.unknown.data1);
  is->read((char*)ptr,size*maxnum);
  int act=int(is->gcount());
  if (act==0) return -1; 
  return (act/size);
}

size_t SDLCALL istream_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
  std::istream* is=reinterpret_cast<std::istream*>(context->hidden.unknown.data1);
  return -1;
}

int SDLCALL istream_close(struct SDL_RWops *context)
{
  std::istream* is=reinterpret_cast<std::istream*>(context->hidden.unknown.data1);
  return 0;
}




SDL_RWops *SDL_RWFromStream(istream_ptr input)
{
  SDL_RWops *rwops = SDL_AllocRW();
  if (!rwops) return 0;
  rwops->hidden.unknown.data1=input.get();
  rwops->seek=istream_seek;
  rwops->read=istream_read;
  rwops->write=istream_write;
  rwops->close=istream_close;
  return rwops;
}

bool        read_contents(const xstring& name, char_vec& cv)
{
  ResourceFile* rf = get_default_resource_file();
  istream_ptr is;
  int size = 0;
  if (rf) 
  {
    size = rf->get_size(name);
    is=rf->get(name);
    if (!is) return false;
  }
  else
  {
    is=istream_ptr(new std::ifstream(name.c_str(), std::ios::in | std::ios::binary));
    if (is->fail()) return false;
    is->seekg(0, std::ios::end);
    size = int(is->tellg());
    is->seekg(0);
  }
  cv.resize(size);
  is->read(&cv.front(),size);
  return true;
}

xstring read_contents_as_string(const xstring& name)
{
  char_vec cv;
  if (!read_contents(name,cv)) return "";
  return (&cv.front());
}

void handle_xml_eol(xstring& s)
{
  for(unsigned p=0;p<s.length();++p)
  {
    if (s[p]=='\r')
    {
      if (s[p+1]=='\n') s.erase(p,1);
      else s[p]='\n';
    }
  }
}

xml_element* load_xml(const xstring& name)
{
  xstring cfg=read_contents_as_string(name);
  if (cfg.empty())
    THROW("Not found: " << name);
  handle_xml_eol(cfg);
  return load_xml_from_text(cfg);
}


} // namespace SDLPP 


