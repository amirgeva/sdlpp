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
  SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
  if (!rw) 
    THROW ("File not found: "+xstring(filename));
  char name_buffer[1024];
  size_t size = size_t(SDL_RWsize(rw));
  SDL_RWseek(rw, 0, RW_SEEK_SET);
  while (SDL_RWtell(rw)<size)
  {
    ResourceHeader rh;
    SDL_RWread(rw, &rh, sizeof(ResourceHeader), 1);
    //f.read((char*)&rh,sizeof(ResourceHeader));
    //if (f.eof()) break;
    if (rh.name_length>1000)
      THROW("Resource name too long (max 1000)");
    SDL_RWread(rw, name_buffer, 1, rh.name_length);
    //f.read(name_buffer,rh.name_length);
    name_buffer[rh.name_length]=0;
    Resource r;
    r.position=int(SDL_RWtell(rw));
    r.size=rh.size;
    SDL_RWseek(rw, rh.size, RW_SEEK_CUR);
    //f.seekg(r.size,std::ios::cur);
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
  block_streambuf(std::istream* source, std::streamoff limit)
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
      std::copy(egptr()-PUTBACK,egptr(),base);
      //std::memmove(base,egptr()-PUTBACK,PUTBACK);
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
    SDL_RWops* rw = SDL_RWFromFile(resource_name, "rb");
    size_t res=size_t(SDL_RWsize(rw));
    SDL_RWclose(rw);
    return res;
  }
  rsc_map::iterator it = m_Resources.find(resource_name);
  if (it == m_Resources.end()) return 0;
  Resource& r = it->second;
  return r.size;
}

SDL_RWops* ResourceFile::get(const xstring& resource_name)
{
  if (m_Filename.empty())
  {
    return SDL_RWFromFile(resource_name, "rb");
  }
  rsc_map::iterator it=m_Resources.find(resource_name);
  if (it==m_Resources.end()) return 0;
  Resource& r=it->second;
  SDL_RWops* rw = SDL_RWFromFile(m_Filename, "rb");
  SDL_RWseek(rw, r.position, RW_SEEK_SET);
  return rw;
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



bool        read_contents(const xstring& name, char_vec& cv)
{
  ResourceFile* rf = get_default_resource_file();
  SDL_RWops* rw = 0;
  int size = 0;
  if (rf) 
  {
    size = rf->get_size(name);
    rw = rf->get(name);
    if (!rw) return false;
  }
  else
  {
    rw = SDL_RWFromFile(name, "rb");
    size = int(SDL_RWsize(rw));
    SDL_RWseek(rw, 0, RW_SEEK_SET);
  }
  cv.resize(size);
  SDL_RWread(rw, &cv[0], 1, size);
  SDL_RWclose(rw);
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
  //if (name.ends_with(".xml")) display_message("Loaded ("+name+"): " + cfg);
  if (cfg.empty())
    THROW("Not found: " << name);
  handle_xml_eol(cfg);
  return load_xml_from_text(cfg);
}


} // namespace SDLPP 


