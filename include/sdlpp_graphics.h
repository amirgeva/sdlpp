#ifndef sdlpp_graphics_h__
#define sdlpp_graphics_h__

#include <SDL.h>
#include <sdlpp_common.h>

namespace SDLPP
{

  class BitmapPixels
  {
    SDL_Surface* m_Surface;
    SDL_Texture* m_Texture;

    BitmapPixels(const BitmapPixels&) {}
    BitmapPixels& operator= (const BitmapPixels&) { return *this; }

    iVec2 m_Size;

    void invalidate_texture()
    {
      if (m_Texture) SDL_DestroyTexture(m_Texture);
      m_Texture = 0;
    }
  public:
    BitmapPixels(unsigned w, unsigned h)
      : m_Surface(0)
      , m_Texture(0)
      , m_Size(w,h)
    {
      m_Surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
      if (!m_Surface) THROW("Failed to create bitmap pixels " << w << 'x' << h);
    }

    BitmapPixels(SDL_Surface* surface)
      : m_Surface(surface)
      , m_Texture(0)
      , m_Size(surface->w,surface->h)
    {}

    ~BitmapPixels()
    {
      invalidate_texture();
      if (m_Surface) SDL_FreeSurface(m_Surface);
    }

    iRect2 get_rect() const { return iRect2(iVec2::Zero(), m_Size); }

    void draw(const iRect2& src, const iRect2& dst);

    Uint32 get_colorkey() const 
    { 
      Uint32 res = 0x12345678;
      SDL_GetColorKey(m_Surface,&res);
      return res;
    }

    void set_colorkey(Uint32 color)
    {
      invalidate_texture();
      SDL_SetColorKey(m_Surface, 1, color);
    }

    Uint32 get_pitch() const { return m_Surface->pitch; }

    const Uint8* get_buffer() const 
    { 
      return reinterpret_cast<const Uint8*>(m_Surface->pixels);
    }
  };

  typedef std::shared_ptr<BitmapPixels> bitmap_pixels_ptr;

  class Bitmap
  {
    bitmap_pixels_ptr m_Pixels;
    iRect2            m_Region;
  public:
    Bitmap() {}

    Bitmap(unsigned w, unsigned h)
      : m_Pixels(new BitmapPixels(w, h))
      , m_Region(0, 0, w, h)
    {}

    Bitmap(bitmap_pixels_ptr pixels)
      : m_Pixels(pixels)
      , m_Region(pixels->get_rect())
    {}

    Bitmap(bitmap_pixels_ptr pixels, const iRect2& region)
      : m_Pixels(pixels)
      , m_Region(region)
    {}

    iRect2 get_rect() const { return iRect2(iVec2::Zero(), get_size()); }
    iVec2 get_size() const { return m_Region.get_size(); }
    int get_width() const { return m_Region.get_width(); }
    int get_height() const { return m_Region.get_height(); }

    Bitmap cut(const iRect2& sub_rect)
    {
      return Bitmap(m_Pixels, sub_rect.translate(m_Region.tl));
    }

    const Uint32* get_row(int y) const
    {
      const Uint8* buffer = m_Pixels->get_buffer();
      Uint32 pitch = get_pitch();
      buffer += pitch*(m_Region.tl.y+y);
      const Uint32* row = reinterpret_cast<const Uint32*>(buffer);
      row += m_Region.tl.x;
      return row;
    }

    Uint32 get_pitch() const { return m_Pixels->get_pitch(); }

    Uint32 get_colorkey() const { return m_Pixels->get_colorkey(); }
    void   set_colorkey(Uint32 color) { m_Pixels->set_colorkey(color); }

    void draw(int x, int y);
    void draw(const iVec2& at);
    void draw(const iRect2& dst);
    void draw(const iRect2& src, const iVec2& dst);
    void draw(const iRect2& src, const iRect2& dst);
  };

  class BitmapLoader : public Loader < Bitmap >
  {
  public:
    virtual bool load(const xstring& name, Bitmap&) override;
  };

  class BitmapCache : public Cache<Bitmap>
  {
  public:
    static BitmapCache* instance()
    {
      static std::unique_ptr<BitmapCache> ptr(new BitmapCache);
      return ptr.get();
    }

  private:
    friend struct std::default_delete<BitmapCache>;
    BitmapCache()
    {
      set_loader(loader_ptr(new BitmapLoader));
    }
    ~BitmapCache() {}
    BitmapCache(const BitmapCache&) {}
    BitmapCache& operator= (const BitmapCache&) { return *this; }
  };


  
  class Graphics : public Singleton
  {
  public:
    static Graphics* instance()
    {
      static std::unique_ptr<Graphics> ptr(new Graphics);
      return ptr.get();
    }

    void initialize(int width, int height, bool full_screen);
  
    virtual void shutdown() override
    {
    }

    SDL_Surface* convert(SDL_Surface* surface)
    {
      return surface;
      //return SDL_ConvertSurface(surface, m_Screen->format, 0);
    }

    SDL_Texture* create_texture(SDL_Surface* surface)
    {
      return SDL_CreateTextureFromSurface(m_Renderer, surface);
    }

    void draw(SDL_Texture* texture, const iRect2& src, const iRect2& dst)
    {
      SDL_Rect rsrc=R(src),rdst=R(dst);
      SDL_RenderCopy(m_Renderer, texture,&rsrc,&rdst);
    }

    void render(SDL_Texture* texture)
    {
    	SDL_RenderCopy(m_Renderer, texture,0,0);
    }

    void fill(Uint32 color)
    {
      Uint8 r, g, b, a;
      a = (color >> 24);
      r = (color >> 16);
      g = (color >>  8);
      b = (color & 255);
      SDL_SetRenderDrawColor(m_Renderer,r,g,b,a);
    }

    void flip();
    
    iVec2 position(float x, float y) const;
  private:
    friend struct std::default_delete<Graphics>;
    Graphics() { display_message("Initializing Graphics Manager"); }
    ~Graphics() {}
    Graphics(const Graphics&) {}
    Graphics& operator= (const Graphics&) { return *this; }

    iVec2         m_Size;
    SDL_Window*   m_Window;
    SDL_Surface*  m_Screen;
    SDL_Renderer* m_Renderer;
    SDL_Texture*  m_BackBuffer;
  };
  
  inline Uint32 MapRGB(int r, int g, int b)
  {
    return 0xFF000000 | ((r & 255) << 16) | ((g & 255) << 8) | (b & 255);
  }

  inline void flip() { Graphics::instance()->flip(); }


} // namespace SDLPP



#endif // sdlpp_graphics_h__
