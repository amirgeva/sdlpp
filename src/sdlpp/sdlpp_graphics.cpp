#include <sdlpp_graphics.h>
#include <sdlpp_io.h>

namespace SDLPP
{

  void Graphics::initialize(int width, int height, bool full_screen)
  {
	m_Size=iVec2(width,height);
	display_message("Creating display "+xstring(width)+"x"+xstring(height));
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
	  THROW("SDL_GetDesktopDisplayMode failed");
    m_Window = SDL_CreateWindow("SDL", 0, 0, dm.w, dm.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS );
    if (!m_Window)
      THROW("Failed to create window");
    SDL_SetWindowFullscreen(m_Window, SDL_TRUE);
    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!m_Renderer)
      THROW("Failed to create renderer");
    m_BackBuffer = SDL_CreateTexture(m_Renderer,SDL_GetWindowPixelFormat(m_Window),
	                                 SDL_TEXTUREACCESS_TARGET,width,height);
	if (!m_BackBuffer) THROW("Failed to create back buffer");
    SDL_SetRenderTarget(m_Renderer,m_BackBuffer);
    //m_Screen = SDL_GetWindowSurface(m_Window);
    //if (!m_Screen) THROW("Invalid graphics format");
  }

  iVec2 Graphics::position(float x, float y) const
  {
	return iVec2(int(x*m_Size.x),int(y*m_Size.y));
  }


  void Graphics::flip()
  {
    //display_message("Flipping...");
	SDL_SetRenderTarget(m_Renderer, NULL);
	SDL_RenderCopy(m_Renderer,m_BackBuffer,0,0);
    SDL_RenderPresent(m_Renderer);
    SDL_RenderClear(m_Renderer);
    SDL_SetRenderTarget(m_Renderer,m_BackBuffer);
    SDL_RenderClear(m_Renderer);
  }


  void BitmapPixels::draw(const iRect2& src, const iRect2& dst)
  {
    if (!m_Texture) m_Texture = Graphics::instance()->create_texture(m_Surface);
    Graphics::instance()->draw(m_Texture, src, dst);
  }

  void Bitmap::draw(int x, int y)
  {
    draw(iVec2(x, y));
  }

  void Bitmap::draw(const iVec2& at)
  {
    m_Pixels->draw(m_Region, iRect2(at, at + m_Region.get_size()));
  }

  void Bitmap::draw(const iRect2& dst)
  {
    m_Pixels->draw(m_Region, dst);
  }

  void Bitmap::draw(const iRect2& src, const iVec2& dst)
  {
    m_Pixels->draw(src.translate(m_Region.tl), iRect2(dst, dst + src.get_size()));
  }

  void Bitmap::draw(const iRect2& src, const iRect2& dst)
  {
    m_Pixels->draw(src.translate(m_Region.tl), dst);
  }


  typedef SDL_Surface* (*image_loader)(SDL_RWops*);

  struct ImageType
  {
    const char* extension;
    const char* loader_name;
  };

  static const ImageType s_ImageTypes[] = {
      { "BMP", "IMG_LoadBMP_RW" },
      { "GIF", "IMG_LoadGIF_RW" },
      { "JPG", "IMG_LoadJPG_RW" },
      { "LBM", "IMG_LoadLBM_RW" },
      { "PCX", "IMG_LoadPCX_RW" },
      { "PNG", "IMG_LoadPNG_RW" },
      { "PNM", "IMG_LoadPNM_RW" },
      { "TGA", "IMG_LoadTGA_RW" },
      { "TIF", "IMG_LoadTIF_RW" },
      { "XCF", "IMG_LoadXCF_RW" },
      { "XPM", "IMG_LoadXPM_RW" },
      { "XV", "IMG_LoadXV_RW" },
  };

  static const int s_NumberOfImageTypes = sizeof(s_ImageTypes) / sizeof(ImageType);

  image_loader get_loader(const xstring& file_name)
  {
    int p = file_name.find_last_of('.');
    if (p < 0) return 0;
    xstring ext = file_name.substr(p + 1);
    for (xstring::iterator it = ext.begin(); it != ext.end(); ++it)
      if (*it >= 'a' && *it <= 'z') *it -= 32;
    const char* loader_name = 0;
    for (int i = 0; i < s_NumberOfImageTypes; ++i)
    {
      if (ext == s_ImageTypes[i].extension)
      {
        loader_name = s_ImageTypes[i].loader_name;
        break;
      }
    }
    if (!loader_name) return 0;
    image_loader il = (image_loader)get_function("SDL2_image", loader_name);
    return il;
  }

  SDL_Surface* load_bitmap(SDL_RWops* rwops, const xstring& name)
  {
    SDL_Surface* loaded = 0;
    image_loader ldr = get_loader(name);
    if (ldr == 0) ldr = get_loader(".bmp");
    if (ldr == 0) return 0;
    loaded = ldr(rwops);
    SDL_FreeRW(rwops);
    if (!loaded) THROW("Image file cannot be loaded: " + name);
    return loaded;
    //return Graphics::instance()->convert(loaded);
  }

  bool BitmapLoader::load(const xstring& name, Bitmap& bmp)
  {
    char_vec v;
    if (read_contents(name, v))
    {
      SDL_Surface* s = load_bitmap(SDL_RWFromConstMem(&v[0], v.size()), "");
      //display_message("Loaded bitmap "+name+"  "+xstring(s->w)+"x"+xstring(s->h));
      bmp = Bitmap(bitmap_pixels_ptr(new BitmapPixels(s)));
      return true;
    }
    return false;
  }


} // namespace SDLPP
