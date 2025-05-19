#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
// #include "bserv.h"
#include "refksrv.h"
#include "crc32.h"
#include "kload_exp.h"
#include "soft\zlib123-dll\include\zlib.h"
#include "numpages.h"
#include "input.h"
#include "keycfg.h"

#include <pngdib.h>
#include <map>
#include <string>

KMOD k_refksrv={MODID,NAMELONG,NAMESHORT,DEFAULT_DEBUG};

BSERV_CFG bserv_cfg;


///// Graphics //////////////////

struct CUSTOMVERTEX { 
	FLOAT x,y,z,w;
	DWORD color;
};

struct CUSTOMVERTEX2 { 
	FLOAT x,y,z,w;
	DWORD color;
	FLOAT tu, tv;
};

//Preview cortar kit
// CUSTOMVERTEX2 g_preview[] = {
// 	{0.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 0.0f}, //1
// 	{0.0f, 256.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 1.0f}, //2
// 	{136.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 0.365f, 0.0f}, //3
// 	{136.0f, 256.0f, 0.0f, 1.0f, 0xff4488ff, 0.365f, 1.0f}, //4
// };


// CUSTOMVERTEX2 g_preview_kit[] = {
// 	{0.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 0.0f}, //1
// 	{0.0f, 200.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 1.0f}, //2
// 	{400.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 1.0f, 0.0f}, //3
// 	{400.0f, 200.0f, 0.0f, 1.0f, 0xff4488ff, 1.0f, 1.0f}, //4
// };

// CUSTOMVERTEX g_preview_outline_kit[] = {
// 	{0.0f, 0.0f, 0.0f, 1.0f, 0xffffffff}, //1
// 	{0.0f, 202.0f, 0.0f, 1.0f, 0xffffffff}, //2
// 	{402.0f, 0.0f, 0.0f, 1.0f, 0xffffffff}, //3
// 	{402.0f, 202.0f, 0.0f, 1.0f, 0xffffffff}, //4
// };

// CUSTOMVERTEX g_preview_outline2_kit[] = {
// 	{0.0f, 0.0f, 0.0f, 1.0f, 0xff000000}, //1
// 	{0.0f, 204.0f, 0.0f, 1.0f, 0xff000000}, //2
// 	{404.0f, 0.0f, 0.0f, 1.0f, 0xff000000}, //3
// 	{404.0f, 204.0f, 0.0f, 1.0f, 0xff000000}, //4
// };

CUSTOMVERTEX2 g_preview[] = {
	{0.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 0.0f}, //1
	{0.0f, 200.0f, 0.0f, 1.0f, 0xff4488ff, 0.0f, 1.0f}, //2
	{136.0f, 0.0f, 0.0f, 1.0f, 0xff4488ff, 1.0f, 0.0f}, //3
	{136.0f, 200.0f, 0.0f, 1.0f, 0xff4488ff, 1.0f, 1.0f}, //4
};

CUSTOMVERTEX g_preview_outline[] = {
	{0.0f, 0.0f, 0.0f, 1.0f, 0xffffffff}, //1
	{0.0f, 204.0f, 0.0f, 1.0f, 0xffffffff}, //2
	{140.0f, 0.0f, 0.0f, 1.0f, 0xffffffff}, //3
	{140.0f, 204.0f, 0.0f, 1.0f, 0xffffffff}, //4
};

CUSTOMVERTEX g_preview_outline2[] = {
	{0.0f, 0.0f, 0.0f, 1.0f, 0xff000000}, //1
	{0.0f, 206.0f, 0.0f, 1.0f, 0xff000000}, //2
	{142.0f, 0.0f, 0.0f, 1.0f, 0xff000000}, //3
	{142.0f, 206.0f, 0.0f, 1.0f, 0xff000000}, //4
};

// Image preview
static IDirect3DVertexBuffer8* g_pVB_preview = NULL;
static IDirect3DVertexBuffer8* g_pVB_preview_outline = NULL;
static IDirect3DVertexBuffer8* g_pVB_preview_outline2 = NULL;

static IDirect3DTexture8* g_preview_tex = NULL;
static IDirect3DDevice8* g_device = NULL;

////////////////////////////////

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)


//End of graphic related stuff
/////////////////////

int selectedBall=-1;
DWORD numBalls=0;
BALLS *balls;


bool isSelectMode=false;
char display[BUFLEN];
char model[BUFLEN];
char texture[BUFLEN];
static std::map<WORD,int> g_HomeBallMap;
static std::map<string,int> g_BallIdMap;
static bool g_homeTeamChoice = false;
static bool g_userChoice = true;
bool autoRandomMode = false;
bool noHomeBall = true;

DWORD gdbBallAddr=0;
DWORD gdbBallSize=0;
DWORD gdbBallCRC=0;
BYTE* ballTexture=NULL;
int ballTextureSize=0;
RECT ballTextureRect;
bool isPNGtexture=false;
char currTextureName[BUFLEN];
IDirect3DTexture8* g_lastBallTex;
IDirect3DTexture8* g_gdbBallTexture;

#define MAP_FIND(map,key) map[key]
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())
	
	

static bool g_needsRestore = TRUE;
static bool g_newBall = false;
static DWORD g_dwSavedStateBlock = 0L;
static DWORD g_dwDrawOverlayStateBlock = 0L;

void DumpBuffer(char* filename, LPVOID buf, DWORD len);
void SafeRelease(LPVOID ppObj);
BOOL FileExists(char* filename);

void bservReset(IDirect3DDevice8* self, LPVOID params);
HRESULT InitVB(IDirect3DDevice8* dev);
void DeleteStateBlocks(IDirect3DDevice8* dev);
HRESULT InvalidateDeviceObjects(IDirect3DDevice8* dev);
HRESULT DeleteDeviceObjects(IDirect3DDevice8* dev);
HRESULT RestoreDeviceObjects(IDirect3DDevice8* dev);
void DrawBallPreview(IDirect3DDevice8* dev);



BOOL ReadConfig(BSERV_CFG* config, char* cfgFile);
void CheckInput();

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
void InitBserv();
void ReadBalls();

void AddBall(LPTSTR sdisplay,LPTSTR smodel,LPTSTR stexture);
void FreeBalls();
void SetBall(DWORD id);
void bservKeyboardProc(int code1, WPARAM wParam, LPARAM lParam);
void bservBeginUniSelect();
void BeginDrawBallLabel();
void EndDrawBallLabel();
void DrawBallLabel(IDirect3DDevice8* self);
void bservAfsReplace(GETFILEINFO* gfi);
void bservUnpack(GETFILEINFO* gfi, DWORD part, DWORD decBuf, DWORD size);


static bool g_newPrev = false;

DWORD LoadPNGTexture(BITMAPINFO** tex, char* filename);
static int read_file_to_mem(char *fn,unsigned char **ppfiledta, int *pfilesize);
void ApplyAlphaChunk(RGBQUAD* palette, BYTE* memblk, DWORD size);
void FreePNGTexture(BITMAPINFO* bitmap);

HRESULT CreateBallTexture(IDirect3DDevice8* dev, UINT width, UINT height, UINT levels, DWORD usage,
        D3DFORMAT format, IDirect3DTexture8** ppTexture);
void FreeBallTexture();
HRESULT STDMETHODCALLTYPE bservCreateTexture(IDirect3DDevice8* self, UINT width, UINT height,UINT levels,
	DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8** ppTexture, DWORD src, bool* IsProcessed);
void bservUnlockRect(IDirect3DTexture8* self,UINT Level);

DWORD SetBallName(char** names, DWORD numNames, DWORD p3, DWORD p4, DWORD p5, DWORD p6, DWORD p7);
void updateHomeBall();

void DumpBuffer(char* filename, LPVOID buf, DWORD len)
{
    FILE* f = fopen(filename,"wb");
    if (f) {
        fwrite(buf, len, 1, f);
        fclose(f);
    }
}


// Calls IUnknown::Release() on an instance
void SafeRelease(LPVOID ppObj)
{
    try {
        IUnknown** ppIUnknown = (IUnknown**)ppObj;
        if (ppIUnknown == NULL)
        {
            Log(&k_refksrv,"Address of IUnknown reference is 0");
            return;
        }
        if (*ppIUnknown != NULL)
        {
            (*ppIUnknown)->Release();
            *ppIUnknown = NULL;
        }
    } catch (...) {
        // problem with a safe-release
        TRACE(&k_refksrv,"Problem with safe-release");
    }
}

BOOL FileExists(char* filename)
{
    TRACE4(&k_refksrv,"FileExists: Checking file: %s", filename);
    HANDLE hFile;
    hFile = CreateFile(TEXT(filename),        // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);                 // no attr. template
     
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return FALSE;
    }
    CloseHandle(hFile);
    return TRUE;
}

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	char ballCfg[BUFLEN];
	
	if (dwReason == DLL_PROCESS_ATTACH)
	{	
		Log(&k_refksrv,"Attaching dll...");
		
		switch (GetPESInfo()->GameVersion) {
            case gvPES6PC: //support for PES6 PC
            case gvPES6PC110: //support for PES6 PC 1.10
            case gvWE2007PC: //support for WE:PES2007 PC
				goto GameVersIsOK;
				break;
		};
		//Will land here if game version is not supported
		Log(&k_refksrv,"Your game version is currently not supported!");
		return false;
		
		//Everything is OK!
		GameVersIsOK:
		
		RegisterKModule(&k_refksrv);
		
		memcpy(dta, dtaArray[GetPESInfo()->GameVersion], sizeof(dta));
		
		strcpy(currTextureName,"\0");
		
		ReadBalls();
		
		//load settings
	    ZeroMemory(ballCfg, BUFLEN);
	    sprintf(ballCfg, "%s\\refkserv.dat", GetPESInfo()->mydir);
	    FILE* f = fopen(ballCfg, "rb");
	    if (f) {
	        fread(&bserv_cfg, sizeof(BSERV_CFG), 1, f);
	        fread(&g_homeTeamChoice, sizeof(bool), 1, f);
	        fread(&autoRandomMode, sizeof(bool), 1, f);
	        fclose(f);
	    } else {
	    	bserv_cfg.selectedBall=-1;
	    	g_userChoice = false;
	    };

        //read preview setting
        bserv_cfg.previewEnabled = TRUE;
	    ZeroMemory(ballCfg, BUFLEN);
	    sprintf(ballCfg, "%s\\refkserv.cfg", GetPESInfo()->mydir);
        ReadConfig(&bserv_cfg, ballCfg);

		SetBall(bserv_cfg.selectedBall);
		g_userChoice = (selectedBall >= 0);

		HookFunction(hk_D3D_Create,(DWORD)InitBserv);
		HookFunction(hk_D3D_CreateTexture,(DWORD)bservCreateTexture);
		HookFunction(hk_D3D_UnlockRect,(DWORD)bservUnlockRect);
		
	    HookFunction(hk_Input,(DWORD)bservKeyboardProc);
	    		
		HookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);
	    HookFunction(hk_DrawKitSelectInfo,(DWORD)DrawBallLabel);
	    HookFunction(hk_OnShowMenu,(DWORD)BeginDrawBallLabel);
	    HookFunction(hk_OnHideMenu,(DWORD)EndDrawBallLabel);
	    HookFunction(hk_D3D_Reset,(DWORD)bservReset);
		
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Log(&k_refksrv,"Detaching dll...");
		
		//save settings
		bserv_cfg.selectedBall=selectedBall;
	    ZeroMemory(ballCfg, BUFLEN);
	    sprintf(ballCfg, "%s\\refkserv.dat", GetPESInfo()->mydir);
	    FILE* f = fopen(ballCfg, "wb");
	    if (f) {
	        fwrite(&bserv_cfg, sizeof(BSERV_CFG), 1, f);
	        fwrite(&g_homeTeamChoice, sizeof(bool), 1, f);
	        fwrite(&autoRandomMode, sizeof(bool), 1, f);
	        fclose(f);
	    }
		
		UnhookFunction(hk_D3D_CreateTexture,(DWORD)bservCreateTexture);
		UnhookFunction(hk_D3D_UnlockRect,(DWORD)bservUnlockRect);
		
		UnhookFunction(hk_Input,(DWORD)bservKeyboardProc);
		UnhookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);
		UnhookFunction(hk_DrawKitSelectInfo,(DWORD)DrawBallLabel);
		UnhookFunction(hk_D3D_Reset,(DWORD)bservReset);
		
		
		FreeBallTexture();
		
		FreeBalls();
		
		g_BallIdMap.clear();
		g_HomeBallMap.clear();
		
		Log(&k_refksrv,"Detaching done.");
	};

	return true;
};

void InitBserv()
{
	
    Log(&k_refksrv, "InitBserv called.");
    
    RegisterAfsReplaceCallback(bservAfsReplace, bservUnpack, NULL);
    
	
	UnhookFunction(hk_D3D_Create,(DWORD)InitBserv);

	return;
};


void bservAfsReplace(GETFILEINFO* gfi)
{
	if (gfi->isProcessed) return;
	
	DWORD afsId = 0, fileId = 0;
	char filename[BUFLEN];
	ZeroMemory(filename, BUFLEN);
	fileId = splitFileId(gfi->fileId, &afsId);
	// LogWithTwoNumbers(&k_refksrv,"afsId: %d , fileId: %d", 
	// 				afsId, fileId);

    if (afsId == 1) { // 0_text.afs
		if (fileId == dta[REF_KIT_MODEL]) {
			// Model
			strcpy(filename,GetPESInfo()->gdbDir);
			strcat(filename,"GDB\\referee kits\\mdl\\");
			strcat(filename,model);
			//////////////////////////////////
		}else if (fileId >= dta[REF_KIT_1] && fileId <= dta[REF_KIT_8_LO] && fileId%2==0){
			// HQ Texture
			strcpy(filename,GetPESInfo()->gdbDir);
			strcat(filename,"GDB\\referee kits\\");
			strcat(filename,texture);
			strcat(filename,"\\");
			strcat(filename,FILE_NAMES[REF_KIT_HQ]);
		}
		else if (fileId >= dta[REF_KIT_1] && fileId <= dta[REF_KIT_8_LO] && fileId%2==1) {
			// LO Texture
			strcpy(filename,GetPESInfo()->gdbDir);
			strcat(filename,"GDB\\referee kits\\");
			strcat(filename,texture);
			strcat(filename,"\\");
			strcat(filename,FILE_NAMES[REF_KIT_LO]);
		}
	}
	if (strlen(filename)>0) {
        loadReplaceFile(filename);
        gfi->isProcessed = true;
    }
	
	return;
}

void bservUnpack(GETFILEINFO* gfi, DWORD part, DWORD decBuf, DWORD size)
{
	//texture
	gdbBallAddr = decBuf;
	gdbBallSize = size;
	gdbBallCRC = GetCRC((BYTE*)gdbBallAddr, gdbBallSize);
	
	return;
}

void ReadBalls()
{
	char tmp[BUFLEN];
	char str[BUFLEN];
	char *comment=NULL;
	char sdisplay[BUFLEN], smodel[BUFLEN], stexture[BUFLEN];
		
	strcpy(tmp,GetPESInfo()->gdbDir);
	strcat(tmp,"GDB\\referee kits\\map.txt");
	
	FILE* cfg=fopen(tmp, "rt");
	if (cfg==NULL) return;
	while (true) {
		ZeroMemory(str, BUFLEN);
		fgets(str, BUFLEN-1, cfg);
		if (feof(cfg)) break;
		
		// skip comments
		comment=NULL;
		comment = strstr(str, "#");
		if (comment != NULL) comment[0] = '\0';
		
		// parse line
		ZeroMemory(sdisplay,BUFLEN);
		ZeroMemory(smodel,BUFLEN);
		ZeroMemory(stexture,BUFLEN);
		if (sscanf(str,"\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"",sdisplay,smodel,stexture)==3)
			AddBall(sdisplay,smodel,stexture);
	};
	fclose(cfg);
	
	//home map
	
    char mapFile[4096];
    ZeroMemory(mapFile,4096);

    sprintf(mapFile, "%sGDB\\referee kits\\competition_map.txt", GetPESInfo()->gdbDir);
    FILE* map = fopen(mapFile, "rt");
    if (map == NULL) {
        LogWithString(&k_refksrv, "Unable to find Competition-kit-map: %s", mapFile);
        return;
    }

	// go line by line
    char buf[4096];
	while (!feof(map))
	{
		ZeroMemory(buf, 4096);
		fgets(buf, 4096, map);
		if (lstrlen(buf) == 0) break;

		// strip off comments
		char* comm = strstr(buf, "#");
		if (comm != NULL) comm[0] = '\0';

        // find team id
        WORD teamId = 0xffff;
        if (sscanf(buf, "%d", &teamId)==1) {
            LogWithNumber(&k_refksrv, "competition Id = %d", teamId);
            char* ballname = NULL;
            // look for comma
            char* pComma = strstr(buf,",");
            if (pComma) {
                // what follows is the filename.
                // It can be contained within double quotes, so 
                // strip those off, if found.
                char* start = NULL;
                char* end = NULL;
                start = strstr(pComma + 1,"\"");
                if (start) end = strstr(start + 1,"\"");
                if (start && end) {
                    // take what inside the quotes
                    end[0]='\0';
                    ballname = start + 1;
                } else {
                    // just take the rest of the line
                    ballname = pComma + 1;
                }

                LogWithString(&k_refksrv, "KitName = {%s}", ballname);

                // store in the home-ball map
                if (MAP_CONTAINS(g_BallIdMap, ballname)) {
                	g_HomeBallMap[teamId] = g_BallIdMap[ballname];
                }
            }
        }
    }
    fclose(map);
	
	return;
};

void AddBall(LPTSTR sdisplay,LPTSTR smodel,LPTSTR stexture)
{
	BALLS *tmp=new BALLS[numBalls+1];
	memcpy(tmp,balls,numBalls*sizeof(BALLS));
	delete balls;
	balls=tmp;
	
	balls[numBalls].display=new char [strlen(sdisplay)+1];
	strcpy(balls[numBalls].display,sdisplay);
	
	balls[numBalls].model=new char [strlen(smodel)+1];
	strcpy(balls[numBalls].model,smodel);
	
	balls[numBalls].texture=new char [strlen(stexture)+1];
	strcpy(balls[numBalls].texture,stexture);
	
	g_BallIdMap[sdisplay]=numBalls;

	numBalls++;
	return;
};

void FreeBalls()
{
	for (int i=0;i<numBalls;i++) {
		delete balls[i].display;
		delete balls[i].model;
		delete balls[i].texture;
	};
	delete balls;
	numBalls=0;
	selectedBall=-1;
	return;
};

void SetBall(DWORD id)
{
	char tmp[BUFLEN];
	
	if (id<numBalls)
		selectedBall=id;
	else if (id<0)
		selectedBall=-1;
	else
		selectedBall=-1;
		
	if (selectedBall<0) {
		strcpy(tmp,"game choice");
		strcpy(model,"\0");
		strcpy(texture,"\0");
	} else {
		strcpy(tmp,balls[selectedBall].display);
		strcpy(model,balls[selectedBall].model);
		strcpy(texture,balls[selectedBall].texture);

		SafeRelease( &g_preview_tex );
		g_newBall=true;
	};
	
	strcpy(display,"Referee Kit: ");
	strcat(display,tmp);
	
	return;
};

void bservKeyboardProc(int code1, WPARAM wParam, LPARAM lParam)
{
	if ((!isSelectMode) || (code1 < 0))
		return; 

	if ((code1==HC_ACTION) && (lParam & 0x80000000)) {
        KEYCFG* keyCfg = GetInputCfg();
		if (wParam == keyCfg->keyboard.keyNext) {
			SetBall(selectedBall+1);
			g_homeTeamChoice = false;
			g_userChoice = true;
		} else if (wParam == keyCfg->keyboard.keyPrev) {
			if (selectedBall<0)
				SetBall(numBalls-1);
			else
				SetBall(selectedBall-1);
				
			g_homeTeamChoice = false;
			g_userChoice = true;
			
		} else if (wParam == keyCfg->keyboard.keyReset) {
			if (!autoRandomMode && g_homeTeamChoice) {
        		autoRandomMode = true;
        		g_homeTeamChoice = false;
        		g_userChoice = true;
	            
        	} else {
	        	if (g_userChoice) g_homeTeamChoice = true;
				SetBall(-1);
				g_homeTeamChoice = !g_homeTeamChoice;
				updateHomeBall();
				g_userChoice = false;
				autoRandomMode = false;
			}
			
			if (autoRandomMode || noHomeBall) {
				LARGE_INTEGER num;
				QueryPerformanceCounter(&num);
				int iterations = num.LowPart % MAX_ITERATIONS;
				for (int j=0;j<iterations;j++) {
					SetBall(selectedBall+1);
		        }
			}

		} else if (wParam == keyCfg->keyboard.keyRandom) {
			LARGE_INTEGER num;
			QueryPerformanceCounter(&num);
			int iterations = num.LowPart % MAX_ITERATIONS;
			for (int j=0;j<iterations;j++)
				SetBall(selectedBall+1);
			
			g_homeTeamChoice = false;
			g_userChoice = true;
		};
	};
	
	return;
};

void bservBeginUniSelect()
{
	updateHomeBall();
	
	if (autoRandomMode || noHomeBall) {
		LARGE_INTEGER num;
		QueryPerformanceCounter(&num);
		int iterations = num.LowPart % MAX_ITERATIONS;
		for (int j=0;j<iterations;j++) {
			SetBall(selectedBall+1);
        }
	}
	return;
}

void BeginDrawBallLabel()
{
	isSelectMode=true;
	dksiSetMenuTitle("Referee kit selection");
	
	SafeRelease( &g_preview_tex );
    g_newBall = true;
	return;
};

void EndDrawBallLabel()
{
	isSelectMode=false;
	return;
};


void DrawBallPreview(IDirect3DDevice8* dev)
{
	if (strlen(model)==0 || strlen(texture)==0)
		return;
	
	if (g_needsRestore || g_newBall) 
	{
		if (g_newBall) {
			InvalidateDeviceObjects(dev);
			DeleteDeviceObjects(dev);
			g_needsRestore = TRUE;
		};
		if (FAILED(RestoreDeviceObjects(dev)))
		{
			Log(&k_refksrv,"DrawBallPreview: RestoreDeviceObjects() failed.");
            return;
		}
		Log(&k_refksrv,"DrawBallPreview: RestoreDeviceObjects() done.");
        g_needsRestore = FALSE;
	}

	// render
	dev->BeginScene();

	// setup renderstate
	dev->CaptureStateBlock( g_dwSavedStateBlock );
	dev->ApplyStateBlock( g_dwDrawOverlayStateBlock );
    
    if (!g_preview_tex && g_newBall) {
        char buf[2048];
        sprintf(buf, "%s\\GDB\\referee kits\\%s\\preview.png", GetPESInfo()->gdbDir, texture);
        if (FAILED(D3DXCreateTextureFromFileEx(dev, buf, 
                    0, 0, 4, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                    D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
                    0, NULL, NULL, &g_preview_tex))) {
            Log(&k_refksrv,"FAILED to load image for kit preview.");
		}
		// LogWithString(&k_refksrv, "preview buf: {%s}", buf);
        g_newBall = false;
    }
    if (g_preview_tex) {
        // outline
        dev->SetVertexShader( D3DFVF_CUSTOMVERTEX );
        dev->SetTexture(0, NULL);
        dev->SetStreamSource( 0, g_pVB_preview_outline2, sizeof(CUSTOMVERTEX));
        dev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);
        dev->SetStreamSource( 0, g_pVB_preview_outline, sizeof(CUSTOMVERTEX));
        dev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);

        // texture
        dev->SetVertexShader( D3DFVF_CUSTOMVERTEX2 );
        dev->SetTexture(0, g_preview_tex);
        dev->SetStreamSource( 0, g_pVB_preview, sizeof(CUSTOMVERTEX2));
        dev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);
    }

	// restore the modified renderstates
	//dev->ApplyStateBlock( g_dwSavedStateBlock );

	dev->EndScene();
	
	return;
};
void DrawBallLabel(IDirect3DDevice8* self)
{
	char display1[256];
	SIZE size;
	DWORD color = 0xffffffff; // white
	
	if (selectedBall<0)
		color = 0xffc0c0c0; // gray if ball is game choice
		
	if (autoRandomMode)
		color = 0xffff3300; // light red for randomly selected ball
		
	if (g_homeTeamChoice) {
		if (noHomeBall)
			color = 0xffff6600; // orange for home-choice, but team has no ball
		else
			color = 0xffffffc0; // pale yellow if ball is home-team choice
	}
	
	UINT g_bbWidth=GetPESInfo()->bbWidth;
	UINT g_bbHeight=GetPESInfo()->bbHeight;
	double stretchX=GetPESInfo()->stretchX;
	double stretchY=GetPESInfo()->stretchY;
	
	KGetTextExtent(display,12,&size);
	//draw shadow
	if (selectedBall>=0)
		KDrawText((g_bbWidth-size.cx)/2+3*stretchX,g_bbHeight*0.77+3*stretchY,0xff000000,12,display,true);
	//print ball label
	KDrawText((g_bbWidth-size.cx)/2,g_bbHeight*0.77,color,12,display,true);

   
 //draw ball preview
    if (bserv_cfg.previewEnabled) {
        DrawBallPreview(self);
    }
    // check input
    CheckInput();


	return;
};

// Load texture from PNG file. Returns the size of loaded texture
DWORD LoadPNGTexture(BITMAPINFO** tex, char* filename)
{
	TRACE4(&k_refksrv,"LoadPNGTexture: loading %s", filename);
    DWORD size = 0;

    PNGDIB *pngdib;
    LPBITMAPINFOHEADER* ppDIB = (LPBITMAPINFOHEADER*)tex;

    pngdib = pngdib_p2d_init();
	//TRACE(&k_refksrv,"LoadPNGTexture: structure initialized");

    BYTE* memblk;
    int memblksize;
    if (read_file_to_mem(filename,&memblk, &memblksize)) {
        TRACE(&k_refksrv,"LoadPNGTexture: unable to read PNG file");
        return 0;
    }
    //TRACE(&k_refksrv,"LoadPNGTexture: file read into memory");

    pngdib_p2d_set_png_memblk(pngdib,memblk,memblksize);
	pngdib_p2d_set_use_file_bg(pngdib,1);
	pngdib_p2d_run(pngdib);

	//TRACE(&k_refksrv,"LoadPNGTexture: run done");
    pngdib_p2d_get_dib(pngdib, ppDIB, (int*)&size);
	//TRACE(&k_refksrv,"LoadPNGTexture: get_dib done");

    pngdib_done(pngdib);
	TRACE(&k_refksrv,"LoadPNGTexture: done done");

	TRACE2(&k_refksrv,"LoadPNGTexture: *ppDIB = %08x", (DWORD)*ppDIB);
    if (*ppDIB == NULL) {
		TRACE(&k_refksrv,"LoadPNGTexture: ERROR - unable to load PNG image.");
        return 0;
    }
	FILE* file = fopen("C:\\Users\\Pato\\Desktop\\Trofeos WE5-WE6FE\\Berlin.dib", "wb");
	if (file) {
		fwrite(memblk, 1, memblksize, file);  // Utilizar *ppDIB para obtener el valor apuntado por ppDIB
		fclose(file);
	}

    // read transparency values from tRNS chunk
    // and put them into DIB's RGBQUAD.rgbReserved fields
    ApplyAlphaChunk((RGBQUAD*)&((BITMAPINFO*)*ppDIB)->bmiColors, memblk, memblksize);

    HeapFree(GetProcessHeap(), 0, memblk);

	TRACE(&k_refksrv,"LoadPNGTexture: done");
	return size;
};

// Read a file into a memory block.
static int read_file_to_mem(char *fn,unsigned char **ppfiledta, int *pfilesize)
{
	HANDLE hfile;
	DWORD fsize;
	//unsigned char *fbuf;
	BYTE* fbuf;
	DWORD bytesread;

	hfile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) return 1;

	fsize=GetFileSize(hfile,NULL);
	if(fsize>0) {
		//fbuf=(unsigned char*)GlobalAlloc(GPTR,fsize);
		//fbuf=(unsigned char*)calloc(fsize,1);
        fbuf = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fsize);
		if(fbuf) {
			if(ReadFile(hfile,(void*)fbuf,fsize,&bytesread,NULL)) {
				if(bytesread==fsize) { 
					(*ppfiledta)  = fbuf;
					(*pfilesize) = (int)fsize;
					CloseHandle(hfile);
					return 0;   // success
				}
			}
			free((void*)fbuf);
		}
	}
	CloseHandle(hfile);
	return 1;  // error
};

/**
 * Extracts alpha values from tRNS chunk and applies stores
 * them in the RGBQUADs of the DIB
 */
void ApplyAlphaChunk(RGBQUAD* palette, BYTE* memblk, DWORD size)
{
   
};

void FreePNGTexture(BITMAPINFO* bitmap)
{
	
};

void CheckInput()
{
    DWORD* inputs = GetInputTable();
    KEYCFG* keyCfg = GetInputCfg();
    for (int n=0; n<8; n++) {
        if (INPUT_EVENT(inputs, n, FUNCTIONAL, keyCfg->gamepad.keyNext)) {
			SetBall(selectedBall+1);
			g_homeTeamChoice = false;
			g_userChoice = true;

        } else if (INPUT_EVENT(inputs, n, FUNCTIONAL, keyCfg->gamepad.keyPrev)) {
			if (selectedBall<0) {
				SetBall(numBalls-1);
            } else {
				SetBall(selectedBall-1);
            }
            g_homeTeamChoice = false;
            g_userChoice = true;

        } else if (INPUT_EVENT(inputs, n, FUNCTIONAL, keyCfg->gamepad.keyReset)) {
        	if (!autoRandomMode && g_homeTeamChoice) {
        		autoRandomMode = true;
        		g_homeTeamChoice = false;
        		g_userChoice = true;
	            
        	} else {
	        	if (g_userChoice) g_homeTeamChoice = true;
				SetBall(-1);
				g_homeTeamChoice = !g_homeTeamChoice;
				updateHomeBall();
				g_userChoice = false;
				autoRandomMode = false;
			}
			
			if (autoRandomMode || noHomeBall) {
				LARGE_INTEGER num;
				QueryPerformanceCounter(&num);
				int iterations = num.LowPart % MAX_ITERATIONS;
				for (int j=0;j<iterations;j++) {
					SetBall(selectedBall+1);
		        }
			}

        } else if (INPUT_EVENT(inputs, n, FUNCTIONAL, keyCfg->gamepad.keyRandom)) {
			LARGE_INTEGER num;
			QueryPerformanceCounter(&num);
			int iterations = num.LowPart % MAX_ITERATIONS;
			for (int j=0;j<iterations;j++) {
				SetBall(selectedBall+1);
            }
            g_homeTeamChoice = false;
            g_userChoice = true;
		}
    }
}



void bservReset(IDirect3DDevice8* self, LPVOID params)
{
	Log(&k_refksrv,"bservReset: cleaning-up.");
	
	InvalidateDeviceObjects(self);
	DeleteDeviceObjects(self);
	
	g_needsRestore = TRUE;
	
	return;
}


void SetPosition(CUSTOMVERTEX2* dest, CUSTOMVERTEX2* src, int n, int x, int y) 
{
    FLOAT xratio = GetPESInfo()->bbWidth / 1024.0;
    FLOAT yratio = GetPESInfo()->bbHeight / 768.0;
    for (int i=0; i<n; i++) {
        dest[i].x = (FLOAT)(int)((src[i].x + x) * xratio);
        dest[i].y = (FLOAT)(int)((src[i].y + y) * yratio);
    }
}

void SetPosition(CUSTOMVERTEX* dest, CUSTOMVERTEX* src, int n, int x, int y) 
{
    FLOAT xratio = GetPESInfo()->bbWidth / 1024.0;
    FLOAT yratio = GetPESInfo()->bbHeight / 768.0;
    for (int i=0; i<n; i++) {
        dest[i].x = (FLOAT)(int)((src[i].x + x) * xratio);
        dest[i].y = (FLOAT)(int)((src[i].y + y) * yratio);
    }
}
/* creates vertex buffers */
HRESULT InitVB(IDirect3DDevice8* dev)
{
	VOID* pVertices;

	// create vertex buffers
	// preview
	if (FAILED(dev->CreateVertexBuffer(sizeof(g_preview), D3DUSAGE_WRITEONLY, 
					D3DFVF_CUSTOMVERTEX2, D3DPOOL_MANAGED, &g_pVB_preview)))
	{
		Log(&k_refksrv,"CreateVertexBuffer() failed.");
		return E_FAIL;
	}
	Log(&k_refksrv,"CreateVertexBuffer() done.");

	if (FAILED(g_pVB_preview->Lock(0, sizeof(g_preview), (BYTE**)&pVertices, 0)))
	{
		Log(&k_refksrv,"g_pVB_preview->Lock() failed.");
		return E_FAIL;
	}
	memcpy(pVertices, g_preview, sizeof(g_preview));
	SetPosition((CUSTOMVERTEX2*)pVertices, g_preview, sizeof(g_preview)/sizeof(CUSTOMVERTEX2), 
            512-64, 384);
	g_pVB_preview->Unlock();

	// preview outline
	if (FAILED(dev->CreateVertexBuffer(sizeof(g_preview_outline), D3DUSAGE_WRITEONLY, 
					D3DFVF_CUSTOMVERTEX2, D3DPOOL_MANAGED, &g_pVB_preview_outline)))
	{
		Log(&k_refksrv,"CreateVertexBuffer() failed.");
		return E_FAIL;
	}
	Log(&k_refksrv,"CreateVertexBuffer() done.");

	if (FAILED(g_pVB_preview_outline->Lock(0, sizeof(g_preview_outline), (BYTE**)&pVertices, 0)))
	{
		Log(&k_refksrv,"g_pVB_preview_outline->Lock() failed.");
		return E_FAIL;
	}
	memcpy(pVertices, g_preview_outline, sizeof(g_preview_outline));
	SetPosition((CUSTOMVERTEX*)pVertices, g_preview_outline, sizeof(g_preview_outline)/sizeof(CUSTOMVERTEX), 
          512-65, 383);
	// SetPosition((CUSTOMVERTEX*)pVertices, g_preview_outline, sizeof(g_preview_outline)/sizeof(CUSTOMVERTEX), 
    //       512-201, 383);
	g_pVB_preview_outline->Unlock();

	// preview outline2
	if (FAILED(dev->CreateVertexBuffer(sizeof(g_preview_outline2), D3DUSAGE_WRITEONLY, 
					D3DFVF_CUSTOMVERTEX2, D3DPOOL_MANAGED, &g_pVB_preview_outline2)))
	{
		Log(&k_refksrv,"CreateVertexBuffer() failed.");
		return E_FAIL;
	}

	Log(&k_refksrv,"CreateVertexBuffer() done.");

	if (FAILED(g_pVB_preview_outline2->Lock(0, sizeof(g_preview_outline2), (BYTE**)&pVertices, 0)))
	{
		Log(&k_refksrv,"g_pVB_preview_outline2->Lock() failed.");
		return E_FAIL;
	}
	memcpy(pVertices, g_preview_outline2, sizeof(g_preview_outline2));
	SetPosition((CUSTOMVERTEX*)pVertices, g_preview_outline2, sizeof(g_preview_outline2)/sizeof(CUSTOMVERTEX), 
            512-66, 382);
	// SetPosition((CUSTOMVERTEX*)pVertices, g_preview_outline2, sizeof(g_preview_outline2)/sizeof(CUSTOMVERTEX), 
    //         512-203, 382);
	g_pVB_preview_outline2->Unlock();


    return S_OK;
}

void DeleteStateBlocks(IDirect3DDevice8* dev)
{
	// Delete the state blocks
	try
	{
        DWORD* vtab = (DWORD*)(*(DWORD*)dev);
        if (vtab && vtab[VTAB_DELETESTATEBLOCK]) {
            if (g_dwSavedStateBlock) {
                dev->DeleteStateBlock( g_dwSavedStateBlock );
                Log(&k_refksrv,"g_dwSavedStateBlock deleted.");
            }
            if (g_dwDrawOverlayStateBlock) {
                dev->DeleteStateBlock( g_dwDrawOverlayStateBlock );
                Log(&k_refksrv,"g_dwDrawOverlayStateBlock deleted.");
            }
        }
	}
	catch (...)
	{
        // problem deleting state block
	}

	g_dwSavedStateBlock = 0L;
	g_dwDrawOverlayStateBlock = 0L;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT InvalidateDeviceObjects(IDirect3DDevice8* dev)
{
	TRACE(&k_refksrv,"InvalidateDeviceObjects called.");
	if (dev == NULL)
	{
		TRACE(&k_refksrv,"InvalidateDeviceObjects: nothing to invalidate.");
		return S_OK;
	}


	Log(&k_refksrv,"InvalidateDeviceObjects: SafeRelease(s) done.");

    DeleteStateBlocks(dev);
    Log(&k_refksrv,"InvalidateDeviceObjects: DeleteStateBlock(s) done.");
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT DeleteDeviceObjects(IDirect3DDevice8* dev)
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT RestoreDeviceObjects(IDirect3DDevice8* dev)
{
    HRESULT hr = InitVB(dev);
    if (FAILED(hr))
    {
		Log(&k_refksrv,"InitVB() failed.");
        return hr;
    }
	Log(&k_refksrv,"InitVB() done.");

    return S_OK;
}



/**
 * Returns true if successful.
 */
BOOL ReadConfig(BSERV_CFG* config, char* cfgFile)
{
	if (config == NULL) return false;

	FILE* cfg = fopen(cfgFile, "rt");
	if (cfg == NULL) return false;

	char str[BUFLEN];
	char name[BUFLEN];
	int value = 0;

	char *pName = NULL, *pValue = NULL, *comment = NULL;
	while (!feof(cfg))
	{
		ZeroMemory(str, BUFLEN);
		fgets(str, BUFLEN-1, cfg);

		// skip comments
		comment = strstr(str, "#");
		if (comment != NULL) comment[0] = '\0';

		// parse the line
		pName = pValue = NULL;
		ZeroMemory(name, BUFLEN); value = 0;
		char* eq = strstr(str, "=");
		if (eq == NULL || eq[1] == '\0') continue;

		eq[0] = '\0';
		pName = str; pValue = eq + 1;

		ZeroMemory(name, NULL); 
		sscanf(pName, "%s", name);

		if (lstrcmp(name, "preview")==0)
		{
			if (sscanf(pValue, "%d", &value)!=1) continue;
			LogWithNumber(&k_refksrv,"ReadConfig: preview = (%d)", value);
            config->previewEnabled = (value == 1);
		}
	}
	fclose(cfg);
	return true;
}

HRESULT CreateBallTexture(IDirect3DDevice8* dev, UINT width, UINT height, UINT levels, DWORD usage,
        D3DFORMAT format, IDirect3DTexture8** ppTexture) 
{
	char tmp[BUFLEN];
	ZeroMemory(tmp,BUFLEN);
	
	updateHomeBall();
	if (selectedBall<0) return false;
		
	if (ballTexture!=NULL && lstrcmpi(currTextureName,texture)==0)
		return true;
		
	FreeBallTexture();
	sprintf(tmp,"%sGDB\\referee kits\\%s",GetPESInfo()->gdbDir,texture);
	
    D3DXIMAGE_INFO imageInfo;
    if (SUCCEEDED(D3DXGetImageInfoFromFile(tmp, &imageInfo))) {
        // it's IMPORTANT not to downsize the texture, because it will
        // lead to crashes, as the game still thinks the texture at least 
        // width*height size.
        ballTextureRect.right = max(imageInfo.Width, width);
        ballTextureRect.bottom = max(imageInfo.Height, height);

        if (FAILED(D3DXCreateTextureFromFileEx(
                    dev, tmp, ballTextureRect.right, ballTextureRect.bottom, 
                    levels, usage, format, D3DPOOL_MANAGED, 
                    D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, ppTexture
                ))) {
            LogWithString(&k_refksrv, "D3DXCreateTextureFromFileEx FAILED for %s", tmp);
            return false;
        }

        strcpy(currTextureName,texture);
    } else {
        LogWithString(&k_refksrv, "D3DXGetImageInfoFromFile FAILED for %s", tmp);
        return false;
    }

	return true;
};

void FreeBallTexture()
{
    SafeRelease(&g_gdbBallTexture);
	strcpy(currTextureName,"\0");
	return;
};

DWORD VtableSet(void* self, int index, DWORD value)
{
    DWORD* vtab = (DWORD*)(*(DWORD*)self);
    DWORD currValue = vtab[index];
    vtab[index] = value;
    return currValue;
}

HRESULT STDMETHODCALLTYPE bservCreateTexture(IDirect3DDevice8* self, UINT width, UINT height,UINT levels,
DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8** ppTexture, DWORD src, bool* IsProcessed)
{
	HRESULT res=D3D_OK;
	RECT texSize;
//LogWithNumber(&k_refksrv, "bservCreateTexture: gdbBallAddr = %08x", gdbBallAddr);
	
	if (*IsProcessed==true)
		return res;
//Log(&k_refksrv, "bservCreateTexture: IsProcessed = false");
//LogWithNumber(&k_refksrv, "bservCreateTexture: src = %08x", src);
	
	g_lastBallTex=NULL;
	
	if (IsBadReadPtr((BYTE*)src,gdbBallSize) || gdbBallCRC!=GetCRC((BYTE*)src,gdbBallSize)) {
		//wrong CRC -> dta changed
		gdbBallAddr=0;
		gdbBallSize=0;
		gdbBallCRC=0;
	};
//LogWithTwoNumbers(&k_refksrv, "bservCreateTexture: %08x vs. %08x", src, (DWORD)gdbBallAddr);
	
	if (src!=0 && src==gdbBallAddr) {
		Log(&k_refksrv,"bservCreateTexture called for ball texture.");
		
        DWORD prevValue = VtableSet(self, VTAB_CREATETEXTURE, (DWORD)OrgCreateTexture);
        if (FAILED(CreateBallTexture(self,width,height,levels,usage,format,&g_gdbBallTexture))) {
            Log(&k_refksrv,"bservCreateTexture: CreateBallTexture FAILED.");
            *IsProcessed = false;
            return res;
        }
        VtableSet(self, VTAB_CREATETEXTURE, prevValue);

		res = OrgCreateTexture(self, ballTextureRect.right, ballTextureRect.bottom,
				levels,usage,format,pool,ppTexture);

        g_lastBallTex = *ppTexture;
        *IsProcessed = true;
		TRACE2(&k_refksrv,"tex = %08x", (DWORD)g_lastBallTex);
	};

	return res;
}

void bservUnlockRect(IDirect3DTexture8* self,UINT Level)
{
	if (g_gdbBallTexture==NULL || g_lastBallTex==NULL)
		return;
		
    IDirect3DSurface8* src = NULL;
    IDirect3DSurface8* dest = NULL;

	//LogWithTwoNumbers(&k_refksrv,"bservUnlockRect: Processing texture %x, level %d",(DWORD)self,Level);
    if (SUCCEEDED(g_lastBallTex->GetSurfaceLevel(0, &dest))) {
        if (SUCCEEDED(g_gdbBallTexture->GetSurfaceLevel(0, &src))) {
            if (SUCCEEDED(D3DXLoadSurfaceFromSurface(
                            dest, NULL, NULL,
                            src, NULL, NULL,
                            D3DX_FILTER_NONE, 0))) {
                Log(&k_refksrv,"Replacing ball texture COMPLETE");

            } else {
                Log(&k_refksrv,"Replacing ball texture FAILED");
            }
            src->Release();
        }
        dest->Release();
    }

	g_lastBallTex=NULL;
	return;
}

DWORD SetBallName(char** names, DWORD numNames, DWORD p3, DWORD p4, DWORD p5, DWORD p6, DWORD p7)
{
	updateHomeBall();
	if (selectedBall>=0 && numNames==3) {
		//strcpy(names[1],balls[selectedBall].display);
		names[1]=balls[selectedBall].display;
	};
	
	return MasterCallNext(names,numNames,p3,p4,p5,p6,p7);
};

/**
 * Return currently selected competition ID.
 */
WORD GetTeamId(int which)
{
    BYTE* mlData;
    if (dta[TEAM_IDS]==0) return 0xffff;
    WORD id = ((BYTE*)dta[TEAM_IDS])[which];
    // LogWithNumber(&k_scoreboard, "Selector ID TEAM: %d", id);
    

    return id;
}

void updateHomeBall()
{
	if (g_homeTeamChoice) {

        WORD menu_op = *(BYTE*)0x3A70CA8; //get menu index
        LogWithNumber(&k_refksrv, "Menu Option: %d", menu_op);
        // LogWithNumber(&k_scoreboard, "Menu Option: %d", menu_op);
  
        if (menu_op == 0) {
        	noHomeBall = true;
            return;
        }
        WORD teamId = GetTeamId(HOME);
        std::map<WORD,int>::iterator hit = g_HomeBallMap.find(teamId);
        if (hit !=  g_HomeBallMap.end()) {
            SetBall(hit->second);
            noHomeBall = false;
        	LogWithNumber(&k_refksrv, "Menu Option: %d", menu_op);

        } else {
        	noHomeBall = true;
            return;
        }
    } else {
    	noHomeBall = false;
    }
    return;
}



