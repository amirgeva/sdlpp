#include <memory>
#include <sdlpp_common.h>
#include <sdlpp_io.h>
#include <sdlpp_font.h>

namespace SDLPP
{

  class FontManager : public Singleton
  {
    typedef int(*init_func)();
    typedef void(*quit_func)();
    typedef void(*close_func)(TTF_Font*);
    typedef TTF_Font* (*open_func)(SDL_RWops *src, int freesrc, int ptsize);
    typedef int(*size_func)(TTF_Font*, const char*, int*, int*);
    typedef SDL_Surface* (*draw_func)(TTF_Font*, const char*, SDL_Color);

    init_func TTF_Init;
    quit_func TTF_Quit;
    open_func TTF_OpenFontRW;
    close_func TTF_CloseFont;
    size_func TTF_SizeUTF8;
    draw_func TTF_RenderUTF8_Solid;

    typedef std::map<xstring, Font> font_map;
    typedef font_map::iterator iterator;
    font_map m_Fonts;
  public:
    static FontManager* instance()
    {
      static std::unique_ptr<FontManager> ptr(new FontManager);
      return ptr.get();
    }

    void clear()
    {
      m_Fonts.clear();
    }

    virtual void shutdown() { clear(); }

    Font& get(const xstring& name, int point_size)
    {
      ResourceFile* rf = get_default_resource_file();
      std::ostringstream os;
      os << name << "_" << point_size;
      xstring full_name = os.str();
      auto it = m_Fonts.find(full_name);
      if (it == m_Fonts.end())
      {
        std::vector<char>* data = 0;
        SDL_RWops*    rw = 0;
        if (!load(m_Fonts[full_name],name,point_size))
        {
          m_Fonts.erase(full_name);
          THROW("Failed to load font: " << name);
        }
        it = m_Fonts.find(full_name);
      }
      return it->second;
    }

    iVec2 get_size(TTF_Font* font, const xstring& text)
    {
      int w, h;
      TTF_SizeUTF8(font, text.c_str(), &w, &h);
      return iVec2(w, h);
    }

    void close_font(TTF_Font* font)
    {
      TTF_CloseFont(font);
    }

    SDL_Surface* draw(TTF_Font* font, const xstring& text, Uint32 color)
    {
      SDL_Color c = { 255, 255, 255, 255 };
      SDL_Surface* s = TTF_RenderUTF8_Solid(font, text.c_str(), c);
      if (!s)
      {
        xstring err = SDL_GetError();
        THROW(err);
      }
      return s;
    }

  private:
    bool load(Font& font, const xstring& name, int point_size)
    {
      ResourceFile* rf = get_default_resource_file();
      if (!read_contents(name, font.m_FontData)) return false;
      font.m_RWops = SDL_RWFromConstMem(&font.m_FontData[0], font.m_FontData.size());
      font.m_TTF_Font = TTF_OpenFontRW(font.m_RWops, 1, point_size);
      if (!font.m_TTF_Font) return false;
      return true;
    }

    friend struct std::default_delete<FontManager>;
    FontManager()
    {
#define FM_F(p,x) if ((x=(p)get_function("SDL2_ttf",#x))==0) THROW("SDL_ttf not found.")
      FM_F(init_func, TTF_Init);
      FM_F(quit_func, TTF_Quit);
      FM_F(open_func, TTF_OpenFontRW);
      FM_F(close_func, TTF_CloseFont);
      FM_F(size_func, TTF_SizeUTF8);
      FM_F(draw_func, TTF_RenderUTF8_Solid);
#undef FM_F
      if (TTF_Init()<0)
        THROW("Failed to initialize SDL_ttf");
    }
    ~FontManager()
    {
      clear();
      if (TTF_Quit) TTF_Quit();
    }
    FontManager(const FontManager&) {}
  };

  Font& get_font(const xstring& name, int point_size)
  {
    return FontManager::instance()->get(name, point_size);
  }

  void Font::destroy()
  {
    if (m_TTF_Font) FontManager::instance()->close_font(m_TTF_Font);
    if (m_RWops) SDL_FreeRW(m_RWops);
    //delete m_FontData;
  }

  Font::~Font()
  {
    destroy();
  }

  iVec2  Font::get_size(const xstring& text)
  {
    return FontManager::instance()->get_size(m_TTF_Font, text);
  }

  Bitmap Font::get_bitmap(const xstring& text, Uint32 color)
  {
    SDL_Surface* surface = FontManager::instance()->draw(m_TTF_Font, text, color);
    return Bitmap(bitmap_pixels_ptr(new BitmapPixels(surface)));
  }

  iVec2   Font::draw(int x, int y, const xstring& text, Uint32 color, int align)
  {
    Bitmap bmp = get_bitmap(text, color);
    if (align > 0 && bmp.get_width() < align) x += (align - bmp.get_width());
    if (align < 0 && bmp.get_width() < (-align)) x += (-align - bmp.get_width()) / 2;
    bmp.draw(x, y);
    return bmp.get_size();
  }

//   iVec2   Font::draw(Bitmap target, int x, int y, const xstring& text, Uint32 color, int align)
//   {
//     Bitmap bmp = get_bitmap(text, color);
//     if (align>0 && bmp.get_width()<align) x += (align - bmp.get_width());
//     if (align<0 && bmp.get_width()<(-align)) x += (-align - bmp.get_width()) / 2;
//     bmp.draw(target, x, y);
//     return bmp.get_size();
//   }

  iVec2  Font::draw(const iVec2& pos, const xstring& text, Uint32 color, int align)
  {
    return draw(pos.x, pos.y, text, color, align);
  }

//   iVec2  Font::draw(Bitmap target, const iVec2& pos, const xstring& text, Uint32 color, int align)
//   {
//     return draw(target, pos.x, pos.y, text, color, align);
//   }



} // namespace SDLPP
