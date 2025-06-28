#pragma once

#include <Windows.h>
#include <WinUser.h>
#include <thread>

#include <tlhelp32.h>

static HWND DEBUGGlobalWindowId = 0;
const int HEADER_OFFSET = 30;
const int HORIZONTAL_OFFSET = 115;

const int DownUpClickDelay = 60;

std::string doubleToString(double value){
    std::ostringstream oss;
    int precision = 2;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

double randomInRange(int min, int max) {
    // From (inclusive) to 
    if (min == max) {
        return min;
    }
    if (max > min) {
        double normal = (double)rand() / (double)RAND_MAX;
        return min + (normal * (max - min));
    }
    else {
        return 0;
    }
}

class ClickerArea {
public:
    ClickerArea(){}

    ClickerArea(int x1, int y1, int x2, int y2) :
        startX(x1),
        startY(y1),
        width(x2 - x1),
        height(y2 - y1) {
    };

    int startX;
    int startY;
    int width;
    int height;

    ClickerArea getAdjustedX(int size) {
        int newStartX = startX - size;
        return ClickerArea(newStartX, startY, newStartX + width, startY + height);
    }

    int centerRandomX() {
        return startX + (width / 2);
        double center = startX + (width / 2);
        double pad = width * 0.25;
        return randomInRange(center - pad, center + pad);
    }

    int centerRandomY() {
        return startY + (height/ 2);
        double center = startY + (height / 2);
        double pad = height * 0.25;
        return randomInRange(center - pad, center + pad);
    }
};


class Areas {
public:
    // bool isSidebarPresent = true;
    bool isSidebarPresent = false;

    ClickerArea waitBigBlindSelect;
    ClickerArea post4JoinNextSelect;
    ClickerArea buyChipsToggle;
    ClickerArea buyChipsInput;
    ClickerArea buyChips_do;
    ClickerArea raiseInput;
    ClickerArea rraise;
    ClickerArea check;
    ClickerArea fold;
    ClickerArea call;
    ClickerArea betInput;
    ClickerArea bet; // NOTE: pretty much the same thing as 'raise'
                     //
    ClickerArea allInBigTop; // NOTE: pretty much the same thing as 'raise'
    ClickerArea allInSmallBottom; // NOTE: pretty much the same thing as 'raise'

    ClickerArea muckCards;

    // Areas() :
    //     waitBigBlindSelect(ClickerArea(382, 507, 566, 547)),
    //     post4JoinNextSelect(ClickerArea(574, 507, 758, 547)),
    //     buyChipsToggle(ClickerArea(15, 520, 180, 545)),
    //     buyChipsInput(ClickerArea(31, 424, 164, 448)),
    //     buyChips_do(ClickerArea(30, 470, 162, 512)),
    //     raiseInput(ClickerArea(683, 491, 776, 517)),
    //     rraise(ClickerArea(644, 437, 776, 484)),
    //     check(ClickerArea(498, 437, 630, 478)),
    //     fold(ClickerArea(358, 437, 490, 477)),
    //     call(ClickerArea(498, 437, 630, 477)),
    //     betInput(ClickerArea(484, 491, 776, 517)),
    //     bet(ClickerArea(644, 437, 776, 484)), // NOTE: pretty much the same thing as 'raise'
    //     //
    //     allInBigTop(ClickerArea(644, 440, 775, 481)),
    //     allInSmallBottom(ClickerArea(677, 523, 775, 546))
    // { }

    Areas()
    { 
        waitBigBlindSelect = ClickerArea(382, 507, 566, 547);
        post4JoinNextSelect = ClickerArea(574, 507, 758, 547);
        buyChipsToggle = ClickerArea(15, 520, 180, 545);
        buyChipsInput = ClickerArea(31, 424, 164, 448);
        buyChips_do = ClickerArea(30, 470, 162, 512);
        raiseInput = ClickerArea(683, 491, 776, 517);
        rraise = ClickerArea(644, 437, 776, 484);
        check = ClickerArea(498, 437, 630, 478);
        fold = ClickerArea(358, 437, 490, 477);
        call = ClickerArea(498, 437, 630, 477);
        betInput = ClickerArea(484, 491, 776, 517);
        bet = ClickerArea(644, 437, 776, 484); // NOTE: pretty much the same thing as 'raise'
        //
        allInBigTop = ClickerArea(644, 440, 775, 481);
        allInSmallBottom = ClickerArea(677, 523, 775, 546);
        muckCards = ClickerArea(573, 507, 757, 546);
    }

    ClickerArea adjust(ClickerArea area) {
        if(isSidebarPresent) {
            return area.getAdjustedX(HORIZONTAL_OFFSET);
        } else {
            return area.getAdjustedX(0);
        }
    }


    // Areas() {
    //     waitBigBlindSelect  = ClickerArea(382, 507, 566, 547);
    //     post4JoinNextSelect = ClickerArea(574, 507, 758, 547);
    //
    //     buyChipsToggle = ClickerArea(15, 520, 180, 545);
    //     buyChipsInput  = ClickerArea(31, 424, 164, 448);
    //     buyChips_do    = ClickerArea(30, 470, 162, 512);
    //
    //     raiseInput = ClickerArea(484, 491, 776, 517);
    //     raise      = ClickerArea(644, 437, 776, 484);
    //
    //     check = ClickerArea(498, 437, 630, 478);
    //     fold  = ClickerArea(358, 437, 490, 477);
    //     call  = ClickerArea(498, 437, 630, 477);
    //
    //     betInput = ClickerArea(484, 491, 776, 517);
    //     bet      = ClickerArea(644, 437, 776, 484); // NOTE: pretty much the same thing as 'raise'
    // }

    // static ClickerArea waitBigBlindSelect  = ClickerArea(382, 507, 566, 547);
    // static ClickerArea post4JoinNextSelect = ClickerArea(574, 507, 758, 547);
    //
    // static ClickerArea buyChipsToggle = ClickerArea(15, 520, 180, 545);
    // static ClickerArea buyChipsInput  = ClickerArea(31, 424, 164, 448);
    // static ClickerArea buyChips_do    = ClickerArea(30, 470, 162, 512);
    //
    // static ClickerArea raiseInput = ClickerArea(484, 491, 776, 517);
    // static ClickerArea raise      = ClickerArea(644, 437, 776, 484);
    //
    // static ClickerArea check = ClickerArea(498, 437, 630, 478);
    // static ClickerArea fold  = ClickerArea(358, 437, 490, 477);
    // static ClickerArea call  = ClickerArea(498, 437, 630, 477);
    //
    // static ClickerArea betInput = ClickerArea(484, 491, 776, 517);
    // static ClickerArea bet      = ClickerArea(644, 437, 776, 484); // NOTE: pretty much the same thing as 'raise'
};


bool isSubstring(const char* haystack, const char* needle) {
    return strstr(haystack, needle) != nullptr;
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    int length = GetWindowTextLength(hWnd);
    char* buffer = new char[length + 1];
    GetWindowTextA(hWnd, buffer, length + 1);
    std::string windowTitle(buffer);
    delete[] buffer;

    if (IsWindowVisible(hWnd) && length != 0) {
        std::cout << hWnd << ":  " << windowTitle << std::endl;
    }
    return TRUE;
}


BOOL CALLBACK enumProc(HWND hwnd, LPARAM lParam) {
    std::cout << "Window Handle: " << hwnd << std::endl;
    return TRUE;
}


bool isPointClickable(HWND hwnd, POINT point) {
    // Save the original cursor position
    POINT originalPos;
    GetCursorPos(&originalPos);

    // Convert the test point from client coordinates to screen coordinates
    POINT screenPoint = point;
    ClientToScreen(hwnd, &screenPoint);

    // Move the cursor to the test point
    int HEADER_OFFSET = 0;
    SetCursorPos(screenPoint.x, screenPoint.y - HEADER_OFFSET);

    // Allow time for the cursor to update (some systems may have a delay)
    Sleep(50);

    HCURSOR handCursor = LoadCursor(NULL, IDC_HAND);

    CURSORINFO Info = { 0 };
    Info.cbSize = sizeof(Info);
    GetCursorInfo(&Info);

    // Move the cursor back to its original position
    SetCursorPos(originalPos.x, originalPos.y);

    // Compare the cursors to see if it changed
    if (handCursor == Info.hCursor) {
        return true; // The area is clickable or interactive
    }

    return false; // The area is not interactive
}



void checkWidthAndClose();

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowProcessId;

    GetWindowThreadProcessId(hwnd, &windowProcessId);

    int found = 0;

    char windowTitle[1024];
    GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
    // logfff("windowProcessId: %d, lParam: %d, %s\n", windowProcessId, lParam, windowTitle);

    if (windowProcessId == lParam) {
        char windowTitle[1024];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        // TODO: This way of filtering is not robust. From what I have seen
        // they sometimes change titles. It could be in different language and
        // etc. It is good enough for current test.

        // found  = isSubstring(windowTitle, "bodog.eu");
        // found |= isSubstring(windowTitle, "Bodog.eu");

        // found |= isSubstring(windowTitle, "Chrome_WidgetWin_1");
        // found |= isSubstring(windowTitle, "2/4 No Limit Hold'em");

        // bool titleFound  = isSubstring(windowTitle, "No Limit Hold");
        bool titleFound  = isSubstring(windowTitle, "Limit Hold");

        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        bool hasMaximize = style & WS_MAXIMIZEBOX;
        bool isVisible = IsWindowVisible(hwnd);

        // if (hasMaximize && isVisible)

        if (titleFound && isVisible) {
            logfff("Game Window: %x, title: %s\n", hwnd, windowTitle);
            DEBUGGlobalWindowId = hwnd;
            SetWindowLong(DEBUGGlobalWindowId, GWL_STYLE, GetWindowLong(DEBUGGlobalWindowId, GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
            found = true;
            // checkWidthAndClose();
        }

    }

    if (found) {
        return false;
    }

    return TRUE; // Continue enumerating
}

HWND getWindowsForPid(int targetProcessId) {
    DEBUGGlobalWindowId = 0;
    // logfff("- DEBUGGlobalWindowId: %x\n", DEBUGGlobalWindowId);
    EnumWindows(EnumWindowsProc, (LPARAM)targetProcessId);
    // logfff("- DEBUGGlobalWindowId: %x\n", DEBUGGlobalWindowId);
    return DEBUGGlobalWindowId;
}

DWORD GetParentProcessId(DWORD processId) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == processId) {
                CloseHandle(hSnapshot);
                return pe32.th32ParentProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0; // Parent process not found
}

HWND initWindow() {
    HANDLE hProcess = GetCurrentProcess();
    DWORD processID = GetProcessId(hProcess);
    DWORD parentProcessID = GetParentProcessId(processID);
    logfff("hProcess: %x %d\n", hProcess, hProcess);
    logfff("processID: %x %d\n", processID, processID);
    logfff("parentProcessID: %x %d\n", parentProcessID, parentProcessID);
    return getWindowsForPid(parentProcessID);
}

bool isWindowCorrect() {
    if (DEBUGGlobalWindowId == 0 || !IsWindow(DEBUGGlobalWindowId)) {
        //logfff("?? DEBUGGlobalWindowId: %x\n", DEBUGGlobalWindowId);
        HWND window = initWindow();
        if (window == 0 || !IsWindow(window)) {
            return false;
        }
    }
    return true;
}

// void testPoint() {
//     isWindowCorrect();
//     // POINT testPoint = {954, 554};
//     POINT testPoint = { 833, 227 };
//     int result = isPointClickable(DEBUGGlobalWindowId, testPoint);
//     logfff("----- Point: %d %d, is clickable: %d", testPoint.x, testPoint.y, result);
// }




void sendClickEvent(HWND window, int mouseX, int mouseY) {
    //PostMessage(window, WM_LBUTTONDOWN, 0, MAKELPARAM(mouseX, mouseY));
    PostMessage(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(mouseX, mouseY));
}

void sendLButtonUp(HWND hwnd, int x, int y) {
    LPARAM lParam = MAKELPARAM(x, y);
    PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, lParam);
}

void DEBUGmoveCursorRelativeToAbsolute(HWND window, int x, int y) {
    RECT rect = { NULL };
    GetWindowRect(DEBUGGlobalWindowId, &rect);

    int cursorY = rect.top + y;
    int cursorX = rect.left + x;

    //logfff("GlobalX: %d, GlobalY: %d\n", cursorX, cursorY);

    SetCursorPos(cursorX, cursorY);
}

void activateAndFocusWindow(HWND hwnd) {
    if (!hwnd) {
        std::cerr << "Invalid window handle!" << std::endl;
        return;
    }

    if (!SetForegroundWindow(hwnd)) {
        std::cerr << "Failed to bring the window to the foreground!" << std::endl;
    }

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    if (!SetFocus(hwnd)) {
        std::cerr << "Failed to set focus to the window!" << std::endl;
    }
}

void simulateMouseClick(int x, int y) {
    INPUT inputs[2] = {};

    // Move the mouse to the specified screen coordinates
    SetCursorPos(x, y);

    // Simulate a left mouse button down event
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;


    // Simulate a left mouse button up event
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    // Send the input events
    SendInput(2, inputs, sizeof(INPUT));
}

void click(HWND window, int x, int y) {
    //y += HEADER_OFFSET;

    // TODO: Sleep random amount?
    // Sleep(2000);

    // NOTE: Sending just either one of this will not work
    sendClickEvent(window, x, y);
    Sleep(DownUpClickDelay); // TODO: Make it between 40 and 80
    sendLButtonUp(window, x, y);

    // DEBUGmoveCursorRelativeToAbsolute(window, x, y);
}


void sendClickGeneral(ClickerArea action, int delay) {
    if (!isWindowCorrect()) {
        logfff("===2=== ERROR: requested action but no window to perform it to. [%d]\n", DEBUGGlobalWindowId);
        return;
    }

    RECT rect = { NULL };
    GetWindowRect(DEBUGGlobalWindowId, &rect);

    // logfff("Window RECT: %d %d %d %d\n", rect.left, rect.right, rect.top, rect.bottom);

    int currentWidth = rect.right - rect.left;
    int currentHeight = rect.bottom - rect.top;

    int REQUIREDwindowWidth = 976;
    int REQUIREDwindowHeight = 599;

    if ((REQUIREDwindowHeight != currentHeight) && (REQUIREDwindowWidth != currentWidth)) {
        SetWindowPos(DEBUGGlobalWindowId, HWND_TOP, rect.left, rect.right, REQUIREDwindowWidth, REQUIREDwindowHeight, SWP_NOZORDER);
        // Fix window size and don't allow resizing
        SetWindowLong(DEBUGGlobalWindowId, GWL_STYLE, GetWindowLong(DEBUGGlobalWindowId, GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
    }
    //if (DEBUGGlobalWindow) {
    //}

    
    int x = action.centerRandomX();
    int y = action.centerRandomY();

    // TODO: Think this logic through. For now starting task in a separate 
    // thread should not be a problem due it is rareness but maybe a queue
    // with a 'job thread', might be better approach
    
    std::thread t([x, y, delay]() {
        // logfff("== t1: %d %d %d\n", x, y, delay);
        if (delay > 0) { Sleep(delay); }
        click(DEBUGGlobalWindowId, x, y);
    });
    t.detach();
}

void sendTextSlowly(HWND window, std::string text) {

    // logfff("------------------- Sending input: %s\n", text.c_str());
    for (char c : text) {
        PostMessage(window, WM_CHAR, static_cast<WPARAM>(c), 0);
        Sleep(60); // TODO: randomize sleep time
    }
}

void sendTextInput(HWND window, int x, int y, std::string text) {
    click(window, x, y);
    sendTextSlowly(window, text);
}

void sendInputGeneral(ClickerArea action, std::string data, int delay) {
    // TODO: This is copy-pasta. Merge and refactor
    if (!isWindowCorrect()) {
        logfff("===3=== ERROR: requested action but no window to perform it to\n");
        return;
    }

    RECT rect = { NULL };
    GetWindowRect(DEBUGGlobalWindowId, &rect);

    
    int currentWidth = rect.right - rect.left;
    int currentHeight = rect.bottom - rect.top;

    int REQUIREDwindowWidth = 976;
    int REQUIREDwindowHeight = 599;

    if ((REQUIREDwindowHeight != currentHeight) && (REQUIREDwindowWidth != currentWidth)) {
        SetWindowPos(DEBUGGlobalWindowId, HWND_TOP, rect.left, rect.right, REQUIREDwindowWidth, REQUIREDwindowHeight, SWP_NOZORDER);
        // Fix window size and don't allow resizing
        SetWindowLong(DEBUGGlobalWindowId, GWL_STYLE, GetWindowLong(DEBUGGlobalWindowId, GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
    }

    int x = action.centerRandomX();
    int y = action.centerRandomY();
        
    std::thread t([x, y, delay, data]() {
        if (delay) {
            Sleep(delay);
        }
        sendTextInput(DEBUGGlobalWindowId, x, y, data);
    });
    t.detach();
}

//void debugStartInputLoop(int pid) {
//    //EnumWindows(enumWindowCallback, NULL);
//
//    HWND window = getWindowsForPid(pid);
//    if (IsWindow(window)) {
//        testDoLoop(window);
//        //testDoLoop((HWND)0x00160EB2);
//    }
//    else {
//        printf(":: Error. Could get correct window.");
//    }
//}


bool checkIfHasSidebar() {
    bool isClickable = true;
    if(isWindowCorrect()) {
        POINT testPoint = { 833, 227 }; // NOTE: General area with ads. Center.
        isClickable = isPointClickable(DEBUGGlobalWindowId, testPoint);
        logfff("is side: %d\n", isClickable);
    } else {
        logfff("Window has not been initialized yet\n");
    }
    return isClickable; 
}

void checkWidthAndClose() {
    POINT testPoint = { 833, 227 }; // NOTE: General area with ads. Center.
    int isClickable = isPointClickable(DEBUGGlobalWindowId, testPoint);

    // if(isClickable) {
    //     // Close side bar
    //     ClickerArea fold = ClickerArea(930, 0, 969, 40);
    //     sendClickGeneral(fold, 0);
    // }
}


// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

const int DEFAULT_WAIT_BEFORE_ACTION = 3000; // seconds
const int DEFAULT_SECOND_TEST_ACTION = DEFAULT_WAIT_BEFORE_ACTION + DownUpClickDelay + 100; // seconds

void DEBUGDoFold(Areas areas) {
    // ClickerArea fold = ClickerArea(358, 437, 490, 477);
    ClickerArea fold = areas.adjust(areas.fold);
    sendClickGeneral(fold, DEFAULT_WAIT_BEFORE_ACTION);
}

// void DEBUGDoQuickFoldSelection(Areas areas) {
//     // ClickerArea quickFold = ClickerArea(366, 508, 495, 548);
//     ClickerArea quickFold = areas.adjust(areas.quickFold);
//     sendClickGeneral(quickFold, DEFAULT_WAIT_BEFORE_ACTION);
// }

void DEBUGDoFoldOrCheck(Areas areas) {
    // ClickerArea check = ClickerArea(498, 437, 630, 478);
    // ClickerArea fold = ClickerArea(358, 437, 490, 477);

    ClickerArea check = areas.adjust(areas.check);
    ClickerArea fold = areas.adjust(areas.fold);

    sendClickGeneral(fold, DEFAULT_WAIT_BEFORE_ACTION);
    sendClickGeneral(check, DEFAULT_SECOND_TEST_ACTION);
}

void DEBUGDoCheck(Areas areas) {
    // ClickerArea check = ClickerArea(498, 437, 630, 478);
    ClickerArea check = areas.adjust(areas.check);
    sendClickGeneral(check, DEFAULT_WAIT_BEFORE_ACTION);
}

void DEBUGDoCall(Areas areas) {
    // ClickerArea call = ClickerArea(498, 437, 630, 477);
    ClickerArea call = areas.adjust(areas.call);
    sendClickGeneral(call, DEFAULT_WAIT_BEFORE_ACTION);

    // NOTE: Temporary hack for cases when a call was requested but all 
    // wee can do is allin and there only button to allin.
    // Better approach would be to send all in directly by detecting that
    // case but this is good enough for now. This button could be an indicator:
    // CO_SELECT_REQ:... "btns":34606080 ...
    ClickerArea allInBigTop = areas.adjust(areas.allInBigTop);
    sendClickGeneral(allInBigTop, DEFAULT_SECOND_TEST_ACTION);
}


void DEBUGDoRaise(Areas areas, double amount) {
    // Areas areas = Areas();
    // std::string data = std::to_string(amount);
    std::string data = doubleToString(amount);
    logfff("Raise DoubleToString d: %f, s: %s\n", amount, data.c_str());


    ClickerArea raiseInput = areas.adjust(areas.raiseInput);
    ClickerArea rraise = areas.adjust(areas.rraise);

    sendInputGeneral(raiseInput, data, DEFAULT_WAIT_BEFORE_ACTION);
    sendClickGeneral(rraise, DEFAULT_WAIT_BEFORE_ACTION + 1000);
}

void DEBUGDoBet(Areas areas, double amount) {
    // Areas areas = Areas();
    // std::string data = std::to_string(amount);
    std::string data = doubleToString(amount);
    logfff("Bet DoubleToString d: %f, s: %s\n", amount, data.c_str());

    // NOTE: Bet input and raise input are pretty much the same thing
    // but may change in the future so thus different areas are used

    ClickerArea betInput = areas.adjust(areas.betInput);
    ClickerArea bet = areas.adjust(areas.bet);

    sendInputGeneral(betInput, data, DEFAULT_WAIT_BEFORE_ACTION);
    sendClickGeneral(bet, DEFAULT_WAIT_BEFORE_ACTION);
}

void DEBUGAllIn(Areas areas) {
    // Areas areas = Areas();

    ClickerArea allInSmallBottom = areas.adjust(areas.allInSmallBottom);
    ClickerArea allInBigTop = areas.adjust(areas.allInBigTop);

    sendClickGeneral(allInSmallBottom, DEFAULT_WAIT_BEFORE_ACTION);
    sendClickGeneral(allInBigTop, DEFAULT_WAIT_BEFORE_ACTION + 1000);
}

void DEBUGDoMuckCards(Areas areas) {
    // ClickerArea call = ClickerArea(498, 437, 630, 477);
    ClickerArea muckCards = areas.adjust(areas.muckCards);
    sendClickGeneral(muckCards, DEFAULT_WAIT_BEFORE_ACTION);
}

// void DEBUGTestBuyChipsInput(Areas areas) {
//     ClickerArea buyChipsToggle = ClickerArea(15, 520, 180, 545);
//     ClickerArea buyChipsInput = ClickerArea(31, 424, 164, 448);
//     ClickerArea buyChips_do = ClickerArea(30, 470, 162, 512);
//     ClickerArea aaa = ClickerArea(341, 170, 684, 198);
//
//     sendClickGeneral(buyChipsToggle, DEFAULT_WAIT_BEFORE_ACTION);
//     std::string data("200");
//     sendInputGeneral(buyChipsInput, data, 5000);
// }

void DEBUGtoggleBuyChips(Areas areas) {
    ClickerArea buyChipsToggle = ClickerArea(15, 520, 180, 545);
    sendClickGeneral(buyChipsToggle, DEFAULT_WAIT_BEFORE_ACTION);
}
