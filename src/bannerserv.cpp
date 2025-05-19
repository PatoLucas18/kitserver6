#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "bannerserv.h"
#include "crc32.h"
#include "kload_exp.h"
#include "numpages.h"
#include "input.h"
#include "keycfg.h"

#include <pngdib.h>
#include <map>
#include <string>

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>


KMOD k_module={MODID,NAMELONG,NAMESHORT,DEFAULT_DEBUG};
DWORD protection;
DWORD newProtection = PAGE_EXECUTE_READWRITE;
BSERV_CFG bserv_cfg;

static std::map<WORD,std::string*> g_chantsMap;


class config_t 
{
public:
    config_t() : _flag(false) {}
    bool _flag;
};

config_t _config;


char* getRandomAdxFile(const char* directory);
bool homeHasChants = false;
bool awayHasChants = false;

#define MAP_FIND(map,key) map[key]
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
void InitBannerserv();
void bservBeginUniSelect();

bool ReadConfig(config_t& config, const char* cfgFile);
void ReadMap();

static int read_file_to_mem(char *fn,unsigned char **ppfiledta, int *pfilesize);


void ClearBannerCallPoint();
void bannerClear(DWORD p1);

bool fileExists(const char* filename);

void HomeReplaceBannerCallPoint();
void AwayReplaceBannerCallPoint();
void BannerLoad(DWORD p1,DWORD p2,DWORD p3);


// Ejemplo: bannerFile como variable global
BYTE* bannerFile = nullptr;


void supp_colours(DWORD param_1,DWORD param_2);
void supp_coloursCallPoint();


BYTE Readcolours(const char* cfgFile);


KEXPORT void HookCallPoint(DWORD addr, void* func, int codeShift, int numNops, bool addRetn)
{
    DWORD target = (DWORD)func + codeShift;
	if (addr && target)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 16, newProtection, &protection)) {
	        bptr[0] = 0xe8;
	        DWORD* ptr = (DWORD*)(addr + 1);
	        ptr[0] = target - (DWORD)(addr + 5);
            // padding with NOPs
            for (int i=0; i<numNops; i++) bptr[5+i] = 0x90;
            if (addRetn)
                bptr[5+numNops]=0xc3;
	        TRACE2X(&k_module, "Function (%08x) HOOKED at address (%08x)", target, addr);
	    }
	}
}



WORD GetTeamId(int which);


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	char ballCfg[BUFLEN];
	
	if (dwReason == DLL_PROCESS_ATTACH)
	{	
		Log(&k_module,"Attaching dll...");
		
		switch (GetPESInfo()->GameVersion) {
            case gvPES6PC: //support for PES6 PC
				goto GameVersIsOK;
				break;
            // case gvPES6PC110: //support for PES6 PC 1.10
            // case gvWE2007PC: //support for WE:PES2007 PC
			// 	goto GameVersIsOK;
			// 	break;
		};
		//Will land here if game version is not supported
		Log(&k_module,"Your game version is currently not supported!");
		return false;
		
		//Everything is OK!
		GameVersIsOK:
		
		RegisterKModule(&k_module);
		

		memcpy(dta, dtaArray[GetPESInfo()->GameVersion], sizeof(dta));
		
		Log(&k_module, "InitBannerserv called.");
		string configFile(GetPESInfo()->mydir);
		configFile += "bannerserv.cfg";
		ReadConfig(_config, configFile.c_str());
		
		
		HookFunction(hk_D3D_Create,(DWORD)InitBannerserv);		
		HookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);
		
        
    	HookCallPoint(0x9622c5, HomeReplaceBannerCallPoint, 6, 0, false); //set addr banner
    	HookCallPoint(0x962357, AwayReplaceBannerCallPoint, 6, 0, false); //set addr banner
    	HookCallPoint(0x96126E, supp_coloursCallPoint, 6, 0, false); //edited
    	HookCallPoint(0x961594, supp_coloursCallPoint, 6, 0, false); //edited Online?

	    
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Log(&k_module,"Detaching dll...");
		
	
		UnhookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);
	
		Log(&k_module,"Detaching done.");
	};

	return true;
};


void InitBannerserv() {

    ReadMap();
	UnhookFunction(hk_D3D_Create,(DWORD)InitBannerserv);

	return;
};

void ReadMap() {
	//map.txt
	
    char mapFile[BUFLEN];
    ZeroMemory(mapFile,BUFLEN);

    sprintf(mapFile, "%sGDB\\banners\\map.txt", GetPESInfo()->gdbDir);
    FILE* map = fopen(mapFile, "rt");
    if (map == NULL) {
        LOG(&k_module, "Unable to find chants map: %s", mapFile);
        return;
    }

	// go line by line
    char buf[BUFLEN];
	while (!feof(map))
	{
		ZeroMemory(buf, BUFLEN);
		fgets(buf, BUFLEN, map);
		if (lstrlen(buf) == 0) break;

		// strip off comments
		char* comm = strstr(buf, "#");
		if (comm != NULL) comm[0] = '\0';

        // find team id
        WORD teamId = 0xffff;
        if (sscanf(buf, "%d", &teamId)==1) {
            LogWithNumber(&k_module, "teamId = %d", teamId);
            char* foldername = NULL;
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
                    foldername = start + 1;
                } else {
                    // just take the rest of the line
                    foldername = pComma + 1;
                }

                LOG(&k_module, "foldername = {%s}", foldername);

                // store in the home-ball map
				g_chantsMap[teamId] = new std::string(foldername);
            }
        }
    }
    fclose(map);
	
	return;
};


#include <vector>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <ctime>

char* getRandomAdxFile(const char* directory) {
    std::string folderPath = GetPESInfo()->mydir;
    std::string subfolder = "GDB\\banners\\";
    std::string combinedPath = folderPath + subfolder + directory;

    std::vector<std::string> files;
    WIN32_FIND_DATA ffd;
    
    // Buscar archivos con extensión .str
    HANDLE hFindStr = FindFirstFile((combinedPath + "\\*.str").c_str(), &ffd);
    if (hFindStr != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(ffd.cFileName);
            }
        } while (FindNextFile(hFindStr, &ffd) != 0);
        FindClose(hFindStr);
    }

    // Buscar archivos con extensión .bin
    HANDLE hFindBin = FindFirstFile((combinedPath + "\\*.bin").c_str(), &ffd);
    if (hFindBin != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(ffd.cFileName);
            }
        } while (FindNextFile(hFindBin, &ffd) != 0);
        FindClose(hFindBin);
    }

    // Si no se encontraron archivos, retornar NULL
    if (files.empty()) {
        return NULL;
    }

    // Selección aleatoria de un archivo
    srand(static_cast<unsigned int>(time(0)));
    std::string randomFile = files[rand() % files.size()];

    // Devolver el nombre del archivo seleccionado
    char* filename = new char[randomFile.length() + 1];
    strcpy(filename, randomFile.c_str());

    return filename;
}


void bservBeginUniSelect() {
		Log(&k_module, "bservBeginUniSelect");		
		return;
}


/**
 * Returns true if successful.
 */
bool ReadConfig(config_t& config, const char* cfgFile) {
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

		if (strcmp(name, "debug")==0)
        {
			if (sscanf(pValue, "%d", &value)!=1) continue;
			LogWithNumber(&k_module,"ReadConfig: debug = (%d)", value);
            k_module.debug = value;
        }
        else if (strcmp(name, "chant.my.team.ml")==0)
        {
			if (sscanf(pValue, "%d", &value)!=1) continue;
			LogWithNumber(&k_module,"ReadConfig: chant.my.team.ml = (%d)", value);
            // config._ = max(value, 4);
            // int otraVariable = 1; // Esta puede ser 1 o 0
            int otraVariable = value; // Esta puede ser 1 o 0

            _config._flag = (otraVariable == 1);

            // _config._flag = max(value, 1);

        }
	}
	fclose(cfg);
	return true;
}
/**
 * Return currently selected (home or away) team ID.
 */
WORD GetTeamId(int which) {
    BYTE* mlData;
    if (dta[TEAM_IDS]==0) return 0xffff;
    WORD id = ((WORD*)dta[TEAM_IDS])[which];
    if (id==0x126 || id==0x127) {
        WORD id1,id2;
        switch (id) {
            case 0x126:
                // saved team (home)
                id1 = *(WORD*)(*(BYTE**)dta[SAVED_TEAM_HOME] + 0x36);
                id2 = *(WORD*)(*(BYTE**)dta[SAVED_TEAM_HOME] + 0x40);
                if (id1 != 0) {
                    id = id1;
                } else {
                    id = id2;
                }
                break;
            case 0x127:
                // saved team (away)
                id1 = *(WORD*)(*(BYTE**)dta[SAVED_TEAM_AWAY] + 0x36);
                id2 = *(WORD*)(*(BYTE**)dta[SAVED_TEAM_AWAY] + 0x40);
                if (id1 != 0) {
                    id = id1;
                } else {
                    id = id2;
                }
                break;
        }
    }
    return id;
}


// Read a file into a memory block.
static int read_file_to_mem(char *fn,unsigned char **ppfiledta, int *pfilesize) {
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


// void HomeReplaceBannerCallPoint() {
//     __asm {
//         pushfd
//         push ebp
//         push eax
//         push ebx
//         push ecx
//         push edx 
//         push esi
//         push edi
//         push 0
//         push ecx
//         push eax
//         call BannerLoad
//         add esp, 0x0c  // pop parameters
//         pop edi
//         pop esi
//         pop edx
//         pop ecx
//         pop ebx
//         pop eax
//         pop ebp
//         popfd
//         mov esi,[eax+ecx*4]
//         test esi,esi
//         retn
//     }
// }

// void AwayReplaceBannerCallPoint() {
//     __asm {
//         pushfd
//         push ebp
//         push eax
//         push ebx
//         push ecx
//         push edx 
//         push esi
//         push edi
//         push 1
//         push edx
//         push eax
//         call BannerLoad
//         add esp, 0x0c  // pop parameters
//         pop edi
//         pop esi
//         pop edx
//         pop ecx
//         pop ebx
//         pop eax
//         pop ebp
//         popfd
//         mov esi,[eax+edx*4]
//         test esi,esi
//         retn
//     }
// }


// void BannerLoad(DWORD p1,DWORD p2,DWORD p3) {
    
//     LOG(&k_module," BannerUnzip2");
//     LogWithNumber(&k_module,"Escribir puntero en = (%d)", p1+p2*4);
   
//     char filename[2048];

// 	WORD homeTeamId = GetTeamId(p3);
//     LogWithNumber(&k_module,"TeamId = (%d)", homeTeamId);
	
//     std::string* homeFolderString = MAP_FIND(g_chantsMap,homeTeamId);
// 	if (homeFolderString != NULL) {
//         LogWithString(&k_module, "mapeo archivo1: %s", (char*)homeFolderString->c_str());
//         char* RandomFile = getRandomAdxFile((char*)homeFolderString->c_str());
//         sprintf(filename,"%sGDB\\banners\\%s\\%s", GetPESInfo()->gdbDir, (char*)homeFolderString->c_str(), RandomFile);

        
//         if (fileExists(filename)) {
//             LogWithString(&k_module, "El archivo existe: : %s", filename);
               
//             int bannerFileSize=0;

//             if (read_file_to_mem(filename,&bannerFile,&bannerFileSize) != 0) {
//                 LogWithString(&k_module, "Unable to read opd: %s", filename);
//                 return;
//             }
//             uintptr_t dw = reinterpret_cast<uintptr_t>(bannerFile);
            
//             if (VirtualProtect((LPVOID)(p1+p2*4), 4, newProtection, &protection)) {
//                 *(DWORD*)(p1+p2*4) = dw ; //esp
//                 LogWithNumber(&k_module,"Escrito puntero = (%d)", dw);
//             }

//         } else {
//             LogWithString(&k_module, "No aplicar Bannerserv. El archivo No existe: : %s", filename);
            
//         }

// 	}

   

//     LOG(&k_module, "ok return");
//     return;
// }


DWORD TEMP_ESI=0;
void HomeReplaceBannerCallPoint() {
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx 
        push esi
        push edi
        push 0
        push ecx
        push eax
        call BannerLoad
        add esp, 0x0c  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov esi,TEMP_ESI
        test esi,esi
        retn
    }
}

void AwayReplaceBannerCallPoint() {
    __asm {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx 
        push esi
        push edi
        push 1
        push edx
        push eax
        call BannerLoad
        add esp, 0x0c  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov esi,TEMP_ESI
        test esi,esi
        retn
    }
}


void BannerLoad(DWORD p1,DWORD p2,DWORD p3) {
    
    LOG(&k_module,"BannerLoad()");

    if (bannerFile) {
        HeapFree(GetProcessHeap(), 0, bannerFile);
        bannerFile = nullptr;
    }
    
    
    char filename[2048];

	WORD homeTeamId = GetTeamId(p3);
    std::string* homeFolderString = MAP_FIND(g_chantsMap,homeTeamId);
	if (homeFolderString != NULL) {
         char* RandomFile = getRandomAdxFile((char*)homeFolderString->c_str());
        sprintf(filename,"%sGDB\\banners\\%s\\%s", GetPESInfo()->gdbDir, (char*)homeFolderString->c_str(), RandomFile);

        
        if (fileExists(filename)) {
            LOG(&k_module,"Banner found TeamId: %d Banner: %s", homeTeamId, filename);
            
            int bannerFileSize=0;
            if (read_file_to_mem(filename,&bannerFile,&bannerFileSize) != 0) {
                LogWithString(&k_module, "Unable to read opd: %s", filename);
                return;
            }
            uintptr_t dw = reinterpret_cast<uintptr_t>(bannerFile);
            TEMP_ESI = dw ; //esp
            LOG(&k_module, "The %d value changed in the pointer", dw);
            return;
        } else {
            LogWithString(&k_module, "Do not apply Bannerserv. The file does not exist.: %s", filename);
            
        }

	}


    TEMP_ESI = *(DWORD*)(p1+p2*4);
    LOG(&k_module, "Default Pointer");
    return;
}


bool fileExists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}




/**
 * COLOR SEGUIDORES
 */

int colorToNumber(std::string color) {
    // Convertir el nombre del color a minúsculas
    std::transform(color.begin(), color.end(), color.begin(), [](unsigned char c){ return std::tolower(c); });

    // LOG(&k_module,"Colot a numero = (%s)", color);
    if (color == "black") return 0;
    if (color == "blue") return 1;
    if (color == "red") return 2;
    if (color == "pink") return 3;
    if (color == "lime_green") return 4;
    if (color == "sky_blue") return 5;
    if (color == "yellow") return 6;
    if (color == "white") return 7;
    if (color == "grey") return 8;
    if (color == "dark_blue") return 9;
    if (color == "maroon") return 10;
    if (color == "purple") return 11;
    if (color == "dark_green") return 12;
    if (color == "gold") return 13;
    if (color == "orange") return 14;
    return -1; // Valor por defecto si el color no es válido
}

BYTE Readcolours(const char* cfgFile) {
    std::ifstream cfg(cfgFile);
    if (!cfg.is_open()) return false;

    std::string line;
    BYTE main_color = -1;
    BYTE secondary_color = -1;

    while (std::getline(cfg, line))
    {
        // Saltar comentarios
        size_t commentPos = line.find("#");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Quitar espacios en blanco al inicio y final
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) continue;

        // Parsear la línea
        size_t eqPos = line.find("=");
        if (eqPos == std::string::npos) continue;

        std::string name = line.substr(0, eqPos);
        std::string valueStr = line.substr(eqPos + 1);

        // Quitar espacios en blanco al inicio y final de name y valueStr
        name.erase(0, name.find_first_not_of(" \t\n\r"));
        name.erase(name.find_last_not_of(" \t\n\r") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);

        int value = colorToNumber(valueStr);
        if (value == -1) {
            value = std::stoi(valueStr);
        }

        if (name == "main.colour")
        {
            main_color = value;
            // std::cout << "ReadConfig: main.colour = " << valueStr << " (" << value << ")" << std::endl;
			LOG(&k_module,"ReadConfig: main.colour = (%d)", value);
        }
        else if (name == "secondary.colour")
        {
            secondary_color = value;
            // std::cout << "ReadConfig: secondary.colour = " << valueStr << " (" << value << ")" << std::endl;
			LOG(&k_module,"ReadConfig: secondary.colour = (%d)", value);
        }

        // Imprimir valores intermedios para depuración
        // std::cout << "Valores intermedios: main_color = " << (int)main_color << ", secondary_color = " << (int)secondary_color << std::endl;
    }

    // Si ambos valores son válidos, actualizar el banner
    if (main_color != -1 && secondary_color != -1) {
        BYTE banner = (secondary_color << 4) | (main_color & 0x0F);
        // std::cout << "Color {" << (int)banner << "}" << std::endl;
        LOG(&k_module,"Color (%d)", banner);
        cfg.close();
        return banner;
    }

    cfg.close();
    return 0; // Devuelve 0 si no se encontraron colores válidos
}



void supp_coloursCallPoint() {
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx 
        push esi
        push edi
        push eax // param_2
        push esi // param_1
        call supp_colours
        add esp, 0x8  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void supp_colours(DWORD param_1,DWORD param_2) {
	LOG(&k_module,"supp_colours");
    // DAT_03b84ba3 = param_1;
    // DAT_03b84ba4 = param_2;

 	char filename[2048];



    WORD homeTeamId = GetTeamId(HOME);
    LogWithNumber(&k_module,"TeamId = (%d)", homeTeamId);
	std::string* homeFolderString = MAP_FIND(g_chantsMap,homeTeamId);
	if (homeFolderString != NULL) {
		sprintf(filename,"%sGDB\\banners\\%s\\colours.txt", GetPESInfo()->gdbDir, (char*)homeFolderString->c_str());
		LogWithString(&k_module, "mapeo archivo local: %s", filename);
		param_1=Readcolours(filename);
	}
    WORD awayTeamId = GetTeamId(AWAY);
    std::string* awayFolderString = MAP_FIND(g_chantsMap,awayTeamId);
    if (awayFolderString != NULL) {
		sprintf(filename,"%sGDB\\banners\\%s\\colours.txt", GetPESInfo()->gdbDir, (char*)awayFolderString->c_str());
		LogWithString(&k_module, "mapeo archivo visita: %s", filename);
		param_2=Readcolours(filename);
	}

	if (VirtualProtect((LPVOID)0x3b84ba3, 2, newProtection, &protection)) {
		*(BYTE*)(0x3b84ba3) = param_1; //estableces puntos ganado goles
		*(BYTE*)(0x3b84ba4) = param_2; //estableces puntos ganado goles
	}
    return;
}
