#pragma once

#include <Windows.h>
#include <WinUser.h>
#include <thread>

#include <tlhelp32.h>


const int HEADER_OFFSET = 30;
const int HORIZONTAL_OFFSET = 115;
const int DownUpClickDelay = 60;
const int DEFAULT_WAIT_BEFORE_ACTION = 3000; // seconds
const int DEFAULT_SECOND_TEST_ACTION = DEFAULT_WAIT_BEFORE_ACTION + DownUpClickDelay + 100; // seconds

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

    // ClickerArea(int x1, int y1, int x2, int y2) :
    //     startX(x1),
    //     startY(y1),
    //     width(x2 - x1),
    //     height(y2 - y1) {
    // };

    ClickerArea(int x1, int y1, int x2, int y2, std::string name, int buttonId) :
        startX(x1),
        startY(y1),
        width(x2 - x1),
        height(y2 - y1), 
        sendActionName(name),
        buttonId(buttonId)
    {};

    int startX;
    int startY;
    int width;
    int height;

    std::string sendActionName;
    int buttonId;

    ClickerArea getAdjustedX(int size) {
        int newStartX = startX - size;
        return ClickerArea(newStartX, startY, newStartX + width, startY + height, sendActionName, buttonId);
    }

    int centerRandomX() {
        return startX + (width / 2);
        // double center = startX + (width / 2);
        // double pad = width * 0.25;
        // return randomInRange(center - pad, center + pad);
    }

    int centerRandomY() {
        return startY + (height/ 2);
        // double center = startY + (height / 2);
        // double pad = height * 0.25;
        // return randomInRange(center - pad, center + pad);
    }
};


class Areas {
public:
    // bool isSidebarPresent = true;
    bool isSidebarPresent = false;

    // ClickerArea waitBigBlindSelect;
    // ClickerArea post4JoinNextSelect;
    // ClickerArea buyChipsToggle;
    // ClickerArea buyChipsInput;
    // ClickerArea buyChips_do;
    // ClickerArea raiseInput;
    // ClickerArea betInput;
    // ClickerArea allInSmallBottom; // NOTE: pretty much the same thing as 'raise'

    ClickerArea rraise;
    ClickerArea check;
    ClickerArea fold;
    ClickerArea call;
    ClickerArea bet; // NOTE: pretty much the same thing as 'raise'
                     //
    ClickerArea allInBigTop; // NOTE: pretty much the same thing as 'raise'
    ClickerArea allInRaiseBigTop; // NOTE: pretty much the same thing as 'raise'

    ClickerArea muckCards;

    Areas()
    { 
        // waitBigBlindSelect = ClickerArea(382, 507, 566, 547);
        // post4JoinNextSelect = ClickerArea(574, 507, 758, 547);
        // buyChipsToggle = ClickerArea(15, 520, 180, 545);
        // buyChipsInput = ClickerArea(31, 424, 164, 448);
        // buyChips_do = ClickerArea(30, 470, 162, 512);
        // raiseInput = ClickerArea(683, 491, 776, 517);
        // betInput = ClickerArea(484, 491, 776, 517);
        // allInSmallBottom = ClickerArea(677, 523, 775, 546);

        rraise = ClickerArea(644, 437, 776, 484, "poker-frontend-game-actions-raise-latency", 512); // btn: 512
        check = ClickerArea(498, 437, 630, 478, "poker-frontend-game-actions-check-latency", 64); // btn: 64
        fold = ClickerArea(358, 437, 490, 477, "poker-frontend-game-actions-fold-latency", 1024); // btn: 1024
        call = ClickerArea(498, 437, 630, 477, "poker-frontend-game-actions-call-latency", 256); // btn: 256
        bet = ClickerArea(644, 437, 776, 484, "poker-frontend-game-actions-bet-latency", 128); // btn: 128

        allInBigTop = ClickerArea(644, 440, 775, 481, "poker-frontend-game-actions-allin-latency", 2048); // btn: 2048
        allInRaiseBigTop = ClickerArea(644, 440, 775, 481, "poker-frontend-game-actions-allin-latency", 4096); // btn: 4096
        muckCards = ClickerArea(573, 507, 757, 546, "-", 0); // TODO: We don't need it now but still find 'muck cars' button id
    }

    ClickerArea adjust(ClickerArea area) {
        /// TODO: Add adjusting of click x,y pixel values?
        if(isSidebarPresent) {
            // return area.getAdjustedX(HORIZONTAL_OFFSET);
            return area;
        } else {
            // return area.getAdjustedX(0);
            return area;
        }
    }
};


bool checkIfHasSidebar() {
    bool isClickable = true;
    // TODO: Implement
    return isClickable; 
}

void initWindow() {
    // TODO: Placeholder. This is copy of another input. 
}

void websocketSend(void* streamPtr, std::string dataToSend) {
    logfff(streamPtr, "---: Send [%x]:---: %s\n", streamPtr, dataToSend.c_str());

    // std::string jsonstring = data.dump();
    //IOBuffer b = stringToIOBuffer(dataToSend);
    scoped_refptr<IOBuffer> b = stringToIOBuffer(dataToSend);

    if(streamPtr) {
        SendFrame(streamPtr, 1, 1, b, dataToSend.size());
    }
    logfff(streamPtr, "--- Sent\n");
}

void sendPing(void* streamPtr) {
    nlohmann::json tempJson;
    tempJson["pid"] = "PING";
    tempJson["uuid"] = generateUUID();
    std::string pingStr = tempJson.dump();
    websocketSend(streamPtr, pingStr);
}

void sendClickGeneral(void* streamPtr, ClickerArea action, int tableState, int buttonId, int betAmount, int delay) {
    // data["message"] = json::object({
    //     {"pid", "CO_SELECT_RES_V2"},
    //     {"type", 1},
    //     {"state", 8},
    //     {"btn", 1024},
    //     {"bet", 0},
    //     {"x", 317},
    //     {"y", 445},
    //     {"time", 30},
    //     {"cardPos", 0},
    // });

    //nlohmann::json tempJson;
    //tempJson["pid"] = "PING";
    //tempJson["uuid"] = generateUUID();
    //std::string pingStr = tempJson.dump();
    //websocketSend(streamPtr, pingStr);
    // --------------------------------------------------------------------------------


    int x = ((ClickerArea)action).centerRandomX();
    int y = ((ClickerArea)action).centerRandomY();

    nlohmann::json data;
    data["transactionName"] = action.sendActionName;
    data["transparentId"] = getRandomTransparentId();
    data["sessionUuid"] = generateUUID();

    data["message"] = nlohmann::json::object({
        {"pid", "CO_SELECT_RES_V2"},
        {"type", 1},
        {"state", tableState},
        {"btn", action.buttonId},
        {"bet", betAmount},
        {"x", x},
        {"y", y},
        {"time", 30},
        {"cardPos", 0},
    });


    std::string dataStr = data.dump();
    websocketSend(streamPtr, dataStr);
}


void sendSitin(void* streamPtr, int availableMoney, int seatNumber) {
    // {
    //   "data": {
    //     "account": 198,
    //     "nickName": "550869473257200",
    //     "pid": "PLAY_SEAT_INFO",
    //     "seat": 1,
    //     "state": 32,
    //     "type": 1
    //   },
    //   "seq": 554,
    //   "tDiff": 0
    // }

    // {
    //   "transparentId": "00-ff8aa1aa7dcf7593c31cb5ccb0d2fcb2-9f907b73828629bc-00",
    //   "transactionName": "poker-frontend-game-actions-sit-back-latency",
    //   "message": {
    //     "availableMoney": 269,
    //     "pid": "PLAY_SEAT_REQ",
    //     "seat": 3,
    //     "state": 32,
    //     "type": 0
    //   }
    // }


    nlohmann::json data;
    data["transactionName"] = "poker-frontend-game-actions-sit-back-latency";
    data["transparentId"] = getRandomTransparentId();
    data["message"] = nlohmann::json::object({
        {"pid", "PLAY_SEAT_REQ"},
        {"seat", seatNumber},
        {"availableMoney", availableMoney},
        {"state", 32}, // TODO: Maybe we should randomize how we send state/type? Sometimes send 16/1, sometimes 32/0
        {"type", 0},
    });
    std::string pingStr = data.dump();
    websocketSend(streamPtr, pingStr);
}


void sendMuckCards(void* streamPtr) {

    nlohmann::json tempJson;
    tempJson["pid"] = "PING";
    tempJson["uuid"] = generateUUID();
    std::string pingStr = tempJson.dump();
    websocketSend(streamPtr, pingStr);
    // --------------------------------------------------------------------------------



    nlohmann::json data;
    data["pid"] = "CO_SHOW_RES";
    data["type"] = 0;
    data["state"] = 32768; // TODO: Remove nameless number. Find and set proper name
    data["btn"] = 3276; // TODO: Remove nameless number. Find and set proper name

    // std::thread t([delay, data, streamPtr]() {
    //     if (delay > 0) { Sleep(delay); }
    //
    logfff(streamPtr, "--- muck\n");
        std::string dataStr = data.dump();
        websocketSend(streamPtr, dataStr);
    // });
    // t.detach();
}


void sendInputGeneral(ClickerArea action, std::string data, int delay) {
    int x = action.centerRandomX();
    int y = action.centerRandomY();
        
    // std::thread t([x, y, delay, data]() {
    //     if (delay) {
    //         Sleep(delay);
    //     }
    //     sendTextInput(DEBUGGlobalWindowId, x, y, data);
    // });
    // t.detach();
}


// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


void DEBUGDoFold(void* streamPtr, Areas areas, int tableState) {
    ClickerArea fold = areas.adjust(areas.fold);
    int amount = 0;
    sendClickGeneral(streamPtr, fold, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);
}

void DEBUGDoFoldOrCheck(void* streamPtr, Areas areas, int tableState) {
    // ClickerArea check = areas.adjust(areas.check);
    // ClickerArea fold = areas.adjust(areas.fold);
    //
    // nlohmann::json data;
    //
    // // TODO: Check this
    // sendClickGeneral(fold, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);
    // sendClickGeneral(check, tableState, -1, amount, DEFAULT_SECOND_TEST_ACTION);

    // TODO: Implement.
    logfff(streamPtr, "------------ Fold or check is not implemented! \n");
}

void DEBUGDoCheck(void* streamPtr, Areas areas, int tableState) {
    ClickerArea check = areas.adjust(areas.check);
    int amount = 0;
    sendClickGeneral(streamPtr, check, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);
}

void DEBUGDoCall(void* streamPtr, Areas areas, int tableState, int amount) {
    ClickerArea call = areas.adjust(areas.call);
    sendClickGeneral(streamPtr, call, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);

    // NOTE: Temporary hack for cases when a call was requested but all 
    // wee can do is allin and there only button to allin.
    // Better approach would be to send all in directly by detecting that
    // case but this is good enough for now. This button could be an indicator:
    // CO_SELECT_REQ:... "btns":34606080 ...
    //
    // ClickerArea allInBigTop = areas.adjust(areas.allInBigTop);
    // sendClickGeneral(allInBigTop, tableState, -1, amount, DEFAULT_SECOND_TEST_ACTION);
}


void DEBUGDoRaise(void* streamPtr, Areas areas, int tableState, int amount) {
    ClickerArea rraise = areas.adjust(areas.rraise);

    sendClickGeneral(streamPtr, rraise, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION + 1000);
}

void DEBUGDoBet(void* streamPtr, Areas areas, int tableState, int amount) {
    std::string data = doubleToString(amount);

    // NOTE: Bet input and raise input are pretty much the same thing
    // but may change in the future so thus different areas are used

    ClickerArea bet = areas.adjust(areas.bet);

    sendClickGeneral(streamPtr, bet, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);
}

void DEBUGDoAllIn(void* streamPtr, Areas areas, int tableState, int amount) {
    ClickerArea allInBigTop = areas.adjust(areas.allInBigTop);
    sendClickGeneral(streamPtr, allInBigTop, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION + 1000);
}

void DEBUGDoAllInRaise(void* streamPtr, Areas areas, int tableState, int amount) {
    ClickerArea allInRaiseBigTop = areas.adjust(areas.allInRaiseBigTop);
    sendClickGeneral(streamPtr, allInRaiseBigTop, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION + 1000);
}


void DEBUGDoSendPing(void* streamPtr) {
    sendPing(streamPtr);
}

void DEBUGDoSitin(void* streamPtr, int availableMoney, int seatNumber) {
    sendSitin(streamPtr, availableMoney, seatNumber);
}


void DEBUGDoMuckCards(void* streamPtr, Areas areas, int tableState) {
    sendMuckCards(streamPtr);
}


// void DEBUGtoggleBuyChips(void* streamPtr, Areas areas, int tableState) {
//     ClickerArea buyChipsToggle = ClickerArea(15, 520, 180, 545);
//     sendClickGeneral(streamPtr, buyChipsToggle, tableState, -1, amount, DEFAULT_WAIT_BEFORE_ACTION);
// }
