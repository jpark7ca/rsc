// 
// RSCClient.cpp : the entry point for the RSCClient application.
// By Jae H. Park 
//
// Created: 02.15.2005
// Rev. 02.25.2005, 03.13.2005, 03.14.2005, 03.15.2005, 03.16.2005
// 08.01.2005, 08.02.2005, 10.20.2005, 05.29.2006, 07.23.2006, 10.02.2006
//


#include "stdafx.h"

// Global Variables and Constants
//

#define MAX_LOADSTRING 100
#define MAX_PASSWORD 8
#define	MAX_IPADDRESS 64

#define PIXTANGLE_WIDTH	64
#define PIXTANGLE_HEIGHT 64

#define THREAD_TIME_DELAY 50	// miliseconds
#define MAX_CLIENTCONNECT 20
#define DEFAULT_COMM_PORT 3337

#define HSCR_STEP 5
#define HSCR_PAGE 50
#define VSCR_STEP 5
#define VSCR_PAGE 50

#define MAX_PASSWD_RETRY 3

// Windows Related Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// RSCClient Window Messages
#define	WM_SCREEN_UPDATE WM_APP+10

// RSCClient Type Definition
typedef struct RSC_CLIENT_PROP{
	BYTE m_prop_password[MAX_PASSWORD];
	BYTE m_prop_server_ipaddr[MAX_IPADDRESS];
	u_short m_prop_server_port;
	bool m_prop_readonly;
	int m_speed; // 0 - auto, 1 - slow, 2 - regular, 3 - fast
	HWND m_hwnd;
	RSCRegion m_region;
	RSC_REGIONPROP m_prop_requested;
	RSCPalette m_pal_requested;
	HBITMAP m_ddb;
	int m_desired_resol; // 0 - default, 1 - width & height defined, 2 - custom: needs to calculate height based on ratio
	int m_desired_resol_width;
	int m_desired_resol_height;
}RSC_CLIENT_PROP;

typedef struct RSC_CLIENT_THREAD{
	DWORD			index;		// Inactive: -1   Active: 0, 1, 2, 3, 4, etc...
	HANDLE			thread;
	DWORD			thread_id;
	RSCSocket		*socket;
	bool			socket_connected;
	RSC_CLIENT_PROP	prop;
}RSC_CLIENT_THREAD;

// RSCClient Variables and Functions
HDC					m_hdctmpmem;
HBITMAP				tmpddb;

RSC_CLIENT_PROP		m_prop;

// Region's segments in each thread
RSC_CLIENT_THREAD	*m_threads=NULL;
long				m_threads_ctr=0;

//
RSCSocket			*g_client_i_s;
bool				g_isconnected_i;
RSCSocket			*g_client_s_s;
bool				g_isconnected_s;

//HANDLE region_cs - Critical section;
CRITICAL_SECTION region_cs;
//HANDLE communication_cs - Critical section;
CRITICAL_SECTION communication_cs;
//HANDLE password_cs - Critical section
CRITICAL_SECTION password_cs;

bool RunClient(void);
//VOID run_client_screen(RSC_CLIENT_PROP *param);
VOID run_client_screen(DWORD *param);
VOID run_client_input(RSC_CLIENT_PROP *param);
//bool wv_updatescreen(void);
//bool wv_updatescreen(DWORD index);
bool wv_updatescreen(DWORD index, PRECT rect);
bool wv_updatefullscreen(DWORD index, RSCSocket *client_s, RSC_CLIENT_PROP *prop, BYTE *region_ptr, BYTE *region_cmp);
bool wv_updateincrementalscreen(DWORD index, RSCSocket *client_s, RSC_CLIENT_PROP *prop, BYTE *region_ptr, BYTE *region_pre, BYTE *pixtangle_ptr, BYTE *pixtangle_cmp, RSCPixtangle *pix);
bool wv_init_screen(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_CLIENT_PROP *param);
bool wv_init_input(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_CLIENT_PROP *param);
bool wv_version(RSCSocket *socket);
bool wv_authenticate(RSCSocket *socket);
bool wv_sendkey(RSCSocket *socket, int wParam, DWORD lParam);
bool wv_sendmouse(RSCSocket *socket, int x, int y, DWORD wParam, UINT message);

// MISC Functions
//long CalcNumbersofSubRegion(long sw, long sh, long maxsrw, long maxsrh);
bool CalcNumbersofSubRegion(long sw, long sh, long maxsrw, long maxsrh, long *num_w, long *num_h);
bool GetSubRegion(long sw, long sh, long maxsrw, long maxsrh, long twc, long thc, RSC_CLIENT_THREAD *reg);

// Windows/Components Manipulation
//
// Horizontal and Vertical Scrollbar

int m_xminscroll, m_xcurscroll, m_xmaxscroll;
int m_yminscroll, m_ycurscroll, m_ymaxscroll;
SCROLLINFO m_si;
bool m_size_chg;
bool m_scroll_chg;
BOOL m_scroll_enabled;

int m_screen_w;
int m_screen_h;
int m_fwinrect_w;
int m_fwinrect_h;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	ConnectionProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	PasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global varibles
	m_xminscroll = 0;
	m_xcurscroll = 0;
	m_xmaxscroll = 0;
	m_yminscroll = 0;
	m_ycurscroll = 0;
	m_ymaxscroll = 0;
	
	m_size_chg = false;
	m_scroll_chg = false;
	m_scroll_enabled = FALSE;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RSCCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_RSCCLIENT);

	// Connection dialog box
	*m_prop.m_prop_server_ipaddr = 0;
	
	// 0 - default, 1 - width & height defined, 2 - custom: needs to calculate height based on ratio
	m_prop.m_desired_resol = 0;


	if(!RegGetDWORDValue(HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Client\\", "server_port", (DWORD *) &m_prop.m_prop_server_port)){
		m_prop.m_prop_server_port = DEFAULT_COMM_PORT;
	}

	if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROP), m_prop.m_hwnd, (DLGPROC) ConnectionProc)==IDOK){
		
		if(!RegSetDWORDValue(REG_OPTION_NON_VOLATILE, HKEY_CURRENT_USER, "SOFTWARE\\RSC\\Client\\", "server_port", m_prop.m_prop_server_port)){
			// Error while writing to Registry
			// MessageBox(NULL, "Error has occured while writing PORT registry value", "RSC Client", MB_OK);
		}

	}else{
		// Cancel the command.
		return -1;
	}
	
	// Initialize the main client thread
	RunClient();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_RSCCLIENT);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_RSCCLIENT;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

	m_prop.m_hwnd = hWnd;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int w_new, h_new;
	int x_new, y_new;
	int xdelta, ydelta;
	int xnewpos, ynewpos;
	int i;
	//long style;

	//Debug
	//TCHAR tb[256];
	

	//RECT screenrect, currentrect;
	int screen_w, screen_h;

	HMONITOR hmonitor;
	MONITORINFO mi;

	UINT cap_hittest;

	switch (message) 
	{
	case WM_CREATE:
		// Debug
		//
		//style = GetWindowLong(hWnd, GWL_STYLE);
		//style |= WS_VSCROLL | WS_HSCROLL;
		//SetWindowLong(hWnd, GWL_STYLE, style);

		// Initialize the variables for horizontal and vertical scrolls
		m_xmaxscroll = 0;
		m_xcurscroll = 0;
		m_xminscroll = 0;
		m_ymaxscroll = 0;
		m_ycurscroll = 0;
		m_yminscroll = 0;

		//IDT_TIMER_SCREEN
		//SetTimer(hWnd, IDT_TIMER_SCREEN, 1000, (TIMERPROC) NULL);  

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	
	case WM_SYSCOMMAND:
		//switch(wParam & 0xFFF0){
		switch(wParam){
			case SC_MAXIMIZE:
				
				///////////////////////////////////////////////////////////////////////////////
				// Set the Windows size														 //
				///////////////////////////////////////////////////////////////////////////////	
				//
				// For multiple monitor system - Windows 2000 or higer only
				//
				hmonitor = MonitorFromWindow(m_prop.m_hwnd, MONITOR_DEFAULTTONEAREST);
				mi.cbSize = sizeof(mi);
				GetMonitorInfo(hmonitor, &mi);
				
				screen_w = mi.rcMonitor.right - mi.rcMonitor.left;
				screen_h = mi.rcMonitor.bottom - mi.rcMonitor.top;

				//SystemParametersInfo(SPI_GETWORKAREA, 0, &screenrect, 0);
				//screen_w = screenrect.right -  screenrect.left;
				//screen_h = screenrect.bottom - screenrect.top;
				
				RECT fwinrect;
				SetRect(&fwinrect, 0, 0, m_prop.m_prop_requested.region_o_w, m_prop.m_prop_requested.region_o_h);
				AdjustWindowRect(&fwinrect, GetWindowLong(m_prop.m_hwnd, GWL_STYLE) & ~WS_VSCROLL & ~WS_HSCROLL, 
				TRUE);

				m_fwinrect_w = fwinrect.right - fwinrect.left;
				m_fwinrect_h = fwinrect.bottom - fwinrect.top;

				m_screen_w  = min(m_fwinrect_w, screen_w);
				m_screen_h = min(m_fwinrect_h, screen_h);
				
				// Debug				
				//GetWindowRect(m_prop.m_hwnd, &currentrect);

				SetWindowPos(m_prop.m_hwnd, HWND_TOP,
					mi.rcMonitor.left, // currentrect.left,
					mi.rcMonitor.top,  // currentrect.top,
					m_screen_w, m_screen_h, 
					SWP_SHOWWINDOW | SWP_FRAMECHANGED);

				SetForegroundWindow(m_prop.m_hwnd);
				//UpdateWindow(m_prop.m_hwnd);
				
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	
	//case WM_TIMER:
	//	switch(wParam)
	//	{
	//		case IDT_TIMER_SCREEN:
	//			
	//			PostMessage(hWnd, WM_PAINT, 0, 0);
	//			//MessageBox(NULL, "TIMER", "Debug", MB_OK);
	//			break;
	//			//UpdateWindow(hWnd);
	//
	//	}
	//	break;

	case WM_SIZE:
					
		//if(g_isconnected_s){
		if(m_threads != NULL){
			//if(m_threads[0].socket_connected){
	
			RECT wrect;
			GetWindowRect(hWnd, &wrect);

			w_new = wrect.right - wrect.left;
			h_new = wrect.bottom - wrect.top;

			if(h_new >= m_fwinrect_h && w_new >= m_fwinrect_w){
				y_new = h_new;
				x_new = w_new;

				if(m_scroll_enabled == TRUE){

					m_scroll_enabled = FALSE;
					ShowScrollBar(hWnd, SB_BOTH, m_scroll_enabled);

					LPARAM lp;
					lp = MAKELPARAM(x_new, y_new);
					PostMessage(hWnd, WM_SIZE, 0, lp);
						
					return 0;
				}
				
			}else{
				y_new = HIWORD(lParam);
				x_new = LOWORD(lParam);
				
				// To avoid the recurrent call of WM_SIZE due to Scrollbar Creation
				if(m_scroll_enabled == FALSE){
					m_scroll_enabled = TRUE;				
					ShowScrollBar(hWnd, SB_BOTH, m_scroll_enabled);
					
					LPARAM lp;
					lp = MAKELPARAM(x_new-GetSystemMetrics(SM_CYHSCROLL), y_new-GetSystemMetrics(SM_CXVSCROLL));
					PostMessage(hWnd, WM_SIZE, 0, lp);
					
					return 0;
				}

			}

			m_size_chg = true;

			// Vertical scroll
			memset(&m_si, 0, sizeof(m_si));
		
			m_ymaxscroll = max(m_prop.m_prop_requested.region_o_h-y_new, 0);
			
			//
			EnterCriticalSection(&region_cs);
			m_ycurscroll = min(m_ycurscroll, m_ymaxscroll);
			//
			LeaveCriticalSection(&region_cs);

			m_si.cbSize = sizeof(m_si);
			m_si.fMask = SIF_ALL;
			m_si.nMin = m_yminscroll;
			//m_si.nMax = m_prop.m_prop_requested.region_h; //m_ymaxscroll;
			m_si.nMax = m_prop.m_prop_requested.region_o_h;
			m_si.nPage = y_new;
			m_si.nPos = m_ycurscroll;

			SetScrollInfo(hWnd, SB_VERT, &m_si, TRUE);


			// Horizontal scroll			
			memset(&m_si, 0, sizeof(m_si));
			//m_xmaxscroll = max(m_prop.m_prop_requested.region_w-x_new, 0); //m_prop.m_prop_requested.region_w;
			m_xmaxscroll = max(m_prop.m_prop_requested.region_o_w-x_new, 0);
			
			//
			EnterCriticalSection(&region_cs);
			m_xcurscroll = min(m_xcurscroll, m_xmaxscroll);
			//
			LeaveCriticalSection(&region_cs);

			m_si.cbSize = sizeof(m_si);
			m_si.fMask = SIF_ALL;
			m_si.nMin = m_xminscroll;
			//m_si.nMax = m_prop.m_prop_requested.region_w; //m_xmaxscroll;
			m_si.nMax = m_prop.m_prop_requested.region_o_w;
			m_si.nPage = x_new;
			m_si.nPos = m_xcurscroll;

			SetScrollInfo(hWnd, SB_HORZ, &m_si, TRUE);
		
		//}
			//UpdateWindow(hWnd);
		}
		break;
	
	case WM_SIZING:
		{
			//MessageBox(NULL, "WM_SIZING", "Debug - Client", MB_OK);			
			RECT *w_size = (LPRECT) lParam;
			RECT w_osize;

			GetWindowRect(hWnd, &w_osize);

			switch (wParam) {
				case WMSZ_RIGHT: 
				case WMSZ_TOPRIGHT:
				case WMSZ_BOTTOMRIGHT:
					
					w_size->right = min(w_size->right, w_osize.left + m_fwinrect_w);
					break;
				case WMSZ_LEFT:
				case WMSZ_TOPLEFT:
				case WMSZ_BOTTOMLEFT:
					
					w_size->left = max(w_size->left, w_osize.right - m_fwinrect_w);
					break;
				}
					
			switch (wParam) {
				case WMSZ_TOP:
				case WMSZ_TOPLEFT:
				case WMSZ_TOPRIGHT:

					w_size->top = max(w_size->top, w_osize.bottom - m_fwinrect_h);
					break;
				case WMSZ_BOTTOM:
				case WMSZ_BOTTOMLEFT:
				case WMSZ_BOTTOMRIGHT:

					w_size->bottom = min(w_size->bottom, w_osize.top + m_fwinrect_h);
					break;
			}
			
			return 0;
		}
	
	case WM_HSCROLL:

		//
		//EnterCriticalSection(&region_cs);
		
		ydelta = 0;
		switch(LOWORD(wParam))
		{
			case SB_PAGEUP:
				xnewpos = m_xcurscroll - HSCR_PAGE;
				break;
			case SB_PAGEDOWN:
				xnewpos = m_xcurscroll + HSCR_PAGE;
				break;
			case SB_LINEUP:
				xnewpos = m_xcurscroll - HSCR_STEP;
				break;
			case SB_LINEDOWN:
				xnewpos = m_xcurscroll + HSCR_STEP;
				break;
			case SB_THUMBPOSITION:
				xnewpos = HIWORD(wParam);
				break;
			default:
				xnewpos = m_xcurscroll;
		}

		// New position must be between 0 and the screen width
		xnewpos = max(0, xnewpos);
		xnewpos = min(m_xmaxscroll, xnewpos);

		// If the current position does not change, do not scroll
		if(xnewpos == m_xcurscroll){
			//
			//LeaveCriticalSection(&region_cs);	
			break;
		}

		// Set the scroll flag
		m_scroll_chg = true;

		// Determine the amount scrolled
		xdelta = xnewpos - m_xcurscroll;

		// Reset the current scroll position
		//
		EnterCriticalSection(&region_cs);
		m_xcurscroll = xnewpos;
		//
		LeaveCriticalSection(&region_cs);

		// Scroll the window.
		ScrollWindowEx(hWnd, -xdelta, -ydelta, (CONST RECT *) NULL, 
			(CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL,
			SW_ERASE ); //SW_INVALIDATE);
		
		// Debug
		//
		//wsprintf(tb, "%d", m_xcurscroll);
		//MessageBox(NULL, tb, "X", MB_OK);

		//
		UpdateWindow(hWnd);
		//RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
		//PostMessage(hWnd, WM_PAINT, 0, 0);



		// Reset the scroll bar
		m_si.cbSize = sizeof(m_si);
		m_si.fMask = SIF_POS;
		m_si.nPos = m_xcurscroll;
		SetScrollInfo(hWnd, SB_HORZ, &m_si, TRUE);

		//
		//LeaveCriticalSection(&region_cs);
		break;

	case WM_VSCROLL:

		//
		//EnterCriticalSection(&region_cs);

		xdelta = 0;
		switch(LOWORD(wParam))
		{
			case SB_PAGEUP:
				ynewpos = m_ycurscroll - VSCR_PAGE;
				break;
			case SB_PAGEDOWN:
				ynewpos = m_ycurscroll + VSCR_PAGE;
				break;
			case SB_LINEUP:
				ynewpos = m_ycurscroll - VSCR_STEP;
				break;
			case SB_LINEDOWN:
				ynewpos = m_ycurscroll + VSCR_STEP;
				break;
			case SB_THUMBPOSITION:
				ynewpos = HIWORD(wParam);
				break;
			default:
				ynewpos = m_ycurscroll;
		}

		// New position must be between 0 and the screen width
		ynewpos = max(0, ynewpos);
		ynewpos = min(m_ymaxscroll, ynewpos);

		// If the current position does not change, do not scroll
		if(ynewpos == m_ycurscroll){
			//
			//LeaveCriticalSection(&region_cs);
			break;
		}

	
		// Set the scroll flag
		m_scroll_chg = true;

		// Determine the amount scrolled
		ydelta = ynewpos - m_ycurscroll;

		// Reset the current scroll position
		//
		EnterCriticalSection(&region_cs);
		m_ycurscroll = ynewpos;
		//
		LeaveCriticalSection(&region_cs);

		// Scroll the window.
		ScrollWindowEx(hWnd, -xdelta, -ydelta, (CONST RECT *) NULL, 
			(CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL,
			SW_ERASE ); //SW_INVALIDATE);
		
		// Debug
		//
		//wsprintf(tb, "%d", m_ycurscroll);
		//MessageBox(NULL, tb, "Y", MB_OK);

		UpdateWindow(hWnd);
		//RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
		//PostMessage(hWnd, WM_PAINT, 0, 0);

		// Reset the scroll bar
		m_si.cbSize = sizeof(m_si);
		m_si.fMask = SIF_POS;
		m_si.nPos = m_ycurscroll;
		SetScrollInfo(hWnd, SB_VERT, &m_si, TRUE);

		//
		//LeaveCriticalSection(&region_cs);

		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		
		////////////////////////////////////////////////////////////
		// Begin Mutex section
		////////////////////////////////////////////////////////////
		//WaitForSingleObject(communication_mutex, INFINITE);
		
		if(g_client_i_s != NULL && g_isconnected_i)
			if(!wv_sendkey(g_client_i_s, (int) wParam, (DWORD) lParam)){
				// [ Needs to be done later ]
				//
				// Error while sending key to the server.
				// kill the process
			}
		
		////////////////////////////////////////////////////////////
		// Release Mutex Section
		////////////////////////////////////////////////////////////
		//ReleaseMutex(communication_mutex);
		
		break;
	case WM_NCLBUTTONDBLCLK:
		cap_hittest = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
		if(cap_hittest == HTCAPTION){
			//MessageBox(NULL, "HTCAPTION", "Debug - Client", MB_OK);
		}else{
			return cap_hittest;
		}

		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:

		////////////////////////////////////////////////////////////
		// Begin Mutex section
		////////////////////////////////////////////////////////////
		//WaitForSingleObject(communication_mutex, INFINITE);
		
		if(g_client_i_s != NULL && g_isconnected_i)
			if(!wv_sendmouse(g_client_i_s, LOWORD(lParam), HIWORD(lParam), wParam, message)){
				// [ Needs to be done later ]
				//
				// Error while sending key to the server.
				// kill the process
			}
		
		////////////////////////////////////////////////////////////
		// Release Mutex Section
		////////////////////////////////////////////////////////////
		//ReleaseMutex(communication_mutex);

		break;
	case WM_SCREEN_UPDATE:
		
		if(m_threads != NULL){
			
			//if(m_threads[(int)lParam].thread != NULL){
				
				//hdc = BeginPaint(hWnd, &ps);
				//MessageBox(NULL, "Screen Update", "Debug", MB_OK);
				
				RECT wrect;
				GetWindowRect(hWnd, &wrect);
				wv_updatescreen((DWORD) lParam, &wrect);
			
				//EndPaint(hWnd, &ps);
			//}
			
			//PostMessage(hWnd, WM_PAINT, 0, 0);
			
			
		}
		
		break;
	case WM_PAINT:
		if(m_threads != NULL){
				
				
			hdc = BeginPaint(hWnd, &ps);

			// TODO: Add any drawing code here...
			RECT wrect;
			GetWindowRect(hWnd, &wrect);
				
			for(i=0; i < m_threads_ctr; i++){

				wv_updatescreen((DWORD) i, &wrect);

			}
			
			if(m_scroll_chg)
				m_scroll_chg = false;

			if(m_size_chg)
				m_size_chg = false;

			EndPaint(hWnd, &ps);
			
			
		}
		break;
	case WM_DESTROY:
		//KillTimer(hWnd, IDT_TIMER_SCREEN);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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

LRESULT CALLBACK ConnectionProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    switch (message) 
    { 
		case WM_INITDIALOG:
			// Default Value: Check IDC_RADIO_AUTO button

			//CheckDlgButton(hDlg, IDC_RADIO_AUTO, BST_CHECKED);
			//EnableWindow(GetDlgItem(hDlg, IDC_RADIO_AUTO), FALSE);
            		
			//
			BYTE buf[10];
			itoa((int) m_prop.m_prop_server_port, (char *) buf, 10);
			SetDlgItemText(hDlg, IDC_EDIT_PORT,(LPSTR) buf);

			// Debug
			//HWND btn =GetDlgItem(hDlg, IDC_RADIO_8BITS) 
			//EnableWindow(btn, FALSE);  
			
			switch(RSCGetCurrentDCbpp())
			{
				case 8:
					EnableWindow(GetDlgItem(hDlg, IDC_RADIO_16BITS), FALSE); 
					EnableWindow(GetDlgItem(hDlg, IDC_RADIO_32BITS), FALSE);
					//CheckDlgButton(hDlg, IDC_RADIO_8BITS, BST_CHECKED);
					break;
				case 15:
				case 16:
					EnableWindow(GetDlgItem(hDlg, IDC_RADIO_32BITS), FALSE);
					//CheckDlgButton(hDlg, IDC_RADIO_16BITS, BST_CHECKED);
					break;
				case 24:
				case 32:
					//CheckDlgButton(hDlg, IDC_RADIO_32BITS, BST_CHECKED);
					break;
				default:
					break;

			}
			
			// Default: 8bits slow network
			CheckDlgButton(hDlg, IDC_RADIO_8BITS, BST_CHECKED);

			// Default Resolution
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CUSTOM_WIDTH), FALSE);	
			CheckDlgButton(hDlg, IDC_RADIO_NO_RES_CHANGE, BST_CHECKED);

			return TRUE;
			break;
		
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
				case IDC_RADIO_CUSTOM_WIDTH:
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CUSTOM_WIDTH), TRUE);	
					break;
				case IDC_RADIO_800_600:
				case IDC_RADIO_1024_768:
				case IDC_RADIO_NO_RES_CHANGE:
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CUSTOM_WIDTH), FALSE);	
					break;

                case IDOK: 
                    if (!GetDlgItemText(hDlg, IDC_EDIT_SERVER, 
                             (LPSTR) m_prop.m_prop_server_ipaddr, MAX_IPADDRESS))
                         *m_prop.m_prop_server_ipaddr=0; 

					BYTE buf[10];
					if(GetDlgItemText(hDlg, IDC_EDIT_PORT, (LPSTR) buf, 10))
						m_prop.m_prop_server_port = atoi((const char *) buf);					

					//if(IsDlgButtonChecked(hDlg, IDC_RADIO_AUTO) == BST_CHECKED){
					//	// Debug
					//	//MessageBox(NULL, "Auto Checked", "Debug", MB_OK);
					//	m_prop.m_speed = 0;
					//}else 
						
					if(IsDlgButtonChecked(hDlg, IDC_RADIO_8BITS) == BST_CHECKED){
						m_prop.m_speed = 1;
					}else if(IsDlgButtonChecked(hDlg, IDC_RADIO_16BITS) == BST_CHECKED){
						m_prop.m_speed = 2;
					}else if(IsDlgButtonChecked(hDlg, IDC_RADIO_32BITS) == BST_CHECKED){
						m_prop.m_speed = 3;
					}else{
						// Not checked translates as AUTO
						m_prop.m_speed = 0;
					}

					// 0 - default, 1 - width & height defined, 2 - custom: needs to calculate height based on ratio
					if(IsDlgButtonChecked(hDlg, IDC_RADIO_NO_RES_CHANGE) == BST_CHECKED){
						m_prop.m_desired_resol = 0;
					}else if(IsDlgButtonChecked(hDlg, IDC_RADIO_1024_768) == BST_CHECKED){
						m_prop.m_desired_resol = 1;
						m_prop.m_desired_resol_width = 1024;
						m_prop.m_desired_resol_height = 768;
					}else if(IsDlgButtonChecked(hDlg, IDC_RADIO_800_600) == BST_CHECKED){
						m_prop.m_desired_resol = 1;
						m_prop.m_desired_resol_width = 800;
						m_prop.m_desired_resol_height = 600;
					}else if(IsDlgButtonChecked(hDlg, IDC_RADIO_CUSTOM_WIDTH) == BST_CHECKED){
						if(GetDlgItemText(hDlg, IDC_EDIT_CUSTOM_WIDTH, (LPSTR) buf, 10)){
							m_prop.m_desired_resol = 2;	
							m_prop.m_desired_resol_width = atoi((const char *) buf);	
						}
					}

					memset(m_prop.m_prop_password, NULL, MAX_PASSWORD);
					memset(buf, 0, 10);

					if(GetDlgItemText(hDlg, IDC_EDIT_PASSWORD, (LPSTR) buf, MAX_PASSWORD+1)){
						memcpy(m_prop.m_prop_password, buf, MAX_PASSWORD);
					}else{
						// PASSWORD Field left blank
						memset(m_prop.m_prop_password, NULL, MAX_PASSWORD);
					}
 
                case IDCANCEL: 
                    EndDialog(hDlg, wParam); 
                    return TRUE; 
            } 
    } 
    return FALSE; 
} 

LRESULT CALLBACK PasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    switch (message) 
    { 
		case WM_INITDIALOG:
			// Default Value: Check IDC_RADIO_AUTO button
			return TRUE;
			break;
		
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
				case IDOK: 
                    
					BYTE buf[10];
					
					memset(m_prop.m_prop_password, NULL, MAX_PASSWORD);
					memset(buf, 0, 10);

					if(GetDlgItemText(hDlg, IDC_EDIT_PASSWORD_DLG, (LPSTR) buf, MAX_PASSWORD+1)){
						memcpy(m_prop.m_prop_password, buf, MAX_PASSWORD);
					}else{
						// PASSWORD Field left blank
						memset(m_prop.m_prop_password, NULL, MAX_PASSWORD);
					}
 
                case IDCANCEL: 
                    EndDialog(hDlg, wParam); 
                    return TRUE; 
            } 
    } 
    return FALSE; 
} 




// RSCClient

bool RunClient(void)
{

	// Release the screen DC and assign current window DC
	if(m_prop.m_region.m_hdc != NULL)
		ReleaseDC(NULL, m_prop.m_region.m_hdc);
	
	// Get the current window's DC
	m_prop.m_region.m_hdc = GetDC(m_prop.m_hwnd);

	// Reset m_region.m_hdcmem if m_region.m_hdcmem were non-NULL MEMDC
	if(m_prop.m_region.m_hdcmem != NULL){
		DeleteDC(m_prop.m_region.m_hdcmem);
		m_prop.m_region.m_hdcmem = NULL;
	}

	// Reset m_ddb value if m_ddb were non-NULL DDB
	if(m_prop.m_ddb != NULL){
		DeleteObject(m_prop.m_ddb);
		m_prop.m_ddb = NULL;
	}

	// Init Critical Section variables
	InitializeCriticalSection(&region_cs);
	InitializeCriticalSection(&communication_cs);
	InitializeCriticalSection(&password_cs);


	//
	// Run run_client_input
	//

	HANDLE worker_t;
	DWORD worker_id;
	g_client_i_s = NULL;
	g_isconnected_i = false;
	
	worker_t = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) run_client_input, &m_prop, 0, &worker_id);
		
	if(worker_id == -1){
		//
		// Error occurred while creating a worker thread
		//
		return false;
	}
	///////////////////////////////////////////////////////////////////////////////


	return true;
}

VOID run_client_input(RSC_CLIENT_PROP *param)
{
	RSCSocket client_s;

	////////////////////////////////////////////////////////////
	// Begin Critical section
	////////////////////////////////////////////////////////////
	EnterCriticalSection(&communication_cs);
	
	// System-Wide Winsock Init
	
	if(!client_s.RSCSocketInit_W()){
		// Error occurred while initializing the win32 socket
		// Send WM_DESTROY or WM_CLOSE message to close the application

		// MessageBox(NULL, "RSCSocketInit_W exiting ...", "Debug", MB_OK);
		LeaveCriticalSection(&communication_cs);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}
	

	// Input update socket
	//
	//if(!client_s.RSCCreateStream(false)){
	if(!client_s.RSCCreateStream(true)){
		// Error occurred while creating the win32 socket session
		client_s.RSCSocketUninst_W();
		
		// MessageBox(NULL, "RSCCreateStream exiting ...", "Debug", MB_OK);
		LeaveCriticalSection(&communication_cs);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);
		
		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;

	}

	// Input update socket
	//
	if(!client_s.RSCConnect(param->m_prop_server_port, (char *) param->m_prop_server_ipaddr)){
		// Error occurred while connecting the RSCServer
		//client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		LeaveCriticalSection(&communication_cs);

		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}

	g_client_i_s = &client_s;

	///////////////////////////////////////////////////////////////////////////////
	// VERSION CHECK AND AUTHENTICATION 
	///////////////////////////////////////////////////////////////////////////////

	// Input update socket
	//
	if(!wv_authenticate(&client_s)){
		// Failed to authenticate this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		g_client_i_s = NULL;
		
		//
		// MessageBox(NULL, "wv_authenticate in input exiting ...", "Debug", MB_OK);
		LeaveCriticalSection(&communication_cs);
		MessageBox(NULL, "Your access request has been denied", "RSC Client", MB_OK | MB_ICONWARNING);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION BETWEEN CLIENT AND SERVER
	///////////////////////////////////////////////////////////////////////////////

	// Input update socket
	// 

	if(!wv_init_input(&client_s, &param->m_region, &param->m_pal_requested, param)){
		// Failed to init the server-client for this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		
		//
		// MessageBox(NULL, "wv_init_input exiting ...", "Debug", MB_OK);
		LeaveCriticalSection(&communication_cs);
		MessageBox(NULL, "An error has occurred while initializing the RSC session", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}


	////////////////////////////////////////////////////////////
	// Release Critical Section
	////////////////////////////////////////////////////////////
	LeaveCriticalSection(&communication_cs);


	////////////////////////////////////////////////////////////
	// Sub-region will be managed by each thread ...
	////////////////////////////////////////////////////////////
	
	//HANDLE worker_t;
	//DWORD worker_id;
	//g_client_s_s = NULL;
	//g_isconnected_s = false;
	long sw, sh;
	long pw, ph;
	long nw, nh;

	sw = m_prop.m_prop_requested.region_o_w;
	pw = sw/2; //3; //4;
	sh = m_prop.m_prop_requested.region_o_h;
	ph = sh/2; //3; //4;

	CalcNumbersofSubRegion(sw, sh, pw, ph, &nw, &nh);
	m_threads_ctr = nw*nh;


	m_threads = new RSC_CLIENT_THREAD[m_threads_ctr];
	
	
	// Reset m_threads[x].index to -1
	for(int i=0; i < m_threads_ctr; i++){
		m_threads[i].index = -1;
		m_threads[i].socket = NULL;
		m_threads[i].socket_connected =false;
	}

	GetSubRegion(sw, sh, pw, ph, nw, nh, m_threads);

	// Debug
	//
	//TCHAR tmpbuf[256];
	//wsprintf(tmpbuf, "%dx%d, %dx%d, %dx%d, %dx%d", m_threads[0].prop.m_prop_requested.region_x, m_threads[0].prop.m_prop_requested.region_y, m_threads[1].prop.m_prop_requested.region_x, m_threads[1].prop.m_prop_requested.region_y, m_threads[2].prop.m_prop_requested.region_x, m_threads[2].prop.m_prop_requested.region_y, m_threads[3].prop.m_prop_requested.region_x, m_threads[3].prop.m_prop_requested.region_y);
	//wsprintf(tmpbuf, "w:%d   h:%d", sw, sh);
	//MessageBox(NULL, tmpbuf, "Client Debug", MB_OK);

	// Debug
	//m_threads_ctr = 1;
	
	for(int i=0; i < m_threads_ctr; i++){
		m_threads[i].index = i;
		m_threads[i].thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) run_client_screen, &m_threads[i].index, 0, &m_threads[i].thread_id);
			
		if(m_threads[i].thread_id == -1){
			//
			// Error occurred while creating a worker thread
			//
			PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
			return;
		}
	}

	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////////
	// WHILE Loops
	///////////////////////////////////////////////////////////////////////////////

	bool isconnected = true;

	// Additional code for Keyboard event 
	g_isconnected_i = isconnected;

	
	while(g_isconnected_i){

		Sleep(5000);
	}

	// Return current thread handle
	client_s.RSCShutdown(SD_BOTH);
	client_s.RSCClose();
	client_s.RSCSocketUninst_W();
	g_client_i_s = NULL;
	
	//
	// MessageBox(NULL, "runclient_input exiting ...", "Debug", MB_OK);

	PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
	return;	

}

/*
VOID run_client_screen(RSC_CLIENT_PROP *param)
{
	RSCSocket client_s, client_i_s;
	RSC_CLIENT_PROP sparam;  // A thread specifc param variable 
	
	////////////////////////////////////////////////////////////
	// Begin Critical Section
	////////////////////////////////////////////////////////////
	EnterCriticalSection(&communication_cs);
	
	// System-Wide Winsock Init
	if(!client_s.RSCSocketInit_W()){
		//
		// Error occurred while initializing the win32 socket
		// Send WM_DESTROY or WM_CLOSE message to close the application
		
		// Error Message
		// MessageBox(NULL, "Error occurred while initializing Win32 socket ...", "RSC Client", MB_OK);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	// Screen update socket
	//
	if(!client_s.RSCCreateStream(true)){
		// Error occurred while creating the win32 socket session
		client_s.RSCSocketUninst_W();
		
		// Error Message
		// MessageBox(NULL, "Error occurred while creating Win32 socket ...", "RSC Client", MB_OK);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);
		
		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;

	}


	// Screen update socket
	//
	if(!client_s.RSCConnect(param->m_prop_server_port, (char *) param->m_prop_server_ipaddr)){
		// Error occurred while connecting the RSCServer
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		
		// 
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}

	g_client_s_s = &client_s;

	///////////////////////////////////////////////////////////////////////////////
	// VERSION CHECK AND AUTHENTICATION 
	///////////////////////////////////////////////////////////////////////////////

	// Screen update socket
	//
	if(!wv_authenticate(&client_s)){
		// Failed to authenticate this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		g_client_s_s = NULL;
		
		// 
		MessageBox(NULL, "Your access request has been denied", "RSC Client", MB_OK | MB_ICONWARNING);
		
		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Start input thread *******
	//
	//
	//HANDLE worker_t1;
	//DWORD worker_id1;
	//g_client_i_s = NULL;
	//g_isconnected_i = false;
	//
	//worker_t1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) run_client_input, &m_prop, 0, &worker_id1);
	//	
	//if(worker_id1 == -1){
	//	//
	//	// Error occurred while creating a worker thread
	//	//
	//
	//	PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
	//	return;
	//}
	//
	///////////////////////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION BETWEEN CLIENT AND SERVER
	///////////////////////////////////////////////////////////////////////////////

	// Screen update socket
	//
	
	// Copy global property values to the local variable (sparam)
	memcpy(&sparam.m_desired_resol, &param->m_desired_resol, sizeof(param->m_desired_resol));
	memcpy(&sparam.m_desired_resol_height, &param->m_desired_resol_height, sizeof(param->m_desired_resol_height));
	memcpy(&sparam.m_desired_resol_width, &param->m_desired_resol_width, sizeof(param->m_desired_resol_width));
	memcpy(&sparam.m_speed, &param->m_speed, sizeof(param->m_speed));
	
	//if(!wv_init_screen(&client_s, &param->m_region, &param->m_pal_requested, param)){
	if(!wv_init_screen(&client_s, &param->m_region, &param->m_pal_requested, &sparam)){
		// Failed to init the server-client for this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		
		//
		// MessageBox(NULL, "wv_init_screen exiting ...", "Debug", MB_OK);
		MessageBox(NULL, "An error has occurred while initializing the RSC session", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	
	////////////////////////////////////////////////////////////
	// Release Critcal Section
	////////////////////////////////////////////////////////////
	LeaveCriticalSection(&communication_cs);


	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION FOR REGION FULL OR PARTIAL UPDATE PROCESS
	///////////////////////////////////////////////////////////////////////////////

	bool isconnected = true;
	BYTE *cmp_pixtangle;
	BYTE *src_pixtangle;
	BYTE *region_ptr[2];
	BYTE cur_region=0;
	BYTE pre_region=1;
	int Bpp;

	//switch(param->m_prop_requested.bpp){
	switch(sparam.m_prop_requested.bpp){
		case 32:
		case 24:
			Bpp = 4;
			break;
		case 16:
		case 15:
			Bpp = 2;
			break;
		case 8:
			Bpp = 1;
			break;
		default:
			client_s.RSCShutdown(SD_BOTH);
			client_s.RSCClose();
			client_s.RSCSocketUninst_W();
			//g_client_i_s = NULL;
			
			//  Debug
			// MessageBox(NULL, "Incorrect Requested BPP exiting ...", "Debug", MB_OK);

			PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
			return;	
	}

	//region_ptr[0] = (BYTE *) malloc(param->m_prop_requested.region_h*param->m_prop_requested.Bps);
	region_ptr[0] = (BYTE *) malloc(sparam.m_prop_requested.region_h*sparam.m_prop_requested.Bps);
	//region_ptr[1] = (BYTE *) malloc(param->m_prop_requested.region_h*param->m_prop_requested.Bps);
	region_ptr[1] = (BYTE *) malloc(sparam.m_prop_requested.region_h*sparam.m_prop_requested.Bps);
	
	cmp_pixtangle = (BYTE *) malloc(PIXTANGLE_WIDTH*PIXTANGLE_HEIGHT*Bpp);
	src_pixtangle = (BYTE *) malloc(PIXTANGLE_WIDTH*PIXTANGLE_HEIGHT*Bpp);

	if(region_ptr[0] == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		
		// Debug
		// MessageBox(NULL, "region_ptr[0] exiting ...", "Debug", MB_OK);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}
	if(region_ptr[1] == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		
		// Debug
		// MessageBox(NULL, "region_ptr[1] exiting ...", "Debug", MB_OK);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}	
	if(cmp_pixtangle == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		
		// Debug
		// MessageBox(NULL, "cmp_pixtangle exiting ...", "Debug", MB_OK);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;			
	}
	if(src_pixtangle == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		free(cmp_pixtangle);
		
		// Debug
		// MessageBox(NULL, "src_pixtangle exiting ...", "Debug", MB_OK);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		return;		
	}

	
	///////////////////////////////////////////////////////////////////////////////
	// Full Update
	///////////////////////////////////////////////////////////////////////////////
		
	////////////////////////////////////////////////////////////
	// Begin Critical Section
	////////////////////////////////////////////////////////////
	// EnterCriticalSection(&communication_cs);

	if(!wv_updatefullscreen(&client_s, &sparam, region_ptr[cur_region], region_ptr[pre_region])){
		// Failed in update_fullscreen
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		free(cmp_pixtangle);
		free(src_pixtangle);
		
		// Debug
		// MessageBox(NULL, "wv_updatefullscreen exiting ...", "Client Debug", MB_OK);

		PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
		
		////////////////////////////////////////////////////////////
		// Release Critical Section
		////////////////////////////////////////////////////////////
		// LeaveCriticalSection(&region_cs);

		return;	
	}
	////////////////////////////////////////////////////////////
	// Release Critical Section
	////////////////////////////////////////////////////////////
	// LeaveCriticalSection(&communication_cs);


	if(cur_region == 0){
		cur_region = 1;
		pre_region = 0;
	}else{
		cur_region = 0;
		pre_region = 1;
	}


	RSCPixtangle pix(sparam.m_prop_requested.region_w, 
					sparam.m_prop_requested.region_h, 
					sparam.m_prop_requested.Bps,
					sparam.m_prop_requested.bpp/8, 
					PIXTANGLE_WIDTH, PIXTANGLE_HEIGHT, false);



	// Set global variable for screen connection to ON
	g_isconnected_s = isconnected;

	///////////////////////////////////////////////////////////////////////////////
	// Set the Windows size														 //
	///////////////////////////////////////////////////////////////////////////////
	
	RECT screenrect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screenrect, 0);
	int screen_w = screenrect.right -  screenrect.left;
	int screen_h = screenrect.bottom - screenrect.top;
	
	RECT fwinrect;


	SetRect(&fwinrect, 0, 0, sparam.m_prop_requested.region_o_w, sparam.m_prop_requested.region_o_h);
	
	AdjustWindowRect(&fwinrect, GetWindowLong(m_prop.m_hwnd, GWL_STYLE), TRUE);

	m_fwinrect_w = fwinrect.right - fwinrect.left;
	m_fwinrect_h = fwinrect.bottom - fwinrect.top;

	m_screen_w  = min(m_fwinrect_w, screen_w);
	m_screen_h = min(m_fwinrect_h, screen_h);


	SetWindowPos(m_prop.m_hwnd, HWND_TOP,
		0,
		0,
		m_screen_w, m_screen_h, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	
	SetForegroundWindow(m_prop.m_hwnd);

	//


	// Time delay if required...
	DWORD delay_tick, delay_elapsed;

	while(isconnected){

		// Time tick-count
		delay_tick = GetTickCount();
		
		if(!wv_updateincrementalscreen(&client_s, &sparam, region_ptr[cur_region], region_ptr[pre_region], src_pixtangle, cmp_pixtangle, &pix)){

			isconnected = false;
			if(g_isconnected_i) g_isconnected_i = false;
		}
		
		if(cur_region == 0){
			cur_region = 1;
			pre_region = 0;
		}else{
			cur_region = 0;
			pre_region = 1;
		}

		delay_elapsed = GetTickCount() - delay_tick;
		
		if(delay_elapsed < THREAD_TIME_DELAY){			
				Sleep(THREAD_TIME_DELAY-delay_elapsed);
		}

		// Reset Tick counters
		delay_elapsed = 0;
		delay_tick = 0;
	
	}

	// Return current thread handle
	client_s.RSCShutdown(SD_BOTH);
	client_s.RSCClose();
	client_s.RSCSocketUninst_W();
	g_client_s_s = NULL;
	g_isconnected_s = isconnected; //false;

	free(region_ptr[0]);
	free(region_ptr[1]);
	free(cmp_pixtangle);
	free(src_pixtangle);
	

	PostMessage(param->m_hwnd, WM_DESTROY, 0, 0);
	return;	

}
*/

VOID run_client_screen(DWORD *param)
{
	RSCSocket client_s, client_i_s;
	//RSC_CLIENT_PROP sparam;  // A thread specifc param variable 
	
	////////////////////////////////////////////////////////////
	// Begin Critical Section
	////////////////////////////////////////////////////////////
	EnterCriticalSection(&communication_cs);
	
	// System-Wide Winsock Init
	if(!client_s.RSCSocketInit_W()){
		//
		// Error occurred while initializing the win32 socket
		// Send WM_DESTROY or WM_CLOSE message to close the application
		
		// Error Message
		// MessageBox(NULL, "Error occurred while initializing Win32 socket ...", "RSC Client", MB_OK);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	// Screen update socket
	//
	if(!client_s.RSCCreateStream(true)){
		// Error occurred while creating the win32 socket session
		client_s.RSCSocketUninst_W();
		
		// Error Message
		// MessageBox(NULL, "Error occurred while creating Win32 socket ...", "RSC Client", MB_OK);
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);
		
		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;

	}


	// Screen update socket
	//
	if(!client_s.RSCConnect(m_prop.m_prop_server_port, (char *) m_prop.m_prop_server_ipaddr)){
		// Error occurred while connecting the RSCServer
		//client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		
		// 
		MessageBox(NULL, "An error has occurred while connecting to the RSC Server", "RSC Client", MB_OK | MB_ICONERROR);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}

	m_threads[*param].socket = &client_s;

	///////////////////////////////////////////////////////////////////////////////
	// VERSION CHECK AND AUTHENTICATION 
	///////////////////////////////////////////////////////////////////////////////

	// Screen update socket
	//
	if(!wv_authenticate(&client_s)){
		// Failed to authenticate this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		m_threads[*param].socket = NULL;
		
		// 
		MessageBox(NULL, "Your access request has been denied", "RSC Client", MB_OK | MB_ICONWARNING);
		
		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	
	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION BETWEEN CLIENT AND SERVER
	///////////////////////////////////////////////////////////////////////////////

	// Screen update socket
	//
	
	// Copy global property values to the local variable (sparam)
	memcpy(&m_threads[*param].prop.m_desired_resol, &m_prop.m_desired_resol, sizeof(m_prop.m_desired_resol));
	memcpy(&m_threads[*param].prop.m_desired_resol_height, &m_prop.m_desired_resol_height, sizeof(m_prop.m_desired_resol_height));
	memcpy(&m_threads[*param].prop.m_desired_resol_width, &m_prop.m_desired_resol_width, sizeof(m_prop.m_desired_resol_width));
	memcpy(&m_threads[*param].prop.m_speed, &m_prop.m_speed, sizeof(m_prop.m_speed));
	
	//if(!wv_init_screen(&client_s, &param->m_region, &param->m_pal_requested, param)){
	if(!wv_init_screen(&client_s, &m_prop.m_region, &m_prop.m_pal_requested, &m_threads[*param].prop)){
		// Failed to init the server-client for this connection
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		
		//
		// MessageBox(NULL, "wv_init_screen exiting ...", "Debug", MB_OK);
		MessageBox(NULL, "An error has occurred while initializing the RSC session", "RSC Client", MB_OK | MB_ICONERROR);
		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;
	}

	
	////////////////////////////////////////////////////////////
	// Release Critcal Section
	////////////////////////////////////////////////////////////
	LeaveCriticalSection(&communication_cs);


	///////////////////////////////////////////////////////////////////////////////
	// INITIALIZATION FOR REGION FULL OR PARTIAL UPDATE PROCESS
	///////////////////////////////////////////////////////////////////////////////

	bool isconnected = true;
	BYTE *cmp_pixtangle;
	BYTE *src_pixtangle;
	BYTE *region_ptr[2];
	BYTE cur_region=0;
	BYTE pre_region=1;
	int Bpp;

	//switch(param->m_prop_requested.bpp){
	switch(m_threads[*param].prop.m_prop_requested.bpp){
		case 32:
		case 24:
			Bpp = 4;
			break;
		case 16:
		case 15:
			Bpp = 2;
			break;
		case 8:
			Bpp = 1;
			break;
		default:
			client_s.RSCShutdown(SD_BOTH);
			client_s.RSCClose();
			client_s.RSCSocketUninst_W();
			//g_client_i_s = NULL;
			
			//  Debug
			// MessageBox(NULL, "Incorrect Requested BPP exiting ...", "Debug", MB_OK);

			PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
			return;	
	}

	//region_ptr[0] = (BYTE *) malloc(param->m_prop_requested.region_h*param->m_prop_requested.Bps);
	region_ptr[0] = (BYTE *) malloc(m_threads[*param].prop.m_prop_requested.region_h*m_threads[*param].prop.m_prop_requested.Bps);
	//region_ptr[1] = (BYTE *) malloc(param->m_prop_requested.region_h*param->m_prop_requested.Bps);
	region_ptr[1] = (BYTE *) malloc(m_threads[*param].prop.m_prop_requested.region_h*m_threads[*param].prop.m_prop_requested.Bps);
	
	cmp_pixtangle = (BYTE *) malloc(PIXTANGLE_WIDTH*PIXTANGLE_HEIGHT*Bpp);
	src_pixtangle = (BYTE *) malloc(PIXTANGLE_WIDTH*PIXTANGLE_HEIGHT*Bpp);
	//cmp_pixtangle = (BYTE *) malloc(RSCGetBPS(PIXTANGLE_WIDTH, m_threads[*param].prop.m_prop_requested.bpp)*PIXTANGLE_HEIGHT);
	//src_pixtangle = (BYTE *) malloc(RSCGetBPS(PIXTANGLE_WIDTH, m_threads[*param].prop.m_prop_requested.bpp)*PIXTANGLE_HEIGHT);
	

	if(region_ptr[0] == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		
		// Debug
		// MessageBox(NULL, "region_ptr[0] exiting ...", "Debug", MB_OK);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}
	if(region_ptr[1] == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		
		// Debug
		// MessageBox(NULL, "region_ptr[1] exiting ...", "Debug", MB_OK);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;	
	}	
	if(cmp_pixtangle == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		
		// Debug
		// MessageBox(NULL, "cmp_pixtangle exiting ...", "Debug", MB_OK);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;			
	}
	if(src_pixtangle == NULL){
		// Failed in malloc
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		free(cmp_pixtangle);
		
		// Debug
		// MessageBox(NULL, "src_pixtangle exiting ...", "Debug", MB_OK);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		return;		
	}


	///////////////////////////////////////////////////////////////////////////////
	// Init DIB Header 
	///////////////////////////////////////////////////////////////////////////////

	//RSC_DIBINFO dibinfo;

	memset(&m_threads[*param].prop.m_region.m_dibinfo.header, 0, sizeof(m_threads[*param].prop.m_region.m_dibinfo.header));
	m_threads[*param].prop.m_region.m_dibinfo.header.biBitCount = m_threads[*param].prop.m_prop_requested.bpp;//prop->m_region.m_regionprop.bpp;
	m_threads[*param].prop.m_region.m_dibinfo.header.biClrImportant = 0;
	m_threads[*param].prop.m_region.m_dibinfo.header.biClrUsed = 0;

	if(m_prop.m_region.m_regionprop.bpp == 8)
		m_threads[*param].prop.m_region.m_dibinfo.header.biCompression = BI_RGB;
	else
		m_threads[*param].prop.m_region.m_dibinfo.header.biCompression = BI_BITFIELDS;

	m_threads[*param].prop.m_region.m_dibinfo.header.biHeight = m_threads[*param].prop.m_prop_requested.region_h;//prop->m_region.m_regionprop.region_h;
	m_threads[*param].prop.m_region.m_dibinfo.header.biPlanes = 1;
	m_threads[*param].prop.m_region.m_dibinfo.header.biSize = sizeof(BITMAPINFOHEADER);
	m_threads[*param].prop.m_region.m_dibinfo.header.biWidth = m_threads[*param].prop.m_prop_requested.region_w;//prop->m_region.m_regionprop.region_w;
	m_threads[*param].prop.m_region.m_dibinfo.header.biXPelsPerMeter = 0;
	m_threads[*param].prop.m_region.m_dibinfo.header.biYPelsPerMeter = 0;

	m_threads[*param].prop.m_region.m_dibinfo.header.biBitCount = m_prop.m_region.m_regionprop.bpp;
	m_threads[*param].prop.m_region.m_dibinfo.header.biCompression = BI_BITFIELDS;
	m_threads[*param].prop.m_region.m_dibinfo.header.biSizeImage = m_threads[*param].prop.m_prop_requested.region_h*m_prop.m_region.m_regionprop.Bps;


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////
	// Full Update
	///////////////////////////////////////////////////////////////////////////////
		
	////////////////////////////////////////////////////////////
	// Begin Critical Section
	////////////////////////////////////////////////////////////
	// EnterCriticalSection(&communication_cs);

	if(!wv_updatefullscreen(*param, &client_s, &m_threads[*param].prop, region_ptr[cur_region], region_ptr[pre_region])){
		// Failed in update_fullscreen
		client_s.RSCShutdown(SD_BOTH);
		client_s.RSCClose();
		client_s.RSCSocketUninst_W();
		//g_client_i_s = NULL;
		free(region_ptr[0]);
		free(region_ptr[1]);
		free(cmp_pixtangle);
		free(src_pixtangle);
		
		// Debug
		// MessageBox(NULL, "wv_updatefullscreen exiting ...", "Client Debug", MB_OK);

		PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
		
		////////////////////////////////////////////////////////////
		// Release Critical Section
		////////////////////////////////////////////////////////////
		// LeaveCriticalSection(&region_cs);

		return;	
	}
	////////////////////////////////////////////////////////////
	// Release Critical Section
	////////////////////////////////////////////////////////////
	// LeaveCriticalSection(&communication_cs);


	if(cur_region == 0){
		cur_region = 1;
		pre_region = 0;
	}else{
		cur_region = 0;
		pre_region = 1;
	}


	RSCPixtangle pix(m_threads[*param].prop.m_prop_requested.region_w, 
					m_threads[*param].prop.m_prop_requested.region_h, 
					m_threads[*param].prop.m_prop_requested.Bps,
					Bpp, //m_threads[*param].prop.m_prop_requested.bpp/8, 
					PIXTANGLE_WIDTH, PIXTANGLE_HEIGHT, false);



	// Set global variable for screen connection to ON
	m_threads[*param].socket_connected = isconnected;
	//g_isconnected_s = isconnected;

	///////////////////////////////////////////////////////////////////////////////
	// Set the Windows size														 //
	///////////////////////////////////////////////////////////////////////////////
	
	RECT screenrect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screenrect, 0);
	int screen_w = screenrect.right -  screenrect.left;
	int screen_h = screenrect.bottom - screenrect.top;
	
	RECT fwinrect;


	SetRect(&fwinrect, 0, 0, m_threads[*param].prop.m_prop_requested.region_o_w, m_threads[*param].prop.m_prop_requested.region_o_h);
	
	AdjustWindowRect(&fwinrect, GetWindowLong(m_prop.m_hwnd, GWL_STYLE), TRUE);

	m_fwinrect_w = fwinrect.right - fwinrect.left;
	m_fwinrect_h = fwinrect.bottom - fwinrect.top;

	m_screen_w  = min(m_fwinrect_w, screen_w);
	m_screen_h = min(m_fwinrect_h, screen_h);


	SetWindowPos(m_prop.m_hwnd, HWND_TOP,
		0,
		0,
		m_screen_w, m_screen_h, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	
	SetForegroundWindow(m_prop.m_hwnd);

	//


	// Time delay if required...
	DWORD delay_tick, delay_elapsed;

	while(isconnected){

		// Time tick-count
		delay_tick = GetTickCount();
		
		if(!wv_updateincrementalscreen(*param, &client_s, &m_threads[*param].prop, region_ptr[cur_region], region_ptr[pre_region], src_pixtangle, cmp_pixtangle, &pix)){

			// Debug
			//MessageBox(NULL, "Error occurred", "Debug", MB_OK);

			isconnected = false;
			if(g_isconnected_i) g_isconnected_i = false;
		}
		
		if(cur_region == 0){
			cur_region = 1;
			pre_region = 0;
		}else{
			cur_region = 0;
			pre_region = 1;
		}

		delay_elapsed = GetTickCount() - delay_tick;
		
		if(delay_elapsed < THREAD_TIME_DELAY){			
				Sleep(THREAD_TIME_DELAY-delay_elapsed);
		}

		// Reset Tick counters
		delay_elapsed = 0;
		delay_tick = 0;
	
	}

	// Return current thread handle
	client_s.RSCShutdown(SD_BOTH);
	client_s.RSCClose();
	client_s.RSCSocketUninst_W();
	m_threads[*param].socket = NULL;
	m_threads[*param].socket_connected = isconnected; //false;

	free(region_ptr[0]);
	free(region_ptr[1]);
	free(cmp_pixtangle);
	free(src_pixtangle);
	

	PostMessage(m_prop.m_hwnd, WM_DESTROY, 0, 0);
	return;	

}


bool wv_sendmouse(RSCSocket *socket, int x, int y, DWORD wParam, UINT message)
{
	RSC_BYTE identifier = 5;
	RSC_BYTE lbtn, mbtn, rbtn;
	RSC_BYTE wheel1, wheel2;
	RSC_UWORD sx, sy;
	
	// No wheel button handling routines specified ...
	wheel1 = 0;
    wheel2 = 0;
	/*
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	*/
	//if(message == WM_LBUTTONDOWN)
	if(wParam & MK_LBUTTON)
		lbtn = 1;
	else
		lbtn = 0;

	if(wParam & MK_MBUTTON)
		mbtn = 1;
	else
		mbtn = 0;

	if(wParam & MK_RBUTTON)
		rbtn = 1;
	else
		rbtn = 0;

	//sx = x;
	//sy = y;
	sx = x + m_xcurscroll;
	sy = y + m_ycurscroll;

	if(!socket->RSCSend((RSC_BYTE *) &identifier, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &lbtn, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &mbtn, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &rbtn, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &wheel1, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &wheel2, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &sx, 2))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &sy, 2))
		return false;

	return true;

}

bool wv_sendkey(RSCSocket *socket, int wParam, DWORD lParam)
{
	bool bdown = ((lParam & 0x80000000l) == 0);
	RSC_BYTE identifier = 4;
	RSC_BYTE down;
	RSC_UWORD vkey;

	if(bdown){
		down = 1;
		//wsprintf(buf, "VKey: %d, Pressed - CTR: %d", wParam, testctr);
	}else{
		down = 0;
		//wsprintf(buf, "VKey: %d, Not Pressed - CTR: %d", wParam, testctr);
	}

	vkey = wParam;

	if(!socket->RSCSend((RSC_BYTE *) &identifier, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &down, 1))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &vkey, 2))
		return false;

	return true;
}

bool wv_updateincrementalscreen(DWORD index, RSCSocket *client_s, RSC_CLIENT_PROP *prop, BYTE *region_ptr, BYTE *region_pre, BYTE *pixtangle_ptr, BYTE *pixtangle_cmp, RSCPixtangle *pix)
{
	RSC_UDWORD enc_length = 0;
	RSCRLE enc;

	RSC_UWORD total_pixtangle;

	RSC_UBYTE *conv_ptr;
	bool converted = false;
	
	RSC_BYTE identifier = 3;
	RSC_BYTE pixtangle_mode = 1; // 1 - Incremental Screen Update
	
	RSC_UDWORD pixtangle_x = 0;
	RSC_UDWORD pixtangle_w = prop->m_prop_requested.region_w;

	RSC_UDWORD pixtangle_y = 0;
	RSC_UDWORD pixtangle_h = prop->m_prop_requested.region_h;

	RSC_BYTE pixtangle_type;

	RSC_UDWORD pixtangle_sx;
	RSC_UDWORD pixtangle_sy;

	////////////////////////////////////////////////////////////
	// Begin Mutex section
	////////////////////////////////////////////////////////////
	//WaitForSingleObject(communication_mutex, INFINITE);

	if(!client_s->RSCSend((RSC_BYTE *) &identifier, 1)){
		return false;
	}
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_mode, 1)){
		return false;
	}
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_x, 4)){
		return false;
	}
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_w, 4)){
		return false;
	}
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_y, 4)){
		return false;
	}
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_h, 4)){
		return false;
	}

	// Copy pre-screen buffer to cur-screen buffer
	//memcpy(region_ptr, region_pre, prop->m_region.m_regionprop.region_h*prop->m_region.m_regionprop.Bps);
	memcpy(region_ptr, region_pre, prop->m_prop_requested.region_h*prop->m_prop_requested.Bps);
	
	// Receive the total number of pixtangle waiting from RSCServer
	if(!client_s->RSCReceive((RSC_BYTE *) &total_pixtangle, 2)){
		// Debug
		//MessageBox(NULL, "7", "Debug", MB_OK);	
		return false;
	}

	////////////////////////////////////////////////////////////
	// Release Mutex Section
	////////////////////////////////////////////////////////////
	//ReleaseMutex(communication_mutex);

	// Debug 
	//unsigned long tmp_enc_length=0;
	
	for(int i=0; i < total_pixtangle; i++){
		
		////////////////////////////////////////////////////////////
		// Begin Mutex section
		////////////////////////////////////////////////////////////
		//WaitForSingleObject(communication_mutex, INFINITE);

		if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_x, 4)){
			return false;
		}
		if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_y, 4)){
			return false;
		}
		if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_w, 4)){
			return false;
		}
		if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_h, 4)){
			return false;
		}
		
		if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_type, 1)){
			return false;
		}
		
		
		switch(pixtangle_type){
			case 0:
				// Raw Encoding
				// Debug
				//MessageBox(NULL, "Raw", "Debug", MB_OK);
				break;
			case 1:
				// RLE
				if(!client_s->RSCReceive((RSC_BYTE *) &enc_length, 4)){
					return false;
				}

				if(enc_length > 0){

					// Init buffers
					memset(pixtangle_cmp, 0, pixtangle_h*pixtangle_w*pix->m_Bpp);
					memset(pixtangle_ptr, 0, pixtangle_h*pixtangle_w*pix->m_Bpp);

					if(!client_s->RSCReceive((RSC_BYTE *) pixtangle_cmp, enc_length)){
						return false;
					}

					//switch(prop->m_region.m_regionprop.bpp){
					switch(prop->m_prop_requested.bpp){
						case 8:
							if(!enc.RSCDecompress8(pixtangle_cmp, pixtangle_ptr, enc_length)){
								return false;
							}
							break;
						case 15:
						case 16:
							if(!enc.RSCDecompress16(pixtangle_cmp, pixtangle_ptr, enc_length)){
								return false;
							}
							break;
						case 24:
						case 32:
							if(!enc.RSCDecompress32(pixtangle_cmp, pixtangle_ptr, enc_length)){
								return false;
							}
							break;
						default:
							// Unknown type
							return false;
					}

				}else{
					if(!client_s->RSCReceive((RSC_BYTE *) pixtangle_ptr, pixtangle_h*pixtangle_w*pix->m_Bpp)){
						// Debug
						//MessageBox(NULL, "18", "Debug", MB_OK);					
						return false;
					}
				}

				////////////////////////////////////////////////////////////
				// Release Mutex Section
				////////////////////////////////////////////////////////////
				//ReleaseMutex(communication_mutex);

				pix->RSCCopyBuffer2Pixtangle(pixtangle_ptr, region_ptr, pixtangle_x, pixtangle_y, pixtangle_w, pixtangle_h);
				break;
			case 2:
				// CopyRect Encoding - comparing with previous frame screen
				if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_sx, 4))
					return false;
				if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_sy, 4))
					return false;

				//
				pix->RSCCopyPixtangle2Pixtangle(region_pre ,region_ptr, pixtangle_sx, pixtangle_sy, pixtangle_w, pixtangle_h, pixtangle_x, pixtangle_y);

				break;
			case 3:
				// CopyRect Encoding - same frame screen
				if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_sx, 4))
					return false;
				if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_sy, 4))
					return false;

				pix->RSCCopyPixtangle2Pixtangle(region_ptr ,region_ptr, pixtangle_sx, pixtangle_sy, pixtangle_w, pixtangle_h, pixtangle_x, pixtangle_y);

				break;

		}
		

	}
	
	//
	// CONVERT TO DDB Starts here
	//
	
	////////////////////////////////////////////////////////////
	// Convert DIB BPP  
	////////////////////////////////////////////////////////////
		
	switch(prop->m_prop_requested.bpp)
		{
			case 8:
				// this code "prop->m_region.m_regionprop.bpp == 8" was added 
				// to fill in dibinfo.colors field before DIB2DDB conversion
				if(m_prop.m_region.m_regionprop.bpp == 8){

					for(int id=0; id < 256; id++){
						prop->m_region.m_dibinfo.colors[id].rgbRed = m_prop.m_pal_requested.m_col_table[id*4];
						prop->m_region.m_dibinfo.colors[id].rgbGreen = m_prop.m_pal_requested.m_col_table[id*4+1];
						prop->m_region.m_dibinfo.colors[id].rgbBlue  = m_prop.m_pal_requested.m_col_table[id*4+2];
					}

				}else if(m_prop.m_region.m_regionprop.bpp == 15){
		
					conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_8to15(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "19", "Debug", MB_OK);					
						return false;
					}

				}else if(m_prop.m_region.m_regionprop.bpp == 16){
					
					conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_8to16(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "20", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 24){

					conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_8to24(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "21", "Debug", MB_OK);					
						return false;
					}

				}else if(m_prop.m_region.m_regionprop.bpp == 32){
				
					conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_8to32(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "22", "Debug", MB_OK);					
						return false;
					}
				}
				break;
			case 15:
				if(m_prop.m_region.m_regionprop.bpp == 8){
					//
					// [Note] Not action needed. This option will be impossible
					// If a current DC is 8 bit, the INIT function will request 
					// 8 bit color from the server. 
					//
				}else if(m_prop.m_region.m_regionprop.bpp == 16){

					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_15to16(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "23", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 24){

					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_15to24(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "24", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 32){
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_15to32(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "25", "Debug", MB_OK);					
						return false;
					}
				}
				break;
			case 16:
				if(m_prop.m_region.m_regionprop.bpp == 8){
					//
					// [Note] Not action needed. This option will be impossible
					// If a current DC is 8 bit, the INIT function will request 
					// 8 bit color from the server. 
					//
				}else if(m_prop.m_region.m_regionprop.bpp == 15){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_16to15(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "26", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 24){
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_16to24(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "27", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 32){
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_16to32(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "28", "Debug", MB_OK);					
						return false;
					}
				}
				break;
			case 24:
				if(m_prop.m_region.m_regionprop.bpp == 8){
					//
					// [Note] Not action needed. This option will be impossible
					// If a current DC is 8 bit, the INIT function will request 
					// 8 bit color from the server. 
					//
				}else if(m_prop.m_region.m_regionprop.bpp == 15){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_24to15(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "29", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 16){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_24to16(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "30", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 32){
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_24to32(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "31", "Debug", MB_OK);					
						return false;
					}
				}
				break;
			case 32:
				if(m_prop.m_region.m_regionprop.bpp == 8){
					//
					// [Note] Not action needed. This option will be impossible
					// If a current DC is 8 bit, the INIT function will request 
					// 8 bit color from the server. 
					//
				}else if(m_prop.m_region.m_regionprop.bpp == 15){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_32to15(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "32", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 16){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_32to16(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "33", "Debug", MB_OK);					
						return false;
					}
				}else if(m_prop.m_region.m_regionprop.bpp == 24){
					//
					// [Note] This option will never be called.
					//
					
					conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
					converted = true;

					if(!m_prop.m_region.RSCConvertDIB_32to24(region_ptr,conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
						delete [] conv_ptr;
						// Debug
						//MessageBox(NULL, "34", "Debug", MB_OK);					
						return false;
					}
				}
				break;
			default:
				// Error: not supported display mode
				// Debug
				//MessageBox(NULL, "35", "Debug", MB_OK);					
				return false;
				break;
		}

	if(!converted){
		conv_ptr = region_ptr;
	}	
		
	////////////////////////////////////////////////////////////
	// Begin Critical section
	////////////////////////////////////////////////////////////
	EnterCriticalSection(&region_cs);

	//memcpy(&m_prop.m_prop_requested.region_x, &prop->m_prop_requested.region_x, sizeof(prop->m_prop_requested.region_x));
	//memcpy(&m_prop.m_prop_requested.region_y, &prop->m_prop_requested.region_y, sizeof(prop->m_prop_requested.region_y));
	//memcpy(&m_prop.m_prop_requested.region_h, &prop->m_prop_requested.region_h, sizeof(prop->m_prop_requested.region_h));
	//memcpy(&m_prop.m_prop_requested.region_w, &prop->m_prop_requested.region_w, sizeof(prop->m_prop_requested.region_w));


	if(prop->m_ddb != NULL){
		DeleteObject(prop->m_ddb);
		prop->m_ddb = NULL;
	}

	prop->m_ddb = m_prop.m_region.RSCDIB2DDB(conv_ptr, &prop->m_region.m_dibinfo, &m_prop.m_pal_requested, DIB_RGB_COLORS);
		

	//
	LeaveCriticalSection(&region_cs);

	if(prop->m_ddb == NULL){
		if(converted) delete[] conv_ptr;
		// Debug
		//MessageBox(NULL, "36", "Debug", MB_OK);					
		return false;
	}
	// Send an update message to the Windows
	PostMessage(m_prop.m_hwnd, WM_SCREEN_UPDATE, 0, index);
	//UpdateWindow(m_prop.m_hwnd);
	
	// Debug
	//RedrawWindow(m_prop.m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

	if(converted) delete[] conv_ptr;

	return true;
}

bool wv_updatefullscreen(DWORD index, RSCSocket *client_s, RSC_CLIENT_PROP *prop, BYTE *region_ptr, BYTE *region_cmp)
{

	RSC_UDWORD enc_length = 0;
	RSCRLE enc;

	RSC_BYTE identifier = 3;
	RSC_BYTE pixtangle_mode = 0; // 0 - Full Screen Update
	RSC_BYTE palette_req = 0;
	RSC_WORD pixtangle_ctr;
	
	//RSC_UDWORD pixtangle_x = 0;
	RSC_UDWORD pixtangle_x = prop->m_prop_requested.region_x;
	RSC_UDWORD pixtangle_w = prop->m_prop_requested.region_w;

	//RSC_UDWORD pixtangle_y = 0;
	RSC_UDWORD pixtangle_y = prop->m_prop_requested.region_y;
	RSC_UDWORD pixtangle_h = prop->m_prop_requested.region_h;

	RSC_UBYTE *conv_ptr;
	bool converted = false;

	////////////////////////////////////////////////////////////
	// Begin Mutex section
	////////////////////////////////////////////////////////////
	//WaitForSingleObject(communication_mutex, INFINITE);

	if(!client_s->RSCSend((RSC_BYTE *) &identifier, 1))
		return false;
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_mode, 1))
		return false;
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_x, 4))
		return false;
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_w, 4))
		return false;
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_y, 4))
		return false;
	if(!client_s->RSCSend((RSC_BYTE *) &pixtangle_h, 4))
		return false;

	// 
	// Pixtangle Part
	//
	if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_ctr, 2))
		return false;
	if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_x, 4))
		return false;
	if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_y, 4))
		return false;
	if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_w, 4))
		return false;
	if(!client_s->RSCReceive((RSC_BYTE *) &pixtangle_h, 4))
		return false;

	// Data Streaming Part
	if(!client_s->RSCReceive((RSC_BYTE *) &enc_length, 4))
		return false;


	if(enc_length > 0){
		if(!client_s->RSCReceive((RSC_BYTE *) region_cmp, enc_length))
			return false;

		switch(prop->m_prop_requested.bpp){
			case 8:
				if(!enc.RSCDecompress8(region_cmp, region_ptr, enc_length))
					return false;
				break;
			case 15:
			case 16:
				if(!enc.RSCDecompress16(region_cmp, region_ptr, enc_length))
					return false;
				break;
			case 24:
			case 32:
				if(!enc.RSCDecompress32(region_cmp, region_ptr, enc_length))
					return false;
				break;
			default:
				// Unknown type
				return false;
		}

	}else{

		if(!client_s->RSCReceive((RSC_BYTE *) region_ptr, pixtangle_h*prop->m_prop_requested.Bps))
			return false;
	
	}

	///////////////////////////////////////////////////////////////////////////////
	// Init DIB Header 
	///////////////////////////////////////////////////////////////////////////////
	/*
	RSC_DIBINFO dibinfo;

	memset(&dibinfo.header, 0, sizeof(dibinfo.header));
	dibinfo.header.biBitCount = prop->m_prop_requested.bpp;//prop->m_region.m_regionprop.bpp;
	dibinfo.header.biClrImportant = 0;
	dibinfo.header.biClrUsed = 0;

	if(m_prop.m_region.m_regionprop.bpp == 8)
		dibinfo.header.biCompression = BI_RGB;
	else
		dibinfo.header.biCompression = BI_BITFIELDS;

	dibinfo.header.biHeight = prop->m_prop_requested.region_h;//prop->m_region.m_regionprop.region_h;
	dibinfo.header.biPlanes = 1;
	dibinfo.header.biSize = sizeof(BITMAPINFOHEADER);
	dibinfo.header.biWidth = prop->m_prop_requested.region_w;//prop->m_region.m_regionprop.region_w;
	dibinfo.header.biXPelsPerMeter = 0;
	dibinfo.header.biYPelsPerMeter = 0;

	dibinfo.header.biBitCount = m_prop.m_region.m_regionprop.bpp;
	dibinfo.header.biCompression = BI_BITFIELDS;
	dibinfo.header.biSizeImage = prop->m_prop_requested.region_h*m_prop.m_region.m_regionprop.Bps;

	*/
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// Convert DIB BPP  
	////////////////////////////////////////////////////////////
	
	switch(prop->m_prop_requested.bpp)
	{
		case 8:
			// this code "prop->m_region.m_regionprop.bpp == 8" was added 
			// to fill in dibinfo.colors field before DIB2DDB conversion
			if(m_prop.m_region.m_regionprop.bpp == 8){

				for(int id=0; id < 256; id++){
					prop->m_region.m_dibinfo.colors[id].rgbRed = m_prop.m_pal_requested.m_col_table[id*4];
					prop->m_region.m_dibinfo.colors[id].rgbGreen = m_prop.m_pal_requested.m_col_table[id*4+1];
					prop->m_region.m_dibinfo.colors[id].rgbBlue  = m_prop.m_pal_requested.m_col_table[id*4+2];
				}

			}else if(m_prop.m_region.m_regionprop.bpp == 15){
						
				conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_8to15(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
					delete [] conv_ptr;
					return false;
				}

			}else if(m_prop.m_region.m_regionprop.bpp == 16){
								
				conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_8to16(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
					delete [] conv_ptr;
					return false;
				}

			}else if(m_prop.m_region.m_regionprop.bpp == 24){
				
				conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_8to24(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
					delete [] conv_ptr;
					return false;
				}

			}else if(m_prop.m_region.m_regionprop.bpp == 32){
				
				conv_ptr = new BYTE[RSCGetBPS(prop->m_region.m_dibinfo.header.biWidth, m_prop.m_region.m_regionprop.bpp)*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_8to32(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight, &m_prop.m_pal_requested)){
					delete [] conv_ptr;
					return false;
				}
			}
			break;
		case 15: 
			if(m_prop.m_region.m_regionprop.bpp == 8){
				//
				// [Note] Not action needed. This option will be impossible
				// If a current DC is 8 bit, the INIT function will request 
				// 8 bit color from the server. 
				//
			}else if(m_prop.m_region.m_regionprop.bpp == 16){
			
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_15to16(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 24){

				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_15to24(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 32){
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_15to32(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}
			break;
		case 16:
			if(m_prop.m_region.m_regionprop.bpp == 8){
				//
				// [Note] Not action needed. This option will not be called.
				// If a current DC is 8 bit, the INIT function will request 
				// 8 bit color from the server. 
				//
			}else if(m_prop.m_region.m_regionprop.bpp == 15){
				//
				// [Note] This option will never be called.
				//
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_16to15(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 24){
			
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;

				if(!m_prop.m_region.RSCConvertDIB_16to24(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 32){
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_16to32(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}
			break;
		case 24:
			if(m_prop.m_region.m_regionprop.bpp == 8){
				//
				// [Note] Not action needed. This option will never be called.
				// If a current DC is 8 bit, the INIT function will request 
				// 8 bit color from the server. 
				//
			}else if(m_prop.m_region.m_regionprop.bpp == 15){
				//
				// [Note] This option will never be called.
				//

				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_24to15(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 16){
				//
				// [Note] This option will never be called.
				//
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_24to16(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 32){
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_24to32(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}
			break;
		case 32:
			if(m_prop.m_region.m_regionprop.bpp == 8){
				//
				// [Note] Not action needed. This option will never be called.
				// If a current DC is 8 bit, the INIT function will request 
				// 8 bit color from the server. 
				//
			}else if(m_prop.m_region.m_regionprop.bpp == 15){
				//
				// [Note] This option will never be called.
				//
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_32to15(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 16){
				//
				// [Note] This option will never be called.
				//
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_32to16(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}else if(m_prop.m_region.m_regionprop.bpp == 24){
				//
				// [Note] This option will never be called.
				//
				
				conv_ptr = new BYTE[m_prop.m_region.m_regionprop.Bps*prop->m_region.m_dibinfo.header.biHeight];
				converted = true;
				
				if(!m_prop.m_region.RSCConvertDIB_32to24(region_ptr, conv_ptr, prop->m_region.m_dibinfo.header.biWidth, prop->m_region.m_dibinfo.header.biHeight)){
					delete [] conv_ptr;
					return false;
				}
			}
			break;
		default:
			// Error: not supported display mode
			return false;
			break;
	}

	if(!converted){
		conv_ptr = region_ptr;
	}
	
	////////////////////////////////////////////////////////////
	// Begin Critical section
	////////////////////////////////////////////////////////////
	EnterCriticalSection(&region_cs);

	//memcpy(&m_prop.m_prop_requested.region_x, &prop->m_prop_requested.region_x, sizeof(prop->m_prop_requested.region_x));
	//memcpy(&m_prop.m_prop_requested.region_y, &prop->m_prop_requested.region_y, sizeof(prop->m_prop_requested.region_y));
	//memcpy(&m_prop.m_prop_requested.region_h, &prop->m_prop_requested.region_h, sizeof(prop->m_prop_requested.region_h));
	//memcpy(&m_prop.m_prop_requested.region_w, &prop->m_prop_requested.region_w, sizeof(prop->m_prop_requested.region_w));

	if(prop->m_ddb != NULL){
		DeleteObject(prop->m_ddb);
		prop->m_ddb = NULL;
	}

	prop->m_ddb = m_prop.m_region.RSCDIB2DDB(conv_ptr, &prop->m_region.m_dibinfo, &m_prop.m_pal_requested, DIB_RGB_COLORS);

	////////////////////////////////////////////////////////////
	// Release Mutex Section
	////////////////////////////////////////////////////////////
	//ReleaseMutex(region_mutex);
	LeaveCriticalSection(&region_cs);

	if(prop->m_ddb == NULL){
		// Debug
		// MessageBox(NULL, "BMP error", "Debug", MB_OK);

		if(converted) delete[] conv_ptr;
		
		return false;
	}

	// Send an update message to the Windows
	//
	// At the end of WM_SCREEN_UPDATE EVENT, LeaveCriticalSection(&region_cs) will be called.
	//
	PostMessage(m_prop.m_hwnd, WM_SCREEN_UPDATE, 0, index);
	//UpdateWindow(m_prop.m_hwnd);

	// Debug
	//RedrawWindow(m_prop.m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

	
	if(converted) delete[] conv_ptr;

	return true;

}

bool wv_updatescreen(DWORD index, PRECT rect)
{
	HGDIOBJ holdobj;
	HPALETTE holdpal;

	//
	// EnterCriticalSection(&region_cs) was called prior to this event function call ...
	//
	EnterCriticalSection(&region_cs);
	//

	if(m_threads[index].prop.m_region.m_hdcmem != NULL){
		DeleteObject(m_threads[index].prop.m_region.m_hdcmem);
		m_threads[index].prop.m_region.m_hdcmem = NULL;
	}
	m_threads[index].prop.m_region.m_hdcmem = CreateCompatibleDC(m_prop.m_region.m_hdc);
	
	if (m_threads[index].prop.m_region.m_hdcmem == NULL){
		////////////////////////////////////////////////////////////
		// Release Critical Section
		////////////////////////////////////////////////////////////

		LeaveCriticalSection(&region_cs);
		return false;
	}

	if(m_prop.m_region.m_regionprop.bpp == 8){
		
		holdpal = SelectPalette(m_prop.m_region.m_hdc, m_prop.m_pal_requested.m_palette, FALSE);
		RealizePalette(m_prop.m_region.m_hdc);
	}
	
	holdobj = SelectObject(m_threads[index].prop.m_region.m_hdcmem, m_threads[index].prop.m_ddb);

	//
	//
	//
	int dx, dy, dw, dh, sx, sy;
	bool isdraw;

	// Debug
	//RECT wrect;
	//GetWindowRect(m_prop.m_hwnd, &wrect);
	//rect = &wrect;

	isdraw = false;

	if((m_threads[index].prop.m_prop_requested.region_x >= m_xcurscroll) && (m_threads[index].prop.m_prop_requested.region_x < m_xcurscroll+(rect->right-rect->left))){
		// yes - draw
		dx = m_threads[index].prop.m_prop_requested.region_x-m_xcurscroll;

		dw = m_threads[index].prop.m_prop_requested.region_w;
		if(dw > ((rect->right-rect->left)-dx))
			dw =(rect->right-rect->left)-dx;

		sx = 0;

		//
		if((m_threads[index].prop.m_prop_requested.region_y >= m_ycurscroll) && (m_threads[index].prop.m_prop_requested.region_y < m_ycurscroll+(rect->bottom-rect->top))){
			// yes - draw
			dy = m_threads[index].prop.m_prop_requested.region_y-m_ycurscroll;

			dh = m_threads[index].prop.m_prop_requested.region_h;
			if(dh > ((rect->bottom-rect->top)-dy))
				dh =(rect->bottom-rect->top)-dy;

			sy = 0;	
			isdraw = true;
			
		}else if((m_threads[index].prop.m_prop_requested.region_y < m_ycurscroll) && ((m_threads[index].prop.m_prop_requested.region_y+m_threads[index].prop.m_prop_requested.region_h) > m_ycurscroll)){
			// yes - draw
			dy = 0;

			dh = (m_threads[index].prop.m_prop_requested.region_y + m_threads[index].prop.m_prop_requested.region_h) - m_ycurscroll;
			if(dh > (rect->bottom-rect->top))
				dh = rect->bottom-rect->top;
			
			sy = m_ycurscroll - m_threads[index].prop.m_prop_requested.region_y;
			isdraw = true;

		}
		//
		
		
	}else if((m_threads[index].prop.m_prop_requested.region_x < m_xcurscroll) && ((m_threads[index].prop.m_prop_requested.region_x+m_threads[index].prop.m_prop_requested.region_w) > m_xcurscroll)){
		// yes - draw
		dx = 0;

		dw = (m_threads[index].prop.m_prop_requested.region_x + m_threads[index].prop.m_prop_requested.region_w) - m_xcurscroll;
		if(dw > (rect->right-rect->left))
			dw = rect->right-rect->left;
		
		sx = m_xcurscroll - m_threads[index].prop.m_prop_requested.region_x;

		//
		if((m_threads[index].prop.m_prop_requested.region_y >= m_ycurscroll) && (m_threads[index].prop.m_prop_requested.region_y < m_ycurscroll+(rect->bottom-rect->top))){
			// yes - draw
			dy = m_threads[index].prop.m_prop_requested.region_y-m_ycurscroll;

			dh = m_threads[index].prop.m_prop_requested.region_h;
			if(dh > ((rect->bottom-rect->top)-dy))
				dh =(rect->bottom-rect->top)-dy;

			sy = 0;	
			isdraw = true;
			
		}else if((m_threads[index].prop.m_prop_requested.region_y < m_ycurscroll) && ((m_threads[index].prop.m_prop_requested.region_y+m_threads[index].prop.m_prop_requested.region_h) > m_ycurscroll)){
			// yes - draw
			dy = 0;

			dh = (m_threads[index].prop.m_prop_requested.region_y + m_threads[index].prop.m_prop_requested.region_h) - m_ycurscroll;
			if(dh > (rect->bottom-rect->top))
				dh = rect->bottom-rect->top;
			
			sy = m_ycurscroll - m_threads[index].prop.m_prop_requested.region_y;
			isdraw = true;

		}
		//

	}

	//if(BitBlt(m_prop.m_region.m_hdc,
	//	m_threads[index].prop.m_prop_requested.region_x, //m_prop.m_region.m_regionprop.region_x,
	//	m_threads[index].prop.m_prop_requested.region_y, //m_prop.m_region.m_regionprop.region_y, 
	//	m_threads[index].prop.m_prop_requested.region_w-m_xcurscroll, //m_prop.m_region.m_regionprop.region_w, 
	//	m_threads[index].prop.m_prop_requested.region_h-m_ycurscroll, //m_prop.m_region.m_regionprop.region_h,
	//	m_threads[index].prop.m_region.m_hdcmem,
	//	m_xcurscroll, 
	//	m_ycurscroll, 
	//	SRCCOPY) == 0){

	if(isdraw){
		if(BitBlt(m_prop.m_region.m_hdc,
			dx,
			dy, 
			dw, 
			dh,
			m_threads[index].prop.m_region.m_hdcmem,
			sx, 
			sy, 
			SRCCOPY) == 0){

				// Error occurred while BitBlt operation
				SelectObject(m_threads[index].prop.m_region.m_hdcmem, holdobj);

				//if(m_prop.m_prop_requested.bpp == 8){
				if(m_prop.m_region.m_regionprop.bpp == 8){
					SelectPalette(m_prop.m_region.m_hdc, holdpal, TRUE);
				}
				
				if(m_threads[index].prop.m_region.m_hdcmem != NULL){
					DeleteObject(m_threads[index].prop.m_region.m_hdcmem);
					m_threads[index].prop.m_region.m_hdcmem = NULL;
				}
				////////////////////////////////////////////////////////////
				// Release Critical Section
				////////////////////////////////////////////////////////////

				LeaveCriticalSection(&region_cs);

				return false;
		}

	}
		
		SelectObject(m_threads[index].prop.m_region.m_hdcmem, holdobj);
	
	//if(m_prop.m_prop_requested.bpp == 8){
	if(m_prop.m_region.m_regionprop.bpp == 8){
		SelectPalette(m_prop.m_region.m_hdc, holdpal, TRUE);
	}
	//}
	
	if(m_threads[index].prop.m_region.m_hdcmem != NULL){
		DeleteObject(m_threads[index].prop.m_region.m_hdcmem);
		m_threads[index].prop.m_region.m_hdcmem= NULL;
	}

	////////////////////////////////////////////////////////////
	// Release Critical Section
	////////////////////////////////////////////////////////////

	LeaveCriticalSection(&region_cs);

	return true;
}

/*
bool wv_init_input(RSCSocket *socket, RSCRegion *region, RSC_REGIONPROP *prop_requested)
{	
	RSC_BYTE access_mode = 0;
	RSC_BYTE red_b = 0, green_b = 0, blue_b = 0;
	
	//RSC_UDWORD display_size = 256;	// Size of string (multi-cast or display name)
	//RSC_BYTE display_str[256];		// Array string that indicates the multi-cast session id or display name.
	
	
	// for non-shared mode
	access_mode = 1;

	// Receive data from Server
	if(!socket->RSCSend(&access_mode, 1))
		return false;	
	
	return true;
}
*/

bool wv_init_input(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_CLIENT_PROP *param)
{	
	RSC_BYTE access_mode = 1;
	RSC_BYTE red_b = 0, green_b = 0, blue_b = 0;
	
	RSC_UDWORD sw;	// Screen width
	RSC_UDWORD sh;	// Screen height
	RSC_UDWORD cx; // Screen x
	RSC_UDWORD cy; // screen y
	RSC_BYTE bpp, sbpp;	// bps (bits per pixel) = 8 bits (color/grayscale), 16 bits, 24 bits and 32 bits.
	
	RSC_BYTE region_palette_req;

	RSC_UDWORD display_size = 256;	// Size of string (multi-cast or display name)
	RSC_BYTE display_str[256];		// Array string that indicates the multi-cast session id or display name.
	
	
	// Receive data from Server
	if(!socket->RSCSend(&access_mode, 1))
		return false;	
	if(!socket->RSCReceive((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCReceive(&sbpp, 1))
		return false;
	if(!socket->RSCReceive(&red_b, 1))
		return false;
	if(!socket->RSCReceive(&green_b, 1))
		return false;
	if(!socket->RSCReceive(&blue_b, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &display_size, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) display_str, 256))
		return false;

	// Determine the color depth based on the following factors:
	//
	// a) Server Color Depth: the color depth the client requests cannot be higher than the server's current color depth.
	// b) Network Speed Consideration: depending TTL (time to live) between client and server, the initial color depth setting
	//    will be determined. However, the color depth determined by TTL cannot be higher than the server's current color depth
	//

	// Set bpp request. Requested bpp (if client's bpp is higher than server's bpp) must be lower or equal than client's bpp

	switch(sbpp){
		case 32:
			if(region->m_regionprop.bpp == 24){ 
				bpp = 24;
				red_b = 8;
				green_b = 8;
				blue_b = 8;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{			
			//	return false;
			//}
			break;
		case 24:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 24;
				red_b = 8;
				green_b = 8;
				blue_b = 8;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 16:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 15:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 8:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		default:
			return false;

	}


	// Based on the color depth request, request 8-bit, 16-bit or 32-bit screen data
	//
	
	switch(param->m_speed){
		case 0: // TO DO LATER
			break;
		case 1: // 8 BITS 
			bpp = 8;
			red_b = 0;
			green_b = 0;
			blue_b = 0;
			break;
		case 2:  // 15/16 BITS
			if(sbpp == 32){

				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 24){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 16){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 15){

				if(region->m_regionprop.bpp == 32){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			break;
		case 3:  // 32 BITS
			if(sbpp == 32){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 32;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 24){

				if(region->m_regionprop.bpp == 32){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 16){

				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 15){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}
				
			}else if(sbpp == 8){

				if(region->m_regionprop.bpp == 32){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}
				
			}
			break;
		default:
			// No Effect
			break;
	}
	
	// Calculate the network speed ..
	// [TO DO LATER]
	
	param->m_prop_requested.region_o_h = sh;
	param->m_prop_requested.region_o_w = sw;

	// Starting of new x and y
	cx = 0; //50;
	cy = 0; //50;
	param->m_prop_requested.region_x = cx;
	param->m_prop_requested.region_y = cy;
	
	// 0 - default, 1 - width & height defined, 2 - custom: needs to calculate height based on ratio
	switch(param->m_desired_resol){
		case 0:
			break;
		case 1:
			sw = param->m_desired_resol_width;
			sh = param->m_desired_resol_height;
			break;
		case 2:
			
			//out_h = (long) (desired_w/(double) ((double) current_w / (double) current_h));
			param->m_desired_resol_height = (int) (param->m_desired_resol_width/(double) ((double) sw / (double) sh));
			sw = param->m_desired_resol_width;
			sh = param->m_desired_resol_height;
			break;
		default:
			break;
	}

	
	// Debug
	//sh = 400;
	//sw = 400;

	param->m_prop_requested.bpp = bpp;
	param->m_prop_requested.region_h = sh;
	param->m_prop_requested.region_w = sw;
	param->m_prop_requested.Bps = RSCGetBPS(sw, bpp);
	param->m_prop_requested.pixel_type = RSCGetPixelTypeFromBitsPerPixel(bpp, red_b, green_b, blue_b);
	
	
	// Reply back to Server with PROPERTIES the client want
	if(!socket->RSCSend((RSC_BYTE *) &cx, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &cy, 4))
		return false;
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

	if(bpp == 8){
		// Debug
		//MessageBox(NULL, "wv_init 8bit IN", "Debug", MB_OK);

		region_palette_req = 0;
		if(!socket->RSCSend(&region_palette_req, 1))
			return false;

		if(!socket->RSCReceive((RSC_BYTE *) &pal->m_col_table, 1024))
			return false;
		
		if(!pal->CreatePaletteFromColorTable())
			return false;

	}
	return true;
}

bool wv_init_screen(RSCSocket *socket, RSCRegion *region, RSCPalette *pal, RSC_CLIENT_PROP *param)
{	
	RSC_BYTE access_mode = 0;
	RSC_BYTE red_b = 0, green_b = 0, blue_b = 0;
	
	RSC_UDWORD sw; // Screen width
	RSC_UDWORD sh; // Screen height
	RSC_UDWORD cx; // Screen x
	RSC_UDWORD cy; // screen y
	RSC_BYTE bpp, sbpp;	// bps (bits per pixel) = 8 bits (color/grayscale), 16 bits, 24 bits and 32 bits.
	
	RSC_BYTE region_palette_req;

	RSC_UDWORD display_size = 256;	// Size of string (multi-cast or display name)
	RSC_BYTE display_str[256];		// Array string that indicates the multi-cast session id or display name.
	
	
	// Receive data from Server
	if(!socket->RSCSend(&access_mode, 1))
		return false;	
	if(!socket->RSCReceive((RSC_BYTE *) &sw, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &sh, 4))
		return false;
	if(!socket->RSCReceive(&sbpp, 1))
		return false;
	if(!socket->RSCReceive(&red_b, 1))
		return false;
	if(!socket->RSCReceive(&green_b, 1))
		return false;
	if(!socket->RSCReceive(&blue_b, 1))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) &display_size, 4))
		return false;
	if(!socket->RSCReceive((RSC_BYTE *) display_str, 256))
		return false;

	// Determine the color depth based on the following factors:
	//
	// a) Server Color Depth: the color depth the client requests cannot be higher than the server's current color depth.
	// b) Network Speed Consideration: depending TTL (time to live) between client and server, the initial color depth setting
	//    will be determined. However, the color depth determined by TTL cannot be higher than the server's current color depth
	//

	// Set bpp request. Requested bpp (if client's bpp is higher than server's bpp) must be lower or equal than client's bpp

	switch(sbpp){
		case 32:
			if(region->m_regionprop.bpp == 24){ 
				bpp = 24;
				red_b = 8;
				green_b = 8;
				blue_b = 8;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{			
			//	return false;
			//}
			break;
		case 24:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 24;
				red_b = 8;
				green_b = 8;
				blue_b = 8;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 16:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 16;
				red_b = 5;
				green_b = 6;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 15:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 15;
				red_b = 5;
				green_b = 5;
				blue_b = 5;
			}else if(region->m_regionprop.bpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		case 8:
			if(region->m_regionprop.bpp == 32){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 24){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 16){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}else if(region->m_regionprop.bpp == 15){ 
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			//else{
			//	return false;
			//}
			break;
		default:
			return false;

	}


	// Based on the color depth request, request 8-bit, 16-bit or 32-bit screen data
	//
	
	switch(param->m_speed){
		case 0: // TO DO LATER
			break;
		case 1: // 8 BITS 
			bpp = 8;
			red_b = 0;
			green_b = 0;
			blue_b = 0;
			break;
		case 2:  // 15/16 BITS
			if(sbpp == 32){

				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 24){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 16){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 15){

				if(region->m_regionprop.bpp == 32){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 8){
				bpp = 8;
				red_b = 0;
				green_b = 0;
				blue_b = 0;
			}
			break;
		case 3:  // 32 BITS
			if(sbpp == 32){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 32;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 24){

				if(region->m_regionprop.bpp == 32){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 24;
					red_b = 8;
					green_b = 8;
					blue_b = 8;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 16){

				if(region->m_regionprop.bpp == 32){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 16;
					red_b = 5;
					green_b = 6;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}

			}else if(sbpp == 15){
				
				if(region->m_regionprop.bpp == 32){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 15;
					red_b = 5;
					green_b = 5;
					blue_b = 5;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}
				
			}else if(sbpp == 8){

				if(region->m_regionprop.bpp == 32){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 24){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 16){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else if(region->m_regionprop.bpp == 15){
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}else{ // region->m_regionprop.bpp == 8
					bpp = 8;
					red_b = 0;
					green_b = 0;
					blue_b = 0;
				}
				
			}
			break;
		default:
			// No Effect
			break;
	}
	
	
	// Calculate the network speed ..
	// [TO DO LATER]
	
	// Record the entire screen width and height
	param->m_prop_requested.region_o_h = sh;
	param->m_prop_requested.region_o_w = sw;
	
	// Starting of new x and y
	//cx = 50;
	//cy = 50;

	cx = param->m_prop_requested.region_x;
	cy = param->m_prop_requested.region_y;

	//param->m_prop_requested.region_x = cx;
	//param->m_prop_requested.region_y = cy;

	// 0 - default, 1 - width & height defined, 2 - custom: needs to calculate height based on ratio
	switch(param->m_desired_resol){
		case 0:
			break;
		case 1:
			sw = param->m_desired_resol_width;
			sh = param->m_desired_resol_height;
			break;
		case 2:
			
			//out_h = (long) (desired_w/(double) ((double) current_w / (double) current_h));
			param->m_desired_resol_height = (int) (param->m_desired_resol_width/(double) ((double) sw / (double) sh));
			sw = param->m_desired_resol_width;
			sh = param->m_desired_resol_height;
			break;
		default:
			break;
	}

	
	// 
	sh = param->m_prop_requested.region_h;
	sw = param->m_prop_requested.region_w;

	param->m_prop_requested.bpp = bpp;
	//param->m_prop_requested.region_h = sh;
	//param->m_prop_requested.region_w = sw;
	param->m_prop_requested.Bps = RSCGetBPS(sw, bpp);
	param->m_prop_requested.pixel_type = RSCGetPixelTypeFromBitsPerPixel(bpp, red_b, green_b, blue_b);
		
	// Reply back to Server with PROPERTIES the client want
	if(!socket->RSCSend((RSC_BYTE *) &cx, 4))
		return false;
	if(!socket->RSCSend((RSC_BYTE *) &cy, 4))
		return false;
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

	if(bpp == 8){
		// Debug
		//MessageBox(NULL, "wv_init 8bit IN", "Debug", MB_OK);

		region_palette_req = 0;
		if(!socket->RSCSend(&region_palette_req, 1))
			return false;

		if(!socket->RSCReceive((RSC_BYTE *) &pal->m_col_table, 1024))
			return false;

		// Debug
		//TCHAR buftmp[256];
		//for(int p=0; p<256; p++){
		//	wsprintf(buftmp, "R-G-B: %d-%d-%d", pal->m_col_table[p*4], pal->m_col_table[p*4+1], pal->m_col_table[p*4+2] );
		//	MessageBox(NULL, buftmp, "Debug", MB_OK); 
		//}

		// Debug
		//MessageBox(NULL, "8bit Color Reduction", "Debug", MB_OK);
		
		if(!pal->CreatePaletteFromColorTable())
			return false;

	}
	return true;
}


bool wv_authenticate(RSCSocket *socket)
{	
	RSC_BYTE auth = 0;
	RSC_BYTE userid[16];
	unsigned char challenge[16];
	
	int retrynum = 0;
	RSCCrypt enc[MAX_PASSWD_RETRY];
	RSC_BYTE status = 0;

	unsigned char key[MAX_PASSWORD], blank[MAX_PASSWORD];
	
	// Copy password to a key variable if the password were specified during ConnectionProc
	memset(key, 0, MAX_PASSWORD);
	memset(blank, 0, MAX_PASSWORD);

	if(memcmp(m_prop.m_prop_password, blank, MAX_PASSWORD) != 0){
		memcpy(key, m_prop.m_prop_password, MAX_PASSWORD);
	}else{
		status = 1;
	}

    // Default USERID=rscuser
	memset(userid, 0, 16);
	strcpy(userid, "rscuser");

	if(!wv_version(socket)){
		return false;
	}

	if(!socket->RSCReceive(&auth, 1))
		return false;

	if(auth == 0){
		// Incompatible RSCP version
		return false;
	}
	
	bool retry = true;
	//
	//
	EnterCriticalSection(&password_cs);
	//
	while(retry){

		// RSC Challenge Response Authentication
		if(!socket->RSCSend(userid, 16)){    // SEND 1
			LeaveCriticalSection(&password_cs);		
			return false;
		}
		if(!socket->RSCReceive((char *)challenge, 16)){    // RECEIVE 2
			LeaveCriticalSection(&password_cs);		
			return false;
		}

		if(status == 1){
			memset(key, 0, MAX_PASSWORD);
			if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PASSWORD), m_prop.m_hwnd, (DLGPROC) PasswordProc)==IDOK){
				//MessageBox(NULL, "Dialog", "Debug", MB_OK);
				memcpy(key, m_prop.m_prop_password, MAX_PASSWORD);
			}
		}else{
			memset(key, 0, MAX_PASSWORD);
			memcpy(key, m_prop.m_prop_password, MAX_PASSWORD);
		}
		
		enc[retrynum].RSCEncryptByteStream(challenge, 16, key);

		if(!socket->RSCSend((RSC_BYTE *)enc[retrynum].enc.text, enc[retrynum].enc.length)){ // SEND 3
			LeaveCriticalSection(&password_cs);		
			return false;
		}

		if(!socket->RSCReceive(&status, 1)){	// SEND 4
			LeaveCriticalSection(&password_cs);		
			return false;
		}

		switch(status){
			case 0:
				retry = false;
				LeaveCriticalSection(&password_cs);
				return true;
				break;
			case 1:
				//memset(key, 0, 8);
				if(retrynum < MAX_PASSWD_RETRY-1){
					retry = true;
					retrynum++;
				}else{
					retry = false;
				}
				break;
			case 2:
				retry =false;
				LeaveCriticalSection(&password_cs);

				return false;
				break;
			case 3:
				retry = false;
				LeaveCriticalSection(&password_cs);

				return false;
				break;
		}
	}
	//
	//
	LeaveCriticalSection(&password_cs);
	//
	
	return true;
}

bool wv_version(RSCSocket *socket)
{
	RSC_BYTE ver_snt[] = "RSC_0001.00";
	RSC_BYTE ver_rcv[17];

	// Receive from server
	if(!socket->RSCReceive(ver_rcv, 17))
		return false;

	if(!strcmp(ver_rcv, "RSC_VERSIONCHECK") == 0)
		return false;

	// Sending RSC_0001.00 ...
	if(!socket->RSCSend(ver_snt, 12))
		return false;
	
	return true;
}

bool CalcNumbersofSubRegion(long sw, long sh, long maxsrw, long maxsrh, long *num_w, long *num_h)
{
	long twc, thc, x , y;

	if(sw < maxsrw || sh < maxsrh)
		return false;

	thc=1;
	for(y=maxsrh; y < sh; y+=maxsrh){		
		if((sh-y) > maxsrh/3){
			thc++;
		}
	}
	
	twc=1;
	for(x=maxsrw; x < sw; x+=maxsrw){
		if((sw-x) > maxsrw/3){
			twc++;
		}
	}

	*num_w = twc;
	*num_h = thc;

	return true;
}

bool GetSubRegion(long sw, long sh, long maxsrw, long maxsrh, long twc, long thc, RSC_CLIENT_THREAD *reg)
{
	long tsw, tsh, x , y;
	long ctr=0;

	if(sw < maxsrw || sh < maxsrh)
		return false;

	for(y=0; y<thc; y++){
		if(y+1 == thc){
			tsh = sh - y*maxsrh;
		}else{
			tsh = maxsrh;
		}

		for(x=0; x<twc; x++){
			if(x+1 == twc){
				tsw = sw - x*maxsrw;
			}else{
				tsw = maxsrw;
			}

			//printf("%dx%d - [%d, %d]\n", x*maxsrw, y*maxsrh, tsw, tsh);
			reg[ctr].prop.m_prop_requested.region_x = x*maxsrw;
			reg[ctr].prop.m_prop_requested.region_y = y*maxsrh;
			reg[ctr].prop.m_prop_requested.region_w = tsw;
			reg[ctr].prop.m_prop_requested.region_h = tsh;
			ctr++;
		}
	}

	return true;

}