// totally legit proggy
//
#include "all_headers.h"
#ifdef _UNICODE
#define UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif // _UNICODE

typedef const TCHAR* LPCTSTR;

#define PLRSZ 0x10
#define SERVERDLL L"server.dll"
#define CLIENTDLL L"client.dll"
#define ENGINEDLL L"engine.dll"


const DWORD player_spotted_by_mask_offset = 0x0000097C;
const DWORD index_offset = 0x00000064;
const DWORD m_bDormant = 0x000000E9;
const DWORD plr_num_offset = 0x00000308; //???
const DWORD local_player_offset = 0x00A844DC;
const DWORD plr_list_offset = 0x4A9F6D4;
const DWORD hp_offset = 0x000000FC;
const DWORD coords_offset = 0x00000134;
const DWORD v_matrix_offset = 0x4A91264;
const DWORD team_num_offset = 0x000000F0;
const DWORD bone_matrix_offset = 0x00002698;
const DWORD head_bone_offset = 0;

DWORD server_dll_base;
DWORD client_dll_base;
DWORD engine_dll_base;

HANDLE hProcess;
HWND csgo_hWnd;
HWND overlay_hWnd;

HDC hDC;
RECT rect;
void get_process_handle();

LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
const MARGINS margin = { 0, 0, 0, 0 };
LPD3DXFONT pFont = NULL;
CDraw Draw;

float view_matrix[4][4];
int world_to_screen(float* from, float* to);
float my_coords[3];
int my_team;
int my_index;

void read_my_coords();
void get_view_matrix();
std::vector<float> get_head_position(int player);
std::stack<enemy *> enemy_players;

int read_bytes(LPCVOID addr, int num, LPVOID buf);
void esp();
void render();
void aim(std::stack<enemy*> copy);
void lock_target(float head_x, float head_y);
void DrawScene(LPDIRECT3DDEVICE9 pDevice);




int main(int argc, char** argv)
{
	get_process_handle();
	GetWindowRect(csgo_hWnd, &rect);
	WinMain(0, 0, 0, 1);
	CloseHandle(hProcess);

	return 0;
}

void read_my_coords(DWORD addr)
{
	DWORD plr_addr;
	read_bytes((void*)(addr), 4, &plr_addr);
	read_bytes((void*)(plr_addr + coords_offset), 12, &my_coords);

	// get my team
	DWORD test = client_dll_base + local_player_offset;
	DWORD result;
	read_bytes((LPCVOID*)test, 4, &result);
	read_bytes((LPCVOID*)(result + team_num_offset), 4, &my_team);

	read_bytes((LPCVOID*)(result + index_offset), 4, &my_index);

}


void esp()
{
	int players_on_map, i, hp, team, spotted_by, spotted, dormant;
	float coords[3];
	DWORD plr_addr;
	DWORD addr = client_dll_base + plr_list_offset;

	system("cls");
	GetWindowRect(csgo_hWnd, &rect);

	//read_bytes((LPCVOID*)(server_dll_base + plr_num_offset), 4,
		//&players_on_map);
	//printf("players on the map: %d\n", players_on_map);

	players_on_map = 20;
	if (players_on_map == 0) return;
	printf("players on the screen:\n");
	read_my_coords(addr);

	while (!enemy_players.empty())
	{
		enemy_players.pop();
	}

	for (i = 0; i < players_on_map; i++) {

		read_bytes((LPCVOID*)(addr + i * PLRSZ), 4, &plr_addr);

		read_bytes((LPCVOID*)(plr_addr + team_num_offset), 4, &team);
		if (team == my_team) continue;

		read_bytes((LPCVOID*)(plr_addr + hp_offset), 4, &hp);
		if (hp == 0) continue;

		read_bytes((LPCVOID*)(plr_addr + m_bDormant), 4, &dormant);
		if ((bool)dormant == true) continue;

		read_bytes((LPCVOID*)(plr_addr + coords_offset), 12, &coords);

		read_bytes((LPCVOID*)(plr_addr + player_spotted_by_mask_offset), 4, &spotted_by);

		spotted = spotted_by & (1 << my_index - 1);


		get_view_matrix();
		float tempCoords[3];
		if (world_to_screen(coords, tempCoords) == 1) {
			printf("player %d health: %d (%g:%g:%g)\n", i, hp,
				coords[0], coords[1], coords[2]);
			std::vector<float> head_pos = get_head_position(i);
			head_pos[0] = head_pos[0] - rect.left;
			head_pos[1] = head_pos[1] - rect.top;
			enemy_players.push(new enemy(tempCoords[0] - rect.left, tempCoords[1] - rect.top, coords, hp, my_coords, head_pos, (bool)spotted));
		}
	}
	
}


//started at the bottom now we here
/*
void draw_health(float x, float y, int health)
{
	char buf[sizeof(int)* 3 + 2];
	_snprintf_s(buf, sizeof buf, "%d", health);
	TextOutA(hDC, x, y, buf, strlen(buf));
}*/


void get_view_matrix()
{
	read_bytes((LPCVOID*)(client_dll_base + v_matrix_offset), 64,
		&view_matrix);
}

std::vector<float> get_head_position(int player) {

	float vecBone[3];
	DWORD addr = client_dll_base + plr_list_offset;
	DWORD plr_addr;
	DWORD plr_bone_matrix;


	read_bytes((LPCVOID*)(addr + player * PLRSZ), 4, &plr_addr);
	read_bytes((LPCVOID*)(plr_addr + bone_matrix_offset), 4, &plr_bone_matrix);
	read_bytes((LPCVOID*)(plr_bone_matrix + 0x30 * 6 + 0x0C), 4, &vecBone[0]);
	read_bytes((LPCVOID*)(plr_bone_matrix + 0x30 * 6 + 0x1C), 4, &vecBone[1]);
	read_bytes((LPCVOID*)(plr_bone_matrix + 0x30 * 6 + 0x2C), 4, &vecBone[2]);

	float tempCoords[3];

	world_to_screen(vecBone, tempCoords);
	std::vector<float> result;
	result.push_back(tempCoords[0]);
	result.push_back(tempCoords[1]);

	return result;
}


int world_to_screen(float* from, float* to)
{
	float w = 0.0f;
	to[0] = view_matrix[0][0] * from[0] + view_matrix[0][1] * from[1] + view_matrix[0][2] * from[2] + view_matrix[0][3];
	to[1] = view_matrix[1][0] * from[0] + view_matrix[1][1] * from[1] + view_matrix[1][2] * from[2] + view_matrix[1][3];
	w = view_matrix[3][0] * from[0] + view_matrix[3][1] * from[1] + view_matrix[3][2] * from[2] + view_matrix[3][3];
	if (w < 0.01f)
		return 0;
	float invw = 1.0f / w;
	to[0] *= invw;
	to[1] *= invw;
	int width = (int)(rect.right - rect.left);
	int height = (int)(rect.bottom - rect.top);
	float x = width / 2;
	float y = height / 2;
	x += 0.5 * to[0] * width + 0.5;
	y -= 0.5 * to[1] * height + 0.5;
	to[0] = x + rect.left;
	to[1] = y + rect.top;
	return 1;
}


void get_process_handle()
{
	LPCWSTR WINDOWNAME = L"Counter-Strike: Global Offensive";
	DWORD pid = 0;
	csgo_hWnd = FindWindow(0, WINDOWNAME);
	if (csgo_hWnd == NULL) {
		printf("FindWindow failed, %08X\n", GetLastError());
		return;
	}
	GetWindowThreadProcessId(csgo_hWnd, &pid);
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
	if (hProcess == 0) {
		printf("OpenProcess failed, %08X\n", GetLastError());
		return;
	}
	hDC = GetDC(csgo_hWnd);
	HMODULE hMods[1024];
	int i;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &pid) == 0) {
		printf("enumprocessmodules failed, %08X\n", GetLastError());
	}
	else {
		for (i = 0; i < (pid / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR))) {
				if (wcsstr(szModName, SERVERDLL) != 0) {
					printf("server.dll base: %08X\n", hMods[i]);
					server_dll_base = (DWORD)hMods[i];
				}
				if (wcsstr(szModName, CLIENTDLL) != 0) {
					printf("client.dll base: %08X\n", hMods[i]);
					client_dll_base = (DWORD)hMods[i];
				}
				if (wcsstr(szModName, ENGINEDLL) != 0) {
					printf("engine.dll base: %08X\n", hMods[i]);
					engine_dll_base = (DWORD)hMods[i];
				}
				//std::wcout << szModName << std::endl;
			}
		}
	}
}

int read_bytes(LPCVOID addr, int num, LPVOID buf)
{
	SIZE_T sz = 0;
	int r = ReadProcessMemory(hProcess, addr, buf, num, &sz);
	if (r == 0 || sz == 0) {
		printf("RPM error, %08X\n", GetLastError());
		return 0;
	}
	return 1;
}



void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
	GetWindowRect(csgo_hWnd, &rect);

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = rect.right - rect.left;    // set the width of the buffer
	d3dpp.BackBufferHeight = rect.bottom - rect.top;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	Draw = CDraw();

	//Draw.AddFont(L"Arial", 15, false, false);
	//Draw.AddFont(L"Verdana", 15, true, false);
	//Draw.AddFont(L"Verdana", 13, true, false);
	//Draw.AddFont(L"Comic Sans MS", 30, true, false);

	D3DXCreateFont(d3ddev, 25, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFont);
}

void DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const WCHAR *fmt)
{
	RECT FontPos = { x, y, x + 120, y + 16 };
	WCHAR buf[1024] = { L'\0' };
	va_list va_alist;

	va_start(va_alist, fmt);
	vswprintf_s(buf, fmt, va_alist);
	va_end(va_alist);
	g_pFont->DrawText(NULL, buf, -1, &FontPos, DT_NOCLIP, color);
}

void render()
{
	// clear the window alpha

	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

	DrawScene(d3ddev);

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}


void DrawScene(LPDIRECT3DDEVICE9 pDevice)
{
	Draw.GetDevice(pDevice);
	Draw.Reset();
	if (Draw.Font()) Draw.OnLostDevice();

	while (!enemy_players.empty()) {
		enemy* enemy = enemy_players.top();
		float x = enemy->x;
		float y = enemy->y;
		float dist = enemy->dist;
		int health = enemy->hp;
		float head[2];
		head[0] = enemy->head[0];
		head[1] = enemy->head[1];

		Draw.Box(head[0] - 10, head[1] - 10, 20, 20, 1, ORANGE(255)); //head box (no scaling)
		//Draw.Box(head[0] - 5000 / dist, head[1] + 5000 / dist, 18100 / dist, 34000 / dist, 1, ORANGE(255)); //top half of body with iffy scaling

		//WCHAR buf[5] = {L'\0'};
		//_snwprintf_s(buf, sizeof buf, L"%d", health);
		//Draw.Text(buf, x, y, centered, 0, false, ORANGE(255), RED(255)); bug with add font
		
		D3DCOLOR health_bar_color;
		if (health < 35) {
			health_bar_color = RED(255);
		}
		else if (health < 50) {
			health_bar_color = DARKORANGE(255);
		}
		else if (health < 75) {
			health_bar_color = YELLOW(255);
		}
		else if (health <= 100) {
			health_bar_color = GREEN(255);
		}
		//Draw.Line(head[0] - 5000 / dist, head[1] + 5000 / dist, head[0] - 5000 / dist, head[1] + 5000 / dist + 34000 / dist, 3, false, health_bar_color); (only works properly with bots)
		Draw.Line(head[0] - 10, head[1] - 10, head[0] - 10, head[1] - 10 + 20, 3, false, health_bar_color);
		enemy_players.pop();
	}
}


float DistanceBetweenCross(float X, float Y)
{
	float ydist = (Y - (rect.bottom-rect.top)/2);
	float xdist = (X - (rect.right-rect.left)/2);
	float Hypotenuse = sqrt(pow(ydist, 2) + pow(xdist, 2));
	return Hypotenuse;
}

bool new_data = false;

void aim(std::stack<enemy*> copy) {
	new_data = true;
	if (GetAsyncKeyState(VK_LCONTROL)) {
		if (copy.size() > 0) {
			enemy * closest_enemy = copy.top();
			float shortest_crosshair_dist = DistanceBetweenCross(closest_enemy->head[0], closest_enemy->head[1]);
			copy.pop();
			while (!copy.empty()) {
				float dist = DistanceBetweenCross(copy.top()->head[0], copy.top()->head[1]);
				if (dist < shortest_crosshair_dist) {
					closest_enemy = copy.top();
					shortest_crosshair_dist = dist;
				}
				copy.pop();
			}

			//std::cout << std::to_string(shortest_crosshair_dist) << std::endl;
			//std::cout << "Locked on target - Distance: " <<std::to_string(closest_enemy->dist) << std::endl;

			if (!closest_enemy->spotted) {
				return;
			}

			if (shortest_crosshair_dist > 60) {
				return;
			}
			

			std::async(lock_target, closest_enemy->head[0] - (rect.right - rect.left) / 2,
				closest_enemy->head[1] - (rect.bottom - rect.top) / 2);
			
		}
	}
}

void lock_target(float head_x, float head_y){
	new_data = false;

	for (int i=0;i<3;i++) {
		float dx = head_x;
		float dy = head_y;

		INPUT input = { 0 };

		input.mi.dx = dx;
		input.mi.dy = dy;
		input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
		input.type = INPUT_MOUSE;

		SendInput(1, &input, sizeof INPUT);

		Sleep(10);
		if (new_data) return;
	}
}

/*** OVERLAY WINDOW STUFF ***/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPCWSTR lpClassName = L"LayeredWindowClass";
	LPCWSTR lpWindowName = L"LayeredWindow";

	int x = (rect.right - rect.left);
	int y = (rect.bottom - rect.top);

	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = RGB(0, 0, 0);
	wc.lpszClassName = lpClassName;

	RegisterClassEx(&wc);

	overlay_hWnd = CreateWindowEx(WS_EX_LAYERED,
		lpClassName, lpWindowName, 
		WS_POPUP,
		rect.left, rect.top, 
		x, y,
		NULL,
		NULL,
		hInstance,
		NULL);

	SetLayeredWindowAttributes(overlay_hWnd, RGB(0, 0, 0), 75, ULW_COLORKEY | LWA_ALPHA);

	ShowWindow(overlay_hWnd, nCmdShow);
	UpdateWindow(overlay_hWnd);

	initD3D(overlay_hWnd);

	MSG msg;
	SetWindowPos(overlay_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	while (TRUE)
	{
		if (!FindWindow(NULL, lpWindowName)) ExitProcess(0);
		
		esp();
		std::stack<enemy*> copy = enemy_players;
		render();
		aim(copy);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			exit(0);

	}
	return msg.wParam;;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	return TRUE;
}

void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

void OnQuit(HWND hwnd, int exitCode)
{

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case(WM_PAINT):
		DwmExtendFrameIntoClientArea(overlay_hWnd, &margin);
		break;
	HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	HANDLE_MSG(hwnd, WM_QUIT, OnQuit);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}