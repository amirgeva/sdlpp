#include <sdlpp_common.h>
#include <sysdep.h>

namespace SDLPP {
/////////////////////////////////////////////////
//
// Windows implementation
//
/////////////////////////////////////////////////
#ifdef WIN32

#include <windows.h>

void DBG(const char* s)
{
  OutputDebugStringA(s);
}

#define module_handle HMODULE

module_handle load_library(const char* name) { return LoadLibraryA(name); }

function get_function_address(module_handle handle, const char* name)
{
  FARPROC fp=GetProcAddress(handle,name);
  return (function)fp;
}

void free_library(module_handle handle)
{
  FreeLibrary(handle);
}

void display_message(const xstring& msg)
{
  MessageBoxA(0,msg.c_str(),"Info",MB_OK);
}

xstring get_current_directory()
{
  char dir[256];
  GetCurrentDirectoryA(256,dir);
  return dir;
}

unsigned get_tick_count()
{
  return GetTickCount();
}

#endif

#ifdef LINUX

#include <dlfcn.h>

#define module_handle void*

module_handle load_library(const char* name)
{
  xstring s="lib";
  s+=name;
  s+=".so";
  return dlopen(s.c_str(),RTLD_LAZY);
}

void free_library(module_handle handle)
{
  dlclose(handle);
}

function get_function_address(module_handle handle, const char* name)
{
  dlerror();
  function f=(function)dlsym(handle,name);
  const char* msg=dlerror();
  if (msg) return 0;
  return f;
}

void display_message(const xstring& msg)
{
  std::cout << msg << std::endl;
}

#endif



class DLLManager
{
  struct Module
  {
    Module(module_handle h=0) : handle(h) {}
    module_handle handle;
    typedef std::map<xstring,function> func_map;
    func_map functions;
  };
public:
  static DLLManager* instance()
  {
    static std::auto_ptr<DLLManager> ptr(new DLLManager);
    return ptr.get();
  }


  function get_function(const xstring& module_name, const xstring& func_name)
  {
    mod_map::iterator it=m_Modules.find(module_name);
    if (it==m_Modules.end())
    {
      module_handle m=load_library(module_name.c_str());
      if (!m) return 0;
      m_Modules[module_name]=Module(m);
      it=m_Modules.find(module_name);
    }
    return get_function(it->second,func_name);
  }
private:
  friend class std::auto_ptr<DLLManager>;
  DLLManager() {}
  ~DLLManager() {}
  DLLManager(const DLLManager&) {}

  typedef std::map<xstring,Module> mod_map;
  mod_map m_Modules;

  function get_function(Module& m, const xstring& func_name)
  {
    Module::func_map::iterator it=m.functions.find(func_name);
    if (it==m.functions.end())
    {
      function f=get_function_address(m.handle,func_name.c_str());
      m.functions[func_name]=f;
      return f;
    }
    return it->second;
  }
};

function get_function(const char* module, const char* name)
{
  return DLLManager::instance()->get_function(module,name);
}

} // namespace SDLPP


