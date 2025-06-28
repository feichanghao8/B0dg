#pragma once

#include <Windows.h>
#include <WinUser.h>

static HWND DEBUGGlobalWindodId = 0;

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

    int centerRandomX() {
        double center = startX + (width / 2);
        double pad = width * 0.25;
        return randomInRange(center - pad, center + pad);
    }

    int centerRandomY() {
        //return startY + (height/ 2);
        double center = startY + (height / 2);
        double pad = height * 0.25;
        return randomInRange(center - pad, center + pad);
    }
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



BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowProcessId;

    GetWindowThreadProcessId(hwnd, &windowProcessId);
    
    int found = 0;



    if (windowProcessId == lParam) {
        char windowTitle[1024];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        // TODO: This way of filtering is not robust. From what I have seen
        // they sometimes change titles. It could be in different language and
        // etc. It is good enough for current test.
        
        /*found  = isSubstring(windowTitle, "bodog.eu");
        found |= isSubstring(windowTitle, "Bodog.eu");*/

        //found |= isSubstring(windowTitle, "Chrome_WidgetWin_1");
        // found |= isSubstring(windowTitle, "2/4 No Limit Hold'em");

        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        bool hasMaximize = style & WS_MAXIMIZEBOX;
        bool isVisible = IsWindowVisible(hwnd);
        if (hasMaximize && isVisible) {
            // NOTE: Main window does not have a maximize property and all game windows
            // have it.
            found = true;
            printf("------> %s\n", windowTitle);
            DEBUGGlobalWindodId = hwnd;
        }

        // if(found) {
        //     DEBUGGlobalWindodId = hwnd;
        // }
    }

    if (found) {
        return false;
    }

    return TRUE; // Continue enumerating
}

HWND getWindowsForPid(int targetProcessId) {
    DEBUGGlobalWindodId = 0;
    EnumWindows(EnumWindowsProc, (LPARAM)targetProcessId);
    return DEBUGGlobalWindodId;
}

void sendClickEvent(HWND window, int mouseX, int mouseY) {
    PostMessage(window, WM_LBUTTONDOWN, 0, MAKELPARAM(mouseX, mouseY));
}

void sendLButtonUp(HWND hwnd, int x, int y) {
    LPARAM lParam = MAKELPARAM(x, y);
    PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, lParam);
}

void click(HWND window, int x, int y) {
    // TODO: We are sending just button up event but real mouse clicks 
    // send a little more info. See if it is important and add it
    // TODO: Also. Maybe check window and the 'raise' it just in case
    // they are checking for that.
    printf("Sending click to x:%d, y:%d\n", x, y);

    // NOTE: Sending just either one of this will not work
    sendClickEvent(window, x, y);
    sendLButtonUp(window, x, y);
}

void testDoLoop(HWND window) {
    //int startY = 254;
    //int endY = 614;
    //int yStepSize = 20;
    //int x = 388;

    //for (int y = startY; y <= endY; y += yStepSize) {
    //    printf("Sending click on x: %x, y: %x\n", x, y);
    //    // TODO: We are sending just button up event but real mouse clicks 
    //    // send a little more info. See if it is important and add it
    //    // TODO: Also. Maybe check window and the 'raise' it just in case
    //    // they are checking for that.
    //    click(window, x, y);
    //    Sleep(1000);
    //}


    ClickerArea buyChipsToggle = ClickerArea(15, 520, 180, 545);
    ClickerArea buyChipsInput = ClickerArea(31, 424, 164, 448);
    ClickerArea buyChips_do = ClickerArea(30, 470, 162, 512);
    ClickerArea raiseInput = ClickerArea(484, 491, 776, 517);
    ClickerArea raise = ClickerArea(644, 437, 776, 484);
    ClickerArea check = ClickerArea(498, 437, 630, 478);
    ClickerArea fold = ClickerArea(358, 437, 490, 477);
    ClickerArea call = ClickerArea(498, 437, 630, 477);

    
    //auto action = raise;
    //auto action = check;
    //auto action = fold;
    //auto action = call;
    auto action = buyChipsToggle;

    //ClickerArea sitout = ClickerArea(15, 518, 181, 546);

    int windowWidth = 976;
    int windowHeight = 599;

    if (window) {
        // Set window size to 800x600 and move it to position (100, 100)
        SetWindowPos(window, HWND_TOP, 100, 100, windowWidth, windowHeight, SWP_NOZORDER);
    }

    int x = action.centerRandomX();
    int y = action.centerRandomY();

    click(window, x, y);


}

bool isPointClickable(HWND hwnd, POINT point) {
    // Save the original cursor position
    POINT originalPos;
    GetCursorPos(&originalPos);

    // Convert the test point from client coordinates to screen coordinates
    POINT screenPoint = point;
    ClientToScreen(hwnd, &screenPoint);

    // Move the cursor to the test point
    int HEADER_SIZE = 0;
    SetCursorPos(screenPoint.x, screenPoint.y - HEADER_SIZE);



    // Allow time for the cursor to update (some systems may have a delay)
    Sleep(100);
    //Sleep(100);

    // Check the new cursor handle
    //HCURSOR newCursor = GetCursor();

    HCURSOR hHandCursor = LoadCursor(NULL, IDC_HAND);

    CURSORINFO Info = { 0 };
    Info.cbSize = sizeof(Info);
    GetCursorInfo(&Info);


    printf("cursor: %d\n", Info.hCursor);
    printf("cursor: %d\n", hHandCursor);

    // Move the cursor back to its original position
    SetCursorPos(originalPos.x, originalPos.y);

    // Compare the cursors to see if it changed
    if (hHandCursor == Info.hCursor) {
        return true; // The area is clickable or interactive
    }

    return false; // The area is not interactive
}


//void testPoint() {
//    isWindowCorrect();
//    POINT testPoint = {954, 554};
//    int result = isPointClickable(DEBUGGlobalWindodId, testPoint);
//    logfff("----- Point: %d %d, is clickable: %d", testPoint.x, testPoint.y, result);
//}

void debugStartInputLoop(int pid) {
	// EnumWindows(enumWindowCallback, NULL);
    
    HWND window = getWindowsForPid(pid);
    printf("Found window: %x\n", window);
    if (IsWindow(window)) {
        SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
        //testDoLoop(window);
        
        //POINT testPoint = { 954, 554 };
        POINT testPoint = { 833, 227 };
        int result = isPointClickable(DEBUGGlobalWindodId, testPoint);
        printf("----- Point: %d %d, is clickable: %d", testPoint.x, testPoint.y, result);


        //testDoLoop((HWND)0x00160EB2);
    }
    else {
        printf(":: Error. Could get correct window.");
    }

    //for (;;) {
    //    /*HCURSOR newCursor = GetCursor();
    //    printf("%d\n", newCursor);*/


    //    HCURSOR hHandCursor = LoadCursor(NULL, IDC_HAND);

    //    CURSORINFO Info = { 0 };
    //    Info.cbSize = sizeof(Info);
    //    GetCursorInfo(&Info);

    //    printf("%d %d\n", Info.hCursor, hHandCursor);


    //    Sleep(1000);
    //}
}

