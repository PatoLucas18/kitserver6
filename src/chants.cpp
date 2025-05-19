#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "chants.h"
#include "crc32.h"
#include "kload_exp.h"
#include "numpages.h"
#include "input.h"
#include "keycfg.h"

#include <map>
#include <string>



#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

KMOD k_chants={MODID,NAMELONG,NAMESHORT,DEFAULT_DEBUG};
DWORD protection;
DWORD newProtection = PAGE_EXECUTE_READWRITE;
BSERV_CFG bserv_cfg;

static std::map<WORD,std::string*> g_chantsMap;


class config_t 
{
public:
    config_t() : _FixMl(false) {}
    bool _FixMl;
};

config_t _config;

bool homeHasChants = false;
bool awayHasChants = false;

#define MAP_FIND(map,key) map[key]
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
void InitBserv();
void bservBeginUniSelect();
void ChantsAfsReplace(GETFILEINFO* gfi);
bool ReadConfig(config_t& config, const char* cfgFile);
void ReadMap();
void relinkChants();


WORD GetTeamId(int which);


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	char ballCfg[BUFLEN];
	
	if (dwReason == DLL_PROCESS_ATTACH)
	{	
		Log(&k_chants,"Attaching dll...");
		
		switch (GetPESInfo()->GameVersion) {
            case gvPES6PC: //support for PES6 PC
            case gvPES6PC110: //support for PES6 PC 1.10
            case gvWE2007PC: //support for WE:PES2007 PC
				goto GameVersIsOK;
				break;
		};
		//Will land here if game version is not supported
		Log(&k_chants,"Your game version is currently not supported!");
		return false;
		
		//Everything is OK!
		GameVersIsOK:
		
		RegisterKModule(&k_chants);
		

		memcpy(dta, dtaArray[GetPESInfo()->GameVersion], sizeof(dta));
		
	
		
		
		Log(&k_chants, "InitBserv called.");
		string configFile(GetPESInfo()->mydir);
		configFile += "chants.cfg";
		ReadConfig(_config, configFile.c_str());
		
		
		HookFunction(hk_D3D_Create,(DWORD)InitBserv);		
		HookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);   
	    
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Log(&k_chants,"Detaching dll...");
		
	
		UnhookFunction(hk_BeginUniSelect, (DWORD)bservBeginUniSelect);
	
		Log(&k_chants,"Detaching done.");
	};

	return true;
};


void InitBserv()
{
    ReadMap();

	if (_config._FixMl){
		Log(&k_chants, "FIX Chants ML 0x154");
		if (VirtualProtect((LPVOID)dta[ML_CHANT_JMP], 1, newProtection, &protection)) {
			*(BYTE*)(dta[ML_CHANT_JMP]) = 0xEB;
		}	
	}
    RegisterAfsReplaceCallback(ChantsAfsReplace);
    
	UnhookFunction(hk_D3D_Create,(DWORD)InitBserv);

	return;
};







void ReadMap()
{
	//map.txt
	
    char mapFile[BUFLEN];
    ZeroMemory(mapFile,BUFLEN);

    sprintf(mapFile, "%sGDB\\chants\\map.txt", GetPESInfo()->gdbDir);
    FILE* map = fopen(mapFile, "rt");
    if (map == NULL) {
        LOG(&k_chants, "Unable to find chants map: %s", mapFile);
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
            LogWithNumber(&k_chants, "teamId = %d", teamId);
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

                LOG(&k_chants, "foldername = {%s}", foldername);

                // store in the home-ball map
				g_chantsMap[teamId] = new std::string(foldername);
            }
        }
    }
    fclose(map);
	
	return;
};



#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

std::vector<std::string> selectedFiles;

char* getRandomAdxFile(char* directory) {

	char* folderPath = GetPESInfo()->mydir;
	char* subfolder = "GDB\\Chants\\";
	int bufferSize = strlen(folderPath) + strlen(subfolder) + strlen(directory) + 1;
	// Crear un buffer para almacenar la cadena resultante
	char* combinedPath = new char[bufferSize];
	// Copiar folderPath al buffer
	strcpy(combinedPath, folderPath);
	// Concatenar subfolder al buffer
	strcat(combinedPath, subfolder);
	// Concatenar subfolder al buffer
	strcat(combinedPath, directory);
	// LogWithString(&k_chants,"Buscando 2 en: {%s}",combinedPath);
	// // char* randomAdxFile = getRandomAdxFile(combinedPath);
	// LogWithString(&k_chants,"Random .adx file: {%s}", getRandomAdxFile(combinedPath));

    std::vector<std::string> files;
    WIN32_FIND_DATA ffd;
    std::string directoryStr(combinedPath);
    HANDLE hFind = FindFirstFile((directoryStr + "\\*.adx").c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind) {
        return NULL;
    } 

    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(ffd.cFileName);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    // Remove already selected files from the list
    for (std::vector<std::string>::iterator it = selectedFiles.begin(); it != selectedFiles.end(); ++it) {
        files.erase(std::remove(files.begin(), files.end(), *it), files.end());
    }

    if(files.empty()) {
        // If all files have been selected, reset the list of selected files and start over
        selectedFiles.clear();
        return getRandomAdxFile(directory);
    } else {
        srand(time(0));
        std::string randomFile = files[rand() % files.size()];
        char* filename = new char[randomFile.length() + 1];
        strcpy(filename, randomFile.c_str());
        
        // Add the selected file to the list of selected files
        selectedFiles.push_back(randomFile);
        
        return filename;
    }
}
void ChantsAfsReplace(GETFILEINFO* gfi)
{
	if (gfi->isProcessed) return;
	
	DWORD afsId = 0, fileId = 0;	
	char filename[BUFLEN];
    ZeroMemory(filename,BUFLEN);
	fileId = splitFileId(gfi->fileId, &afsId);

	// ensure it is x_TEXT.afs
	if (afsId == AFS_FILE) {
		if (fileId >= HOME_CHANT_ID && fileId < HOME_CHANT_ID + TOTAL_CHANTS && homeHasChants){
		// if (fileId >= 315 && fileId <= 320) {
			WORD homeTeamId = GetTeamId(HOME);
			std::string* homeFolderString = MAP_FIND(g_chantsMap,homeTeamId);
			if (homeFolderString != NULL) {
    			// sprintf(filename,"%sGDB\\chants\\%s\\%s", GetPESInfo()->gdbDir, (char*)homeFolderString->c_str(), FILE_NAMES[fileId - HOME_CHANT_ID]);
				sprintf(filename,"%sGDB\\chants\\%s\\%s", GetPESInfo()->gdbDir, (char*)homeFolderString->c_str(), getRandomAdxFile((char*)homeFolderString->c_str()));
			}
		}
		if (fileId >= AWAY_CHANT_ID && fileId < AWAY_CHANT_ID + TOTAL_CHANTS && awayHasChants) {
		// if (fileId >= AWAY_CHANT_ID && fileId <= (AWAY_CHANT_ID+4)) {
			WORD awayTeamId = GetTeamId(AWAY);
			std::string* awayFolderString = MAP_FIND(g_chantsMap,awayTeamId);
			if (awayFolderString != NULL) {
				// sprintf(filename,"%sGDB\\chants\\%s\\%s", GetPESInfo()->gdbDir, (char*)awayFolderString->c_str(), FILE_NAMES[fileId - AWAY_CHANT_ID]);
				sprintf(filename,"%sGDB\\chants\\%s\\%s", GetPESInfo()->gdbDir, (char*)awayFolderString->c_str(), getRandomAdxFile((char*)awayFolderString->c_str()));
			}
		}
		
		if (strlen(filename)>0) {
			loadReplaceFile(filename);
			gfi->isProcessed = true;
		}
	
	}

	return;
}


void bservBeginUniSelect()
{
		Log(&k_chants, "chantsBeginUniSelect");

		// check if home and away team has a chants designated in map.txt
		WORD homeTeamId = GetTeamId(HOME);
		WORD awayTeamId = GetTeamId(AWAY);
		std::string* homeFolderString = MAP_FIND(g_chantsMap,homeTeamId);
		std::string* awayFolderString = MAP_FIND(g_chantsMap,awayTeamId);
		if (homeFolderString != NULL) {
			homeHasChants = true;
		}
		else{
			homeHasChants = false;
		}
		if (awayFolderString != NULL) {
			awayHasChants = true;
		}
		else{
			awayHasChants = false;
		}
		relinkChants();
		return;


}


void relinkChants()
{
	// WORD homeTeamId = GetTeamId(HOME);
	// WORD awayTeamId = GetTeamId(AWAY);

	if (_config._FixMl){
		Log(&k_chants, "FIX Chants ML");
		WORD my_team = *(WORD*)dta[MY_TEAM_ADDRESS]; //my team ml
		DWORD chant_team = *(DWORD*)(my_team*4 + dta[CHANTS_ADDRESS]); //home team

		if (VirtualProtect((LPVOID)dta[ML_CHANT_HOME_ADDRESS], 8, newProtection, &protection)) {
				*(DWORD*)(dta[ML_CHANT_HOME_ADDRESS]) = chant_team;
				*(DWORD*)(4+dta[ML_CHANT_HOME_ADDRESS]) = chant_team;
		}
		LOG(&k_chants, "FIX Chants Team %d - chant %d",my_team,chant_team);


		// if (my_team == homeTeamId) {
			
		// 	LOG(&k_chants, "FIX Chants HOME %d - %d",homeTeamId,chant_team);
		// }

		// if (my_team == awayTeamId) {
			
		// 	LOG(&k_chants, "FIX Chants AWAY %d - %d",awayTeamId,chant_team);
		// }

			
	}

	// if (VirtualProtect((LPVOID)dta[CHANTS_ADDRESS], dta[TOTAL_TEAMS] * 4, newProtection, &protection)) {
	// 	if (homeHasChants){
			
	// 		*(WORD*)(dta[CHANTS_ADDRESS] + homeTeamId * 4) = HOME_CHANT_ID;
	// 		*(WORD*)(dta[CHANTS_ADDRESS] + homeTeamId * 4 + 2) = AFS_FILE;
	// 	}
	// 	if (awayHasChants){
	// 		*(WORD*)(dta[CHANTS_ADDRESS] + awayTeamId * 4) = AWAY_CHANT_ID;
	// 		*(WORD*)(dta[CHANTS_ADDRESS] + awayTeamId * 4 + 2) = AFS_FILE;
	// 	}
	// };



}


/**
 * Returns true if successful.
 */
bool ReadConfig(config_t& config, const char* cfgFile)
{
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
			LogWithNumber(&k_chants,"ReadConfig: debug = (%d)", value);
            k_chants.debug = value;
        }
        else if (strcmp(name, "chant.my.team.ml")==0)
        {
			if (sscanf(pValue, "%d", &value)!=1) continue;
			LogWithNumber(&k_chants,"ReadConfig: chant.my.team.ml = (%d)", value);
            int otraVariable = value; // Esta puede ser 1 o 0
            _config._FixMl = (otraVariable == 1);
            // _config._FixMl = max(value, 1);
        }
	}
	fclose(cfg);
	return true;
}
/**
 * Return currently selected (home or away) team ID.
 */
WORD GetTeamId(int which)
{
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
