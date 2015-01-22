#include <sdlpp.h>
#include <sdlpp_io.h>

using namespace SDLPP;


ResourceFile g_ResourceFile("");  // Work with direct files
//ResourceFile g_ResourceFile("jungleboy.rsc");  // Work with packed resource file

bool g_next_screen=false;
bool g_game_over=false;
int  g_food_left=0;
int  g_lives=3;
int  g_score=0;
bool g_easy=true;
bool g_only_keyboard=false;

