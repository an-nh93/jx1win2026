#include "KCore.h"
#include "KObjSet.h"
#include "KNpcSet.h"
#include "KSubWorld.h"
#include "KNpc.h"
#include "KIniFile.h"
#include "KSubWorldSet.h"
#include "KMissleSet.h"
#include "LuaFuns.h"
#include "KPlayerSet.h"
#include "KPlayer.h"
#include "KPakList.h"
KSubWorldSet g_SubWorldSet;

//TamLTM delay timer them
/*#ifndef _SERVER
unsigned long KSubWorldSet::s_uLastTime = 0L;
float KSubWorldSet::s_fScale = 1.0f;
#endif
//end */

KSubWorldSet::KSubWorldSet()
{
	m_nLoopRate = 0;
	m_MapListCount = 0;
	m_sMapListInfo = NULL;
	m_nGameVersion = ITEM_VERSION;
#ifndef _SERVER
	m_dwPing = 0;
#endif
}

KSubWorldSet::~KSubWorldSet()
{
	if (m_sMapListInfo)
	{
		delete [] m_sMapListInfo;
		m_sMapListInfo = NULL;
	}
}

int KSubWorldSet::SearchWorld(DWORD dwID)
{
	for (int i = 0; i < MAX_SUBWORLD; i++)
	{
		if ((DWORD)SubWorld[i].m_SubWorldID == dwID)
			return i;
	}
	return -1;
}

BOOL KSubWorldSet::Load(LPSTR szFileName)
{
	KIniFile	IniFile;
	char		szKeyName[32];
	int			nWorldID;
	int			nWorldCount = 0;

	IniFile.Load(szFileName);
	IniFile.GetInteger("Init", "Count", 1, &nWorldCount);
	if (nWorldCount > MAX_SUBWORLD)
		return 0;
	for (int i = 0; i < nWorldCount; i++)
	{
		sprintf((char*)szKeyName, "World%03d", i);
		IniFile.GetInteger("World", szKeyName, 1000, &nWorldID);
#ifdef _SERVER
		SubWorld[i].m_nIndex = i;
		if (!SubWorld[i].LoadMap(nWorldID))
		{
			printf("[%03d] Map Loader Failed\n",nWorldID);
		} else {
			printf("[%03d] Map Loader Ok\n",nWorldID);
		}
#endif
	}

//TamLTM Bang hoi chiem linh -> load gia tri bang hoi
#ifdef _SERVER
	try
	{
		KLuaScript * pPlayScript =(KLuaScript*) g_GetScript("\\script\\item\\banghoi\\banghoi.lua");
		if (!pPlayScript)
		{
		}
		else
		{
			int nTopIndex = 0;
			pPlayScript->SafeCallBegin(&nTopIndex);
			pPlayScript->CallFunction("LoadTongMapMain",0,"");
			pPlayScript->SafeCallEnd(nTopIndex);
		}
	}
	catch(...)
	{
		printf("Xay ra loi chay Spcrit dieu khien \\script\\item\\banghoi\\banghoi.lua !!!!!");
	}

#endif
//end code

	return TRUE;
}

BOOL KSubWorldSet::LoadFile()
{
	char		szKeyMap[MAPID_NUM][16] = {"", "City", "Capital", "Cave", "Battlefield", "Field", "Others", "Country", "Tong"};

	KIniFile Ini;
	if (Ini.Load(MAPLIST_SETTING_FILE))
	{
		char szKeyName[32];
		char szMapType[32];

		Ini.GetInteger("List", "Count", 0, &m_MapListCount);
		if (m_MapListCount <= 0)
			return FALSE;

		m_sMapListInfo = (MAPLIST_INFO *)new MAPLIST_INFO[m_MapListCount+1];

		for (int i = 0; i <= m_MapListCount; i++)
		{
			sprintf(szKeyName, "%d_name", i);
			Ini.GetString("List", szKeyName, "", m_sMapListInfo[i].szName, sizeof(m_sMapListInfo[i].szName));
			m_sMapListInfo[i].nKind = MAPID_UNKNOWN;
			sprintf(szKeyName, "%d_MapType", i);
			Ini.GetString("List", szKeyName, "", szMapType, sizeof(szMapType));
			for (int j = 0; j < MAPID_NUM; j++)
			{
				if (strcmp(szMapType, szKeyMap[j]) == 0)
					m_sMapListInfo[i].nKind = j;
			}
		}
	}
	else
		return FALSE;

	return TRUE;
}

int nActiveRegionCount;

void KSubWorldSet::MainLoop()
{
	m_nLoopRate++;

#ifndef _SERVER
	this->m_cMusic.Play(SubWorld[0].m_SubWorldID, SubWorld[0].m_dwCurrentTime, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_FightMode);

#endif
	nActiveRegionCount = 0;
	for (int i = 0; i < MAX_SUBWORLD; i++)
	{
		SubWorld[i].Activate();
#ifndef _SERVER
		NpcSet.CheckBalance();
#endif
	}

//	if ((m_nLoopRate % 100) == 0)
//		printf("Region:%d:%d\n", m_nLoopRate, nActiveRegionCount);
#ifdef _SERVER
	PlayerSet.AutoSave();

//TamLTM them delay timer
/*#else
	NpcSet.CheckBalance();
	static	KTimer	s_Timer;
	unsigned long uTimeNow = s_Timer.GetElapse();
	KSubWorldSet::s_fScale = (float)(uTimeNow - KSubWorldSet::s_uLastTime)/(float)(1000.0/GAME_FPS);
	if(KSubWorldSet::s_fScale > 10.0f)
	{
		KSubWorldSet::s_fScale = 10.0f;
	}
	else if(KSubWorldSet::s_fScale < 0.03f)
	{
		KSubWorldSet::s_fScale = 0.03f;
	}

	KSubWorldSet::s_uLastTime = uTimeNow;
//end */
#endif
}

/// <summary>
/// //////////////////////
/// </summary>
void KSubWorldSet::MessageLoop()
{

	for (int i = 0; i < MAX_SUBWORLD; i++)
	{
		SubWorld[i].MessageLoop();
	}
}

BOOL KSubWorldSet::SendMessage(int nSubWorldID, DWORD dwMsgType, int nParam1, int nParam2, int nParam3)
{
	KWorldMsgNode *pNode = NULL;

	pNode = new KWorldMsgNode;
	if (!pNode)
		return FALSE;

	pNode->m_dwMsgType	= dwMsgType;
	pNode->m_nParam[0]	= nParam1;
	pNode->m_nParam[1]	= nParam2;
	pNode->m_nParam[2]	= nParam3;
	if (pNode->m_dwMsgType == 4001) g_DebugLog("Send !!!!");
	return SubWorld[nSubWorldID].m_WorldMessage.Send(pNode);
}

void KSubWorldSet::Close()
{
	for (int i = 0; i < MAX_SUBWORLD; i++)
	{
		SubWorld[i].Close();
	}
	NpcSet.RemoveAll();
#ifndef _SERVER
	Player[CLIENT_PLAYER_INDEX].m_ItemList.RemoveAll();
	Player[CLIENT_PLAYER_INDEX].m_cTeam.Release();
	g_Team[0].Release();
	m_cMusic.Stop();
#endif
}
#ifndef _SERVER
void KSubWorldSet::Paint()
{
	SubWorld[0].Paint();
}
#endif
//Son fix ket map
#ifdef _SERVER
BOOL KSubWorldSet::GetRevivalPosFromId(DWORD dwSubWorldId, int nRevivalId, POINT* pPos) //TamLTM Fix lai kieu bool
{
	if (!pPos)
		return FALSE;

	KIniFile IniFile;
	char	szKeyName[32];
	char	szSection[32];
	int nX = 0;//51200;
	int nY = 0;//102400;
	g_SetFilePath(SETTING_PATH); //\\settings
	IniFile.Load("RevivePos.ini");
	sprintf(szSection, "%d", dwSubWorldId);
	sprintf(szKeyName, "%d", nRevivalId);

//	printf("%d-%d\n%s-%s\n", dwSubWorldId, nRevivalId, szSection, szKeyName);

	if (nRevivalId < 1)		// luc tao nhan vat
	{
		int nMin = 0;
		int nMax = 0;
		IniFile.GetInteger2(szSection, "region", &nMin, &nMax);	// fix by TamLTM
		sprintf(szKeyName, "%d", nMin);
	}

	IniFile.GetInteger2(szSection, szKeyName, &nX, &nY);
//	printf("%d-%d\n", nX, nY);

	if (nX && nY)
	{
		pPos->x = nX;
		pPos->y = nY;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif
//end code
