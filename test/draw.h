#include "all_headers.h"

enum circle_type { full, half, quarter };
enum text_alignment { lefted, centered, righted };

#define MAX_FONTS 6
#define WHITE(alpha)         D3DCOLOR_ARGB(alpha, 255, 255, 255)
#define BLACK(alpha)         D3DCOLOR_ARGB(alpha, 0, 0, 0)
#define RED(alpha)           D3DCOLOR_ARGB(alpha, 255, 0, 0)
#define GREEN(alpha)         D3DCOLOR_ARGB(alpha, 0, 128, 0)
#define LAWNGREEN(alpha)     D3DCOLOR_ARGB(alpha, 124, 252, 0)
#define BLUE(alpha)          D3DCOLOR_ARGB(alpha, 0, 200, 255)
#define DEEPSKYBLUE(alpha)   D3DCOLOR_ARGB(alpha, 0, 191, 255)
#define SKYBLUE(alpha)       D3DCOLOR_ARGB(alpha, 0, 122, 204)
#define YELLOW(alpha)        D3DCOLOR_ARGB(alpha, 255, 255, 0)
#define ORANGE(alpha)        D3DCOLOR_ARGB(alpha, 255, 165, 0)
#define DARKORANGE(alpha)    D3DCOLOR_ARGB(alpha, 255, 140, 0)
#define PURPLE(alpha)        D3DCOLOR_ARGB(alpha, 125, 0, 255)
#define CYAN(alpha)          D3DCOLOR_ARGB(alpha, 0, 255, 255)
#define PINK(alpha)          D3DCOLOR_ARGB(alpha, 255, 20, 147)
#define GRAY(alpha)          D3DCOLOR_ARGB(alpha, 128, 128, 128)
#define DARKGRAY(alpha)      D3DCOLOR_ARGB(alpha, 73, 73, 73)
#define DARKERGRAY(alpha)    D3DCOLOR_ARGB(alpha, 31, 31, 31)

struct vertex
{
	FLOAT x, y, z, rhw;
	DWORD color;
};

class CDraw
{
public:
	CDraw()
	{
		g_pVB = NULL;
		g_pIB = NULL;
		int FontNr = 0;
	}

	struct sScreen
	{
		float Width;
		float Height;
		float x_center;
		float y_center;
	} Screen;

	ID3DXFont *pFont[MAX_FONTS];

	void Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution, float scale, float rotation);

	//=============================================================================================
	void Line(float x1, float y1, float x2, float y2, float width, bool antialias, DWORD color);

	void Box(float x, float y, float w, float h, float linewidth, DWORD color);
	void BoxFilled(float x, float y, float w, float h, DWORD color);
	void BoxBordered(float x, float y, float w, float h, float border_width, DWORD color, DWORD color_border);
	void BoxRounded(float x, float y, float w, float h, float radius, bool smoothing, DWORD color, DWORD bcolor);

	void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color);
	void CircleFilled(float x, float y, float rad, float rotate, int type, int resolution, DWORD color);

	void Text(WCHAR *text, float x, float y, int orientation, int font, bool bordered, DWORD color, DWORD bcolor);
	void Message(char *text, float x, float y, int font, int orientation);
	//=============================================================================================

	//=============================================================================================
	bool Font();
	void AddFont(LPCWSTR Caption, float size, bool bold, bool italic);
	void FontReset();
	void OnLostDevice();
	//=============================================================================================

	void GetDevice(LPDIRECT3DDEVICE9 pDev) { pDevice = pDev; }
	void Reset();
private:
	LPDIRECT3DDEVICE9 pDevice;
	LPDIRECT3DVERTEXBUFFER9 g_pVB;    // Buffer to hold vertices
	LPDIRECT3DINDEXBUFFER9  g_pIB;    // Buffer to hold indices

	int FontNr;
	LPD3DXSPRITE sSprite;
};

extern CDraw Draw;