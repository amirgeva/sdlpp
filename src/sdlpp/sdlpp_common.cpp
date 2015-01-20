#include <sdlpp_common.h>
#include <sdlpp_graphics.h>

namespace SDLPP {

Singleton::Singleton() { SingletonManager::instance()->register_singleton(this); }



} // namespace SDLPP