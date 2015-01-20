#ifndef sdlpp_font_h__
#define sdlpp_font_h__

#include <sdlpp_graphics.h>

struct TTF_Font;

namespace SDLPP
{

  class Font
  {
    TTF_Font*         m_TTF_Font;
    std::vector<char> m_FontData;
    SDL_RWops*        m_RWops;

    friend class FontManager;
    //Font(TTF_Font* font, std::vector<char>* font_data, SDL_RWops* rw) : m_TTF_Font(font), m_FontData(font_data), m_RWops(rw) {}
    void destroy();

    Font(const Font& rhs) {}
    Font& operator= (const Font& rhs) { return *this; }
  public:
    Font() : m_TTF_Font(0), m_FontData(0), m_RWops(0) {}
    virtual ~Font();

    iVec2  get_size(const xstring& text);
    Bitmap get_bitmap(const xstring& text, Uint32 color);
    iVec2  draw(int x, int y, const xstring& text, Uint32 color, int align = 0);
    iVec2  draw(const iVec2& pos, const xstring& text, Uint32 color, int align = 0);
//     iVec2  draw(Bitmap target, int x, int y, const xstring& text, Uint32 color, int align = 0);
//     iVec2  draw(Bitmap target, const iVec2& pos, const xstring& text, Uint32 color, int align = 0);
  };

  Font& get_font(const xstring& name, int point_size);


} // namespace SDLPP


#endif // sdlpp_font_h__
