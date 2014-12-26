#include "resource.h"

/* TODO:
1.) Make track text centered
2.) Get rid of extra bars on track text
3.) Optimize code
4.) Make player skinnible
5.) Add more file format support
-WAV/PCM/CDA
-OGG VORBIS
-MIDI
6.) Playlist support (custom format, not wmf/asx crap)
7.) Make custom progressbar (or make current transparent && borderless)
8.) Fix registry entries
*/

HWND hwnd,hPlay,hStop,hPause,hOpen,hProgress = NULL;
HANDLE hBitmap		= NULL;
HDC dcSkin			= NULL;
BOOL bDone			= FALSE;
int Status			= 0;
int ScreenWidth		= 254;
int ScreenHeight	= 125;
POINT curpoint, point;
RECT MainRect, rect;
HINSTANCE hInst;
MCIDEVICEID pDevice = 0;
MCI_OPEN_PARMS op;
char lastFile[1024];
char Title[1024];
char szFileTitle[1024];
int minutes, seconds;
int rSeconds;

LRESULT CALLBACK WndProc( HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam );
void PlayAudio( const char *Filename );
void StopAudio();

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
    WNDCLASSEX wc;
    MSG Msg;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0,0,0));
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hIconSm = LoadIcon( NULL, IDI_WINLOGO );
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "JuMP";
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassEx( &wc );

    hwnd = CreateWindowEx( 0, "JuMP", "JuMP Player", WS_POPUPWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, 0, 0, hInstance, 0 );
    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );

    HBITMAP hSkinBmp = (HBITMAP)LoadImage (GetModuleHandle (NULL), "Images/Background.bmp",
                                 IMAGE_BITMAP,0, 0,LR_DEFAULTCOLOR | LR_LOADFROMFILE);
    dcSkin = CreateCompatibleDC(0);
    HBITMAP hOldBmp = (HBITMAP)SelectObject( dcSkin, hSkinBmp );
    InvalidateRect(hwnd, NULL, TRUE);

    hInst = hInstance;
    DWORD lastTime = (DWORD)(GetTickCount() * 0.001f);
    DWORD curTime;
    rect.left = 10;
    rect.right = 320;
    rect.top = 50;
    rect.bottom = 100;

    while( !bDone )
    {
        if(Status == 1)
        {
            if( ::PeekMessage(&Msg, hwnd, 0, 0, PM_NOREMOVE) )
            {
                if(GetMessage( &Msg, hwnd, 0, 0 ))
                {
                    TranslateMessage( &Msg );
                    DispatchMessage( &Msg );
                }
            }
            curTime = (DWORD)(GetTickCount() * 0.001f);
            if( curTime - lastTime >= 1 ) // 1 second
            {
                SendMessage(hProgress, PBM_SETSTEP, 1, 0);
                SendMessage(hProgress, PBM_STEPIT, 0, 0);
                if(seconds == 1 && minutes == 0)
                {
                    sprintf( Title, "%s - %02d:%02d", szFileTitle, minutes, seconds );
                    //StopAudio();
                    Status = 0;
                }
                if(seconds != 0){seconds--;}
                else
                {
                    minutes--;
                    seconds = 59;
                }
                InvalidateRect(hwnd, &rect, TRUE);
                lastTime = curTime;
            }
            SendMessage(hwnd, WM_PAINT, 0, 0);
        }
        else
        {
            if(GetMessage( &Msg, hwnd, 0, 0 ))
            {
                TranslateMessage( &Msg );
                DispatchMessage( &Msg );
            }
        }
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    switch( Msg )
    {
        case WM_CREATE:
        {
        	//TiXmlDocument playlist( "play.jmp" );
            //playlist.LoadFile();

            hPlay = CreateWindow( "BUTTON", "Play", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_BITMAP,
                                  26, 101, 49, 17, hwnd, (HMENU)PLAY, hInst, NULL );
            hPause = CreateWindow( "BUTTON", "Pause", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_BITMAP,
                                   76, 101, 49, 17, hwnd, (HMENU)PAUSE, hInst, NULL );
            hStop = CreateWindow( "BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_BITMAP,
                                  126, 101, 49, 17, hwnd, (HMENU)STOP, hInst, NULL );
            hOpen = CreateWindow( "BUTTON", "Open", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_BITMAP,
                                  176, 101, 49, 17, hwnd, (HMENU)OPEN, hInst, NULL );

            hBitmap = LoadImage (GetModuleHandle (NULL), "Images/Play.bmp",
                                 IMAGE_BITMAP,0, 0,LR_DEFAULTCOLOR | LR_LOADFROMFILE);
            SendMessage(hPlay, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
                        (LPARAM)hBitmap);

            hBitmap = LoadImage (GetModuleHandle (NULL), "Images/Pause.bmp",
                                 IMAGE_BITMAP,0, 0,LR_DEFAULTCOLOR | LR_LOADFROMFILE);
            SendMessage(hPause, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
                        (LPARAM)(HANDLE) hBitmap);

            hBitmap = LoadImage (GetModuleHandle (NULL), "Images/Stop.bmp",
                                 IMAGE_BITMAP,0, 0,LR_DEFAULTCOLOR | LR_LOADFROMFILE);
            SendMessage(hStop, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
                        (LPARAM)(HANDLE) hBitmap);

            hBitmap = LoadImage (GetModuleHandle (NULL), "Images/Open.bmp",
                                 IMAGE_BITMAP,0, 0,LR_DEFAULTCOLOR | LR_LOADFROMFILE);
            SendMessage(hOpen, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
                        (LPARAM)(HANDLE) hBitmap);

            hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL,PBS_SMOOTH | WS_CHILD
                                       | WS_VISIBLE ,13, 88, 224, 11,hwnd, 0, hInst, 0);
            SendMessage(hProgress, PBM_SETBARCOLOR, 0, (COLORREF)RGB(128, 0, 0));
            SendMessage(hProgress, PBM_SETBKCOLOR, 0, (COLORREF)RGB(255, 33 , 33)); //MAKE TRANSPARENT!!!
            /*
            HKEY hKey;
            RegCreateKeyEx(HKEY_CLASSES_ROOT,".jmp",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"JincS.jmp\0",10);
            RegSetValueEx(hKey,"PerceivedType", 0, REG_SZ,(CONST BYTE*)"JuMP\0",6);
            RegCreateKeyEx(HKEY_CLASSES_ROOT,"JincS.jmp",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"JuMP File\0",11);
            RegCreateKeyEx(HKEY_CLASSES_ROOT,"JincS.jmp\\DefaultIcon",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"JuMP.exe,1\0",27);
            RegCreateKeyEx(HKEY_CLASSES_ROOT,"JincS.jmp\\Shell",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegCreateKeyEx(HKEY_CLASSES_ROOT,"JincS.jmp\\Shell\\Open",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegCreateKeyEx(HKEY_CLASSES_ROOT,"JincS.jmp\\Shell\\Open\\Command",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"C:\\Dev-Cpp\\DevCpp.exe \"%1\"\0",32);
            RegCreateKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\JincS",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"(c)2006 JincS. All Rights Reserved.",35);
            RegCreateKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\JincS\\MP3",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegCreateKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\JincS\\MP3\\InstallPath",0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,0);
            RegSetValueEx(hKey,NULL, 0, REG_SZ,(CONST BYTE*)"C:\\Dev-Cpp\\DevCpp.exe",23);
            RegCloseKey(hKey);
            */
        }
        break;
        case WM_COMMAND:
        {
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                {
                    switch(LOWORD(wParam))
                    {
                        case PLAY:
                        {
                            if(Status == 0)
                            {
                                PlayAudio( lastFile );
                                Status = 1;
                            }
                            if(Status == 2)
                            {
                                mciSendCommand( pDevice, MCI_RESUME, 0, 0 );
                                Status = 1;
                            }
                        }
                        break;
                        case PAUSE:
                        {
                            mciSendCommand( pDevice, MCI_PAUSE, 0, 0 );
                            Status = 2;
                        }
                        break;
                        case STOP:
                        {
                            StopAudio();
                            Status = 0;
                        }
                        break;
                        case OPEN:
                        {
                            char szFile[1024];
                            OPENFILENAME ofn;
                            ZeroMemory(&ofn,sizeof(OPENFILENAME));
                            ofn.lStructSize = sizeof(OPENFILENAME);
                            ofn.hwndOwner = GetParent(hwnd);
                            ofn.lpstrFilter = "MP3 Files(*.mp3)\0*.mp3\0";//JuMP Playlists(*.jmp)\0*.jmp\0";
                            ofn.nFilterIndex = 1;
                            ofn.lpstrFile = szFile;
                            ofn.nMaxFile = sizeof(szFile);
                            ofn.lpstrTitle = "JincS MP3 Player - Choose A File...";
                            ofn.lpstrFileTitle = szFileTitle;
                            ofn.nMaxFileTitle = 1024;
                            ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;
                            if (GetOpenFileName(&ofn))
                            {
                                if(Status == 1)
                                {
                                    StopAudio();
                                    Status = 0;
                                }
                                HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ,
                                                          FILE_SHARE_READ, 0, OPEN_EXISTING,
                                                          FILE_ATTRIBUTE_NORMAL, 0);
                                if (hFile == (HANDLE)-1)
                                {
                                    MessageBox(NULL,"Could not open this file", "File I/O Error", MB_ICONSTOP);
                                    return FALSE;
                                }
                                PlayAudio( ofn.lpstrFile );
                                Status = 1;
                            }
                        }
                        break;
                    }
                }
                break;
                default:
                {
                    sprintf( Title, "%s - %02d:%02d", szFileTitle, minutes, seconds );
                    InvalidateRect(hwnd, &rect, TRUE);
                }
                break;
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            SetCapture( hwnd );
            GetWindowRect(hwnd, &MainRect);
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);
            if(point.x >= 189 && point.x <= 202 && point.y >= 6 && point.y <= 19)
            {
                ShowWindow( hwnd, SW_MINIMIZE );
                InvalidateRect(hwnd, &rect, TRUE);
            }
            if(point.x >= 207 && point.x <= 220 && point.y >= 6 && point.y <= 19)
            {
                ShowWindow( hwnd, SW_RESTORE );
                InvalidateRect(hwnd, &rect, TRUE);
            }
            if(point.x >= 225 && point.x <= 238 && point.y >= 6 && point.y <= 19)
            {
                SendMessage( hwnd, WM_DESTROY, 0, 0 );
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
        }
        break;
        case WM_MOUSEMOVE:
        {
            GetCursorPos(&curpoint);
            if(wParam==MK_LBUTTON)
            {
                MoveWindow(hwnd, curpoint.x - point.x, curpoint.y - point.y,
                           MainRect.right - MainRect.left, MainRect.bottom - MainRect.top,
                           TRUE);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd,&ps);
            BitBlt(ps.hdc, 0,0,ScreenWidth-4,ScreenHeight, dcSkin, 0,0, SRCCOPY);
            SetBkMode( ps.hdc, TRANSPARENT );
            SetTextColor(ps.hdc,RGB(160,34,34));
            TextOut( ps.hdc, 16, 56, Title, 45 );
            EndPaint(hwnd,&ps);
        }
        break;
        case WM_CLOSE:
        case WM_DESTROY:
        {
            Status=0;
            bDone=TRUE;
            PostQuitMessage( 0 );
        }
        break;
        default:
        {
            if(Title!=NULL&&szFileTitle!=NULL&&minutes!=NULL&&seconds!=NULL)
            {
                sprintf( Title, "%s - %02d:%02d", szFileTitle, minutes, seconds );
            }
        }
        break;
    }
    return DefWindowProc( hwnd, Msg, wParam, lParam );
}

void PlayAudio( const char *Filename )
{
    if(Status == 1)
    {
        return;
    }
    op.dwCallback = 0;
    op.lpstrDeviceType = (char*)MCI_ALL_DEVICE_ID;
    op.lpstrElementName = Filename;
    op.lpstrAlias = 0;
    if ( mciSendCommand( 0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD)&op) == 0)
    {
        pDevice = op.wDeviceID;
    }
    MCI_PLAY_PARMS pp;
    pp.dwCallback = 0;
    if(Status == 0)
    {
        strcpy( lastFile, Filename );
        char szCommandBuffer[512] ={char()}, szReturnBuffer[32] ={char()};
        ::sprintf(szCommandBuffer, "open \"%s\" alias myfile", Filename);
        DWORD dwDummy = ::mciSendString(szCommandBuffer, NULL, 0, NULL);
        if(!dwDummy)
        {
            ::sprintf(szCommandBuffer, "status myfile length", Filename);
            dwDummy = ::mciSendString(szCommandBuffer, szReturnBuffer, sizeof(szReturnBuffer), NULL);
            ::mciSendString(szCommandBuffer, NULL, 0, NULL);
        }
        rSeconds = atoi(szReturnBuffer)/1000;
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, rSeconds));
        float nSecond = atof(szReturnBuffer)/1000/60;
        seconds = (int)nSecond;
        nSecond -= seconds;
        seconds = (int)(nSecond * 60);
        minutes = atoi(szReturnBuffer)/1000/60;
        sprintf( Title, "%s - %02d:%02d", szFileTitle, minutes, seconds );
        InvalidateRect(hwnd, &rect, TRUE);
    }
    mciSendCommand(pDevice, MCI_PLAY, MCI_NOTIFY, (DWORD)&pp);
}

void StopAudio()
{
    mciSendCommand( pDevice, MCI_STOP, 0, 0 );
    mciSendCommand( pDevice, MCI_CLOSE, 0, 0 );
    mciSendString( "close myfile", 0, 0, 0);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
}
