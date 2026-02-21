// D3D_Shell.h
// ???? Direct3D ???????????.
// ??????????CD3D_Shell??????????.

#include "precompile.h"
#include "d3d_shell.h"
#include <algorithm>

CD3D_Shell g_D3DShell;						// The global D3D Shell...
D3DFORMAT g_PixelFormat[3] = {D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X8R8G8B8};

// Create the Sucka...
bool CD3D_Shell::Create()
{
	FreeAll();								// Make sure everything is all clean before we start...

	// ???? D3D ????(????????????????D3D??)...
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D) return false;

	// ????????????...
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_DesktopFormat))) { FreeAll(); return false; }

	// ????????? (??????, ??, ?????)...
	if (!BuildDeviceList()) { FreeAll(); return false; }
	
	return true;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Shell::Reset()
{
	m_pD3D = NULL;

	m_AdapterList.clear();
}

// Frees all the member vars and resets afterwards...
void CD3D_Shell::FreeAll()
{
	if (m_pD3D) {
		uint32 iRefCnt = m_pD3D->Release(); } // assert(iRefCnt==0);

	Reset();
}

// ????????? - ?????????????:
//	1. ?????????????????????? (?????????),
//	2. ?????????????????????????.
//	3. ????????????????????.
//	4. ?????????????????????????...
bool CD3D_Shell::BuildDeviceList()
{
	if (!m_pD3D) return false;
	m_AdapterList.clear();				// Clear the Adapter List First...

    // ?????????????????????? (?????????)
    for (UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); ++iAdapter)
	{
        D3DAdapterInfo AdapterInfo;		// Fill in adapter info
		AdapterInfo.iAdapterNum		= iAdapter;
        m_pD3D->GetAdapterIdentifier(iAdapter,0,&AdapterInfo.AdapterID);
        AdapterInfo.iNumDevices		= 0;
        AdapterInfo.iCurrentDevice	= 0;

		// Enumerate all display modes and formats on this adapter...
        vector<D3DModeInfo>		modes;	
        vector<D3DFORMAT>		formats;
		for(int i=0; i<3; i++)
		{
			uint32 iNumAdapterModes = m_pD3D->GetAdapterModeCount(iAdapter, g_PixelFormat[i]);

			D3DDISPLAYMODE DesktopMode;		// ??????????????????????
			m_pD3D->GetAdapterDisplayMode(iAdapter, &DesktopMode);
			formats.push_back(DesktopMode.Format);

			// ???????????????????????
			for (uint32 iMode = 0; iMode < iNumAdapterModes; iMode++)
			{
				D3DDISPLAYMODE d3dDisplayMode; 
				// ????????????
				m_pD3D->EnumAdapterModes(iAdapter, g_PixelFormat[i],iMode, &d3dDisplayMode);
				D3DModeInfo DisplayMode; 
				DisplayMode.Width	 = d3dDisplayMode.Width; 
				DisplayMode.Height	 = d3dDisplayMode.Height;
				DisplayMode.Format	 = d3dDisplayMode.Format;

				// ?????????????
				if (DisplayMode.Width < 640 || DisplayMode.Height < 480) continue;

				// ??????????????? (????????????????????????)
				uint32 m;
				for (m=0; m<modes.size(); ++m) {
					if ((modes[m].Width == DisplayMode.Width) && (modes[m].Height == DisplayMode.Height) && (modes[m].Format == DisplayMode.Format)) break; }

				// ?????????????µ????????????????
				if (m == modes.size()) 
				{
					// ??????????????????????????????????????????
					uint32 f;
					for (f=0; f<formats.size(); ++f)
					{
						if (DisplayMode.Format == formats[f]) break; 
					}

					if (f==formats.size()) formats.push_back(DisplayMode.Format);

					modes.push_back(DisplayMode); 
				}
			}
		}

        sort(modes.begin(),modes.end());		// ???????????

        // ??????????????????? (?????????????????)
		uint32		iNumDeviceTypes = 2L;
		TCHAR*		strDeviceDescs[] = { "HAL", "REF" };
		D3DDEVTYPE	DeviceTypes[]    = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

        for (UINT iDevice = 0; iDevice < iNumDeviceTypes; ++iDevice)
		{
            D3DDeviceInfo Device;		// Fill in device info
            Device.DeviceType		 = DeviceTypes[iDevice];
            m_pD3D->GetDeviceCaps(iAdapter, DeviceTypes[iDevice], &Device.d3dCaps);
            Device.strDesc			 = strDeviceDescs[iDevice];
            Device.iCurrentMode		 = 0;
            Device.bCanDoWindowed	 = FALSE;
            Device.bWindowed		 = FALSE;
            Device.bStereo			 = FALSE;
            Device.MultiSampleType	 = D3DMULTISAMPLE_NONE;

            // ??????????????, ??????????????????T&L????
			vector<bool> bConfirmedFormats;
			vector<bool> bCanDoHWTnL;

            for (DWORD f=0; f<formats.size(); ++f)
			{
				bool bConfirmed = false; bool bHWTnL = false;

                // ????????????
                if (FAILED(m_pD3D->CheckDeviceType(iAdapter,Device.DeviceType,formats[f],formats[f],FALSE)))
				{
					bConfirmed = false; 
				}
				else
				{
					bConfirmed = true;
					// Confirm the device for HW vertex processing
					if (Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
						bHWTnL = true; 
					else 
						bHWTnL = false;
				}

				bConfirmedFormats.push_back(bConfirmed);
				bCanDoHWTnL.push_back(bHWTnL);
			}

            // ???????????????????????????????????????
            for (uint32 m=0; m<modes.size(); ++m)
			{
                for (uint32 f=0; f<formats.size(); ++f)
				{
                    if (modes[m].Format == formats[f]) 
					{
                        if (bConfirmedFormats[f])
						{
							modes[m].bHWTnL = bCanDoHWTnL[f];
                            Device.Modes.push_back(modes[m]); 
						}
					}
				}
			}

            // ??????800??600??????? (?????????16???)
            uint32 m;
            for (m=0; m<Device.Modes.size(); ++m)
			{
                if (Device.Modes[m].Width==800 && Device.Modes[m].Height==600)
				{
                    Device.iCurrentMode = m;
                    if (Device.Modes[m].Format == D3DFMT_R5G6B5 || Device.Modes[m].Format == D3DFMT_X1R5G5B5 || Device.Modes[m].Format == D3DFMT_A1R5G5B5)
						break;
				}
			}

            // ??????????????????????
            // (???????????????formats[0]??)
            if (bConfirmedFormats[0])
			{
                Device.bCanDoWindowed = TRUE;
                Device.bWindowed      = TRUE;
			}

            // ????????????, ?????????????
			AdapterInfo.Devices.push_back(Device);
		}
    
        // ????????????, ??????????????????
        m_AdapterList.push_back(AdapterInfo); }

    // Return an error if no compatible devices were found
    if (!m_AdapterList.size()) return false;

    return true;
}

// ?????????????ã?????????? (g_Device)...
//	Pick Device Named szDevName if there's one of this name, otherwise, will pick what it thinks is best...
D3DDeviceInfo* CD3D_Shell::PickDefaultDev(D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_REF) continue;
			if (!g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_HAL) continue;
			if (g_bRunWindowed && !itDevice->bWindowed) continue;

			*pAdapterInfo = &(*itAdapter); 
            return &(*itDevice);
		}
	} 
	return NULL;
}

D3DModeInfo* CD3D_Shell::PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth)
{
	for (vector<D3DModeInfo>::iterator itMode = pDeviceInfo->Modes.begin(); itMode != pDeviceInfo->Modes.end(); ++itMode)
	{
		if (itMode->Width  != g_nScreenWidth)  continue;
		if (itMode->Height != g_nScreenHeight) continue;
		if (g_bRunWindowed && itMode->Format != m_DesktopFormat.Format) continue; 
		if (!g_bRunWindowed) {
			switch (iBitDepth) {
			case 32 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 24 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 16 : if (itMode->Format != D3DFMT_R5G6B5   && itMode->Format != D3DFMT_X1R5G5B5) continue; break; } }

		return &(*itMode);
	}
	return NULL;
}

// ???????????????/????????????????
void CD3D_Shell::ListDevices()
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			g_DebugLog("Device: %s", itDevice->strDesc);
		}
	}
}

D3DDeviceInfo* CD3D_Shell::FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (strcmp(strDesc,itDevice->strDesc)==0)
			{
				*pAdapterInfo = &(*itAdapter); return &(*itDevice); 
			}
		}
	}
	return NULL;
}

D3DAdapterInfo*	CD3D_Shell::GetAdapterInfo(unsigned int uAdapterID)
{
	if (uAdapterID < m_AdapterList.size())
		return &m_AdapterList[uAdapterID];
	return NULL;
}

D3DDeviceInfo*	CD3D_Shell::GetDeviceInfo(unsigned int uAdapterID, unsigned int uDeviceID)
{
	D3DAdapterInfo* pAdapter = GetAdapterInfo(uAdapterID);
	if (pAdapter)
	{
		if (uDeviceID < pAdapter->Devices.size())
			return &pAdapter->Devices[uDeviceID];
	}
	return NULL;
}

D3DModeInfo* CD3D_Shell::GetModeInfo(unsigned int uAdapterID,unsigned int uDeviceID,unsigned int uModeID)
{
	D3DDeviceInfo* pDevice = GetDeviceInfo(uAdapterID, uDeviceID);
	if (pDevice)
	{
		if (uModeID < pDevice->Modes.size())
			return &pDevice->Modes[uModeID];
	}
	return NULL;
}