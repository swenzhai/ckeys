#include "Keyboard.h"
#include "KeyNames.h"
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include "../libs/jsmn.h"
using namespace std;


//  ctor
Keys::Keys()
{
	//  filled vk to str maps from csKeyNames
	for (int k=0; k < 256; ++k)
	{
		string s = csKeyNames[k];
		vk2str[k] = s;
		str2vk[s] = k;
	}
}

//  find data path
//-----------------------------------------------------------
bool Keys::Init()
{
	pathData = "data/";
	files.push_back("default");  //todo read dir
	files.push_back("ck3");
	files.push_back("ck4");
	return true;
}

void Keys::Destroy()
{
	keys.clear();
	vk2key.clear();
}

//  read json layout from file
void Keys::LoadIndex(int id)
{
	if (id < files.size())
		LoadFromJson(pathData + files[id] + ".json");
}


//  load layout, read json layout from file
//-----------------------------------------------------------------------------------------------------------
void Keys::LoadFromJson(std::string path)
{
	//  clear last
	Destroy();

	//  load
	ifstream f(path);
	stringstream ss;  ss << f.rdbuf();
	string str = ss.str();
	const char* s = str.c_str();

	//  jsmn json vars
	int i,r;
	jsmn_parser p;
	jsmntok_t t[512];  // const max
	jsmn_init(&p);

	//  parse
	string q, re;
	r = jsmn_parse(&p, s, strlen(s), t, sizeof(t)/sizeof(t[0]));
	if (r < 0)
		re += "Failed to parse JSON: " + r;

	//  layout
	int x=0, y=0;
	float w=1.f, h=1.f;  string prim;

	//  params
	const float sc = 1.3f;
	const int  sx = 40 * sc, sy = sx, se = 5,  // size x,y, empty margin
			 yfnt = 14 * sc, x0 = 30, y0 = 120;  // font, start pos

	//  iterate all
	//-----------------------------------------------------------
	for (i = 1; i < r; i++)
	{
		if (t[i].type == JSMN_ARRAY)
		{   x = 0;  y += sy;  w = 1.f;  h = 1.f;  // next row
		}
		else
		if (t[i].type == JSMN_STRING)
		{
			//  key text
			string s = str.substr(t[i].start, t[i].end - t[i].start);
			if (s[0]>='a' && s[0]<='z')
				prim = s;
			else
			{
				//  replace
				bool has2 = replK(s, "\\n", "\n");  // key has 2 descr: upper, lower
				replK(s, "Lock", "");  // rem Lock
				replK(s, "\\\\", "\\");
				replK(s, "\\\"", "\"");

				//  font scale
				float sf = w < 0.7f ? 0.6f/*ck4 mini*/ : 0.8f;
				//float sf = k2 ? 0.8f :
				//    k.length() > 1 ? 0.7f : 1.f;

				//  add key  --
				Key k;  k.x = x0 + x;  k.y = y0 + y;
				k.w = sx * w - se;  k.h = sy * h - se;
				k.sc = sf * yfnt;   k.s = s;
				keys.push_back(k);

				x += w * sx;
				w = 1.f;  h = 1.f;  // reset

				//  vk to key
				if (has2)  // use the lower
				{
					size_t p = s.find("\n");
					bool rep = p != std::string::npos;
					if (rep)
						s = s.substr(p+1);
				}
				int vk = str2vk[s];
				if (vk)  // if found
					vk2key[vk] = keys.size()-1;
		}   }
		else
		if (t[i].type == JSMN_PRIMITIVE)
		{
			string p = str.substr(t[i].start, t[i].end - t[i].start);
			if (p[0]>='a' && p[0]<='z')
				prim = p;
			else
			{   float f = 0.f;
				sscanf(p.c_str(), "%f", &f);

				if (prim == "x")  x += f * sx;
				else
				if (prim == "y")  y += f * sy;
				else
				if (prim == "w")  w = f;
				else
				if (prim == "h")  h = f;
			}
		}
	}
}
