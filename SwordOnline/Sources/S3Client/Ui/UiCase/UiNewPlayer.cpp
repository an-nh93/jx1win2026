#include "KWin32.h"
#include "KIniFile.h"
#include "../Elem/WndMessage.h"
#include "../Elem/Wnds.h"
#include "UiNewPlayer.h"
#include "UiInformation.h"
#include "UiLoginBg.h"
#include "UiConnectInfo.h"
#include "UiSelNativePlace.h"
#include "UiSelPlayer.h"
#include "../UiBase.h"
#include "../UiShell.h"
#include "../UiSoundSetting.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../core/src/coreshell.h"
#include "../../../Engine/Src/Text.h"
#include "../../../Engine/Src/KDebug.h"
#include <crtdbg.h>

extern iCoreShell *g_pCoreShell;

#define    SCHEME_INI_NEWPLAYER    "UiNewPlayer.ini"
#define    SERIES_INI                "\\Ui\\Series.ini"

//TamLTM check ky tu dat biet
#define CHECK_KEY_KY_TU_DAT_BIET_INPUT_LOGIN    "!@#$%^&*()|?'{ },./;<>"

static const char *s_szSeriesSectionList[series_num] =
        {
                "Gold",
                "Wood",
                "Water",
                "Fire",
                "Earth",
        };

const char *PROP_GetTitleString(OBJ_ATTRIBYTE_TYPE eType) {
    _ASSERT(eType >= series_metal && eType < series_num);
    return (s_szSeriesSectionList[eType]);
}

KUiNewPlayer *KUiNewPlayer::m_pSelf = NULL;

KUiNewPlayer::KUiNewPlayer() {
    memset(&m_Info, 0, sizeof(m_Info));
    m_Info.Attribute = series_metal;
    m_Info.Gender = OBJ_G_MALE;
    m_szLoginBg[0] = 0;
    memset(&m_propTypeInfoTable, 0, sizeof(m_propTypeInfoTable));
    m_propTypeInfoTable[0].pBtn = (KWndWindow *) &m_Gold;
    m_propTypeInfoTable[1].pBtn = (KWndWindow *) &m_Wood;
    m_propTypeInfoTable[2].pBtn = (KWndWindow *) &m_Water;
    m_propTypeInfoTable[3].pBtn = (KWndWindow *) &m_Fire;
    m_propTypeInfoTable[4].pBtn = (KWndWindow *) &m_Earth;
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º´ò¿ª´°¿Ú£¬·µ»ØÎ¨Ò»µÄÒ»¸öÀà¶ÔÏóÊµÀý
//--------------------------------------------------------------------------
KUiNewPlayer *KUiNewPlayer::OpenWindow(int nNativePlaceId) {
    if (m_pSelf == NULL) {
        m_pSelf = new KUiNewPlayer;
        if (m_pSelf)
            m_pSelf->Initialize();
    }
    if (m_pSelf) {
        m_pSelf->m_Info.NativePlaceId = nNativePlaceId;
        UiSoundPlay(UI_SI_POPUP_OUTGAME_WND);
        m_pSelf->UpdateProperty();
        KUiLoginBackGround::SetConfig(m_pSelf->m_szLoginBg);
        m_pSelf->Show();
    }
    return m_pSelf;
}

KUiNewPlayer::~KUiNewPlayer() {
    for (int i = 0; i < series_num; i++)
        m_propTypeInfoTable[i].pBtn = 0;
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º¹Ø±Õ´°¿Ú£¬Í¬Ê±¿ÉÒÔÑ¡ÔòÊÇ·ñÉ¾³ý¶ÔÏóÊµÀý
//--------------------------------------------------------------------------
void KUiNewPlayer::CloseWindow(bool bDestroy) {
    if (m_pSelf) {
        if (bDestroy == false)
            m_pSelf->Hide();
        else {
            m_pSelf->m_Name.Clear(false);
            m_pSelf->Destroy();
            m_pSelf = NULL;
        }
    }
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º³õÊ¼»¯
//--------------------------------------------------------------------------
void KUiNewPlayer::Initialize() {
    AddChild(&m_Male);
    AddChild(&m_Female);
    AddChild(&m_Name);
    AddChild(&m_OK);
    AddChild(&m_Cancel);
    AddChild(&m_Container);
    AddChild(&m_PropertyShow);
    AddChild(&m_Series);
    AddChild(&m_Gold);
    AddChild(&m_Wood);
    AddChild(&m_Water);
    AddChild(&m_Fire);
    AddChild(&m_Earth);

    char Buff[256];
    KIniFile Ini;
    if (Ini.Load(SERIES_INI)) {
        for (int i = 0; i < series_num; i++) {
            Ini.GetString(s_szSeriesSectionList[i], "PropText", "", Buff, sizeof(Buff));
            m_propTypeInfoTable[i].nShowTextLen = strlen(Buff);
            m_propTypeInfoTable[i].nShowTextLen = TEncodeText(Buff, m_propTypeInfoTable[i].nShowTextLen);
            memcpy(m_propTypeInfoTable[i].propertyShow, Buff, m_propTypeInfoTable[i].nShowTextLen);

            Ini.GetString(s_szSeriesSectionList[i], "SelSound_m", "",
                          m_propTypeInfoTable[i].szMaleSound, sizeof(m_propTypeInfoTable[i].szMaleSound));
            Ini.GetString(s_szSeriesSectionList[i], "SelSound_f", "",
                          m_propTypeInfoTable[i].szFemaleSound, sizeof(m_propTypeInfoTable[i].szFemaleSound));

            Ini.GetString(s_szSeriesSectionList[i], "SelSeries", "",
                          m_propTypeInfoTable[i].szSeries, sizeof(m_propTypeInfoTable[i].szSeries));
        }
    }

    g_UiBase.GetCurSchemePath(Buff, 256);
    m_pSelf->LoadScheme(Buff);

    SelGender();

    Wnd_AddWindow(this, WL_TOPMOST);
}

void KUiNewPlayer::SelGender() {
    char szFile[128];
    bool bMale = (m_Info.Gender == OBJ_G_MALE);
    KUiSelPlayer::GetRoleImageName(szFile, m_szPlayerImgPrefix, false, m_Info.Attribute, bMale ? 1 : 2);
    m_Male.CheckButton(bMale);
    m_Male.SetImage(ISI_T_SPR, szFile);
    m_Male.SetFrame(0);

    KUiSelPlayer::GetRoleImageName(szFile, m_szPlayerImgPrefix, true, m_Info.Attribute, bMale ? 2 : 1);
    m_Female.CheckButton(!bMale);
    m_Female.SetImage(ISI_T_SPR, szFile);
    m_Female.SetFrame(0);

    Wnd_SetFocusWnd(&m_Name);
    m_bJustClicked = true;
    UiSoundPlay(m_Info.Gender == OBJ_G_MALE ?
                m_propTypeInfoTable[m_Info.Attribute].szMaleSound :
                m_propTypeInfoTable[m_Info.Attribute].szFemaleSound);
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£ºÔØÈë´°¿ÚµÄ½çÃæ·½°¸
//--------------------------------------------------------------------------
void KUiNewPlayer::LoadScheme(const char *pScheme) {
    char Buff[300];
    KIniFile Ini;
    sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI_NEWPLAYER);
    if (Ini.Load(Buff)) {
        Init(&Ini, "NewPlayer");
        Ini.GetString("NewPlayer", "LoginBg", "", m_szLoginBg, sizeof(m_szLoginBg));
        Ini.GetString("NewPlayer", "PlayerImgPrefix", "", m_szPlayerImgPrefix, sizeof(m_szPlayerImgPrefix));

        m_Male.Init(&Ini, "Male");
        m_Female.Init(&Ini, "Female");
        m_Male.RemoveImage();
        m_Female.RemoveImage();
        m_Name.Init(&Ini, "Name");
        m_OK.Init(&Ini, "OK");
        m_Cancel.Init(&Ini, "Cancel");
        m_Container.Init(&Ini, "Container");
        m_PropertyShow.Init(&Ini, "PropertyShow");
        m_Series.Init(&Ini, "Series");

        for (int i = 0; i < series_num; i++) {
            m_propTypeInfoTable[i].pBtn->Init(&Ini, s_szSeriesSectionList[i]);
            Ini.GetString(s_szSeriesSectionList[i], "MaleImg", "",
                          m_propTypeInfoTable[i].szMaleImg, sizeof(m_propTypeInfoTable[i].szMaleImg));
            Ini.GetString(s_szSeriesSectionList[i], "FemaleImg", "",
                          m_propTypeInfoTable[i].szFemaleImg, sizeof(m_propTypeInfoTable[i].szFemaleImg));
        }
    }
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º´°¿ÚÏûÏ¢º¯Êý
//--------------------------------------------------------------------------
int KUiNewPlayer::WndProc(unsigned int uMsg, unsigned int uParam, int nParam) {
    int nRet = 0;
    switch (uMsg) {
        case WND_N_BUTTON_CLICK:
            OnClickButton((KWndWindow *) uParam);
            break;
        case WM_KEYDOWN:
            if (uParam == VK_ESCAPE) {
                OnCancel();
                nRet = 1;
            } else if (uParam == VK_RETURN) {
                OnOk();
                nRet = 1;
            }
            break;
        default:
            nRet = KWndShowAnimate::WndProc(uMsg, uParam, nParam);
            break;
    }
    return nRet;
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£ºÏìÓ¦µã»÷°´Å¥
//--------------------------------------------------------------------------
void KUiNewPlayer::OnClickButton(KWndWindow *pWnd) {
    if (pWnd == (KWndWindow *) &m_Male) {
        UiSoundPlay(UI_SI_PLAYER_ATTRIB);
        m_Info.Gender = OBJ_G_MALE;
        SelGender();
    } else if (pWnd == (KWndWindow *) &m_Female) {
        UiSoundPlay(UI_SI_PLAYER_ATTRIB);
        m_Info.Gender = OBJ_G_FEMALE;
        SelGender();
    } else if (pWnd == (KWndWindow *) &m_OK)
        OnOk();
    else if (pWnd == (KWndWindow *) &m_Cancel)
        OnCancel();
    else {
        int i;
        for (i = 0; i < series_num; i++) {
            if (pWnd == m_propTypeInfoTable[i].pBtn) {
                m_Info.Attribute = i;
                break;
            }
        }
        if (i < series_num) {
            UiSoundPlay(UI_SI_PLAYER_ATTRIB);
            UpdateProperty();
        }
    }
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º¸üÐÂÊôÐÔËµÃ÷
//--------------------------------------------------------------------------
void KUiNewPlayer::UpdateProperty() {
    for (int i = 0; i < series_num; i++) {
        ((KWndButton *) m_propTypeInfoTable[i].pBtn)->CheckButton(i == m_Info.Attribute);
        if (m_Info.Attribute == i)
            m_Series.SetImage(ISI_T_SPR, m_propTypeInfoTable[i].szSeries);
    }
    if (m_Info.Attribute == series_metal) {
        m_Info.Gender = OBJ_G_MALE;
        SelGender();
        m_Male.Enable(true);
        m_Female.Enable(false);
    } else if (m_Info.Attribute == series_water) {
        m_Info.Gender = OBJ_G_FEMALE;
        SelGender();
        m_Female.Enable(true);
        m_Male.Enable(false);
    } else {
        m_Info.Gender = OBJ_G_MALE;
        SelGender();
        m_Male.Enable(true);
        m_Female.Enable(true);
    }

    m_PropertyShow.SetText(m_propTypeInfoTable[m_Info.Attribute].propertyShow,
                           m_propTypeInfoTable[m_Info.Attribute].nShowTextLen);

    Wnd_SetFocusWnd(&m_Name);
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£ºÍê³É½ÇÉ«Ñ¡Ôñ½çÃæ
//--------------------------------------------------------------------------
void KUiNewPlayer::OnOk() {
    //TamLTM add sleep waint create nhan vat.
    Sleep(2);

    if (GetInputInfo()) {
        //Create nhan vat thanh cong.
        g_LoginLogic.CreateRole(&m_Info);
        //Thong bao dang tao nhan vat hoac xoa nhan vat
        KUiConnectInfo::OpenWindow(CI_MI_CREATING_ROLE, LL_S_IN_GAME, m_Info.NativePlaceId);
        //An cua so
        CloseWindow(false);
    }
}

//--------------------------------------------------------------------------
//	¹¦ÄÜ£º·µ»Ø¡°½ÇÉ«Ñ¡Ôñ½çÃæ¡±
//--------------------------------------------------------------------------
void KUiNewPlayer::OnCancel() {
    CloseWindow(false);
    KUiSelNativePlace::OpenWindow(m_Info.NativePlaceId);
}


void KUiNewPlayer::Breathe() {
    int nLoopBackMale = m_Male.NextFrame();
    int nLoopBackFemale = m_Female.NextFrame();

    if (m_bJustClicked) {
        char szFileName[128];
        KWndImage *pBtn = NULL;
        if ((m_Info.Gender == OBJ_G_MALE) && nLoopBackMale)
            pBtn = &m_Male;
        else if (m_Info.Gender == OBJ_G_FEMALE && nLoopBackFemale)
            pBtn = &m_Female;
        if (pBtn) {
            KUiSelPlayer::GetRoleImageName(szFileName, m_szPlayerImgPrefix, m_Info.Gender, m_Info.Attribute, 0);
            pBtn->SetImage(ISI_T_SPR, szFileName);
            pBtn->SetFrame(0);
            m_bJustClicked = false;
        }
    }
}

//TamLTM Check ky tu dat biet khi dat ten nhan vat
/*
* Cach su dung Ham check ky tu - chuoi ky tu dc kiem tra hop le -> "!@#$"
	if (-1 != CheckKyTuDatBieString( password, "!@#$" ) )
	{
		// Xu lý truong hop ky tu or password hop le
	}
	else
	{
		// ky or password không hop le
	}
*/
int KUiNewPlayer::CheckKyTuDatBieString(char *lpstrBuffer, char *lpstrControl) {
    char *pSet = NULL;
    char *lpstrScan = lpstrBuffer;

    if ((lpstrControl == NULL) || (lpstrBuffer == NULL))
        return -1;

    //Tìm thay ki tu dau tien thì dung lai
    while (*lpstrScan) {
        for (pSet = lpstrControl; *pSet; ++pSet) {
            if (*pSet == *lpstrScan)
                return (int) (lpstrScan - lpstrBuffer);
        }
        lpstrScan++;
    }

    return -1;
}
// End code

#include "../ChatFilter.h"

extern CChatFilter g_ChatFilter;

int KUiNewPlayer::GetInputInfo() {
    int nLen = m_Name.GetText(m_Info.Name, sizeof(m_Info.Name), false);

    int i = 0;
    for (i = 0; i < nLen;) {
        unsigned char cCode = (unsigned char) m_Info.Name[i];
        if (cCode > 0x80)
            i++;
        else if (cCode <= 0x20 || cCode > 0x7e)
            break;
        else
            i++;
    }

    if (!g_ChatFilter.IsTextPass(m_Info.Name))
        i = 0;

    // Check ky tu dat biet here
    if (-1 != CheckKyTuDatBieString(m_Info.Name, CHECK_KEY_KY_TU_DAT_BIET_INPUT_LOGIN)) {
        // Xu lý truong hop ky tu or password hop le
        //g_DebugLog("Xu lý truong hop ky tu or password không hop le %d + %d", i, nLen);
        CloseWindow(false);

        //Show error check ky tu dat biet
        KUiConnectInfo::OpenWindow(CI_MI_INVALID_KYTU_DACBIET_INPUT, CI_NS_NEW_ROLE_WND, m_Info.NativePlaceId);
    } else {
        if (i < nLen) {
            CloseWindow(false);
            //g_DebugLog("i < nLen %d < %d", i < nLen);

            //Neu nLen ma nho hon input hoac ky tu khong hop le co khoang trong' thi return.
            //CI_MI_INVALID_LOGIN_INPUT1 -> Tªn nh©n vËt kh«ng thÕ chøa kho¶ng trèng! = 17
            KUiConnectInfo::OpenWindow(CI_MI_INVALID_LOGIN_INPUT1, CI_NS_NEW_ROLE_WND, m_Info.NativePlaceId);

            return false;
        }

        if (nLen >= LOGIN_ROLE_NAME_MIN_LEN && nLen <= LOGIN_ROLE_NAME_MAX_LEN)
            return true;

        // ky or password không hop le
        //g_DebugLog("ky tu dat biet or password hop le");

        // Close windown khi input hop le
        CloseWindow(false);

        //input phai 6 -> 16 ky tu create nhan vat ko hop le thong bao, id: 18=§é dµi tªn ph¶i tõ 6 ®Õn 16 ký tù.
        KUiConnectInfo::OpenWindow(CI_MI_INVALID_LOGIN_INPUT2, CI_NS_NEW_ROLE_WND, m_Info.NativePlaceId);
    }

    //end code

    return false;
}
