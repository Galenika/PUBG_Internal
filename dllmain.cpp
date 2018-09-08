#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <d3d11.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <D3DX11.h>
#include <dxgi.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <vector>
#include <d3dcompiler.h>
#include "MinHook\include\MinHook.h"
#include "FW1FontWrapper\FW1FontWrapper.h"
#include "\Users\Kurt\Downloads\D3D11Renderer_[unknowncheats.me]_\D3D11Renderer.h"

#pragma comment(lib, "libMinHook-x64-v141-mtd.lib")
#pragma comment(lib, "FW1FontWrapper.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define FUNC_FINDER_ADDR		0x836F20

ID3D11DeviceContext *pContext = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

char dlldir[320];
char *GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}
void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	std::ofstream logfile(GetDirectoryFile("log.txt"), std::ios::app);
	if (logfile.is_open() && text)	logfile << text << std::endl;
	logfile.close();
}

DWORD_PTR dwGetBaseAddress()
{
	return (DWORD_PTR)GetModuleHandle(NULL);
}

#pragma region PUBG_Funcs

namespace globals
{
	char *espBoxTypeStr[3] = { "2D Box", "3D Box", "2D Filled Box" };
	char *espEnableStr[3] = { "Off", "On" };
	bool bPlayerESP, bSnapLines, bDrawNames, bDrawDistance, bDrawHealth;
	bool bItemESP;
	bool bDebugESP;
	int g_espBoxStyle = 0;
	float g_espDrawDistance;

	int ActorIDs[4];
	int ErangelMainID;
	int DroppedItemGroupID;
	int DroppedItemInteractionComponentID;
	int DroppedItemID;
}

class ULevel;
class UGameInstance;
class ULocalPlayer;
class UGameViewportClient;
class APlayerController;
class UPlayer;
struct FString;
class AActor;

bool GetGName(int id, char *outname)
{
	DWORD_PTR modBase = (DWORD_PTR)GetModuleHandle(NULL);
	DWORD_PTR GNames = *(DWORD_PTR*)(modBase + 0x36D8590);
	if (!GNames)
		return false;
	DWORD_PTR chunk = *(DWORD_PTR*)(GNames + (id / 0x4000) * 8);
	if (!chunk)
		return false;
	DWORD_PTR nameptr = *(DWORD_PTR*)(chunk + (8 * (id % 0x4000)));
	if (!nameptr)
		return false;

	int str_len = 0;
	while (true)
	{
		char c = *(char*)(nameptr + 0x10 + str_len);
		if (c == 0x0)
			break;
		str_len++;
		if (str_len > 150)
			return false;
	}
	memcpy(outname, (LPVOID)(nameptr + 0x10), str_len);
	//memset(&outname[str_len], 0, 4);
	return true;
}

char *GetGNameAddr(DWORD_PTR addr)
{
	char outname[64] = {};
	GetGName(*(DWORD*)(addr + 0x18), outname);
	return outname;
}

template <class T>
struct TArray
{
public:
	inline TArray()
	{
		Data = nullptr;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline int Count()
	{
		return count;
	}

	inline const T& operator[](int i) const
	{
		return Data[i];
	};
private:
	T* Data;
	int count;
	int max;
};

struct FString
{
public:

	inline wchar_t *GetWideString()
	{
		return this->str;
	}

	inline char *GetString()
	{
		char mbs[256] = {};
		wcstombs(mbs, this->str, this->count);
		return mbs;
	}

private:
	wchar_t *str;
	int count;
	int max;
};

class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3()
	{
		x = y = z = 0;
	}

	Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	float Dot(const Vector3 &vec)
	{
		return this->x * vec.x + this->y * vec.y + this->z * vec.z;
	}

	float Length()
	{
		return (float)sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	Vector3 operator-(const Vector3 &vec)
	{
		return Vector3(this->x - vec.x, this->y - vec.y, this->z - vec.z);
	}
	Vector3 operator+(const Vector3 &vec)
	{
		return Vector3(this->x + vec.x, this->y + vec.y, this->z + vec.z);
	}
};

void AngleVectors(Vector3 angles, Vector3 *forward)
{
	float sp, sy, cp, cy;
	float angle;

	angle = angles.y * (M_PI / 180.0f);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * (M_PI / 180.0f);
	sp = sin(angle);
	cp = cos(angle);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

class UWorld
{
public:
	char              _0x0000[0x30];
	ULevel*           pLevel; //0x30
	char              _0x0038[0x108];
	UGameInstance*    pOwningGameInstance; //0x140
};

class ULevel
{
public:
	unsigned char				_0x0000[0xA0]; //0x00
	TArray<AActor*>		entityList; //0xA0
};

class UGameInstance
{
public:
	char					_0x0000[0x38];
	TArray<ULocalPlayer*>	LocalPlayers;
};

class ULocalPlayer
{
public:
	char                        _0x0000[0x30];
	APlayerController*    PlayerController; //0x30
	char                          _0x0038[0x20];
	UGameViewportClient*    ViewportClient; //0x58
};

class UGameViewportClient
{
public:
	char                    _0x0000[0x80];
	UWorld*           World; //0x80
	UGameInstance*    GameInstance; //0x88
};

struct FMinimalViewInfo
{
	Vector3                                     Location;                                                 // 0x0000(0x000C) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	Vector3                                     Rotation;                                                 // 0x000C(0x000C) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	float                                              FOV;                                                      // 0x0018(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	float                                              OrthoWidth;                                               // 0x001C(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	float                                              OrthoNearClipPlane;                                       // 0x0020(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	float                                              OrthoFarClipPlane;                                        // 0x0024(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	float                                              AspectRatio;                                              // 0x0028(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_IsPlainOldData)
	unsigned char                                      bConstrainAspectRatio;                               // 0x002C(0x0001) (CPF_Edit, CPF_BlueprintVisible)
	unsigned char                                      bUseFieldOfViewForLOD;
};

struct FCameraCacheEntry
{
	char							_0x0000[0x10];
	FMinimalViewInfo			POV;
};

class APlayerCameraManager
{
public:
	char                        _0x0000[0x3B8];
	float                       DefaultFOV; //0x3B8
	char                        _0x03BC[0x4];
	float                       DefaultOrthoWidth; //0x3C0
	char                        _0x03C4[0x4];
	float                       DefaultAspectRatio; //0x3C8
	char                        _0x03CC[0x44];
	FCameraCacheEntry    CameraCache; //0x410
};

struct FScriptMulticastDelegate
{
	char UnknownData[16];
};

class USceneComponent
{
public:
	char		_0x0000[0x1E0];
	Vector3		RelativeLocation; //0x1E0
	Vector3		RelativeRotation; //0x1EC
	char		_0x01F8[0x60];
	Vector3		ComponentVelocity; //0x258
};

class APawn
{

};
class AMatineeActor
{

};
class FName
{

};
class UActorComponent
{

};

class APlayerState
{
public:
	char       _0x0000[0x3A8];
	FString    PlayerName; //0x3A8
};

struct pADroppedItemArray
{
	DWORD_PTR		pActor; //0x0
	unsigned char	_0x0008[0x8]; //0x8
};

class AActor
{
public:
	char                _0x0000[0x18];
	int                 id; //0x18
	char                _0x001C[0x164];
	USceneComponent*    RootComponent; //0x180
	char                _0x0188[0x150];
	TArray<pADroppedItemArray>           DroppedItemArray; //0x2D8
	char                _0x02E8[0xD8];
	APlayerState*       PlayerState; //0x3C0
	char				_0x03C8[0xCA0];
	float				health; //0x1068
	float				healthMax; //0x106C

	bool IsAlive()
	{
		return this->health > 0 && this->health <= 100.0f;
	}

	bool IsPlayer()
	{
		return this->id == globals::ActorIDs[0] || this->id == globals::ActorIDs[1] || this->id == globals::ActorIDs[2] || this->id == globals::ActorIDs[3];
	}

	bool IsDroppedItem()
	{
		return this->id == globals::DroppedItemID;
	}

	bool IsDroppedItemGroup()
	{
		return this->id == globals::DroppedItemGroupID;
	}

	bool IsDroppedItemInteractionComponent()
	{
		return this->id == globals::DroppedItemInteractionComponentID;
	}

	int K2_TeleportTo(Vector3, Vector3);
	void DisableInput(APlayerController *PlayerController);
	void EnableInput(APlayerController *PlayerController);
};

int AActor::K2_TeleportTo(Vector3 loc, Vector3 rot)
{
	auto result = (*(int(__fastcall**)(AActor*, Vector3*, Vector3*))(*(DWORD64*)this + 0x470))(this, &loc, &rot);
	return result;
}

void AActor::DisableInput(APlayerController * PlayerController)
{
	(*(int(__fastcall**)(AActor*, APlayerController*))(*(DWORD64*)this + 0x270))(this, PlayerController);
}

void AActor::EnableInput(APlayerController * PlayerController)
{
	(*(int(__fastcall**)(AActor*, APlayerController*))(*(DWORD64*)this + 0x268))(this, PlayerController);
}

class APlayerController
{
public:
	char						_0x0000[0x3A8];
	AActor*						Actor; //0x3A8
	char						_0x03B0[0x58];
	ULocalPlayer*				LocalPlayer; //0x408
	char                     _0x0410[0x28];
	APlayerCameraManager*		PlayerCameraManager; //0x438
	char                        _0x0440[0xC];
	Vector3                     TargetViewRotation; //0x44C

	bool IsVisible(AActor *target);
};

bool APlayerController::IsVisible(AActor *target)
{
	Vector3 fillVec = Vector3(0, 0, 0);
	auto result = (*(int(__fastcall**)(APlayerController*, AActor*, Vector3*, bool))(*(DWORD64*)this + 0x620))(this, target, &fillVec, false);
	return result;
}

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{

	float radPitch = (float)(rot.x * M_PI / 180.f);
	float radYaw = (float)(rot.y  * M_PI / 180.f);
	float radRoll = (float)(rot.z * M_PI / 180.f);

	float SP = (float)sin(radPitch);
	float CP = (float)cos(radPitch);
	float SY = (float)sin(radYaw);
	float CY = (float)cos(radYaw);
	float SR = (float)sin(radRoll);
	float CR = (float)cos(radRoll);
	D3DXMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;
	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;
	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;
	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;
	return matrix;
}

bool WorldToScreen(Vector3 worldLoc, APlayerCameraManager *cameraManager, Vector3 &screenLoc)
{
	screenLoc = Vector3(0, 0, 0);

	auto POV = cameraManager->CameraCache.POV;
	Vector3 rotation = POV.Rotation;

	D3DXMATRIX tempMatrix = Matrix(rotation);
	Vector3 vAxisX, vAxisY, vAxisZ;
	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = worldLoc - POV.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.0f)
		vTransformed.z = 1.0f;

	float fovAngle = POV.FOV;
	float screenCenterX = 1920 / 2;
	float screenCenterY = 1080 / 2;

	screenLoc.x = screenCenterX + vTransformed.x * (screenCenterX / (float)tan(fovAngle * M_PI / 360)) / vTransformed.z;
	screenLoc.y = screenCenterY - vTransformed.y * (screenCenterX / (float)tan(fovAngle * M_PI / 360)) / vTransformed.z;

	return true;
}

char *itemEspWeapons[] = { "AKM", "Kar98k", "M16A4", "M416", "SCAR-L", "SKS", "Micro UZI", "12 Gauge", "P1911", "Pan", "P92", "S1897", "Crossbow" };
char *itemEspAttachments[] = { "Extended Mag (AR, S12K)", "Compensator(AR, S12K)", "Ext.QuickDraw Mag(AR, S12K)", "Suppressor(AR, S12K)", "Suppressor (Sniper Rifle)", "Compensator (Sniper Rifle)", "Ext. QuickDraw Mag (M24, AWM, SKS)", "Angled Foregrip (M416, SCAR-L, UMP9)", "Vertical Foregrip (M416, SCAR-L, UMP9, Vector)", "2x Scope", "4x Scope", "8x Scope", "Red Dot Sight", "Cheek Pad (Sniper Rifle)" };
char *itemEspEquipment[] = { "Backpack (Lv. 1)", "Backpack (Lv. 2)", "Backpack (Lv. 3)", "Helmet (Lv. 1)", "Helmet (Lv. 2)", "Helmet (Lv. 3)", "Police Vest (Lv. 1)", "Police Vest (Lv. 2)", "Police Vest (Lv. 3)" };
char *itemEspMedic[] = { "First Aid Kit", "Painkiller", "Energy Drink" };


bool DoesStrArrayContain(char **array, char *str)
{
	int count = sizeof(array) / sizeof(char*);
	for (int i = 0; i < count; i++)
	{
		if (!strcmp(array[i], str))
			return true;
	}
	return false;
}

#define DRAW_STRING_MAX 50
int drawStringCount = 0;
void DrawString(char *text, float size, float x, float y, Color color)
{
	if (drawStringCount < DRAW_STRING_MAX)
	{
		wchar_t wstr[256] = {};
		mbstowcs(wstr, text, 256);
		int abgr = (color.A << 24) | (color.B << 16) | (color.G << 8) | (color.R);
		pFontWrapper->DrawString(pContext, wstr, size, x, y, abgr, FW1_RESTORESTATE);
		drawStringCount++;
	}
}

void DrawString(wchar_t *text, float size, float x, float y, Color color)
{
	if (drawStringCount < DRAW_STRING_MAX)
	{
		int abgr = (color.A << 24) | (color.B << 16) | (color.G << 8) | (color.R);
		pFontWrapper->DrawString(pContext, text, size, x, y, abgr, FW1_RESTORESTATE);
		drawStringCount++;
	}
}

void DrawStringFmt(float size, float x, float y, Color color, char *fmt, ...)
{
	char text[4096] = {};
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);
	DrawString(text, size, x, y, color);
}

typedef struct _PUBG_ADDR
{
	DWORD_PTR			baseAddr;
	UWorld*				uWorld;
	UGameInstance*		uGameInstance;
	ULocalPlayer*		uLocalPlayer;
	UGameViewportClient* uViewportClient;
	UWorld*				pWorld;
	ULevel*				uLevel;
}PUBG_ADDR, *PPUBG_ADDR;

typedef struct _DEBUG_PRINT
{
	char str[64];
	int timeCreated;
	int timeToDelete;
}DEBUG_PRINT, *PDEBUG_PRINT;

PUBG_ADDR PUBG;
std::vector<DEBUG_PRINT> g_wcDebugPrint;

void DebugPrint(char *fmt, ...)
{
	char text[4096] = {};
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);
	DEBUG_PRINT debug;
	memcpy(debug.str, text, 64);
	debug.timeCreated = clock();
	debug.timeToDelete = clock() + 4000;
	g_wcDebugPrint.push_back(debug);
}

bool isPUBGThreadRunning = false;
DWORD __stdcall PUBGThread(LPVOID)
{
	isPUBGThreadRunning = true;
	DWORD_PTR base = dwGetBaseAddress();
	while (isPUBGThreadRunning)
	{
		
	}
	return 0;
}

#pragma endregion

#pragma region D3D11_HOOK

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall *D3D11CreateQueryHook) (ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery);
#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
D3D11PresentHook phookD3D11Present = NULL;
D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
D3D11CreateQueryHook phookD3D11CreateQuery = NULL;
ID3D11Device *pDevice = NULL;
IDXGISwapChain *g_pSwapChain;
DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pDeviceVTable = NULL;
ID3D11RasterizerState * rwState;
ID3D11RasterizerState * rsState;
ID3D11Texture2D* RenderTargetTexture;
ID3D11RenderTargetView* RenderTargetView = NULL;
ID3D11PixelShader* psRed = NULL;
ID3D11PixelShader* psOrange = NULL;
ID3D11PixelShader* psBlue = NULL;
IFW1Factory *pFW1Factory = NULL;
ID3D11Buffer *veBuffer;
UINT Stride = 0;
UINT veBufferOffset = 0;
D3D11_BUFFER_DESC vedesc;
ID3D11Buffer *inBuffer;
DXGI_FORMAT inFormat;
UINT        inOffset;
D3D11_BUFFER_DESC indesc;

D3D11Renderer *renderer;

enum eDepthState
{
	ENABLED,
	DISABLED,
	READ_NO_WRITE,
	NO_READ_NO_WRITE,
	_DEPTH_COUNT
};

ID3D11DepthStencilState* myDepthStencilStates[static_cast<int>(eDepthState::_DEPTH_COUNT)];

void SetDepthStencilState(eDepthState aState)
{
	pContext->OMSetDepthStencilState(myDepthStencilStates[aState], 1);
}

HRESULT GenerateShader(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, float r, float g, float b)
{
	char szCast[] = "struct VS_OUT"
		"{"
		" float4 Position : SV_Position;"
		" float4 Color : COLOR0;"
		"};"

		"float4 main( VS_OUT input ) : SV_Target"
		"{"
		" float4 fake;"
		" fake.a = 0.3f;"
		" fake.r = %f;"
		" fake.g = %f;"
		" fake.b = %f;"
		" return fake;"
		"}";
	ID3D10Blob* pBlob;
	char szPixelShader[1000];
	sprintf_s(szPixelShader, szCast, r, g, b);
	ID3DBlob* d3dErrorMsgBlob;
	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, &pBlob, &d3dErrorMsgBlob);
	if (FAILED(hr))
		return hr;
	hr = pD3DDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pShader);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

namespace MenuGUI
{
	const float BUTTON_HEIGHT = 20.0f;

	Color accentColor = Color(255, 92, 145, 188);
	Color background = Color(230, 30, 30, 30);
	Color lightBG = Color(255, 80, 80, 80);

	int g_sliderIndex, g_comboBoxIndex;
	float menuX = 50.0f, menuY = 50.0f, menuW = 0, menuH = 0;
	int clickTimer;
	bool isMovingMenu;
	int moveOffset[2];
	bool isUsingElement;
	bool isAComboBoxInUse;

	void GUISetupMenu(float w, float h)
	{
		g_sliderIndex = 0;
		g_comboBoxIndex = 0;
		menuW = w;
		menuH = h;
	}

	bool isInBounds(POINT point, float x, float y, float w, float h)
	{
		if (point.x > x && point.x < x + w && point.y > y && point.y < y + h)
			return true;
		return false;
	}

	bool leftMouseDown()
	{
		return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	}

	void GUIBackground()
	{
		POINT point;
		GetCursorPos(&point);
		if (isInBounds(point, menuX, menuY - 20.0f, menuW, 20.0f))
		{
			if (!isMovingMenu)
			{
				moveOffset[0] = point.x - menuX;
				moveOffset[1] = point.y - (menuY - 20.0f);
				isMovingMenu = true;
			}
		}
		if (isMovingMenu)
		{
			if (leftMouseDown())
			{
				menuX = point.x - moveOffset[0];
				menuY = point.y + 20 - moveOffset[1];
			}
			else
			{
				isMovingMenu = false;
			}
		}
		renderer->FillRect(menuX, menuY - 20.0f, menuW, 20.0f, accentColor);
		renderer->FillRect(menuX, menuY - 20.0f, 20.0f, 20.0f, Color(255, 255, 0, 0));
		renderer->FillRect(menuX, menuY, menuW, menuH, background);
	}

	void GUIText(float x, float y, float size, char *text)
	{
		float realX = menuX + x;
		float realY = menuY + y;
		DrawString(text, size, realX, realY, Color(255, 255, 255, 255));
	}

	bool GUIButton(float x, float y, char *text)
	{
		float realX = menuX + x;
		float realY = menuY + y;
		float width = (strlen(text) * 7.45f) + 5.0f;
		POINT point;
		GetCursorPos(&point);
		if (!isAComboBoxInUse)
		{
			if (isInBounds(point, realX, realY, width, BUTTON_HEIGHT))
			{
				Color downColor = accentColor;
				downColor.A = 90;
				renderer->FillRect(realX, realY, width, BUTTON_HEIGHT, downColor);
				DrawString(text, 13, realX + 2.5f, realY + 1, Color(255, 255, 255, 255));
				if (leftMouseDown())
				{
					if (clock() > clickTimer)
					{
						clickTimer = clock() + 200;
						return true;
					}
				}
				return false;
			}
		}
		renderer->FillRect(realX, realY, width, BUTTON_HEIGHT, accentColor);
		DrawString(text, 13, realX + 2.5f, realY + 1, Color(255, 255, 255, 255));
		return false;
	}

	bool GUICheckBox(float x, float y, char *text, bool toggle)
	{
		float realX = menuX + x;
		float realY = menuY + y;
		POINT point;
		GetCursorPos(&point);

		renderer->FillRect(realX, realY, 17, 17, lightBG);
		DrawString(text, 13, realX + 22, realY - 1, Color(255, 255, 255, 255));
		if (toggle)
		{
			Color togOn = accentColor;
			togOn.A = 255;
			renderer->FillRect(realX + 3, realY + 3, 11, 11, togOn);
		}
		if (!isAComboBoxInUse)
		{
			float width = (strlen(text) * 7.45f) + 17;
			if (isInBounds(point, realX, realY, width, 17))
			{
				if (leftMouseDown())
				{
					if (clock() > clickTimer)
					{
						clickTimer = clock() + 200;
						return true;
					}
				}
			}
		}
		return false;
	}

	const float SLIDER_HEIGHT = 20.0f;
	bool movingSlider[10];
	int sliderValue[10];
	int sliderRealValue[10];
	float sliderX[10];
	int GUISlider(float x, float y, float w, int max)
	{
		float realX = menuX + x;
		float realY = menuY + y;
		sliderX[g_sliderIndex] = realX + sliderValue[g_sliderIndex];

		//background
		renderer->FillRect(realX, realY, w, SLIDER_HEIGHT, lightBG);
		//slider
		renderer->FillRect(realX, realY, sliderX[g_sliderIndex] - realX, SLIDER_HEIGHT, Color(255, accentColor.R, accentColor.G, accentColor.B));
		//border
		renderer->DrawBorderBox(realX, realY, w, SLIDER_HEIGHT, Color(255, 255, 255, 255));

		//real value
		char strValue[26] = {};
		sprintf_s(strValue, "%i", sliderRealValue[g_sliderIndex]);
		float strValueWidth = (strlen(strValue) * 7.45f);
		DrawString(strValue, 13, realX + (w / 2) - (strValueWidth / 2) + (7.45f / 2), realY + 1, Color(255, 255, 255, 255));

		if (!isAComboBoxInUse)
		{
			POINT point;
			GetCursorPos(&point);
			if (isInBounds(point, realX, realY, w, SLIDER_HEIGHT))
			{
				if (!movingSlider[g_sliderIndex])
				{
					if (leftMouseDown())
					{
						movingSlider[g_sliderIndex] = true;
					}
				}
			}
			if (movingSlider[g_sliderIndex])
			{
				if (leftMouseDown())
				{
					sliderValue[g_sliderIndex] = point.x - realX;
					if (sliderValue[g_sliderIndex] <= 0)
						sliderValue[g_sliderIndex] = 0;
					if (sliderValue[g_sliderIndex] >= w)
						sliderValue[g_sliderIndex] = w;

					sliderX[g_sliderIndex] = realX + sliderValue[g_sliderIndex];
					float testValue = ((float)sliderValue[g_sliderIndex] * ((float)max / (float)w));
					sliderRealValue[g_sliderIndex] = (int)ceil(testValue);
				}
				else
				{
					movingSlider[g_sliderIndex] = false;
				}
			}
			g_sliderIndex++;
		}
		return sliderRealValue[g_sliderIndex - 1];
	}

	void GUIGroupBox(float x, float y, float w, float h, char *text)
	{
		float realX = menuX + x;
		float realY = menuY + y;
		float barWidth = 1.0f;
		float textWidth = (strlen(text) * 7.44f);
		//left bar
		renderer->FillRect(realX, realY, barWidth, h, Color(255, 255, 255, 255));
		//bottom bar
		renderer->FillRect(realX, realY + h, w, barWidth, Color(255, 255, 255 ,255));
		//right bar
		renderer->FillRect(realX + w, realY, barWidth, h, Color(255, 255, 255, 255));
		//top left bar
		renderer->FillRect(realX, realY, 13.0f, barWidth, Color(255, 255, 255, 255));
		//top right bar
		float gapWidth = 33.0f + textWidth;
		renderer->FillRect(realX + (gapWidth - 8), realY, w - (gapWidth - 8), barWidth, Color(255, 255, 255, 255));
		DrawString(text, 13, realX + 20.0f, realY - 8, Color(255, 255, 255, 255));
	}

	const float COMBOBOX_HEIGHT = 20.0f;
	bool isComboBoxOpen[10];
	int comboBoxSelectedItemIndex[10];
	int activeComboBoxIndex;

	int GUIComboBox(float x, float y, float w, char **items, int itemCount)
	{
		float realX = menuX + x;
		float realY = menuY + y;

		//background
		renderer->FillRect(realX, realY, w, COMBOBOX_HEIGHT, lightBG);
		if (!isComboBoxOpen[g_comboBoxIndex])
		{
			renderer->DrawBorderBox(realX, realY, w, COMBOBOX_HEIGHT, Color(255, 255, 255, 255));
			//item text
			DrawString(items[comboBoxSelectedItemIndex[g_comboBoxIndex]], 13, realX + 4, realY + 1, Color(255, 255, 255, 255));
		}

		POINT point;
		GetCursorPos(&point);
		if (!isAComboBoxInUse)
		{
			if (isInBounds(point, realX, realY, w, COMBOBOX_HEIGHT))
			{
				if (!isComboBoxOpen[g_comboBoxIndex])
				{
					if (leftMouseDown())
					{
						isAComboBoxInUse = true;
						isComboBoxOpen[g_comboBoxIndex] = true;
					}
				}
			}
		}
		if (isComboBoxOpen[g_comboBoxIndex])
		{
			char *currentItem = items[comboBoxSelectedItemIndex[g_comboBoxIndex]];
			for (int i = 0; i < itemCount; i++)
			{
				//if (strcmp(items[i], currentItem))
					renderer->FillRect(realX, realY + COMBOBOX_HEIGHT + (i * COMBOBOX_HEIGHT), w, COMBOBOX_HEIGHT, lightBG);
					//renderer->DrawBorderBox(realX, realY + COMBOBOX_HEIGHT + (i * COMBOBOX_HEIGHT), w, COMBOBOX_HEIGHT, Color(255, 255, 255, 255));
					DrawString(items[i], 13, realX + 4, realY + COMBOBOX_HEIGHT + (i * COMBOBOX_HEIGHT) + 1, Color(255, 255, 255, 255));
			}
			if (!isInBounds(point, realX, realY, w, COMBOBOX_HEIGHT + (COMBOBOX_HEIGHT * itemCount)))
			{
				if (leftMouseDown())
				{
					isAComboBoxInUse = false;
					isComboBoxOpen[g_comboBoxIndex] = false;
				}
			}
			else
			{
				//find what index you selected
				float itemBoxY = realY + COMBOBOX_HEIGHT;

				if (isInBounds(point, realX, itemBoxY, w, COMBOBOX_HEIGHT * itemCount))
				{
					if (leftMouseDown())
					{
						for (int i = 0; i < itemCount; i++)
						{
							float min = i * 20.0f;
							float max = (i + 1) * 20.0f;
							if (point.y > itemBoxY + min && point.y < itemBoxY + max)
							{
								comboBoxSelectedItemIndex[g_comboBoxIndex] = i;
								break;
							}
						}
						isAComboBoxInUse = false;
						isComboBoxOpen[g_comboBoxIndex] = false;
						g_comboBoxIndex++;
						return comboBoxSelectedItemIndex[g_comboBoxIndex - 1];
					}
				}
			}
		}
		g_comboBoxIndex++;
		return -1;
	}

	void GUIEndMenu()
	{

	}
}

void DrawPUBGModMenu()
{
	MenuGUI::GUISetupMenu(800.0f, 400.0f);
	MenuGUI::GUIBackground();

	//ESP OPTIONS
	MenuGUI::GUIGroupBox(20.0f, 20.0f, 300.0f, 350.0f, "ESP Settings");
	//PLAYER ESP
	if (MenuGUI::GUICheckBox(35.0f, 40.0f, "Player ESP", globals::bPlayerESP))
	{
		globals::bPlayerESP = !globals::bPlayerESP;
		DebugPrint("bPlayerESP: %s", globals::bPlayerESP ? "ON" : "OFF");
	}
	if (MenuGUI::GUICheckBox(35.0f, 65.0f, "Draw Names", globals::bDrawNames))
	{
		globals::bDrawNames = !globals::bDrawNames;
		DebugPrint("bDrawNames: %s", globals::bDrawNames ? "ON" : "OFF");
	}
	if (MenuGUI::GUICheckBox(35.0f, 90.0f, "Draw Distance", globals::bDrawDistance))
	{
		globals::bDrawDistance = !globals::bDrawDistance;
		DebugPrint("bDrawDistance: %s", globals::bDrawDistance ? "ON" : "OFF");
	}
	if (MenuGUI::GUICheckBox(35.0f, 115.0f, "Draw Health", globals::bDrawHealth))
	{
		globals::bDrawHealth = !globals::bDrawHealth;
		DebugPrint("bDrawHealth: %s", globals::bDrawHealth ? "ON" : "OFF");
	}
	if (MenuGUI::GUICheckBox(35.0f, 140.0f, "Snap Lines", globals::bSnapLines))
	{
		globals::bSnapLines = !globals::bSnapLines;
		DebugPrint("bSnapLines: %s", globals::bSnapLines ? "ON" : "OFF");
	}
	//ITEM ESP
	if (MenuGUI::GUICheckBox(170.0f, 40.0f, "Item ESP", globals::bItemESP))
	{
		globals::bItemESP = !globals::bItemESP;
		DebugPrint("bItemESP: %s", globals::bItemESP ? "ON" : "OFF");
	}
	if (MenuGUI::GUICheckBox(170.0f, 65.0f, "Debug ESP", globals::bDebugESP))
	{
		globals::bDebugESP = !globals::bDebugESP;
		DebugPrint("bDebugESP: %s", globals::bDebugESP ? "ON" : "OFF");
	}


	MenuGUI::GUIText(35.0f, 210.0f, 13, "Box Style");
	int tmpBoxStyle;
	if ((tmpBoxStyle = MenuGUI::GUIComboBox(35.0f, 230.0f, 150.0f, globals::espBoxTypeStr, 3)) != -1)
	{
		globals::g_espBoxStyle = tmpBoxStyle;
		DebugPrint("espBoxStyle: %i", globals::g_espBoxStyle);
	}
	if (MenuGUI::GUIButton(35.0f, 260.0f, "Test Button"))
	{
		/*AActor *localActor = ->LocalPlayers[0]->PlayerController->Actor;
		if (localActor && localActor->RootComponent)
		{
			std::vector<AActor*> tmpPlayers = g_vPlayerList;
			int size = tmpPlayers.size();
			for (int i = 0; i < size; i++)
			{
				AActor *actor = tmpPlayers[i];
				if (!actor || !actor->PlayerState || !actor->RootComponent || actor == localActor)
					continue;

				actor->K2_TeleportTo(localActor->RootComponent->RelativeLocation, Vector3(0, 0, 0));
			}
		}*/
		APlayerController *controller = PUBG.uLocalPlayer->PlayerController;
		AActor *localActor = controller->Actor;

		localActor->DisableInput(NULL);
	}

	MenuGUI::GUIText(35.0f, 165.0f, 13, "Draw Distance");
	globals::g_espDrawDistance = (float)MenuGUI::GUISlider(35.0f, 185.0f, 200.0f, 1000);
}

bool hasCachedGNames = false;
void CacheGNames()
{
	int actorCount = 0;
	for (int i = 0; i < 100000; i++)
	{
		char gname[256] = {};
		if (GetGName(i, gname))
		{
			if (!strcmp(gname, "PlayerMale_A") || !strcmp(gname, "PlayerMale_A_C") || !strcmp(gname, "PlayerFemale_A") || !strcmp(gname, "PlayerFemale_A_C"))
			{
				DebugPrint("Cached %s", gname);
				globals::ActorIDs[actorCount] = i;
				actorCount++;
			}
			if (!strcmp(gname, "Erangel_Main"))
			{
				DebugPrint("Cached %s", gname);
				globals::ErangelMainID = i;
			}
			if (!strcmp(gname, "DroppedItemInteractionComponent"))
			{
				DebugPrint("Cached %s", gname);
				globals::DroppedItemInteractionComponentID = i;
			}
			if (!strcmp(gname, "DroppedItemGroup"))
			{
				DebugPrint("Cached %s", gname);
				globals::DroppedItemGroupID = i;
			}
			if (!strcmp(gname, "DroppedItem"))
			{
				DebugPrint("Cached %s", gname);
				globals::DroppedItemID = i;
			}
		}
	}
}

bool IsInGame()
{
	DWORD id = *(DWORD*)((DWORD_PTR)PUBG.uWorld + 0x18);
	return id == globals::ErangelMainID;
}

bool present_HadRun = false;
HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!present_HadRun)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}

		renderer = new D3D11Renderer(pSwapChain);
		renderer->Initialize();

		D3D11_DEPTH_STENCIL_DESC  stencilDesc;
		stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		stencilDesc.StencilEnable = true;
		stencilDesc.StencilReadMask = 0xFF;
		stencilDesc.StencilWriteMask = 0xFF;
		stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		stencilDesc.DepthEnable = true;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::ENABLED)]);

		stencilDesc.DepthEnable = false;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::DISABLED)]);

		stencilDesc.DepthEnable = false;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		stencilDesc.StencilEnable = false;
		stencilDesc.StencilReadMask = UINT8(0xFF);
		stencilDesc.StencilWriteMask = 0x0;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::NO_READ_NO_WRITE)]);

		stencilDesc.DepthEnable = true;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //
		stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		stencilDesc.StencilEnable = false;
		stencilDesc.StencilReadMask = UINT8(0xFF);
		stencilDesc.StencilWriteMask = 0x0;

		stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::READ_NO_WRITE)]);


		D3D11_RASTERIZER_DESC rwDesc;
		pContext->RSGetState(&rwState); // retrieve the current state
		rwState->GetDesc(&rwDesc);    // get the desc of the state
		rwDesc.FillMode = D3D11_FILL_WIREFRAME;
		rwDesc.CullMode = D3D11_CULL_NONE;
		pDevice->CreateRasterizerState(&rwDesc, &rwState);

		D3D11_RASTERIZER_DESC rsDesc;
		pContext->RSGetState(&rsState); // retrieve the current state
		rsState->GetDesc(&rsDesc);    // get the desc of the state
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_BACK;
		pDevice->CreateRasterizerState(&rsDesc, &rsState);

		HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		hResult = pFW1Factory->CreateFontWrapper(pDevice, L"Consolas", &pFontWrapper);
		pFW1Factory->Release();

		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&RenderTargetTexture)))
		{
			pDevice->CreateRenderTargetView(RenderTargetTexture, NULL, &RenderTargetView);
			RenderTargetTexture->Release();
		}
		present_HadRun = true;
	}

	if (!psRed)
		GenerateShader(pDevice, &psRed, 1.0f, 0.0f, 0.0f);

	if (!psBlue)
		GenerateShader(pDevice, &psBlue, 0.0f, 0.38f, 1.0f);

	if (!psOrange)
		GenerateShader(pDevice, &psOrange, 1.0f, 0.48f, 0.0f);

	pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

	if (pFontWrapper)
	{
		drawStringCount = 0;
		for (int i = g_wcDebugPrint.size() - 1; i >= 0; i--)
		{
			DEBUG_PRINT debug = g_wcDebugPrint[i];
			if (debug.timeToDelete > clock())
			{
				DrawString(debug.str, 20, 1500, 50 + (i * 25.0f), Color(255, 255, 255, 255));
			}
			else
			{
				g_wcDebugPrint.erase(g_wcDebugPrint.begin() + i);
			}
		}

		DWORD_PTR base = dwGetBaseAddress();
		UWorld *uWorld = (UWorld*)(*(DWORD_PTR*)(base + 0x37D57A8));
		UGameInstance *uGameInstance = uWorld->pOwningGameInstance;
		ULocalPlayer *uLocalPlayer = uGameInstance->LocalPlayers[0];
		UGameViewportClient *uViewportClient = uLocalPlayer->ViewportClient;
		UWorld *pWorld = uViewportClient->World;
		ULevel *uLevel = pWorld->pLevel;
		PUBG.baseAddr = base;
		PUBG.uWorld = uWorld;
		PUBG.uGameInstance = uGameInstance;
		PUBG.uLocalPlayer = uLocalPlayer;
		PUBG.uViewportClient = uViewportClient;
		PUBG.pWorld = pWorld;
		PUBG.uLevel = uLevel;

		renderer->BeginScene();

		//DrawPUBGModMenu();

		if (hasCachedGNames)
		{
			if (globals::bPlayerESP || globals::bItemESP || globals::bDebugESP)
			{
				APlayerCameraManager *camManager = PUBG.uLocalPlayer->PlayerController->PlayerCameraManager;
				AActor *localActor = PUBG.uLocalPlayer->PlayerController->Actor;
				if (localActor)
				{
					Vector3 localActorPos = localActor->RootComponent->RelativeLocation;
					int entityCount = PUBG.uLevel->entityList.Count();
					for (int i = 0; i < entityCount; i++)
					{
						AActor *actor = PUBG.uLevel->entityList[i];
						if (!actor)
							continue;
						if (actor == localActor)
							continue;
						if (!actor->RootComponent)
							continue;

						if (globals::bDebugESP)
						{
							Vector3 entityScr;
							Vector3 entityLoc = actor->RootComponent->RelativeLocation;
							Vector3 vDelta = localActorPos - entityLoc;
							float fDelta = vDelta.Length() / 100;
							if (fDelta < 500.0f)
							{
								WorldToScreen(entityLoc, camManager, entityScr);

								char itemgname[256] = {};
								if (GetGName(actor->id, itemgname))
								{
									DrawStringFmt(12.0f, entityScr.x, entityScr.y, Color(255, 255, 255, 255), "%s", itemgname);
								}
							}
						}

						if (globals::bPlayerESP)
						{
							if (actor->IsPlayer())
							{
								Vector3 scrFeet, scrHead;
								Vector3 enemyLocation = actor->RootComponent->RelativeLocation;
								Vector3 enemyHead = enemyLocation;
								enemyHead.z += 90.0f;
								Vector3 enemyFeet = enemyLocation;
								enemyFeet.z -= 90.0f;

								Vector3 vDelta = localActorPos - enemyLocation;
								float fDelta = vDelta.Length() / 100;
								if (fDelta < globals::g_espDrawDistance)
								{
									WorldToScreen(enemyHead, camManager, scrHead);
									WorldToScreen(enemyFeet, camManager, scrFeet);

									float height = scrFeet.y - scrHead.y;
									float width = height / 2.0f;

									bool isVisible = PUBG.uGameInstance->LocalPlayers[0]->PlayerController->IsVisible(actor);
									Color espColor;
									if (isVisible)
										espColor = Color(255, 255, 255, 0);
									else
										espColor = Color(255, 255, 0, 0);

									if (globals::bDrawDistance)
									{
										char disStr[100] = {};
										sprintf_s(disStr, "%0.1f M", fDelta);
										DrawString(disStr, 13, scrHead.x - (width / 2), scrHead.y + (height * 1.4f), Color(255, 255, 255, 255));
									}
									if (globals::bDrawNames)
									{
										//wchar_t *playerName = actor->PlayerState->PlayerName.GetWideString();
										//pFontWrapper->DrawString(pContext, playerName, 13, scrHead.x - (width / 2), scrHead.y + height, 0xFFFFFFFF, FW1_RESTORESTATE);
									}
									if (globals::bSnapLines)
									{
										renderer->DrawLine(1920 / 2, 1080, scrFeet.x, scrFeet.y, espColor);
									}
									if (globals::bDrawHealth)
									{
										float health = actor->health;
										Color healthColor = Color(255, 255, 255, 0);
										healthColor.R = 255.0f - (health * 2.55f);
										healthColor.G = health * 2.55f;
										float testValue = health * (height / 100.0f);
										float shit = height - testValue;
										renderer->FillRect(scrHead.x + (width / 1.5f), scrHead.y + shit, 3.0f, testValue, healthColor);
									}
									if (globals::g_espBoxStyle == 0)
									{
										renderer->DrawBorderBox(scrHead.x - (width / 2), scrHead.y, width, height, espColor);
									}
									else if (globals::g_espBoxStyle == 1)
									{

									}
									else if (globals::g_espBoxStyle == 2)
									{
										espColor.A = 90;
										renderer->FillRect(scrHead.x - (width / 2), scrHead.y, width, height, espColor);
										renderer->DrawBorderBox(scrHead.x - (width / 2), scrHead.y, width, height, Color(255, 0, 0, 0));
									}
								}
							}
						}
						if (globals::bItemESP)
						{
							if (actor->IsDroppedItemGroup())
							{
								Vector3 screenLoc;
								Vector3 lootRelLoc = actor->RootComponent->RelativeLocation;

								Vector3 vDelta = localActorPos - lootRelLoc;
								float fDelta = vDelta.Length() / 100;
								if (fDelta < 150.0f)
								{
									if (actor->DroppedItemArray.Count() > 0)
									{
										for (int index = 0; index < actor->DroppedItemArray.Count(); index++)
										{
											AActor *lootItem = (AActor*)actor->DroppedItemArray[index].pActor;
											if (!lootItem)
												continue;

											if (lootItem->IsDroppedItemInteractionComponent())
											{
												Vector3 tmpLoc = *(Vector3*)((DWORD_PTR)lootItem + 0x1E0);
												Vector3 itemLocation = lootRelLoc + tmpLoc;

												WorldToScreen(itemLocation, camManager, screenLoc);

												DWORD_PTR uItem = *(DWORD_PTR*)((DWORD_PTR)lootItem + 0x448);
												if (!uItem)
													continue;
												DWORD_PTR FTextItem = *(DWORD_PTR*)(uItem + 0x40);
												if (!FTextItem)
													continue;
												FString itemName;
												memcpy(&itemName, (LPVOID)(FTextItem + 0x28), sizeof(FString));

												DrawString(itemName.GetWideString(), 12.0f, screenLoc.x, screenLoc.y, Color(255, 255, 255, 255));
											}
										}
									}
								}
							}
							else if (actor->IsDroppedItem())
							{
								Vector3 screenLoc;
								Vector3 lootLoc = actor->RootComponent->RelativeLocation;
								Vector3 vDelta = localActorPos - lootLoc;
								float fDelta = vDelta.Length() / 100;
								if (fDelta < 150.0f)
								{
									WorldToScreen(lootLoc, camManager, screenLoc);
									
									DWORD_PTR uItem = *(DWORD_PTR*)((DWORD_PTR)actor + 0x03A0);
									if (!uItem)
										continue;
									DWORD_PTR FTextItem = *(DWORD_PTR*)(uItem + 0x40);
									if (!FTextItem)
										continue;
									FString itemName;
									memcpy(&itemName, (LPVOID)(FTextItem + 0x28), sizeof(FString));

									DrawString(itemName.GetWideString(), 12.0f, screenLoc.x, screenLoc.y, Color(255, 255, 255, 255));
								}
							}
						}
					}
				}
			}
		}
		renderer->EndScene();
	}

	/*

			AActor *apActor = (AActor*)(pActor);
			if (apActor->IsDroppedItemInteractionComponent())
			{
				Vector3 tmpLoc = *(Vector3*)(pActor + 0x1E0);
				Vector3 itemLoction = lootRelativeLocation + tmpLoc;
				WorldToScreen(itemLoction, cameraManager, screenLoc);

				DWORD_PTR uItem = *(DWORD_PTR*)(pActor + 0x448);
				if (!uItem)
					continue;
				DWORD_PTR FTextItem = *(DWORD_PTR*)(uItem + 0x40);
				if (!FTextItem)
					continue;
				FString itemName;
				memcpy(&itemName, (LPVOID)(FTextItem + 0x28), sizeof(FString));

				if (DoesStrArrayContain(itemEspWeapons, itemName.GetString()) || DoesStrArrayContain(itemEspEquipment, itemName.GetString()))
				{
					pFontWrapper->DrawString(pContext, itemName.GetWideString(), 15, screenLoc.x, screenLoc.y, 0xFF00FF00, FW1_RESTORESTATE);
				}
			}
		}
	}*/

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

int pStride = 36;
void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* tpContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	pContext->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
	if (veBuffer)
		veBuffer->GetDesc(&vedesc);
	if (veBuffer != NULL) { veBuffer->Release(); veBuffer = NULL; }

	pContext->IAGetIndexBuffer(&inBuffer, &inFormat, &inOffset);
	if (inBuffer)
		inBuffer->GetDesc(&indesc);
	if (inBuffer != NULL) { inBuffer->Release(); inBuffer = NULL; }

	if (Stride == pStride && vedesc.ByteWidth >= 170000 /*Broken Glass*/ && vedesc.ByteWidth != (122766 * pStride) && vedesc.ByteWidth != (21363 * pStride) && vedesc.ByteWidth != (6343 * pStride) /*Buggy*/ \
		&& vedesc.ByteWidth != (30049 * pStride) && vedesc.ByteWidth != (17292 * pStride) && vedesc.ByteWidth != (6481 * pStride) /*Dacia*/ \
		&& vedesc.ByteWidth != (48197 * pStride) && vedesc.ByteWidth != (31616 * pStride) && vedesc.ByteWidth != (10102 * pStride) /*UAZ B*/ \
		&& vedesc.ByteWidth != (46533 * pStride) && vedesc.ByteWidth != (28729 * pStride) && vedesc.ByteWidth != (9815 * pStride) /*UAZ*/ \
		&& vedesc.ByteWidth != (46717 * pStride) && vedesc.ByteWidth != (26879 * pStride) && vedesc.ByteWidth != (9906 * pStride) /*UAZ C*/ \
		&& vedesc.ByteWidth != (42064 * pStride) && vedesc.ByteWidth != (25256 * pStride) && vedesc.ByteWidth != (15867 * pStride)  /*Motorbike Sidecart*/ \
		&& vedesc.ByteWidth != (32281 * pStride) && vedesc.ByteWidth != (19271 * pStride) && vedesc.ByteWidth != (8801 * pStride) /*Motorbike*/ \
		&& vedesc.ByteWidth != (33118 * pStride) && vedesc.ByteWidth != (18738 * pStride) && vedesc.ByteWidth != (5630 * pStride) /*Boat PG117*/ \
		&& vedesc.ByteWidth != (72170 * pStride) /*Destroyed Door A*/ \
		&& vedesc.ByteWidth != (44708 * pStride) /*Destroyed Door F*/ \
		&& vedesc.ByteWidth != (39156 * pStride) /*Destroyed Door A2*/ \
		&& vedesc.ByteWidth != (28286 * pStride) /*Destroyed Wood E11*/ \
		&& vedesc.ByteWidth != (24923 * pStride) /*Destroyed Wood E12*/ \
		&& vedesc.ByteWidth != (23026 * pStride) /*Destroyed Fence Short A*/ \
		&& vedesc.ByteWidth != (19196 * pStride) /*Destroyed Fence Short B*/ \
		&& vedesc.ByteWidth != (15332 * pStride) /*Destroyed Fence Short C*/ \
		&& vedesc.ByteWidth != (23035 * pStride) /*Destroyed Fence A*/ \
		&& vedesc.ByteWidth != (25962 * pStride) /*Destroyed Fence B*/ \
		&& vedesc.ByteWidth != (19931 * pStride) /*Destroyed Fence C*/ \
		&& vedesc.ByteWidth != (19222 * pStride) /*Destroyed Fence D*/ \
		&& vedesc.ByteWidth != (8658 * pStride)  /*M16*/ \
		&& vedesc.ByteWidth != (8351 * pStride)  /*AK*/ \
		&& vedesc.ByteWidth != (9974 * pStride)  /*SCAR-L*/ \
		&& vedesc.ByteWidth != (12166 * pStride)  /*HK416*/ \
		&& vedesc.ByteWidth != (6518 * pStride)  /*UZI*/ \
		&& vedesc.ByteWidth != (12021 * pStride) /*SKS*/ \
		&& vedesc.ByteWidth != (5470 * pStride) /*S686*/ \
		&& vedesc.ByteWidth != (11292 * pStride) /*K89K*/)
	{
		SetDepthStencilState(DISABLED);
		pContext->PSSetShader(psRed, NULL, NULL);
		phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
		SetDepthStencilState(READ_NO_WRITE);
		pContext->PSSetShader(psOrange, NULL, NULL);
	}

	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hookD3D11CreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery)
{
	if (pQueryDesc->Query == D3D11_QUERY_OCCLUSION)
	{
		D3D11_QUERY_DESC oqueryDesc = CD3D11_QUERY_DESC();
		(&oqueryDesc)->MiscFlags = pQueryDesc->MiscFlags;
		(&oqueryDesc)->Query = D3D11_QUERY_TIMESTAMP;

		return phookD3D11CreateQuery(pDevice, &oqueryDesc, ppQuery);
	}
	return phookD3D11CreateQuery(pDevice, pQueryDesc, ppQuery);
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
DWORD __stdcall D3D11_Hook(LPVOID)
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandleA("dxgi.dll");
		Sleep(10);
	} while (!hDXGIDLL);
	Sleep(100);
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);
	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;

	IDXGISwapChain* d3dSwapChain = 0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&g_pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		MessageBoxA(hWnd, "Failed to create directX device and swapchain!", "Error", MB_ICONERROR);
		return NULL;
	}

	pSwapChainVtable = (DWORD_PTR*)g_pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];
	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	if (MH_Initialize() != MH_OK)
		return 0;

	//MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hookD3D11Present, reinterpret_cast<void**>(&phookD3D11Present));
	//MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]);
	MH_CreateHook((DWORD_PTR*)pContextVTable[12], hookD3D11DrawIndexed, reinterpret_cast<void**>(&phookD3D11DrawIndexed));
	MH_EnableHook((DWORD_PTR*)pContextVTable[12]);
	MH_CreateHook((DWORD_PTR*)pDeviceVTable[24], hookD3D11CreateQuery, reinterpret_cast<void**>(&phookD3D11CreateQuery));
	MH_EnableHook((DWORD_PTR*)pDeviceVTable[24]);

	//CacheGNames();
	//hasCachedGNames = true;

	pDevice->Release();
	pContext->Release();
	g_pSwapChain->Release();

	return 0;
}

#pragma endregion

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)D3D11_Hook, 0, 0, 0);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		isPUBGThreadRunning = false;
		MH_Uninitialize();
		MH_DisableHook((DWORD_PTR*)pSwapChainVtable[8]);
		/*MH_DisableHook((DWORD_PTR*)pContextVTable[12]);
		MH_DisableHook((DWORD_PTR*)pDeviceVTable[24]);*/
		break;
	}
	return TRUE;
}

