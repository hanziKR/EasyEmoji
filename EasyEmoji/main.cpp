//#include <Windows.h>
//#include <iostream>
//#include <string>
//#include <fstream>
//#include <codecvt>
//
//
//#include "resource.h"
//
//wchar_t list[9][40] = {};
//
//
//using namespace std;
//
//

//int WINAPI WinMain(
//    _In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPSTR lpCmdLine,
//    _In_ int nShowCmd) {
//    codecvt_utf8<wchar_t>* converter = new codecvt_utf8<wchar_t>;
//
//    wcout.imbue(locale(locale::empty(), converter));
//
//    wifstream s("emoji.txt");
//    s.imbue(locale(locale::empty(), converter));
//
//    wstring line;
//    while (getline(s, line)) {
//        wcout << line << endl;
//    }
//
//    if (!RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, 'Q'));
//    MSG msg = { 0 };
//    while (GetMessage(&msg, NULL, 0, 0) != 0)
//    {
//        if (msg.message == WM_HOTKEY)
//        {
//            printf("WM_HOTKEY received\n");
//        }
//    }
//
//    delete converter;
//
//    return TRUE;
//}
//int main() {
//    return WinMain(0, 0, 0, 0);
//}
#include <Windows.h>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <fstream>
#include <vector>

#include "resource.h"

using namespace std;

namespace clipboard {
    bool open() {
        return OpenClipboard(0);
    }
    void close() {
        CloseClipboard();
    }
    void copy(wstring str) {
        EmptyClipboard();
        unsigned int size = (str.size() + 1) * 2;

        HGLOBAL hGlobal = GlobalAlloc(GMEM_ZEROINIT, size);
        if (hGlobal) {
            void* lpszData = GlobalLock(hGlobal);
            if (lpszData) {
                memcpy(lpszData, str.c_str(), size);
                GlobalUnlock(hGlobal);
                SetClipboardData(CF_UNICODETEXT, hGlobal);
            }
        }
    }
    void copy(string str) {
        EmptyClipboard();
        unsigned int size = str.size() + 1;

        HGLOBAL hGlobal = GlobalAlloc(GMEM_ZEROINIT, size);
        if (hGlobal) {
            void* lpszData = GlobalLock(hGlobal);
            if (lpszData) {
                memcpy(lpszData, str.c_str(), size);
                GlobalUnlock(hGlobal);
                SetClipboardData(CF_TEXT, hGlobal);
            }
        }
    }
    string get() {
        string data;

        HANDLE hClipboardData = GetClipboardData(CF_TEXT);
        if (hClipboardData) {
            char* pchData = (char*)GlobalLock(hClipboardData);
            if (pchData) {
                data = pchData;
                GlobalUnlock(hClipboardData);
            }
        }

        return data;
    }
    void paste() {
        INPUT inputs[4] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = 'V';

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = 'V';
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_CONTROL;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    }
}

void makeInput(wstring data) {
    if (!clipboard::open()) return;
    string cdata = clipboard::get();
    clipboard::copy(data.c_str());
    clipboard::close();

    clipboard::paste();

    Sleep(100);

    if (!clipboard::open()) return;
    clipboard::copy(cdata);
    clipboard::close();
}


static vector<wstring> emojis;

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        HICON hIcon = (HICON)LoadImage(GetModuleHandleW(0), MAKEINTRESOURCEW(IDI_ICON1),
            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
        if (hIcon) {
            SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        POINT p;
        if (GetCursorPos(&p)) {
            POINT winp;
            RECT desktopSize;
            RECT winsize;

            const HWND hDesktop = GetDesktopWindow();
            GetWindowRect(hDesktop, &desktopSize);
            GetWindowRect(hwndDlg, &winsize);

            winp = p;

            if (winp.x > desktopSize.right - winsize.right) {
                winp.x = desktopSize.right - winsize.right;
            }
            if (winp.y > desktopSize.bottom - winsize.bottom - 80) {
                winp.y = desktopSize.bottom - winsize.bottom - 80;
            }

            SetWindowPos(hwndDlg, HWND_TOPMOST, winp.x, winp.y, 0, 0, SWP_NOSIZE);
            SetForegroundWindow(hwndDlg);
        }

        HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

        for (auto i : emojis) {
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)i.c_str());
        }

        return TRUE;
    }
    case WM_COMMAND: {
        if (wParam == 66537) {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

            int i = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

            EndDialog(hwndDlg, i);
            return TRUE;
        }
        else if (wParam == 2) {
            EndDialog(hwndDlg, -1);
            return TRUE;
        }
        return FALSE;
    }
    default: {
        return FALSE;
    }
    }
}


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd) {
    //ShowDesktop();
    const codecvt_utf8<wchar_t>* converter = new codecvt_utf8<wchar_t>;
    const locale utf8_locale = locale(locale::empty(), converter);

    wifstream stream(L"emoji.txt");
    stream.imbue(utf8_locale);
    wcout.imbue(utf8_locale);

    wstring line;
    while (getline(stream, line)) {
        emojis.push_back(line);
    }

    //wcout << line << endl;
    
    RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_NOREPEAT, VK_OEM_PERIOD);

    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            //SendString(line.c_str(), line.length());
            //makeInput(line);
            int i = DialogBox(0, MAKEINTRESOURCE(IDD_DIALOG), 0, (DLGPROC)DialogProc);
            if (i != -1) {
                makeInput(emojis[i]);
            }
            //MessageBox(0, emojis[i].c_str(), L"", 0);
        }
    }

    system("pause");
}