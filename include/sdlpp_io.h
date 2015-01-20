#ifndef H_SDLPP_IO
#define H_SDLPP_IO

#include <xml.h>

namespace SDLPP 
{

template<typename IT>
class KeyIterator
{
  IT m_Iter;
public:
  typedef KeyIterator<IT> self;
  typedef typename std::iterator_traits<IT>::value_type::first_type value_type;

  KeyIterator(IT iter) : m_Iter(iter) {}
  const value_type& operator*() const { return m_Iter->first; }
  const value_type* operator->() const { return &(m_Iter->first); }
  KeyIterator& operator++() { ++m_Iter; return *this; }
  bool operator!=(const self& rhs) const { return m_Iter != rhs.m_Iter; }
};

typedef std::shared_ptr<std::istream> istream_ptr;


/** A Resource File is an aggregate of several files into one.
    Use an instance of the ResourceFileWriter class to create such a file
    from a set of data files.
*/
class ResourceFile
{
  struct Resource { int position; int size; };
  typedef std::map<xstring,Resource> rsc_map;
  rsc_map   m_Resources;
  xstring   m_Filename;
  ResourceFile(const ResourceFile& rhs) {}
  ResourceFile& operator= (const ResourceFile& rhs) { return *this; }
public:
  /** Open a resource file for reading.  Resources are indexed for later lookup.
      If there is more than one resource with the same name, the last one in
      the file will be used. 
      Passing a NULL value in the filename will use resources from the file system directly.
  */
  ResourceFile(const char* filename);
  virtual ~ResourceFile() {}

  /** Returns a dynamically allocated stream to read the resource requested.
      Caller must delete the stream when done. */
  istream_ptr get(const xstring& resource_name);

  /** Return the size (in bytes) of the resource */
  size_t      get_size(const xstring& resource_name);

  typedef rsc_map::const_iterator base_iterator;
  typedef KeyIterator<base_iterator> const_iterator;
  const_iterator begin() const { return const_iterator(m_Resources.begin()); }
  const_iterator end() const { return const_iterator(m_Resources.end()); }
};

void set_default_resource_file(ResourceFile* rf);
ResourceFile* get_default_resource_file();


/** This class is used to create a resource file.
    It is usually used outside the main application by a utility that gathers
    various data files and creates a single aggregate.

    The utility  rscfile  uses this class.
*/
class ResourceFileWriter
{
  std::ofstream m_File;
  ResourceFileWriter(const ResourceFileWriter& rhs) {}
  ResourceFileWriter& operator= (const ResourceFileWriter& rhs) { return *this; }
public:
  /** Open a resource file for writing.  If append is true, new resources will
      be added while maintaining the existing ones.
      Otherwise, existing resources will be removed.
  */
  ResourceFileWriter(const char* filename, bool append=true);
  virtual ~ResourceFileWriter() {}

  /** Add a resource by copying the contents of a file. */
  void add_resource(const char* filename);

  /** Add a resource from a memory buffer. */
  void add_resource(const char* name, const char* buffer, int length);
};

/** Create an SDL IO object that implements stdio basic IO operations
    by forwarding the calls to the matching calls in a stream.
    This is useful for using the SDL functions that load from a file,
    to actually load from a stream, and thus from a resource file.

    This function is used internally for loading BMP and WAV from a stream
*/
SDL_RWops *SDL_RWFromStream(istream_ptr input);

bool    read_contents(const xstring& name, char_vec& cv);
xstring read_contents_as_string(const xstring& name);

void handle_xml_eol(xstring& s);
xml_element* load_xml(const xstring& name);

} // namespace SDLPP

#endif // H_SDLPP_IO

