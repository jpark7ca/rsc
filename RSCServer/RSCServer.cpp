// 
// RSCServer.cpp : the entry point for the RSCServer application.
// By Jae H. Park 
//
// Created: 02.04.2005
// Rev. 03.01.2005, 04.01.2005, 07.01.2005, 08.01.2005, 09.11.2005
// 10.20.2005, 07.12.2006, 07.30.2006, 10.02.2006
//

#include "stdafx.h"

// Global Variables and Constants
//

#define MAX_LOADSTRING 100
#define MAX_PASSWORD 8
#define	MAX_IPADDRESS 6
#define PIXTANGLE_WIDTH	64
#define PIXTANGLE_HEIGHT 64
#define THREAD_TIME_DELAY 1 //500	// miliseconds
#define DEFAULT_COMM_PORT 3337
#define MAX_PASSWD_RETRY 3

#define MAX_CLIENTCONNECT 20	// 2 for one RSC session

typedef struct RSC_PROP{
	// RSC Properties
	BYTE m_prop_password[MAX_PASSWORD];
	BYTE m_prop_server_ipaddr[MAX_IPADDRESS];
	u_short m_prop_server_port;
	u_short m_prop_needpassword;			// 0-False	1-True
	u_short m_prop_needuserperm;			// 0-False	1-True
	u_short m_prop_removewallpaper;			// 0-False	1-True
	u_short m_prop_disable_keyboard_mouse;	// 0-False	1-True
	
	// Main Window Handler
	HWND m_hwnd;
}RSC_PROP;

typedef struct RSC_THREAD_PROP{
	HANDLE thread;
	RSC_PROP prop;
	SOCKET socket;
}RSC_THREAD_PROP;

typedef struct RSC_MOUSE_EVENT{
	RSC_BYTE lbtn, mbtn, rbtn;
	RSC_BYTE wheel1, wheel2;
	RSC_UWORD sx, sy;
}RSC_MOUSE_EVENT;

typedef struct RSC_DECRYPTED{
	RSCCrypt dec;
	unsigned char *response;
}RSC_DECRYPTED;

// Define TrayIcon Contants
#define NIIF_NONE 0
#define	WM_ICON_NOTIFY WM_APP+10
#define	WM_ICON_EXIT WM_APP+11

// Define TrayIcon Variables
NOTIFYICONDATA	m_tnd;
HICON			m_icon;
HICON			m_icon_inv;
HWND            m_hWnd;
HINSTANCE		m_hInstance;

// TrayIcon Declarations of Functions
LRESULT OnTrayNotification(UINT wParam, LONG lParam);
BOOL InitializeTrayIcon(HINSTANCE);
BOOL AddTrayIcon();
BOOL ModifyTrayIcon();
BOOL RemoveTrayIcon();

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Global Windows Declarations of Functions
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	PropProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	TimeoutProc(HWND, UINT, WPARAM, LPARAM);

// RSC Application Variables and Functions
RSC_PROP m_prop;
HWND m_progbar;
int m_progtm;

//HANDLE password_cs - Critical section
CRITICAL_SECTION userperm_cs;

void LoadProperties(void);
bool RunServer(void);
VOID run_server(RSC_PROP *param);
VOID talk2viewer(RSC_THREAD_PROP *param);

//
// Worker Thread Functions
//

// bool wv_init(RSCSocket *socket, RSCRegion *region, RSC_REGIONPROP *prop, bool *isinput);
// bool wv_newscreen(RSCSocket *socket, RSCRegion region[], BYTE *cur_region, RSC_REGIONPROP *prop);
// bool wv_compare_n_update_conv(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, RSC_REGIONPROP *prop, RSCRegion *region);
// bool wv_compare_n_update_conv(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, RSC_REGIONPROP *prop, RSCRegion *region, RSCPalette *pal);

bool wv_init(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_REGIONPROP *prop, bool *isinput);
bool wv_version(RSCSocket *socket);
bool wv_authenticate(RSCSocket *socket);
bool wv_newscreen(RSCSocket *socket, RSCPalette *pal, RSCRegion region[], BYTE *cur_region, RSC_REGIONPROP *prop);
bool wv_compare_n_update_conv_optimized(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, RSC_REGIONPROP *prop, RSCRegion *region, RSCPalette *pal);
bool wv_receivedkey(RSCSocket *socket);
bool wv_receivemouse(RSCSocket *socket, RSCRegion *region, RSC_MOUSE_EVENT *mouseevent);

// Message Registrations
const UINT WM_TASKBARCREATED =  ::RegisterWindowMessage("TaskbarCreated");


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RSCSERVER, szWindowClass, MAX_LOADSTRING);
	
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	
	// Application Main Entry Point
	LoadProperties();
	RunServer();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= 0;
	wcex.hbrBackground	= 0;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= 0;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	// Icon loadings
	m_icon = ::LoadIcon(hInstance, (LPCTSTR)IDI_RSCSERVER);
	m_icon_inv = ::LoadIcon(hInstance, (LPCTSTR)IDI_RSCSERVER_INV);

	m_hWnd = ::CreateWindow(szWindowClass, "", WS_POPUP, 
                            CW_USEDEFAULT,CW_USEDEFAULT, 
                            CW_USEDEFAULT,CW_USEDEFAULT, 
                            NULL, 0,
                            hInst, 0);

    // load up the NOTIFYICONDATA structure
    m_tnd.cbSize = sizeof(NOTIFYICONDATA);
    m_tnd.hWnd   = m_hWnd;
    m_tnd.uID    = IDC_RSCSERVER;
    m_tnd.hIcon  = m_icon;
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON;
    m_tnd.uCallbackMessage = WM_ICON_NOTIFY;
  
    BOOL bResult = TRUE;
	bResult = AddTrayIcon();
	
    return bResult;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	int wmId, wmEvent;
	u_short tmp_port;

	if(message == WM_TASKBARCREATED){
		AddTrayIcon();
	}

	switch (message) 
	{
		case WM_ICON_EXIT:
			DestroyWindow(hWnd);
			break;

		case WM_ICON_NOTIFY:
			return OnTrayNotification((UINT) wParam, (LONG) lParam);

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			// Parse the menu selections:
			switch (wmId)
			{
				case ID_POPMENU_ABOUT:					
					// Example ...
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
					break;
				case ID_POPMENU_PROPERTIES:

					tmp_port = m_prop.m_prop_server_port;

					if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROP), m_hWnd, (DLGPROC) PropProc)==IDOK){
						// Complete the command; szItemName contains the 
						// name of the item to delete.
						
						if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "server_port", m_prop.m_prop_server_port)){
							// Error while writing to Registry
							// MessageBox(NULL, "Error has occured while writing PORT registry value", "RSC Server", MB_OK);
						}

						if(!RegSetBinValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "password", m_prop.m_prop_password, MAX_PASSWORD)){
							// Error while writing to Registry
							// MessageBox(NULL, "Error has occured while writing password registry value", "RSC Server", MB_OK);
						}

						if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "needuserperm", m_prop.m_prop_needuserperm)){
							// Error while writing to Registry
							// MessageBox(NULL, "Error has occured while writing needuserperm registry value", "RSC Server ", MB_OK);
						}

						if(m_prop.m_prop_server_port != tmp_port){
							MessageBox(NULL, "Due to changing TCP/IP port, you need to restart the RSC server. This action will close the application", "RSC Server", MB_OK | MB_ICONINFORMATION);
							DestroyWindow(hWnd);
						}

					}else{
						// Cancel the command.
						
					} 
					break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_PAINT:
			// Not implemented
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			RemoveTrayIcon();
			break;
		
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK TimeoutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDesk; 
	RECT rc, rcDlg, rcDesk;

	switch (message)
	{
	case WM_INITDIALOG:
		
		//////////////////////////////////////////////////////////////
		// Center and Z-Order the dialog box
		//
		
		SetActiveWindow(hDlg);

		hDesk = GetDesktopWindow();

		GetWindowRect(hDesk, &rcDesk);
		GetWindowRect(hDlg, &rcDlg);
		CopyRect(&rc, &rcDesk);

		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
		OffsetRect(&rc, -rc.left, -rc.top); 
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

		SetWindowPos(hDlg, 
                 HWND_TOPMOST, 
                 rcDesk.left + (rc.right / 2), 
                 rcDesk.top + (rc.bottom / 2), 
                 0, 0,          // Ignores size arguments. 
                 SWP_NOSIZE); 
		
		
		//
		// Bring it to the foreground HACK by Nishant Sivakumar
		// (http://www.codeproject.com/dialog/dlgboxtricks.asp?df=100&forumid=3124&exp=0&select=1293035)
		//
		//
		AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(),TRUE);
		SetForegroundWindow(hDlg);
		SetFocus(hDlg); // to play safe
		AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(),FALSE);

		//////////////////////////////////////////////////////////////

		m_progtm = 0;
		m_progbar = GetDlgItem(hDlg, IDC_PROGRESS_BAR); 
		SendMessage(m_progbar, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
		SendMessage(m_progbar, PBM_SETSTEP, (WPARAM) 1, 0); 

		SetTimer(hDlg, IDT_TIMER_ACCEPT, 1000, (TIMERPROC) NULL);  
		
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, IDCANCEL);			
			
			return TRUE;
		}
		break;
	case WM_TIMER:  
		switch (wParam) 
		{ 
			case IDT_TIMER_ACCEPT: 
				// process the 1-second timer 
				if(m_progtm < 10){
					m_progtm++;
					SendMessage(m_progbar, PBM_STEPIT, 0, 0); 
				}else{
					EndDialog(hDlg, IDCANCEL);
				}
	 
				return FALSE; 
		}
		break;

	case WM_DESTROY:
		KillTimer(hDlg, IDT_TIMER_ACCEPT);
		break;

	}
	return FALSE;

}

BOOL AddTrayIcon()
{
	RemoveTrayIcon();
	return Shell_NotifyIcon(NIM_ADD, &m_tnd);
}

BOOL ModifyTrayIcon()
{
	return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}


BOOL RemoveTrayIcon()
{
	return Shell_NotifyIcon(NIM_DELETE, &m_tnd);
}

LRESULT OnTrayNotification(UINT wParam, LONG lParam) 
{
    if (wParam != m_tnd.uID)
        return 0L;
    
    if (LOWORD(lParam) == WM_RBUTTONUP)
    {    
        HMENU hMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(m_tnd.uID));
        if (!hMenu)
            return 0;

        HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
        if (!hSubMenu)
            return 0;

        POINT pos;
        GetCursorPos(&pos);

        ::SetForegroundWindow(m_tnd.hWnd);  
		::TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, m_hWnd, NULL);

		::PostMessage(m_tnd.hWnd, WM_NULL, 0, 0);

        DestroyMenu(hMenu);
    }else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
    
	}
    return 1;
}

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK PropProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    switch (message) 
    { 
		case WM_INITDIALOG:
			{
				// Default Value:
				BYTE buf[10], *buf1;
				itoa((int) m_prop.m_prop_server_port, (char *) buf, 10);
				SetDlgItemText(hDlg, IDC_EDIT_PROP_PORT,(LPSTR) buf);

				if(m_prop.m_prop_password[0] != NULL){ 
					RSCCrypt dec;
					
					dec.RSCLoadEncrytped(m_prop.m_prop_password, MAX_PASSWORD, MAX_PASSWORD);
					buf1 = dec.RSCDecryptByteStream();
					memcpy(buf, buf1, 8);
					buf[8] = NULL;
					dec.RSCFreeMemory(buf1);
				}else{
					buf[0] = NULL;
				}
				SetDlgItemText(hDlg, IDC_EDIT_PROP_PASSWORD, (LPSTR) buf);
				
				if(m_prop.m_prop_needuserperm == 1)
					CheckDlgButton(hDlg, IDC_CHECK_PROMPT_INCOMING_CONN, BST_CHECKED);
				else
					CheckDlgButton(hDlg, IDC_CHECK_PROMPT_INCOMING_CONN, BST_UNCHECKED);

				
			}
			return TRUE;
			break;
		
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK:
					BYTE buf[10];

					if(GetDlgItemText(hDlg, IDC_EDIT_PROP_PORT, (LPSTR) buf, 10))
						m_prop.m_prop_server_port = atoi((const char *) buf);

					memset(buf, 0, 10);
					if(GetDlgItemText(hDlg, IDC_EDIT_PROP_PASSWORD, (LPSTR) buf, MAX_PASSWORD+1)){
						RSCCrypt enc;
						if(!enc.RSCEncryptByteStream(buf, MAX_PASSWORD)){
							EndDialog(hDlg, wParam); 
							return TRUE; 
						}
						memcpy(m_prop.m_prop_password, enc.enc.text, MAX_PASSWORD);
					}

					if(IsDlgButtonChecked(hDlg, IDC_CHECK_PROMPT_INCOMING_CONN) == BST_CHECKED){
						m_prop.m_prop_needuserperm = 1;
					}else{
						m_prop.m_prop_needuserperm = 0;
					}
		 					
                case IDCANCEL: 
                    EndDialog(hDlg, wParam); 
                    return TRUE; 
            } 
    } 
    return FALSE; 
} 

void LoadProperties(void)
{
	// Check the value of password registry key
	int datalen;
	bool dialogbox = false;

	if(!RegGetBinValue(HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "password", m_prop.m_prop_password, MAX_PASSWORD, &datalen)){
		
		dialogbox = true;
		m_prop.m_prop_password[0] = NULL;
	
	}else{

		if(datalen < MAX_PASSWORD){
			dialogbox = true;
			m_prop.m_prop_password[0] = NULL;
		}

	}
	
	m_prop.m_prop_server_port = DEFAULT_COMM_PORT;

	if(!RegGetDWORDValue(HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "server_port", (DWORD *) &m_prop.m_prop_server_port)){
		dialogbox = true;
	}

	if(!RegGetDWORDValue(HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "needuserperm", (DWORD *) &m_prop.m_prop_needuserperm)){
		m_prop.m_prop_needuserperm = 0;
		if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "needuserperm", m_prop.m_prop_needuserperm)){
			// Error while writing to Registry
			// MessageBox(NULL, "Error has occured while writing needuserperm registry value", "RSC Server ", MB_OK);
		}
	}

	if(dialogbox){
		if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROP), m_hWnd, (DLGPROC) PropProc)==IDOK){
				// Complete the command; szItemName contains the 
				// name of the item to delete.
				
			if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "server_port", m_prop.m_prop_server_port)){
				// Error while writing to Registry
				// MessageBox(NULL, "Error has occured while writing PORT registry value", "RSC Server ", MB_OK);
			}

			if(!RegSetBinValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "password", m_prop.m_prop_password, MAX_PASSWORD)){
				// Error while writing to Registry
				// MessageBox(NULL, "Error has occured while writing password registry value", "RSC Server", MB_OK);
			}

			if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Server\\", "needuserperm", m_prop.m_prop_needuserperm)){
				// Error while writing to Registry
				// MessageBox(NULL, "Error has occured while writing needuserperm registry value", "RSC Server ", MB_OK);
			}

		}else{
			// Cancel the command.
			MessageBox(NULL, "You need to initialize the IP address and TCP/IP port", "RSC Server", MB_OK | MB_ICONINFORMATION);
			DestroyWindow(m_hWnd);
				
		} 
	}

	//
	// TO DO LATER
	//
	//m_prop.m_prop_needpassword = false;
	//m_prop.m_prop_needuserperm = false;
	//m_prop.m_prop_removewallpaper = false;
	//m_prop.m_prop_disable_keyboard_mouse = false;
	m_prop.m_hwnd = m_hWnd;

}

bool RunServer(void)
{
	HANDLE server_t;
	DWORD server_id;

	// Init Critical Section variable 
	InitializeCriticalSection(&userperm_cs);

	server_t = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) run_server, &m_prop, 0, &server_id);
	
	if(server_id == -1){
		// return error
		return false;
	}

	return true;
}

VOID run_server(RSC_PROP *param)
{
	RSCSocket server_s;
	SOCKET accepted_s;
	HANDLE worker_t;
	DWORD worker_id;
	RSC_PROP rsc_prop = *param;
	int cur_thread=0;
	RSC_THREAD_PROP worker_p[MAX_CLIENTCONNECT];

	// Init worker_p with thread member assigned with NULL 
	for(int i=0; i<MAX_CLIENTCONNECT; i++){
		worker_p[i].thread = NULL;
	}


	if(!server_s.RSCSocketInit_W()){
		// Error occurred while initializing the win32 socket
		//MessageBox(NULL, "An error has occurred while initializing Win32 Socket", "RSC Server", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//if(!server_s.RSCCreateStream(false)){
	if(!server_s.RSCCreateStream(true)){
		// Error occurred while creating the server socket
		server_s.RSCSocketUninst_W();
		//MessageBox(NULL, "An error has occurred while creating Win32 Socket", "RSC Server", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//if(!server_s.RSCBind(rsc_prop.m_prop_server_port, rsc_prop.m_prop_server_ipaddr)){
	if(!server_s.RSCBind(rsc_prop.m_prop_server_port, NULL)){  
		//Error occurred while binding the server socket to the ip and port
		server_s.RSCClose();
		server_s.RSCSocketUninst_W();
		//MessageBox(NULL, "An error has occurred while binding Win32 Socket", "RSC Server", MB_OK | MB_ICONINFORMATION);
		return;
	}

	if(!server_s.RSCListen(5000)){
		// Error occurred while listening the socket
		server_s.RSCClose();
		server_s.RSCSocketUninst_W();
		// MessageBox(NULL, "An error has occurred in Win32 Socket Listen function", "RSC Server", MB_OK | MB_ICONINFORMATION);
		return;
	}
		
	while(true){
		// Waiting for clients to connect ...
		accepted_s = server_s.RSCAccept();
		if(accepted_s == INVALID_SOCKET){
			// An error has occured
			// Accepted client socket is not valid
		}else{
			// Reset the value of cur_thread (-1) 
			cur_thread = -1;
			
			//  Assign a thread if there is an available connection slot
			for(int i=0; i<MAX_CLIENTCONNECT; i++){
				if(worker_p[i].thread == NULL){
					cur_thread = i;
					worker_p[cur_thread].prop = rsc_prop;
					worker_p[cur_thread].socket = accepted_s;
					i = MAX_CLIENTCONNECT;
				}
			}
			
			// Check whether the current connection is assigned to a worker thread or not 
			if(cur_thread != -1){
				//worker_t = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) talk2viewer, &(worker_p[cur_thread]), 0, &worker_id);
				worker_t = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) talk2viewer, &(worker_p[cur_thread]), CREATE_SUSPENDED, &worker_id);
				
				if(worker_id == -1){
					// 
					// An error has occurred while creating a worker thread
					
					if(closesocket(accepted_s) == SOCKET_ERROR){
						//
						// An error has occurred while closing the socket
					}
				}
				// Set Thread Priority to the lowest
				SetThreadPriority(worker_t, THREAD_PRIORITY_NORMAL);

				// Resume the thread
				ResumeThread(worker_t);


			}else{
				// Can't allocate a space for new connection thread
				if(closesocket(accepted_s) == SOCKET_ERROR){
					// 
					// An error has occurred while closing the socket
				}
			}

		}
	}
}

VOID talk2viewer(RSC_THREAD_PROP *param)
{

	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZE LOCAL VARIABLES AND CLASSES
	///////////////////////////////////////////////////////////////////////////////

	// Socket related
	RSCSocket session(param->socket);
	bool isconnected = true;
	
	RSCRegion region[2];
	// Sample Code
	//RSCRegion region[2] = {RSCRegion(800,600), RSCRegion(800, 600)};	
	BYTE cur_region;
	RSCPalette pal;

	// Screen related
	//
	RSC_PROP prop = param->prop; // RSC Server's global properties
	RSC_REGIONPROP client_prop;	 // RSCClient's requested properties

	// Input (keyboard & mouse) related
	RSC_MOUSE_EVENT mouseevent;
	bool isinput = false;

	// Thread related
	HANDLE cur_thread;
	
	// Others
	RSC_BYTE identifier;
	//DWORD delay_tick, delay_elapsed;


	///////////////////////////////////////////////////////////////////////////////
	// MOUSE EVENT INIT
	///////////////////////////////////////////////////////////////////////////////

	mouseevent.lbtn = 0;
	mouseevent.mbtn = 0;
	mouseevent.rbtn = 0;
	mouseevent.sx = 0;
	mouseevent.sy = 0;
	mouseevent.wheel1 = 0;
	mouseevent.wheel2 = 0;

	///////////////////////////////////////////////////////////////////////////////
	// CURRENT THEREAD INIT
	///////////////////////////////////////////////////////////////////////////////

	cur_thread = GetCurrentThread();
	param->thread = cur_thread;

	cur_region = 0;

	///////////////////////////////////////////////////////////////////////////////
	// VERSION CHECK AND AUTHENTICATION 
	///////////////////////////////////////////////////////////////////////////////

	if(!wv_authenticate(&session)){
		param->thread = NULL;
		session.RSCShutdown(SD_BOTH);
		session.RSCClose();

		// Update Icon
		m_tnd.hIcon = m_icon;
		ModifyTrayIcon();

		return;
		// Debug
		//MessageBox(NULL, "Failed during wv_authenticate()", "DEBUG", MB_OK);
	}

	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION BETWEEN CLIENT AND SERVER
	///////////////////////////////////////////////////////////////////////////////

	//if(!wv_init(&session, &region[cur_region], &client_prop, &isinput)){
	if(!wv_init(&session, &region[cur_region], &pal, &client_prop, &isinput)){
		param->thread = NULL;
		session.RSCShutdown(SD_BOTH);
		session.RSCClose();

		// Update Icon
		m_tnd.hIcon = m_icon;
		ModifyTrayIcon();

		return;
		// Debug
		//MessageBox(NULL, "Failed during wv_init()", "DEBUG", MB_OK);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Ask for user permission - only for screen thread
	///////////////////////////////////////////////////////////////////////////////
	if(!isinput){
		if(m_prop.m_prop_needuserperm == 1){
			// Prompt a dialog box
				
			// Enter the critical section
			EnterCriticalSection(&userperm_cs);
				
			InitCommonControls(); 
			//if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_ACCEPT), m_hWnd, (DLGPROC)TimeoutProc) != IDOK){
			if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_ACCEPT), NULL, (DLGPROC)TimeoutProc) != IDOK){
				
				// Leave the critical section
				LeaveCriticalSection(&userperm_cs);

				param->thread = NULL;
				session.RSCShutdown(SD_BOTH);
				session.RSCClose();

				// Update Icon
				m_tnd.hIcon = m_icon;
				ModifyTrayIcon();

				return;	
			}

			// Leave the critical section
			LeaveCriticalSection(&userperm_cs);	

		}
	}

	// Update Icon
	m_tnd.hIcon = m_icon_inv;
	ModifyTrayIcon();

	///////////////////////////////////////////////////////////////////////////////
	// Adjust the screen size
	///////////////////////////////////////////////////////////////////////////////
	
	region[0].RSCResetRegion(client_prop.region_x, client_prop.region_y, client_prop.region_w, client_prop.region_h);
	region[1].RSCResetRegion(client_prop.region_x, client_prop.region_y, client_prop.region_w, client_prop.region_h);

	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZE DIB INFORMATION
	///////////////////////////////////////////////////////////////////////////////

	if(!isinput){
		// Retrieve DDB information
		if(!region[0].RSCCapture()){
			// Failed to capture the region
			param->thread = NULL;
			session.RSCShutdown(SD_BOTH);
			session.RSCClose();	

			// Update Icon
			m_tnd.hIcon = m_icon;
			ModifyTrayIcon();

			return;
		}	
		if(!region[1].RSCCapture()){
			// Failed to capture the region
			param->thread = NULL;
			session.RSCShutdown(SD_BOTH);
			session.RSCClose();

			// Update Icon
			m_tnd.hIcon = m_icon;
			ModifyTrayIcon();

			return;
		}

		// Configure initial DIB informaiton

		if(region[0].m_regionprop.bpp == 8){
			// If 8-bits, use Palette	

			HPALETTE syspal=NULL, oldpal=NULL;
			
			syspal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
			//pal.SetRGBIndex(region[0].m_hdc, RSC_PALETTE_HALFTONE);
			//syspal = pal.m_palette;
			
			oldpal = SelectPalette(region[0].m_hdc, syspal, FALSE);
			
			region[0].RSCInitDIB(syspal, region[0].m_regionprop.bpp, BI_RGB);
			region[1].RSCInitDIB(syspal, region[1].m_regionprop.bpp, BI_RGB);
			
			SelectPalette(region[0].m_hdc, oldpal, TRUE);
			DeleteObject(syspal);
			
		}else{
			// If 15/16/24/32 bits, use RGB instead of Palette
			region[0].RSCInitDIB(NULL, region[0].m_regionprop.bpp, BI_BITFIELDS);
			region[1].RSCInitDIB(NULL, region[1].m_regionprop.bpp, BI_BITFIELDS);
		}

	}
	// Debug - Find out the size of DIB
	//TCHAR dbuf[256];
	//wsprintf(dbuf, "%d", region[0].m_dibinfo.header.biSizeImage);
	//MessageBox(NULL, dbuf, "Debug", MB_OK);

	///////////////////////////////////////////////////////////////////////////////	
	// MAIN LOOP BEGINS HERE
	///////////////////////////////////////////////////////////////////////////////	
	//SetThreadPriority(cur_thread, THREAD_PRIORITY_LOWEST);

	while(isconnected){

		if(session.RSCReceive(&identifier, 1)){
		
			switch(identifier){
				
				case 1:
					break;
				case 2:
					break;
				case 3:
					// Check the screen DC change

					if(region[cur_region].RSCIsScreenChanged()){
						
						//
							// Debug
							//
							// If yes, then change the region information
							//
							// if(!region[cur_region].RSCResetRegion()){
							//	 param->thread = NULL;
							//	 session.RSCShutdown(SD_BOTH);
							//	 session.RSCClose();
							// }
							//

						// Disconnect the client	
						isconnected = false;
						break;
					}

					// Set the thread priority high
					SetThreadPriority(cur_thread, THREAD_PRIORITY_HIGHEST);
					
					if(!wv_newscreen(&session, &pal, region, &cur_region, &client_prop)){				
						isconnected = false;
					}
					
					//Set the thread priority normal
					SetThreadPriority(cur_thread, THREAD_PRIORITY_NORMAL);

					break;
				case 4:
					// Keyboard InputStream from client
					if(!wv_receivedkey(&session))
						isconnected = false;

					break;
				case 5:
					// Mouse InputStream from client
					if(!wv_receivemouse(&session, &region[cur_region], &mouseevent))		
						isconnected = false;

					break;

			}

		}else{
			// Failed to get identifier		
			isconnected = false;
		}	
	}

	///////////////////////////////////////////////////////////////////////////////	
	// MAIN LOOP ENDS HERE
	///////////////////////////////////////////////////////////////////////////////	

	param->thread = NULL;
	session.RSCShutdown(SD_BOTH);
	session.RSCClose();
	
	// Update Icon
	m_tnd.hIcon = m_icon;
	ModifyTrayIcon();
}

bool wv_compare_n_update_conv_optimized(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, RSC_REGIONPROP *prop, RSCRegion *region, RSCPalette *pal)
{
	RSCPixtangle pix;
	bool converted = false;
	int prop_Bpp;

	switch(prop->bpp){
		case 32:
		case 24: // Bpp = 4
			prop_Bpp = 4;
			break;
		case 16:
		case 15: // Bpp = 2
			prop_Bpp = 2;
			break;
		case 8: // Bpp = 1
			prop_Bpp = 1;
			break;
	}

	pix.RSCCreateListPos(screen_w, screen_h, Bps, Bpp, pixtangle_w, pixtangle_h);

	switch(pix.m_Bpp){
		case 4:
		case 3:
			pix.RSCGetListPixtangle32(prev, next);
			//pix.RSCGetListPixtangle32(prev, next, prop->region_x, prop->region_y);
			break;
		case 2:
			pix.RSCGetListPixtangle16(prev, next);
			break;
		case 1:
			pix.RSCGetListPixtangle8(prev, next);
			break;
		default:
			return false;
	}
	

	// Send the count of total pixtangles
	if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos_ctr, 2))
		return false;
	
	// Send the data stream
	//
	RSC_UDWORD enc_length = 0; // 0 - Raw Encoding; otherwise, RLE length of compressed pixel stream
	RSCRLE enc;

	BYTE *org_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*Bpp]; // Space allocation
	if( org_data == 0x0 ){
		return false; // Insufficient memory	
	}

	BYTE *conv_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( conv_data== 0x0 ){
		delete[] org_data;
		return false; // Insufficient memory	
	}

	BYTE *cmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( cmp_data== 0x0 ){
		delete[] conv_data;	
		delete[] org_data;
		return false; // Insufficient memory
	}

	BYTE *tmp_data;

	// Send pixtangle update(s)
	for(int i = 0; i < pix.m_pos_ctr; i++){

		// SEND pos[i] data to the client
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].x, 4)){
			delete[] conv_data;
			delete[] cmp_data;
			delete[] org_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].y, 4)){
			delete[] conv_data;
			delete[] cmp_data;
			delete[] org_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].w, 4)){
			delete[] conv_data;
			delete[] cmp_data;
			delete[] org_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].h, 4)){
			delete[] conv_data;
			delete[] cmp_data;
			delete[] org_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].type, 1)){
		//if(!socket->RSCSend((RSC_BYTE *) &tp, 1)){	
			delete[] conv_data;
			delete[] cmp_data;
			delete[] org_data;
			return false;
		}
	
		switch(pix.m_pos[i].type){
			case 0:
				// Raw Encoding
				break;
			case 1:
				// RLE Encoding
				pix.RSCCopyPixtangle2Buffer(next, org_data, pix.m_pos[i].x, pix.m_pos[i].y, pix.m_pos[i].w, pix.m_pos[i].h);
				
				/////
				switch(bpp){
					case 8:
						// The conversion from lower bit screen depth to higher bit screen depth is disabled ...
						if(prop->bpp == 15){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 16){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 24){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 32){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}
						break;
					case 15:
						if(prop->bpp == 8){

							converted = true;

							if(!pix.RSCConvertPixtangle_15to8(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h, pal)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}

						}else if(prop->bpp == 16){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 24){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 32){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}
						break;
					case 16:
						if(prop->bpp == 8){

							converted = true;

							if(!pix.RSCConvertPixtangle_16to8(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h, pal)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}

						}else if(prop->bpp == 15){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_16to15(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}
							
						}else if(prop->bpp == 24){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}else if(prop->bpp == 32){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}
						break;
					case 24:
						if(prop->bpp == 8){

							converted = true;

							if(!pix.RSCConvertPixtangle_24to8(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h, pal)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}

						}else if(prop->bpp == 15){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_24to15(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}

						}else if(prop->bpp == 16){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_24to16(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;

							}

						}else if(prop->bpp == 32){
							delete[] conv_data; delete[] cmp_data; delete[] org_data;
							return false;
						}
						
						break;
					case 32:
						if(prop->bpp == 8){

							converted = true;

							// Debug
							//MessageBox(NULL, "TEST", "Debug", MB_OK);

							if(!pix.RSCConvertPixtangle_32to8(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h, pal)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;

							}

						}else if(prop->bpp == 15){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_32to15(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;

							}

						}else if(prop->bpp == 16){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_32to16(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;
							}

						}else if(prop->bpp == 24){
							
							converted = true;

							if(!pix.RSCConvertPixtangle_32to24(org_data, conv_data, pix.m_pos[i].w, pix.m_pos[i].h)){
								delete[] conv_data; delete[] cmp_data; delete[] org_data;
								return false;

							}

						}
						break;
				}
				
				//
				if(!converted){
					tmp_data = org_data;
				}else{
					tmp_data = conv_data;
				}

				switch(prop_Bpp){
					case 4:
						if(!enc.RSCCompress32(tmp_data, cmp_data, pix.m_pos[i].w*pix.m_pos[i].h*prop_Bpp, &enc_length))	
							enc_length = 0;
						break;
					case 2:
						if(!enc.RSCCompress16(tmp_data, cmp_data, pix.m_pos[i].w*pix.m_pos[i].h*prop_Bpp, &enc_length))
							enc_length = 0;
						break;
					case 1:
						if(!enc.RSCCompress8(tmp_data, cmp_data, pix.m_pos[i].w*pix.m_pos[i].h*prop_Bpp, &enc_length))
							enc_length = 0;
						break;
					default:
						delete[] conv_data;
						delete[] cmp_data;
						delete[] org_data;
						return false;
				}

				if(enc_length > 0){
					// RLE compressed pixel stream is sent
					if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
						delete[] conv_data;
						delete[] cmp_data;
						delete[] org_data;
						return false;
					}
					if(!socket->RSCSend((RSC_BYTE *) cmp_data, enc_length)){
						delete[] conv_data;
						delete[] cmp_data;
						delete[] org_data;
						return false;
					}
				}else{
					if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
						delete[] conv_data;
						delete[] cmp_data;
						delete[] org_data;
						return false;
					}
					if(!socket->RSCSend((RSC_BYTE *) tmp_data, pix.m_pos[i].w*pix.m_pos[i].h*prop_Bpp)){
						delete[] conv_data;
						delete[] cmp_data;
						delete[] org_data;
						return false;
					}
				}
				break;
			case 2:
			case 3:
				// CopyRect Encoding
				// 2 - with previous frame screen
				// 3 - with same frame screen
				if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].sx, 4)){
					delete[] conv_data;
					delete[] cmp_data;
					delete[] org_data;
					return false;
				}
				if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].sy, 4)){
					delete[] conv_data;
					delete[] cmp_data;
					delete[] org_data;
					return false;
				}

				break;
		
		}
			
	}

	delete[] conv_data;
	delete[] cmp_data;
	delete[] org_data;
	
	return true;
}

bool wv_receivemouse(RSCSocket *socket, RSCRegion *region, RSC_MOUSE_EVENT *mouseevent)
{
	RSC_BYTE lbtn, mbtn, rbtn;
	RSC_BYTE wheel1, wheel2;
	RSC_UWORD sx, sy;
	DWORD flag;
	int x, y;

	if(!socket->RSCReceive((RSC_BYTE *) &lbtn, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &mbtn, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &rbtn, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &wheel1, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &wheel2, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sx, 2))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sy, 2))
		return false;
	
	if(mouseevent->sx != sx ||  mouseevent->sy != sy){ 
		flag = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
		mouseevent->sx = sx;
		mouseevent->sy = sy;
	}else{	
		flag = 0;
	}

	if(lbtn == 1){
		if(mouseevent->lbtn == 0){
			flag = flag | MOUSEEVENTF_LEFTDOWN;
			mouseevent->lbtn = 1;
		}
	}else{
		if(mouseevent->lbtn == 1)	{
			flag = flag | MOUSEEVENTF_LEFTUP;
			mouseevent->lbtn = 0;
		}
	}

	if(mbtn == 1){
		if(mouseevent->mbtn == 0){
			flag = flag | MOUSEEVENTF_MIDDLEDOWN;
			mouseevent->mbtn = 1;
		}
	}else{
		if(mouseevent->mbtn == 1){
			flag = flag | MOUSEEVENTF_MIDDLEUP;
			mouseevent->mbtn = 0;
		}
	}

	if(rbtn == 1){
		if(mouseevent->rbtn == 0){
			flag = flag | MOUSEEVENTF_RIGHTDOWN;
			mouseevent->rbtn = 1;
		}
	}else{
		if(mouseevent->rbtn == 1){
			flag = flag | MOUSEEVENTF_RIGHTUP;
			mouseevent->rbtn = 0;
		}
	}
	
	if(region->m_regionprop.isscaled == TRUE){
		sx =  ((double)region->m_regionprop.region_o_w / (double)region->m_regionprop.region_w) * sx;
		sy =  ((double)region->m_regionprop.region_o_h / (double)region->m_regionprop.region_h) * sy;
	}

	mouseevent->sx = sx;
	mouseevent->sy = sy;
	mouseevent->wheel1 = wheel1;
	mouseevent->wheel2 = wheel2;

	if(region->m_regionprop.isscaled == TRUE){
		x = (sx*65535) / region->m_regionprop.region_o_w;
		y = (sy*65535) / region->m_regionprop.region_o_h;
	}else{
		// Debug
		//
		// Whether isscaled is set or not
		//
		x = (sx*65535) / region->m_regionprop.region_o_w;
		y = (sy*65535) / region->m_regionprop.region_o_h;
	}

	mouse_event(flag, x, y, wheel1, wheel2);

	return true;

}

bool wv_receivedkey(RSCSocket *socket)
{
	RSC_UWORD vkey;
	RSC_BYTE down;
	DWORD kdown;

	if(!socket->RSCReceive((RSC_BYTE *) &down, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &vkey, 2))
		return false;

	if(down == 0) // down 1-Pressed  0-Not Pressed
		kdown = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	else
		kdown = KEYEVENTF_EXTENDEDKEY;

	keybd_event((char)vkey, 0x45, kdown, 0 );
	return true;

}

bool wv_newscreen(RSCSocket *socket, RSCPalette *pal, RSCRegion region[], BYTE *cur_region, RSC_REGIONPROP *prop)
{
	RSC_BYTE region_mode = 1;
	RSC_UDWORD region_x;
	RSC_UDWORD region_w;
	RSC_UDWORD region_y;
	RSC_UDWORD region_h;

	RSC_UBYTE *data_ptr, *data_pre, *comp_ptr, *conv_ptr;
	RSC_WORD pixtangle_ctr;
	RSC_UDWORD pixtangle_x, pixtangle_y, pixtangle_h, pixtangle_w;
	RSC_UDWORD enc_length = 0;
	RSCRLE enc;
	RSC_DIBINFO conv_dibh;
	bool converted = false;

	if(!socket->RSCReceive((RSC_BYTE *) &region_mode, 1))
		return false;

	if(!socket->RSCReceive((RSC_BYTE *) &region_x, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_w, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_y, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_h, 4))
		return false;

	switch(region_mode){
		case 0: // Full Update
			{
			// Capture the screen
			if(!region[*cur_region].RSCCapture())
				return false;

			// Configure initial DIB informaiton
			if(region[*cur_region].m_regionprop.bpp == 8){
				
				// [Note] If m_pdib is initialized, do not call RSCInitDIB
				if(region[*cur_region].m_pdib == NULL){
					// If 8-bits, use Palette	
					HPALETTE syspal=NULL, oldpal=NULL;
					
					syspal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

					oldpal = SelectPalette(region[*cur_region].m_hdc, syspal, FALSE);
						
					region[*cur_region].RSCInitDIB(syspal, region[*cur_region].m_regionprop.bpp, BI_RGB);
						
					SelectPalette(region[*cur_region].m_hdc, oldpal, TRUE);
					DeleteObject(syspal);	
				}
				//
				
			}else{
				
				// [Note] If m_pdib is initialized, do not call RSCInitDIB
				if(region[*cur_region].m_pdib == NULL){
					// If 15/16/24/32 bits, use RGB instead of Palette
					region[*cur_region].RSCInitDIB(NULL, region[*cur_region].m_regionprop.bpp, BI_BITFIELDS);
				}

			}

			// Convert DDB to DIB
			if(!region[*cur_region].RSCDDB2DIB(region[*cur_region].m_ddbinfo.bmHeight, region[*cur_region].m_dibhsize, DIB_RGB_COLORS))
				return false;
			

			// Sending the data
			data_ptr = (RSC_UBYTE *) region[*cur_region].m_pdib + region[*cur_region].m_dibhsize;
			pixtangle_ctr = 1;
			
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_ctr, 2))
				return false;
			

			pixtangle_x = region[*cur_region].m_regionprop.region_x;
			pixtangle_y = region[*cur_region].m_regionprop.region_y;
			pixtangle_w = region[*cur_region].m_regionprop.region_w;
			pixtangle_h = region[*cur_region].m_regionprop.region_h;
		
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_x, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_y, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_w, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_h, 4))
				return false;

			// Conversion if necessary
			memset(&conv_dibh, 0, sizeof(conv_dibh.header));

			// Init DIB Header for converted image
			conv_dibh.header.biClrImportant = 0;
			conv_dibh.header.biClrUsed = 0;
			conv_dibh.header.biHeight = prop->region_h; // client request of screen height
			conv_dibh.header.biWidth = prop->region_w; // client request of screen width
			conv_dibh.header.biXPelsPerMeter = 0;
			conv_dibh.header.biYPelsPerMeter = 0;
			conv_dibh.header.biPlanes = 1;
			conv_dibh.header.biSize = sizeof(BITMAPINFOHEADER);

			switch(region[*cur_region].m_regionprop.bpp){
				case 8: // Server bpp
					if(prop->bpp == 15){
						return false;
					}else if(prop->bpp == 16){
						return false;
					}else if(prop->bpp == 24){
						return false;
					}else if(prop->bpp == 32){
						return false;
					}
					break;
					//
				case 15: // Server bpp
					if(prop->bpp == 8){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS; // BI_RGB;
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;

						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_15to8(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 16){

                        return false;

					}else if(prop->bpp == 24){
						
						return false;
						
					}else if(prop->bpp == 32){
						
						return false;

					}
					
					break;
				case 16: // Server bpp
					if(prop->bpp == 8){						
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS; // BI_RGB;
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;

						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_16to8(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_16to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 24){
							
						return false;
						
					}else if(prop->bpp == 32){
						
						return false;

					}
					
					break;
				case 24: // Server bpp
					if(prop->bpp == 8){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS; // BI_RGB;
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;

						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to8(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 16){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to16(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 32){
						
						return false;

					}
					
					break;
				case 32: // Server bpp
					if(prop->bpp == 8){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS; // BI_RGB;
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;

						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to8(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 16){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to16(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 24){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to24(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}
					break;
			}

			
			if(!converted){
				conv_ptr = data_ptr;
			}
			
			// Temporary buffer
			comp_ptr = new BYTE[prop->Bps*prop->region_h]; // = prop->Bps*conv_dibh.header.biHeight;
			switch(prop->bpp){
				case 32:
				case 24:
					if(!enc.RSCCompress32(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;
				case 16:
					if(!enc.RSCCompress16(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;	
				case 15:
					break;
				case 8:
					if(!enc.RSCCompress8(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;	
				default:
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
					//break;
			}

			if(enc_length > 0){
				// RLE compressed pixel stream is sent
				if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
				if(!socket->RSCSend((RSC_BYTE *) comp_ptr, enc_length)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
			}else{
				if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
				if(!socket->RSCSend((RSC_BYTE *) conv_ptr, prop->region_h*prop->Bps)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
			}

			delete[] comp_ptr;
			if(converted) delete[] conv_ptr;
			
			// REGION SWAP
			if(*cur_region == 0)
				*cur_region = 1;
			else 
				*cur_region = 0;
			}
			break;

		case 1: // Incremental Update

			// Capture the screen
			if(!region[*cur_region].RSCCapture())
				return false;
			
			// Configure initial DIB informaiton
			if(region[*cur_region].m_regionprop.bpp == 8){
								
				// [Note] If m_pdib is initialized, do not call RSCInitDIB
				if(region[*cur_region].m_pdib == NULL){
					// If 8-bits, use Palette	
					HPALETTE syspal=NULL, oldpal=NULL;

					syspal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

					oldpal = SelectPalette(region[*cur_region].m_hdc, syspal, FALSE);
					
					region[*cur_region].RSCInitDIB(syspal, region[*cur_region].m_regionprop.bpp, BI_RGB);
					
					SelectPalette(region[*cur_region].m_hdc, oldpal, TRUE);
					DeleteObject(syspal);
				}

			}else{
				
				// [Note] If m_pdib is initialized, do not call RSCInitDIB
				if(region[*cur_region].m_pdib == NULL){
					// If 15/16/24/32 bits, use RGB instead of Palette
					region[*cur_region].RSCInitDIB(NULL, region[*cur_region].m_regionprop.bpp, BI_BITFIELDS);
				}
				

			}

			int pre_region;
			if(*cur_region == 0) pre_region = 1;
			else pre_region = 0;


			// Convert DDB to DIB
			if(!region[*cur_region].RSCDDB2DIB(region[*cur_region].m_ddbinfo.bmHeight, region[*cur_region].m_dibhsize, DIB_RGB_COLORS))
				return false;
			
			// Sending the data
			data_ptr = (RSC_UBYTE *) region[*cur_region].m_pdib + region[*cur_region].m_dibhsize;
			data_pre = (RSC_UBYTE *) region[pre_region].m_pdib + region[pre_region].m_dibhsize;

			//if(!wv_compare_n_update_conv(socket, data_pre, data_ptr, region[*cur_region].m_regionprop.region_w, region[*cur_region].m_regionprop.region_h, region[*cur_region].m_regionprop.bpp, region[*cur_region].m_regionprop.bpp/8, region[*cur_region].m_regionprop.Bps, PIXTANGLE_WIDTH, PIXTANGLE_HEIGHT, prop, &region[*cur_region])){ //RSC_REGIONPROP *prop, RSCRegion *region
			int m_Bpp;
			switch(region[*cur_region].m_regionprop.bpp){
				case 32:
				case 24: // Bpp = 4
					m_Bpp = 4;
					break;
				case 16:
				case 15: // Bpp = 2
					m_Bpp = 2;
					break;
				case 8: // Bpp = 1
					m_Bpp = 1;
					break;
			}

			if(!wv_compare_n_update_conv_optimized(socket, data_pre, data_ptr, region[*cur_region].m_regionprop.region_w, region[*cur_region].m_regionprop.region_h, region[*cur_region].m_regionprop.bpp, m_Bpp, region[*cur_region].m_regionprop.Bps, PIXTANGLE_WIDTH, PIXTANGLE_HEIGHT, prop, &region[*cur_region], pal)){			
				return false;
			}

			// REGION SWAP
			if(*cur_region == 0)
				*cur_region = 1;
			else 
				*cur_region = 0;

			break;

		default:
			break;
	}

	return true;
}

bool wv_init(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_REGIONPROP *prop, bool *isinput)
{
	RSC_BYTE access_mode;
	RSC_BYTE red_b = 0, green_b = 0, blue_b = 0;
	
	RSC_UDWORD sw;	// Screen width
	RSC_UDWORD sh;	// Screen height
	RSC_UDWORD cx;  // Starting point for X
	RSC_UDWORD cy;  // Starting point for Y

	RSC_BYTE bpp;	// bps (bits per pixel) = 8 bits (color/grayscale), 16 bits, 24 bits and 32 bits.
	
	RSC_BYTE region_palette_req;

	RSC_UDWORD display_size = 256;	// Size of string (multi-cast or display name)
	RSC_BYTE display_str[256];		// Array string that indicates the multi-cast session id or display name.
	
	// GetComputerName(display_str);
	// [TO DO LATER]

	strcpy(display_str, "RSC Desktop");

	switch(region->m_dib_pixel_type){
		case DIB_8BPP:
				red_b = 0;
				green_b = 0;
				blue_b = 0;
				break;
		case DIB_16RGB555:
				red_b = 5;
				green_b = 5;
				blue_b = 5;
				break;
		case DIB_16RGB565:
				red_b = 5;
				green_b = 6;
				blue_b = 5;
				break;
		case DIB_24RGB888:
		case DIB_32RGB888:
		case DIB_32RGBA888:
				red_b = 8;
				green_b = 8;
				blue_b = 8;
				break;
		default:
			// Invalid Screen Format ...
			return false;
			break;
	}


	//
	//
	//
	
	//sw = region->m_regionprop.region_w;
	//sh = region->m_regionprop.region_h;
	sw = region->m_regionprop.region_o_w;
	sh = region->m_regionprop.region_o_h;
	bpp = region->m_regionprop.bpp;
	
	if(!socket->RSCReceive(&access_mode, 1))
		return false;

	// For Screen Stream data flow
	if(access_mode == 0){
		// Call Disconnect_All() functions before continuing with this connection.
		// [TO DO LATER]
	}

	// For Input Steam flow - begins here
	if(access_mode == 1){
		// TBD
		// Call Disconnect_All() functions before continuing with this connection.
		*isinput = true;
		
		// Testing module
		//return true;
	}
	if(access_mode == 3){
		// Do not disconnect previously connected clients.
		*isinput = true;

		// Testing module
		// return true;	
	}
	// ends here
    

	if(!socket->RSCSend((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCSend(&bpp, 1))
		return false;
	if(!socket->RSCSend(&red_b, 1))
		return false;
	if(!socket->RSCSend(&green_b, 1))
		return false;
	if(!socket->RSCSend(&blue_b, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &display_size, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) display_str, 256))
		return false;
	
	// Reply back from Client with PROPERTIES the client want
	if(!socket->RSCReceive((RSC_BYTE *) &cx, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &cy, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCReceive(&bpp, 1))
		return false;
	if(!socket->RSCReceive(&red_b, 1))
		return false;
	if(!socket->RSCReceive(&green_b, 1))
		return false;
	if(!socket->RSCReceive(&blue_b, 1))
		return false;
	
	//
	// Only if the client requests the 8-bit color screen ...
	//
	if(bpp == 8){
		if(!socket->RSCReceive((RSC_BYTE *) &region_palette_req, 1))
			return false;	
		//
		// Palette RGBQUAD sending ...
		//
		switch(region->m_regionprop.bpp){
			case 8:
					if(!pal->SetRGBIndex(region->m_hdc, RSC_PALETTE_SYSTEM))
						return false;
					if(!pal->FillColorTable(RSC_PALETTE_SYSTEM))
						return false;
		
					if(!socket->RSCSend((RSC_BYTE *) &pal->m_col_table, 1024))
						return false;
					break;
			case 15:
			case 16:
			case 24:
			case 32:

				if(!pal->SetRGBIndex(region->m_hdc, RSC_PALETTE_HALFTONE))
					return false;
				if(!pal->FillColorTable(RSC_PALETTE_HALFTONE))
					return false;
					
				if(!socket->RSCSend((RSC_BYTE *) &pal->m_col_table, 1024))
					return false;
				break;
		}
	}

	
	//
	// Debug
	//
	prop->region_x = cx;
	prop->region_y = cy;
	//
	//
	//

	prop->bpp = bpp;
	prop->region_h = sh;
	prop->region_w = sw;
	prop->Bps = RSCGetBPS(sw, bpp);
	prop->pixel_type = RSCGetPixelTypeFromBitsPerPixel(bpp, red_b, green_b, blue_b);
	
	return true;
}

bool wv_authenticate(RSCSocket *socket)
{
	RSC_BYTE auth = 0;
	RSC_BYTE userid[16];

	if(!wv_version(socket)){
		// Incompatible RSCP version
		auth = 0;
		if(!socket->RSCSend(&auth, 1))
			return false;

		return false;
	}else{

		// RSC Challenge Response Authentication Request
		unsigned char challenge[16];
		unsigned char enc_response[16];
		unsigned char *passwd;

		RSC_DECRYPTED dec[MAX_PASSWD_RETRY];
		//RSCCrypt dec[MAX_PASSWD_RETRY], pwd;
		RSCCrypt pwd;
		RSC_BYTE status = 0; //0: accepted, 1: not accepted, 2: suspicious attempts detected, 3: Max. number of clients is reached.

		for(int i=0; i < MAX_PASSWD_RETRY;i++)
			dec[i].dec.RSCKeyGen(challenge, 16);

		auth = 1;
		if(!socket->RSCSend(&auth, 1))
			return false;

		// Password Decrypted
		if(!pwd.RSCLoadEncrytped(m_prop.m_prop_password, 8, 8))
			return false;
		passwd = pwd.RSCDecryptByteStream();

		bool retry=true;
		int retrynum = 0;
		while(retry){
		
			auth = 1;

			if(!socket->RSCReceive(userid, 16))   // RECEIVE 1
				return false;

			if(!socket->RSCSend((RSC_BYTE *)challenge, 16))   // SEND 2
				return false;
			
			if(!socket->RSCReceive((RSC_BYTE *)enc_response, 16))    // RECEIVE 3
				return false;

			if(!dec[retrynum].dec.RSCLoadEncrytped(enc_response, 16, 16))
				return false;

			dec[retrynum].response = dec[retrynum].dec.RSCDecryptByteStream(passwd);

			if(memcmp(dec[retrynum].response, challenge, 16) != 0){
				
				dec[retrynum].dec.RSCFreeMemory(dec[retrynum].response);
				
				if(retrynum < MAX_PASSWD_RETRY-1){
					status = 1;
					retrynum++;
					// Debug
					// MessageBox(NULL, "RETRY", "Debug", MB_OK);
				}else{
					status = 3;
				}
			}else{
				status = 0;
			}
			
			switch(status){
				case 0:
					if(!socket->RSCSend(&status, 1))   //SEND 4
						return false;
					retry = false;
					
					if(passwd != NULL)
						pwd.RSCFreeMemory(passwd);				

					break;
				case 1:
					if(!socket->RSCSend(&status, 1))
						return false;
					retry = true;
					break;
				//case 2:
				//	//retry = false;
				//	break;
				case 3:
					if(!socket->RSCSend(&status, 1))
						return false;
					retry = false;

					if(passwd != NULL)
						pwd.RSCFreeMemory(passwd);

					return false;
					break;
			}
		}
	}

	return true;
}

bool wv_version(RSCSocket *socket)
{
	
	RSC_BYTE ver_rcv[12];
	RSC_BYTE rsc_ver[] = "RSC_VERSIONCHECK";

	// Sending RSC_VERSIONCHECK ...
	if(!socket->RSCSend(rsc_ver, 17))
		return false;
	
	// Receive from client
	if(!socket->RSCReceive(ver_rcv, 12))
		return false;

	if(!strcmp(ver_rcv, "RSC_0001.00") == 0)
		return false;

	return true;
}

//
// Sample Functions 
//
// wv_compare_n_upate_xxx samples
//
//
/*
bool wv_compare_n_update(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, int Bpp, int Bps, int pixtangle_w, int pixtangle_h)
{
	RSCPixtangle pix;

	pix.RSCCreateListPos(screen_w, screen_h, Bps, Bpp, pixtangle_w, pixtangle_h);
	switch(pix.m_Bpp){
		case 4:
		case 3:
			pix.RSCGetListPixtangle32(prev, next);
			break;
		case 2:
			pix.RSCGetListPixtangle16(prev, next);
			break;
		case 1:
			pix.RSCGetListPixtangle8(prev, next);
			break;
		default:
			return false;
	}
	
	// Send the count of total pixtangles
	if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos_ctr, 2))
		return false;
	
	// Send the data stream
	//
	RSC_UDWORD enc_length = 0; // 0 - Raw Encoding; otherwise, RLE length of compressed pixel stream
	BYTE *tmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp];
	BYTE *cmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp];
	RSCRLE enc;

	if( tmp_data== 0x0 )
		return false; // Insufficient memory
	
	if( cmp_data== 0x0 )
		return false; // Insufficient memory

	for(int i = 0; i < pix.m_pos_ctr; i++){
		// SEND pos[i] data to the client
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].x, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].y, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].w, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].h, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			return false;
		}

		pix.RSCCopyPixtangle2Buffer(next, tmp_data, pix.m_pos[i].x, pix.m_pos[i].y, pix.m_pos[i].w, pix.m_pos[i].h);

		switch(pix.m_Bpp){
			case 4:
			case 3:
				if(!enc.RSCCompress32(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 2:
				if(!enc.RSCCompress16(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 1:
				if(!enc.RSCCompress8(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			default:
				delete[] tmp_data;
				delete[] cmp_data;
				return false;
		}

		if(enc_length > 0){
			// RLE compressed pixel stream is sent
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) cmp_data, enc_length)){
				delete[] tmp_data;
				delete[] cmp_data;
				return false;
			}
		}else{
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) tmp_data, pix.m_pos[i].w*pix.m_pos[i].h*pix.m_Bpp)){
				delete[] tmp_data;
				delete[] cmp_data;
				return false;
			}
		}
	}
	
	delete[] tmp_data;
	delete[] cmp_data;
	
	return true;
}

bool wv_compare_n_update_conv(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, 
							  int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, 
							  RSC_REGIONPROP *prop, RSCRegion *region)
{
	RSCPixtangle pix;
	RSC_DIBINFO conv_dibh;
	bool converted = false;
	BYTE *conv_data;
	int prop_Bpp;

	switch(prop->bpp){
		case 32:
		case 24: // Bpp = 4
			prop_Bpp = 4;
			break;
		case 16:
		case 15: // Bpp = 2
			prop_Bpp = 2;
			break;
		case 8: // Bpp = 1
			prop_Bpp = 1;
			break;
	}

	pix.RSCCreateListPos(screen_w, screen_h, Bps, Bpp, pixtangle_w, pixtangle_h);

	switch(pix.m_Bpp){
		case 4:
		case 3:
			pix.RSCGetListPixtangle32(prev, next);
			break;
		case 2:
			pix.RSCGetListPixtangle16(prev, next);
			break;
		case 1:
			pix.RSCGetListPixtangle8(prev, next);
			break;
		default:
			return false;
	}
	
	//
	// Debug
	//
	//TCHAR dstr1[128];
	//wsprintf(dstr1, "%d", pix.m_pos_ctr);
	//MessageBox(NULL, dstr1, "Debug", MB_OK);

	// Send the count of total pixtangles
	if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos_ctr, 2))
		return false;
	
	// Send the data stream
	//
	RSC_UDWORD enc_length = 0; // 0 - Raw Encoding; otherwise, RLE length of compressed pixel stream
	RSCRLE enc;

	// Conversion if necessary
	memset(&conv_dibh, 0, sizeof(conv_dibh.header));

	// Init DIB Header for converted image
	conv_dibh.header.biClrImportant = 0;
	conv_dibh.header.biClrUsed = 0;
//	conv_dibh.header.biHeight = prop->region_h; // client request of screen height
//	conv_dibh.header.biWidth = prop->region_w; // client request of screen width
	conv_dibh.header.biXPelsPerMeter = 0;
	conv_dibh.header.biYPelsPerMeter = 0;
	conv_dibh.header.biPlanes = 1;
	conv_dibh.header.biSize = sizeof(BITMAPINFOHEADER);

	//
	// Conversion if necessary
	//
	switch(bpp){
		case 8:
			//
			// TO DO LATER
			//
			if(prop->bpp == 15){
			}else if(prop->bpp == 16){
			}else if(prop->bpp == 24){
			}else if(prop->bpp == 32){
			}
			break;
		case 15:
			if(prop->bpp == 8){
				// TO DO LATER
			}else if(prop->bpp == 16){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			break;
		case 16:
			if(prop->bpp == 8){
				// TO DO LATER
			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			break;
		case 24:
			if(prop->bpp == 8){
				// TO DO LATER
			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;


			}else if(prop->bpp == 16){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			break;
		case 32:
			if(prop->bpp == 8){
				// TO DO LATER
			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 16){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			break;
	}

	BYTE *tmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( tmp_data== 0x0 ){
		return false; // Insufficient memory	
		delete[] conv_data;
	}

	BYTE *cmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( cmp_data== 0x0 ){
		delete [] tmp_data;	
		delete[] conv_data;
		return false; // Insufficient memory
	}

	if(!converted){
		conv_data = next;
	}

	// Debug
	//TCHAR bufd[128];
	//wsprintf(bufd, "%d", pix.m_pos_ctr);
	//MessageBox(NULL, bufd, "Debug", MB_OK);
	// 
	for(int i = 0; i < pix.m_pos_ctr; i++){
		// SEND pos[i] data to the client
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].x, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].y, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].w, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].h, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}

		pix.RSCCopyPixtangle2Buffer(conv_data, tmp_data, pix.m_pos[i].x, pix.m_pos[i].y, pix.m_pos[i].w, pix.m_pos[i].h);
		
		switch(prop_Bpp){
			case 4:
				if(!enc.RSCCompress32(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 2:
				if(!enc.RSCCompress16(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 1:
				if(!enc.RSCCompress8(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			default:
				delete[] cmp_data;
				delete[] tmp_data;
				if(converted) delete[] conv_data;
				return false;
		}

		if(enc_length > 0){
			// RLE compressed pixel stream is sent
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) cmp_data, enc_length)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
		}else{
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) tmp_data, pix.m_pos[i].w*pix.m_pos[i].h*pix.m_Bpp)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
		}
			
	}
	
	delete[] tmp_data;
	delete[] cmp_data;
	if(converted) delete[] conv_data;
	
	return true;
}


bool wv_compare_n_update_conv(RSCSocket *socket, BYTE *prev, BYTE *next, RSC_UDWORD  screen_w, RSC_UDWORD screen_h, 
							  int bpp, int Bpp, int Bps, int pixtangle_w, int pixtangle_h, 
							  RSC_REGIONPROP *prop, RSCRegion *region, RSCPalette *pal)
{
	RSCPixtangle pix;
	RSC_DIBINFO conv_dibh;
	bool converted = false;
	BYTE *conv_data;
	int prop_Bpp;

	switch(prop->bpp){
		case 32:
		case 24: // Bpp = 4
			prop_Bpp = 4;
			break;
		case 16:
		case 15: // Bpp = 2
			prop_Bpp = 2;
			break;
		case 8: // Bpp = 1
			prop_Bpp = 1;
			break;
	}

	pix.RSCCreateListPos(screen_w, screen_h, Bps, Bpp, pixtangle_w, pixtangle_h);

	switch(pix.m_Bpp){
		case 4:
		case 3:
			pix.RSCGetListPixtangle32(prev, next);
			break;
		case 2:
			pix.RSCGetListPixtangle16(prev, next);
			break;
		case 1:
			pix.RSCGetListPixtangle8(prev, next);
			break;
		default:
			return false;
	}
	
	//
	// Debug
	//
	//TCHAR dstr1[128];
	//wsprintf(dstr1, "%d", pix.m_pos_ctr);
	//MessageBox(NULL, dstr1, "Debug", MB_OK);

	// Send the count of total pixtangles
	if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos_ctr, 2))
		return false;
	
	// Send the data stream
	//
	RSC_UDWORD enc_length = 0; // 0 - Raw Encoding; otherwise, RLE length of compressed pixel stream
	RSCRLE enc;

	// Conversion if necessary
	memset(&conv_dibh, 0, sizeof(conv_dibh.header));

	// Init DIB Header for converted image
	conv_dibh.header.biClrImportant = 0;
	conv_dibh.header.biClrUsed = 0;
//	conv_dibh.header.biHeight = prop->region_h; // client request of screen height
//	conv_dibh.header.biWidth = prop->region_w; // client request of screen width
	conv_dibh.header.biXPelsPerMeter = 0;
	conv_dibh.header.biYPelsPerMeter = 0;
	conv_dibh.header.biPlanes = 1;
	conv_dibh.header.biSize = sizeof(BITMAPINFOHEADER);

	//
	// Conversion if necessary
	//
	switch(bpp){
		case 8:
			//
			// [Note]
			// Converting 8bit to higher color depth in order 
			// to be sent via network is meaningless.
			//
			// Screen INIT will block any attempt to change from 8 bits to 
			// higher color depth.
			// 
			if(prop->bpp == 15){
			}else if(prop->bpp == 16){
			}else if(prop->bpp == 24){
			}else if(prop->bpp == 32){
			}
			break;
		case 15:
			if(prop->bpp == 8){

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_RGB;
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to8(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
					delete[] conv_data;
					return false;
				}

				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 16){
				//
				// [Note]
				// Converting 15bit to 16/24/32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 15 bits to 
				// higher color depth (16/24/32).
				// 
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				//
				// [Note]
				// Converting 15bit to 16/24/32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 15 bits to 
				// higher color depth (16/24/32).
				// 

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				//
				// [Note]
				// Converting 15bit to 16/24/32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 15 bits to 
				// higher color depth (16/24/32).
				// 

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_15to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			
			break;
		case 16:
			if(prop->bpp == 8){

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_RGB;
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to8(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
					delete[] conv_data;
					return false;
				}

				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				//
				// [Note]
				// Converting 16bit to 24/32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 16 bits to 
				// higher color depth (24/32).
				// 

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				//
				// [Note]
				// Converting 16bit to 24/32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 16 bits to 
				// higher color depth (24/32).
				// 

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_16to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			
			break;
		case 24:
			if(prop->bpp == 8){

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_RGB;
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to8(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
					delete[] conv_data;
					return false;
				}

				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;


			}else if(prop->bpp == 16){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 32){
				//
				// [Note]
				// Converting 24bit to 32 color depth in order 
				// to be sent via network is meaningless.
				//
				// Screen INIT will block any attempt to change from 24 bits to 
				// higher color depth (32).
				// 

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_24to32(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			
			break;
		case 32:
			if(prop->bpp == 8){

				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_RGB;
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to8(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight, pal)){
					delete[] conv_data;
					return false;
				}

				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 15){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to15(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data; 
					return false;
				}
				
				// Reset the value of pix.m_Bpp and pix.m_Bps
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 16){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to16(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}else if(prop->bpp == 24){
				conv_dibh.header.biBitCount = prop->bpp;
				conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
				conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
				conv_dibh.header.biHeight = prop->region_h; // client request of screen height
				conv_dibh.header.biWidth = prop->region_w; // client request of screen width

				conv_data = new BYTE[prop->Bps*conv_dibh.header.biHeight];
				converted = true;

				if(!region->RSCConvertDIB_32to24(next, conv_data, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
					delete[] conv_data;
					return false;
				}
				// Reset the value of pix.m_Bpp and pix.m_Bps if there is a conversion of screen buffer
				pix.m_Bpp = prop_Bpp;
				pix.m_Bps = prop->Bps;

			}
			break;
	}

	
	BYTE *tmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( tmp_data== 0x0 ){
		return false; // Insufficient memory	
		delete[] conv_data;
	}

	BYTE *cmp_data = new BYTE[pix.m_pixtangle_w*pix.m_pixtangle_h*prop_Bpp]; // Space allocation after being converted
	if( cmp_data== 0x0 ){
		delete [] tmp_data;	
		delete[] conv_data;
		return false; // Insufficient memory
	}

	if(!converted){
		conv_data = next;
	}

	// Debug
	//TCHAR bufd[128];
	//wsprintf(bufd, "%d", pix.m_pos_ctr);
	//MessageBox(NULL, bufd, "Debug", MB_OK);
	// 
	for(int i = 0; i < pix.m_pos_ctr; i++){
		// SEND pos[i] data to the client
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].x, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].y, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].w, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}
		if(!socket->RSCSend((RSC_BYTE *) &pix.m_pos[i].h, 4)){
			delete[] tmp_data;
			delete[] cmp_data;
			if(converted) delete[] conv_data;
			return false;
		}

		pix.RSCCopyPixtangle2Buffer(conv_data, tmp_data, pix.m_pos[i].x, pix.m_pos[i].y, pix.m_pos[i].w, pix.m_pos[i].h);
		
		switch(prop_Bpp){
			case 4:
				if(!enc.RSCCompress32(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 2:
				if(!enc.RSCCompress16(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			case 1:
				if(!enc.RSCCompress8(tmp_data, cmp_data, pix.m_pixtangle_w*pix.m_pixtangle_h*pix.m_Bpp, &enc_length))
					enc_length = 0;
				break;
			default:
				delete[] cmp_data;
				delete[] tmp_data;
				if(converted) delete[] conv_data;
				return false;
		}

		if(enc_length > 0){
			// RLE compressed pixel stream is sent
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) cmp_data, enc_length)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
		}else{
			if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
			if(!socket->RSCSend((RSC_BYTE *) tmp_data, pix.m_pos[i].w*pix.m_pos[i].h*pix.m_Bpp)){
				delete[] tmp_data;
				delete[] cmp_data;
				if(converted) delete[] conv_data;
				return false;
			}
		}
			
	}
	
	delete[] tmp_data;
	delete[] cmp_data;
	if(converted) delete[] conv_data;
	
	return true;
}

bool wv_newscreen(RSCSocket *socket, RSCRegion region[], BYTE *cur_region, RSC_REGIONPROP *prop)
{
	RSC_BYTE region_mode = 1;
	//RSC_BYTE region_palette_req;
	RSC_UDWORD region_x;
	RSC_UDWORD region_w;
	RSC_UDWORD region_y;
	RSC_UDWORD region_h;

	RSC_UBYTE *data_ptr, *data_pre, *comp_ptr, *conv_ptr;
	RSC_WORD pixtangle_ctr;
	RSC_UDWORD pixtangle_x, pixtangle_y, pixtangle_h, pixtangle_w;
	RSC_UDWORD enc_length = 0;
	RSCRLE enc;
	RSC_DIBINFO conv_dibh;
	bool converted = false;

	if(!socket->RSCReceive((RSC_BYTE *) &region_mode, 1))
		return false;
	
	//
	// Only if the client requests the 8-bit color screen ...
	//
	//if(prop->bpp == 8){
	//	if(!socket->RSCReceive((RSC_BYTE *) &region_palette_req, 1))
	//		return false;	
	//}
	//

	if(!socket->RSCReceive((RSC_BYTE *) &region_x, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_w, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_y, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &region_h, 4))
		return false;

	switch(region_mode){
		case 0: // Full Update
			// Capture the screen
			if(!region[*cur_region].RSCCapture())
				return false;

			// Configure initial DIB informaiton
			if(region[*cur_region].m_regionprop.bpp == 8){
				// If 8-bits, use Palette	
				HPALETTE syspal=NULL, oldpal=NULL;

				syspal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

				
				oldpal = SelectPalette(region[*cur_region].m_hdc, syspal, FALSE);
				
				region[*cur_region].RSCInitDIB(syspal, region[*cur_region].m_regionprop.bpp, BI_RGB);
				
				SelectPalette(region[*cur_region].m_hdc, oldpal, TRUE);
				DeleteObject(syspal);

			}else{
				// If 15/16/24/32 bits, use RGB instead of Palette
				region[*cur_region].RSCInitDIB(NULL, region[*cur_region].m_regionprop.bpp, BI_BITFIELDS);
			}


			// Convert DDB to DIB
			if(!region[*cur_region].RSCDDB2DIB(region[*cur_region].m_ddbinfo.bmHeight, region[*cur_region].m_dibhsize, DIB_RGB_COLORS))
				return false;
			

			// Sending the data
			data_ptr = (RSC_UBYTE *) region[*cur_region].m_pdib + region[*cur_region].m_dibhsize;
			pixtangle_ctr = 1;
			
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_ctr, 2))
				return false;
			
			// Palette RGBQUAD sending ...
			// TO DO

			pixtangle_x = region[*cur_region].m_regionprop.region_x;
			pixtangle_y = region[*cur_region].m_regionprop.region_y;
			pixtangle_w = region[*cur_region].m_regionprop.region_w;
			pixtangle_h = region[*cur_region].m_regionprop.region_h;
		
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_x, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_y, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_w, 4))
				return false;
			if(!socket->RSCSend((RSC_BYTE *) &pixtangle_h, 4))
				return false;

			// Conversion if necessary
			memset(&conv_dibh, 0, sizeof(conv_dibh.header));

			// Init DIB Header for converted image
			conv_dibh.header.biClrImportant = 0;
			conv_dibh.header.biClrUsed = 0;
			conv_dibh.header.biHeight = prop->region_h; // client request of screen height
			conv_dibh.header.biWidth = prop->region_w; // client request of screen width
			conv_dibh.header.biXPelsPerMeter = 0;
			conv_dibh.header.biYPelsPerMeter = 0;
			conv_dibh.header.biPlanes = 1;
			conv_dibh.header.biSize = sizeof(BITMAPINFOHEADER);

			switch(region[*cur_region].m_regionprop.bpp){
				case 8: // Server bpp
					//
					// TO DO LATER
					//
					if(prop->bpp == 15){
					}else if(prop->bpp == 16){
					}else if(prop->bpp == 24){
					}else if(prop->bpp == 32){
					}
					break;
				case 15: // Server bpp
					if(prop->bpp == 8){
						//
						// TO DO LATER
						//
					}else if(prop->bpp == 16){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_15to16(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 24){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_15to24(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 32){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_15to32(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}
					break;
				case 16: // Server bpp
					if(prop->bpp == 8){
						// TO DO LATER
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_16to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 24){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_16to24(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 32){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_16to32(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}
					break;
				case 24: // Server bpp
					if(prop->bpp == 8){
						// TO DO LATER
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 16){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to16(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 32){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_24to32(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}
					break;
				case 32: // Server bpp
					if(prop->bpp == 8){
						// TO DO LATER
					}else if(prop->bpp == 15){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to15(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 16){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to16(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}else if(prop->bpp == 24){
						conv_dibh.header.biBitCount = prop->bpp;
						conv_dibh.header.biCompression = BI_BITFIELDS;// or BI_RGB if 8 bpp
						conv_dibh.header.biSizeImage = prop->region_h*prop->Bps;
	
						conv_ptr = new BYTE[prop->Bps*conv_dibh.header.biHeight];
						converted = true;

						if(!region[*cur_region].RSCConvertDIB_32to24(data_ptr, conv_ptr, conv_dibh.header.biWidth, conv_dibh.header.biHeight)){
							delete[] conv_ptr;
							return false;
						}
					}
					break;
			}

			if(!converted){
				conv_ptr = data_ptr;
			}
			// Temporary buffer
			comp_ptr = new BYTE[prop->Bps*prop->region_h]; // = prop->Bps*conv_dibh.header.biHeight;
			switch(prop->bpp){
				case 32:
				case 24:
					if(!enc.RSCCompress32(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;
				case 16:
					if(!enc.RSCCompress16(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;	
				case 15:
					break;
				case 8:
					if(!enc.RSCCompress8(conv_ptr, comp_ptr, prop->region_h*prop->Bps, &enc_length))
						enc_length = 0;
					break;	
				default:
					// *** Needs correction - why does not return false?
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
					//break;
			}

			if(enc_length > 0){
				// RLE compressed pixel stream is sent
				if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
				if(!socket->RSCSend((RSC_BYTE *) comp_ptr, enc_length)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
			}else{
				if(!socket->RSCSend((RSC_BYTE *) &enc_length, 4)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
				if(!socket->RSCSend((RSC_BYTE *) conv_ptr, prop->region_h*prop->Bps)){
					delete[] comp_ptr;
					if(converted) delete[] conv_ptr;
					return false;
				}
			}

			delete[] comp_ptr;
			if(converted) delete[] conv_ptr;
			
			// REGION SWAP
			if(*cur_region == 0)
				*cur_region = 1;
			else 
				*cur_region = 0;
			
			break;

		case 1: // Incremental Update

			// Capture the screen
			if(!region[*cur_region].RSCCapture())
				return false;
			

			// Configure initial DIB informaiton
			if(region[*cur_region].m_regionprop.bpp == 8){
				// If 8-bits, use Palette	
				HPALETTE syspal=NULL, oldpal=NULL;

				syspal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
				oldpal = SelectPalette(region[*cur_region].m_hdc, syspal, FALSE);
				
				region[*cur_region].RSCInitDIB(syspal, region[*cur_region].m_regionprop.bpp, BI_RGB);
				
				SelectPalette(region[*cur_region].m_hdc, oldpal, TRUE);
				DeleteObject(syspal);

			}else{
				// If 15/16/24/32 bits, use RGB instead of Palette
				region[*cur_region].RSCInitDIB(NULL, region[*cur_region].m_regionprop.bpp, BI_BITFIELDS);
			}

			int pre_region;
			if(*cur_region == 0) pre_region = 1;
			else pre_region = 0;


			// Convert DDB to DIB
			if(!region[*cur_region].RSCDDB2DIB(region[*cur_region].m_ddbinfo.bmHeight, region[*cur_region].m_dibhsize, DIB_RGB_COLORS))
				return false;
			
			// Sending the data
			data_ptr = (RSC_UBYTE *) region[*cur_region].m_pdib + region[*cur_region].m_dibhsize;
			data_pre = (RSC_UBYTE *) region[pre_region].m_pdib + region[pre_region].m_dibhsize;

			int m_Bpp;
			switch(region[*cur_region].m_regionprop.bpp){
				case 32:
				case 24: // Bpp = 4
					m_Bpp = 4;
					break;
				case 16:
				case 15: // Bpp = 2
					m_Bpp = 2;
					break;
				case 8: // Bpp = 1
					m_Bpp = 1;
					break;
			}
			if(!wv_compare_n_update_conv(socket, data_pre, data_ptr, region[*cur_region].m_regionprop.region_w, region[*cur_region].m_regionprop.region_h, region[*cur_region].m_regionprop.bpp, m_Bpp, region[*cur_region].m_regionprop.Bps, PIXTANGLE_WIDTH, PIXTANGLE_HEIGHT, prop, &region[*cur_region])){ //RSC_REGIONPROP *prop, RSCRegion *region
				return false;
			}


			// REGION SWAP
			if(*cur_region == 0)
				*cur_region = 1;
			else 
				*cur_region = 0;

			break;

		default:
			break;
	}

	return true;
}

bool wv_init(RSCSocket *socket, RSCRegion *region, RSC_REGIONPROP *prop, bool *isinput)
{
	RSC_BYTE access_mode;
	RSC_BYTE red_b = 0, green_b = 0, blue_b = 0;
	
	RSC_UDWORD sw;	// Screen width
	RSC_UDWORD sh;	// Screen height
	RSC_BYTE bpp;	// bps (bits per pixel) = 8 bits (color/grayscale), 16 bits, 24 bits and 32 bits.
	
	RSC_UDWORD display_size = 256;	// Size of string (multi-cast or display name)
	RSC_BYTE display_str[256];		// Array string that indicates the multi-cast session id or display name.
	
	// GetComputerName(display_str);
	// [TO DO LATER]

	strcpy(display_str, "RSC Desktop");

	switch(region->m_dib_pixel_type){
		case DIB_8BPP:
				red_b = 0;
				green_b = 0;
				blue_b = 0;
				break;
		case DIB_16RGB555:
				red_b = 5;
				green_b = 5;
				blue_b = 5;
				break;
		case DIB_16RGB565:
				red_b = 5;
				green_b = 6;
				blue_b = 5;
				break;
		case DIB_24RGB888:
		case DIB_32RGB888:
		case DIB_32RGBA888:
				red_b = 8;
				green_b = 8;
				blue_b = 8;
				break;
		default:
			// Invalid Screen Format ...
			return false;
			break;
	}

	sw = region->m_regionprop.region_w;
	sh = region->m_regionprop.region_h;
	bpp = region->m_regionprop.bpp;
	
	if(!socket->RSCReceive(&access_mode, 1))
		return false;

	// For Screen Stream data flow
	if(access_mode == 0){
		// Call Disconnect_All() functions before continuing with this connection.
		// [TO DO LATER]
	}

	// For Input Steam flow - begins here
	if(access_mode == 1){
		// Call Disconnect_All() functions before continuing with this connection.
		*isinput = true;
		return true;
	}
	if(access_mode == 3){
		// Do not disconnect previously connected clients.
		*isinput = true;
		return true;	
	}
	// ends here
    

	if(!socket->RSCSend((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCSend(&bpp, 1))
		return false;
	if(!socket->RSCSend(&red_b, 1))
		return false;
	if(!socket->RSCSend(&green_b, 1))
		return false;
	if(!socket->RSCSend(&blue_b, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &display_size, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) display_str, 256))
		return false;
	
	// Reply back from Client with PROPERTIES the client want
	if(!socket->RSCReceive((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCReceive(&bpp, 1))
		return false;
	if(!socket->RSCReceive(&red_b, 1))
		return false;
	if(!socket->RSCReceive(&green_b, 1))
		return false;
	if(!socket->RSCReceive(&blue_b, 1))
		return false;

	prop->bpp = bpp;
	prop->region_h = sh;
	prop->region_w = sw;
	prop->Bps = RSCGetBPS(sw, bpp);
	prop->pixel_type = RSCGetPixelTypeFromBitsPerPixel(bpp, red_b, green_b, blue_b);
	
	return true;
}

*/