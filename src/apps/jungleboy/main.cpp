#include <sdlpp.h>
#include <sdlpp_io.h>

using namespace SDLPP;

extern bool g_only_keyboard;

#include "boy.h"
#include "ogre.h"
#include "dragon.h"
#include "cloud.h"
#include "food.h"
#include "pickup.h"
#include "platforms.h"

extern ResourceFile g_ResourceFile;

void print_food()
{
  xstring food_left = "Food Left: " + xstring(g_food_left) + "\r\n";
  //OutputDebugStringA(food_left);
}

Font& game_font() 
{ 
  return get_font("rsc/arcade.ttf",20); 
}

xml_element* find_descendant(xml_element* root, const xstring& type)
{
  if (!root) return 0;
  if (root && root->get_type() == type) return root;
  xml_element::iterator b=root->begin(),e=root->end();
  for(;b!=e;++b)
  {
    xml_element* son=*b;
    xml_element* res=find_descendant(son,type);
    if (res) return res;
  }
  return 0;
}

struct FloorRun
{
  FloorRun(int row=0, int from=0, int to=0)
    : y(row), x0(from), x1(to) {}
  int y,x0,x1;
};

typedef std::vector<FloorRun> floor_vec;

inline bool floor_interfers(const FloorRun& a, const FloorRun& b)
{
  if (udiff(a.y,b.y)>2) return false;
  if (a.x1<b.x0 || b.x1<a.x0) return false;
  return true;
}

void generate_floors(int screen)
{
  srand(screen);
  g_food_left=0;
  g_next_screen=false;
  floor_vec v;
  v.reserve(100);
  if (g_easy)
    v.push_back(FloorRun(14,0,20));
  else
    v.push_back(FloorRun(14,0,4));
  for(int i=0;i<500;++i)
  {
    int x0=irand(17);
    int len=3+irand(6);
    int x1=x0+len;
    if (x1>20) x1=20;
    FloorRun f(3+irand(12),x0,x1);
    {
      bool fail=false;
      floor_vec::iterator b=v.begin(),e=v.end();
      for(;b!=e && !fail;++b)
      {
        const FloorRun& sec=*b;
        if (floor_interfers(f,sec)) 
          fail=true;
      }
      if (!fail) v.push_back(f);
    }
  }
  {
    bool first_floor=true;
    floor_vec::iterator b=v.begin(),e=v.end();
    for(;b!=e;++b,first_floor=false)
    {
      const FloorRun& f=*b;
      int y=f.y;
      int x=f.x0;
      int len=f.x1-f.x0;
      GrassFloor* gf=new GrassFloor(GrassFloor::LEFT);
      gf->set_position(iVec2(x*32,y*32));
      for(int i=0;i<(len-2);++i)
      {
        ++x;
        gf=new GrassFloor(GrassFloor::CENTER);
        gf->set_position(iVec2(x*32,y*32));
        if (irand(6)==0)
        {
          Food* fd=new Food;
          fd->set_position(iVec2(x*32,(y-1)*32));
          g_food_left++;
        }
        else
        if (irand(10)==0 && !first_floor && len>4)
        {
          Ogre* ogre=new Ogre;
          ogre->set_position(iVec2(x*32,y*32-54));
        }
        else
        if (irand(12)==0)
        {
          Pickup* p=new Pickup;
          p->set_position(iVec2(x*32,(y-1)*32));
        }
      }
      ++x;
      gf=new GrassFloor(GrassFloor::RIGHT);
      gf->set_position(iVec2(x*32,y*32));
    }
  }
}

#define aMAIN int a_main(int argc, char* argv[])

aMAIN
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		display_message("SDL_Init Error");
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == nullptr){
		display_message("SDL_CreateWindow Error");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr){
		SDL_DestroyWindow(win);
		display_message("SDL_CreateRenderer Error");
		SDL_Quit();
		return 1;
	}
	SDL_Surface *bmp = SDL_LoadBMP("rsc/rboy.bmp");
	if (bmp == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		display_message("SDL_LoadBMP Error");
		SDL_Quit();
		return 1;
	}
	SDL_Rect r={0,0,bmp->w,bmp->h};
	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);
	if (tex == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		display_message("SDL_CreateTextureFromSurface Error");
		SDL_Quit();
		return 1;
	}
	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, &r);
	SDL_RenderPresent(ren);
    SDL_Delay(5000);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

#define C { display_message(__FILE__+xstring(":")+xstring(__LINE__)); }

int main(int argc, char* argv[])
{
  bool full_screen=false;
  if (0)
  {
	  xml_element* cfg_doc=load_xml_from_file("config.xml");
	  {
	    xml_element* el=find_descendant(cfg_doc,"screen");
	    if (el)
	    {
	      xstring s=el->get_attribute("full");
	      full_screen=(s=="YES");
	    }
	  }
	  {
	    xml_element* el=find_descendant(cfg_doc,"difficulty");
	    if (el)
	    {
	      xstring s=el->get_attribute("easy");
	      g_easy=(s=="YES");
	    }
	  }
	  {
	    xml_element* el=find_descendant(cfg_doc,"joystick");
	    if (el)
	    {
	      xstring s=el->get_attribute("enable");
	      g_only_keyboard=!(s=="YES");
	    }
	  }
	  delete cfg_doc;
  }
  
  try
  {
    Application app;
    int fs=0;
    bool cfg=true;
    set_default_resource_file(&g_ResourceFile);
    //SDL_WM_SetCaption("Jungle Boy","jungleboy.ico");
    app.init_audio(22050,false);
    app.init_graphics(640,480,full_screen);
    SDL_Delay(500);
    //Bitmap& screen=GraphicsManager::instance()->get_screen();
    GameView gv(640,480);
    Bitmap bg = BitmapCache::instance()->get("rsc/bg.bmp");
    SDL_ShowCursor(SDL_DISABLE);
    srand(SDL_GetTicks());
    //SoundStream music(g_ResourceFile,"rsc/time-to-go.mp3");
    //music.play();
    {
      int screen_number=0;
      bool playing=true;
      JungleBoy boy;
      while (playing)
      {
        ANIMATION_SCENE;
        ObjectCreator<Cloud>(16); // dynamically allocate clouds (volatile objects)
        generate_floors(++screen_number);
        print_food();
        boy.reset();
        boy.set_position(iVec2(10,360));
        int last_ticks=SDL_GetTicks();
        g_next_screen=false;
        while (!g_next_screen)
        {
          if (boy.get_death_duration()>3000 && !g_game_over)
          {
            --screen_number;
            if (--g_lives==0) g_game_over=true;
            else
            break;
          }
          if (is_pressed(SDLK_ESCAPE)) { playing=false; break; }
          if (g_game_over)
          {
            poll();
            //Graphics::instance()->fill(MapRGB(0, 128, 255));
            bg.draw(iRect2(0, 0, 640, 480));
            get_font("rsc/arcade.ttf",120).draw(10,200,"Game Over",MapRGB(0,0,255),-640);
            flip();
            SDL_Delay(10);
            continue;
          }
          if (!g_easy && irand(500-screen_number*2)==0) new Dragon;
          int cur_ticks=SDL_GetTicks();
          int dt=cur_ticks-last_ticks;
          last_ticks=cur_ticks;
          poll();
          try {
            advance(dt);
          } catch (...) {}
          //Graphics::instance()->fill(MapRGB(0, 128, 255));
          bg.draw(iRect2(0, 0, 640, 480));
          {
            std::ostringstream os;
            os << "Lives: " << g_lives;
            game_font().draw(0,0,os.str(),0xFFFFFFFF);
          }
          {
            std::ostringstream os;
            os << "Score: " << g_score;
            game_font().draw(0, 20, os.str(), 0xFFFFFFFF);
          }
          {
            std::ostringstream os;
            os << "Level: " << screen_number;
            game_font().draw(0, 40, os.str(), 0xFFFFFFFF);
          }
          render(gv);
          flip();
          SDL_Delay(10);
        }
      }
    }
    //music.stop();
  } catch (const xstring& msg) {
    display_message(msg);
  } catch (const std::string& msg) {
    display_message(msg);
  }

  return 0;
}
