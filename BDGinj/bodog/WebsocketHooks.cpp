#include "nlohmann/json.hpp"
#include "minhook/MinHook.h"

#include <string.h>
#include <memory.h>

#include "general.h"
#include "external_data.h"

// TODO: Once we settle on format we can abstract away function into a class and load class based on some 
// swith if needed. If not just remove regular window input as it may be not needed.
// #include "Input.h" 
#include "InputWebsocketWrite.h" 

#include "Game.h"
#include "IniParser.h"

// Game globalGame = Game();
// Game* globalGame = 0;

// struct abc {
//     Game* globalGame = 0;
//     int window = 0;
// };

int(__fastcall* detourWriteWebocketOrig)(void* thisptr, void* ignore, bool a, WebSocketFrameHeader::OpCode op_code, scoped_refptr<IOBuffer> buffer, size_t size);
int gRebuyAtBB {};

// TODO: Rename all 'game' to 'table'?
class InjectionState {
public:
    IniParser config;
    std::map<void*, Game*> games;

    InjectionState(IniParser config)
        : config(config)
    {};

    bool isGamePresent(void* key) {
        if(games.find(key) != games.end()) {
            return true;
        }
        return false;
    }

    Game* initializeGame() {
        char* ip = _strdup((char*)config.get("Server", "ServerIp", "").c_str()); // TODO: Clean memory
        int port = atoi(config.get("Server", "ServerPort", "").c_str());

        logfff(1, "Initializing game to a client(%s, %d)\n", ip , port);

        auto client = Client(ip, port);
        return new Game(client);
    }

    void removeGame(void* streamPtr) {
        auto it = games.find(streamPtr);
        if (it != games.end()) {
            Game* game = getGame(streamPtr);

            logfff(streamPtr, "[[Remove]] %x\n", streamPtr);
            games.erase(it);

            delete game; // TODO: Make
        } else {
            logfff(streamPtr, "Error: were asked to remove game %x but it is not present\n", streamPtr);
        }
    }

    Game* addGame(void* streamPtr) {
        if(isGamePresent(streamPtr)) {
            logfff(streamPtr, "ERROR: We are trying to add GAME but we already have it in map. Probably not removed at the end of game or error occured");
            removeGame(streamPtr);
        } 

        Game* game = initializeGame();
        games.insert({streamPtr, game});

        logfff(streamPtr, "[[Add]] %x %x\n", streamPtr, game);

        return game;
    }

    Game* getGame(void* key) {
        Game* result = 0;
        if(isGamePresent(key)) {
            result = games[key];
        }
        return result;
    }

    Game* getOrAddGame(void* key) {
        if(isGamePresent(key)) {
            return getGame(key);
        } else {
            return addGame(key);
        }
    }
};


InjectionState* globalInjectionState;



std::vector<std::string> findAllKeysWithPrefix(const nlohmann::json& jsonObject, std::string prefix) {
    std::vector<std::string> result = {};

    if (jsonObject.is_object()) {
        for (auto &it : jsonObject.items()) {
            std::string key = it.key();
            if(key.starts_with(prefix)) {
                result.push_back(key);
            }
        }
    }

    return result;
}

// std::string getActionName(Game* game, int seat, int bet, int raise, int button) {
//     int PROBABLY_FOLD_BUTTON = 1024;
//     int PROBABLY_BET_BUTTON1 = 2048;
//     int PROBABLY_BET_BUTTON2 = 128; // Pre check?
//
//     bool isAllin = game->isRequestAllin(seat, bet + raise);
//
//     std::string betType;
//     if(raise > 0) { 
//         if(isAllin) {
//             betType = "allins";
//         } else {
//             betType = "raises";
//         }
//     } 
//     else if (raise == 0 && bet > 0)  {
//         int bet = 0;
//         bet |= button == PROBABLY_BET_BUTTON1;
//         bet |= button == PROBABLY_BET_BUTTON2;
//         if(bet) {
//             betType = "bets"; 
//         } else {
//             betType = "calls"; 
//         }
//     } 
//     else if (raise == 0 && bet == 0)  {
//         if(button == PROBABLY_FOLD_BUTTON) {
//             betType = "folds"; 
//         } else {
//             betType = "checks"; 
//         }
//     }
//     else {
//         betType = "UNKNOWN_BET";
//     }
//     logfff("action selection: bet: %d, raise: %d, button: %d, result: %s\n", bet, raise, button, betType.c_str());
//
//     return betType;
// }
// std::string getActionName(int seat, int bet, int raise, int button) {
//     int BUTTON_ALL_IN = 2048;
//     int BUTTON_ALL_IN_RAISE = 4096;
//     int BUTTON_BET = 128;
//     int BUTTON_CALL = 256;
//     int BUTTON_CHECK = 64;
//     int BUTTON_FOLD = 1024;
//     int BUTTON_RAISE = 512;
//     int BUTTON_FOLD_AND_SHOW = 1048576;
//
//     std::string betType;
//
//     if(button == BUTTON_ALL_IN)            { betType = "allins"; }
//     else if(button == BUTTON_ALL_IN_RAISE) { betType = "allins"; }
//     else if(button == BUTTON_BET)          { betType = "bets"; }
//     else if(button == BUTTON_CALL)         { betType = "calls"; }
//     else if(button == BUTTON_CHECK)        { betType = "checks"; }
//     else if(button == BUTTON_FOLD)         { betType = "folds"; }
//     else if(button == BUTTON_FOLD_AND_SHOW){ betType = "folds"; }
//     else if(button == BUTTON_RAISE)        { betType = "raises"; }
//     else {
//         logfff(game->getStream(), "Not known action button: %d\n", button);
//         betType = "[UNKNOWN]";
//     }
//
//     logfff(game->getStream(), "action selection: bet: %d, raise: %d, button: %d, result: %s\n", bet, raise, button, betType.c_str());
//
//     return betType;
// }

nlohmann::json probeNumber(char* rawData, int dataLength) {
    //int dataLength = strlen(rawData); // TODO: strlen is not the best approach. Have some size limits
    int maxLookInto = 10;

    int index = -1;
    while (index < dataLength && index < maxLookInto) {
        index++;
        char c = rawData[index];
        bool isNumber = c >= '0' && c <= '9';
        if (!isNumber) {
            break;
        }
    }

    if (index != -1) {
        char token = rawData[index];
        if (token == '|') {
            int messageSize = atoi(rawData);
            char* Data = rawData + index + 1;
            if(*Data == '{') {
                std::string tString = std::string(Data, messageSize);
                try {
                    const auto jsonResult = nlohmann::json::parse(tString);
                    return jsonResult;
                }
                catch (...) {
                    // TODO: Error processing
                    //logfff("Error parsing json. Not a json: %.*s\n", dataLength, rawData);
                }
            } else {
                logfff(3, "Skipping json parsing as it is not json: %s\n", Data);
            }
        }
    }

    return nlohmann::json();
}

bool jsonKeyExists(const nlohmann::json& j, const std::string& key) {
    return j.find(key) != j.end();
}

void processGameSequence(void* thisptr, nlohmann::json json, Game* game)
{
    if (game == 0) {
        logfff(thisptr, "Game is not initialized");
        return;
    }

    // TODO: This should be set once and separately for each stream. Also check for stream finish/reset
    game->setStream(thisptr);

    int sequenceNumber = json["seq"].get<int>();

    int tDiff = json["tDiff"].get<int>();  // TODO: Verify existence
    nlohmann::json data = json["data"];

    if(sequenceNumber == 10) {
        initWindow();
    }

    if (!data.contains("pid")) {
        // { "data": { "gid": "Joined" }, "seq": 0, "tDiff": 0 }
        // NOTE: We don't care about this message.
        return;
    }
    std::string pid = data["pid"].get<std::string>();

    logfff(thisptr, "%22s: %s\n", pid.c_str(), json.dump().c_str());

    // game->autoActionHack();

    game->checkAndSendDelayedInput();

    if (pid == "PONG") {
        // {"pid":"PONG","uuid":"6812d4e7-b633-4fd0-9df6-a843da77d002"}
        // {"pid":"PONG","uuid":"455cf87a-8ebe-43cc-8b4c-b832e6a5f620"}
        //DEBUGtoggleBuyChips();
        //DEBUGTestBuyChipsInput();
    }
    else if (pid == "CO_ALLIN_SEAT_INFO") {
        //{ "allinSeats": [1, 0, 1, 0, 0, 0, 0, 0, 0] , "pid" : "CO_ALLIN_SEAT_INFO" }
        //{ "allinSeats": [1, 0, 0, 1, 0, 0, 0, 0, 0] , "pid" : "CO_ALLIN_SEAT_INFO" }
        //{ "allinSeats": [1, 0, 1, 1, 0, 0, 0, 0, 0] , "pid" : "CO_ALLIN_SEAT_INFO" }
        //{ "allinSeats": [0, 1, 0, 1, 0, 0, 0, 0, 1] , "pid" : "CO_ALLIN_SEAT_INFO" }
    }
    else if (pid == "CO_BCARD1_INFO") {
        //{ "card":9, "pid" : "CO_BCARD1_INFO", "pos" : 4 },
        //{ "card":32,"pid" : "CO_BCARD1_INFO","pos" : 5 }
        //{ "card":49, "pid" : "CO_BCARD1_INFO", "pos" : 4 }
        int id = data["card"].get<int>();
        std::vector<int> ids;
        ids.push_back(id);
        game->sendDealCommunityCards(ids);

        // // Card is dealt so it means 'round' has ended
        // game->clearMaxRaise();
    }
    else if (pid == "CO_BCARD3_INFO") {
        //{ "bcard": [35, 19, 16] , "pid" : "CO_BCARD3_INFO" }
        //{ "bcard": [26, 15, 33] , "pid" : "CO_BCARD3_INFO" }
        //{ "bcard": [45, 20, 43] , "pid" : "CO_BCARD3_INFO" }
        //{ "bcard": [18, 36, 4] , "pid" : "CO_BCARD3_INFO" }
        //{ "bcard": [21, 25, 5] , "pid" : "CO_BCARD3_INFO" }

        std::vector<int> cardIds = data["bcard"].get<std::vector<int>>();

        game->sendDealCommunityCards(cardIds);

        // // Card is dealt so it means 'round' has ended.
        // game->clearMaxRaise();
    }
    else if (pid == "CO_BLIND_INFO") {
        //{ "account":5800, "baseStakes" : 0, "bet" : 200, "btn" : 2, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 7 }
        //{ "account":84769, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 8 }
        //{ "account":11600, "baseStakes" : 0, "bet" : 400, "btn" : 8, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 2 }
        //{ "account":84569, "baseStakes" : 0, "bet" : 200, "btn" : 2, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 8 }
        //{ "account":79600, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 9 }
        //{ "account":79400, "baseStakes" : 0, "bet" : 200, "btn" : 2, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 9 }
        //{ "account":39600, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 1 }
        //{ "account":120400, "baseStakes" : 0, "bet" : 200, "btn" : 2, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 1 }
        //{ "account":10000, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 2 }
        //{ "account":201795, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 4 }
        //{ "account":201995, "baseStakes" : 0, "bet" : 200, "btn" : 2, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 4 }
        //{ "account":5400, "baseStakes" : 0, "bet" : 400, "btn" : 4, "dead" : 0, "pid" : "CO_BLIND_INFO", "seat" : 7 }

        int account = data["account"].get<int>();
        double bet = data["bet"].get<double>();
        int seatNumber = data["seat"].get<int>();
        int dead = data["dead"].get<int>();
        int button = data["btn"].get<int>();

        // TODO: Process 'dead'. They added value to the pot but "won't play"?
        //
        // int playerId = seat;
        // int raise = 0;
        // int amount = bet + raise;
        // int button = 0;
        // std::string betType = getActionName(bet, raise, button);
        // game->sendPlayerAction(playerId, betType, amount);

        int BUTTON_SMALL_BLIND = 2;
        int BUTTON_BIG_BLIND = 4;

        int BUTTON_POST_BIG_BLIND = 8;
        int BUTTON_POST_BIG_BLIND2 = 16;
        int BUTTON_POST_BIG_BLIND3 = 131072;

        if (button == BUTTON_SMALL_BLIND) {
            game->setSmallBlindPlayerNumber(seatNumber);
        }
        else if (button == BUTTON_BIG_BLIND) {
            game->setBigBlindPlayerNumber(seatNumber);
        }
        else {
            // TODO: Wrong/Unknown button?
            //
            // NOTE: Since there are quite a few number of buttons that do 'post blind'
            // let's assume that everyhing that is not blind and big blind to be a 'post blind'
            game->addPostBigBlindNumber(seatNumber);
        }

        auto &player = game->getSeats()[seatNumber - 1];

        player.setRoundBet((int)bet);
        logfff(thisptr, "Player %d (%d) setting round bet to %d\n",
            seatNumber, player.getPid(), (int)bet);

        if(dead) {
            // TODO: Remove player? Set SetState::Sitout
        }

        // NOTE: Setting player as playing as it is clearly playing a blind.
        // TODO: Should we check if player is NOT 'dead' or should we check that it
        // is a BUTTON_POST_BIG_BLIND?
        bool isNormal = true;
        game->setSeatState(seatNumber, SeatState::Playing, isNormal);
    }
    else if (pid == "CO_BLIND_REQ") {
        //{ "bet":400, "btns" : 196616, "dead" : 0, "pid" : "CO_BLIND_REQ" }
        //{ "bet":400, "btns" : 65540, "dead" : 0, "pid" : "CO_BLIND_REQ" }

        int betAmount = data["bet"].get<int>();
        int btns = data["btns"].get<int>(); // TODO: Process button types?


        // DEBUGDoCall(thisptr, game->getAreas(), game->getTableState(), betAmount); // TODO: Do we need to do this? It looks like a straddle. @Research
    }
    else if (pid == "CO_CARDTABLE_INFO") {
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat2" : [10, 31] , "seat3" : [32896, 32896] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat2" : [51, 27] , "seat3" : [32896, 32896] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat2" : [29, 3] , "seat3" : [32896, 32896] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat2" : [48, 51] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }
        //{ "pid":"CO_CARDTABLE_INFO", "seat1" : [32896, 32896] , "seat4" : [32896, 32896] , "seat7" : [32896, 32896] , "seat8" : [32896, 32896] , "seat9" : [32896, 32896] }

        try {
            int aiSeatNumber = -1;

            int index = 1;
            std::vector<Seat> testActiveSeats = {};
            for (Seat& seat : game->getSeats()) {
                int seatId = index;
                std::string key = "seat" + std::to_string(seatId);
                logfff(thisptr, "::: %s %d round bet %d\n", key.c_str(), seatId, seat.getRoundBet());

                if(data.contains(key)) {
                    std::vector cardIndexes = data[key].get<std::vector<int>>();

                    seat.setState(SeatState::Playing);

                    testActiveSeats.push_back(seat);

                    int CLOSED_CARD = 32896; // TODO: Dublicate code. Move to common place.
                    if(cardIndexes.size() > 0 && cardIndexes[0] != CLOSED_CARD) {
                        seat.setCards(cardIndexes);
                        aiSeatNumber = index;
                        logfff(thisptr, "\t\t%s %d\n", vectorToString(cardIndexes).c_str(), aiSeatNumber);
                    }
                }
                index++;
            }

            game->setAiPlayerId(aiSeatNumber);
            if(aiSeatNumber == -1) {
                // TODO: Did we disconnect?
                logfff(thisptr, "Cards were dealt but we are not playing on this table\n");
            }

            game->setActivePlayers(testActiveSeats);

            // NOTE: Previously we sent this after all blinds but it is hard to manage server state and 
            // on top of that it looks like we don't always get all required information. So this is current
            // new test solution to always know current active players. We do this by getting all card information
            // and cards will be provided only to the players that are playing. So we probably can safely assume
            // that all players here are active and will be playing.
            // TODO: Test more if this theory actuallly works 100% of the time.
            game->sendTableGameInit();

            if (game->getAiPlayerId() > 0) {
                // TODO: check
                std::string key = "seat" + std::to_string(game->getAiPlayerId());
                if(data.contains(key)) {
                    std::vector cardIndexes = data[key].get<std::vector<int>>();
                    game->sendStartingHand(cardIndexes);
                }
            }
        }
        catch (const std::exception& e) {
            logfff(thisptr, "Error:... %s\n", e.what());
        }

    }
    else if (pid == "CO_CHIPTABLE_INFO") {
        //{ "curPot": [11770] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 0, "seat" : 0 }
        //{ "curPot": [87241] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 1, "seat" : 0 }
        //{ "curPot": [87241] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 1, "returnBet" : 9169, "seat" : 1 }
        //{ "curPot": [87241] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 1, "seat" : 0 }
        //{ "curPot": [26538] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 1, "returnBet" : 273354, "seat" : 4 }
        //{ "curPot": [26538] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 1, "seat" : 0 }
        //{ "curPot": [1400] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 0, "returnBet" : 272954, "seat" : 4 }
        //{ "curPot": [120600, 94482] , "curRake" : [0, 0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 2, "returnBet" : 187113, "seat" : 4 }
        //{ "curPot": [120600, 94482] , "curRake" : [0, 0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 2, "seat" : 0 }
        //{ "curPot": [31400, 138000] , "curRake" : [0, 0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 2, "returnBet" : 202195, "seat" : 4 }
        //{ "curPot": [31400, 138000] , "curRake" : [0, 0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 2, "seat" : 0 }
        //{ "pid":"CO_CHIPTABLE_INFO", "potCount" : 0, "returnBet" : 400, "seat" : 4 }
        //{ "curPot": [1200] , "curRake" : [0] , "pid" : "CO_CHIPTABLE_INFO", "potCount" : 0, "seat" : 0 }

    }
    else if (pid == "CO_CURRENT_PLAYER") {
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 7 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 8 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 1 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 3 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 4 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 5 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 8 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 1 }
        //{ "pid":"CO_CURRENT_PLAYER", "seat" : 3 }

        int seat = data["seat"].get<int>();
        logfff(thisptr, "Current player: %d\n", seat);
    }
    else if (pid == "CO_DEALER_SEAT") {
        // {"pid":"CO_DEALER_SEAT","seat":4}
        // {"pid":"CO_DEALER_SEAT","seat":5}
        int dealerSeatId = data["seat"].get<int>();
        game->setDealerSeat(dealerSeatId);

    }
    else if (pid == "CO_PCARD_INFO") {
        // {"card":[27,29],"pid":"CO_PCARD_INFO","seat":1,"type":0}
        // {"card":[51,38],"pid":"CO_PCARD_INFO","seat":3,"type":0}
        // {"card":[27,29],"pid":"CO_PCARD_INFO","seat":1,"type":0}
        // {"card":[51,38],"pid":"CO_PCARD_INFO","seat":3,"type":0}
        // {"card":[29,0],"pid":"CO_PCARD_INFO","seat":1,"type":0}
        // {"card":[22,8],"pid":"CO_PCARD_INFO","seat":4,"type":0}
        // {"card":[29,0],"pid":"CO_PCARD_INFO","seat":1,"type":0}
        // {"card":[22,8],"pid":"CO_PCARD_INFO","seat":4,"type":0}

        int seatId = data["seat"].get<int>();
        std::vector<int> cards = data["card"].get<std::vector<int>>();
        game->setCardsForSeat(seatId, cards);
    }
    else if (pid == "CO_POT_INFO") {
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[0,0,87241,0,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[26538,0,0,0,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[0,0,0,1400,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":1,"returnHi":[0,0,0,94482,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[120600,0,0,0,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":1,"returnHi":[0,0,0,0,0,0,0,0,138000],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[0,0,0,0,0,0,0,0,31400],"returnLo":[0,0,0,0,0,0,0,0,0]}
        // {"kicker":0,"kickerCard":[0,0,0,0,0,0,0,0,0],"pid":"CO_POT_INFO","potNo":0,"returnHi":[0,0,0,0,0,0,0,0,0],"returnLo":[0,0,0,0,0,0,0,0,0]}

    }
    else if (pid == "CO_PRESELECT_BLINDS") {
        // {"amountToPost":400,"pid":"CO_PRESELECT_BLINDS"}
        // {"amountToPost":400,"pid":"CO_PRESELECT_BLINDS"}
        // {"amountToPost":400,"pid":"CO_PRESELECT_BLINDS"}
        // {"amountToPost":400,"pid":"CO_PRESELECT_BLINDS"}
        // {"amountToPost":0,"pid":"CO_PRESELECT_BLINDS"}
    }
    else if (pid == "CO_RABBITCARD_INFO") {
        // NOTE: This is showing a rabbit at the end. Not important.
    }
    else if (pid == "CO_RESULT_INFO") {
        // {"account":[9169,12000,87241,285923,0,40000,6000,85169,80000],"handHi1":[2,5,6,3,1],"handHi3":[0,1,2,5,3],"pid":"CO_RESULT_INFO"}
        // {"account":[26538,11200,87241,273354,0,0,5800,84769,80000],"handHi1":[2,1,6,5,4],"handHi4":[2,6,5,0,1],"pid":"CO_RESULT_INFO"}
        // {"account":[40000,10800,87241,274354,0,54219,5800,84569,79600],"pid":"CO_RESULT_INFO"}
        // {"account":[120600,10400,0,281595,0,54219,5800,84569,79400],"handHi1":[2,1,4,0,6],"handHi3":[6,1,5,0,3],"handHi4":[1,0,6,5,3],"pid":"CO_RESULT_INFO"}
        // {"account":[120400,0,0,202195,0,0,5800,84569,169400],"handHi2":[5,6,1,3,0],"handHi4":[5,6,3,1,0],"handHi9":[0,1,2,5,6],"pid":"CO_RESULT_INFO"}
        // {"account":[120400,0,0,202195,0,0,5800,84569,169400],"pid":"CO_RESULT_INFO"}

        std::vector account = data["account"].get<std::vector<int>>();
        game->sendFinalResults(account);

        game->updateSeatTotals(account);
        game->checkUpdateBalance();
    }
    else if (pid == "CO_SELECT_INFO") {
        // {"account":81526,"bet":1137,"btn":512,"pid":"CO_SELECT_INFO","raise":2074,"seat":5}
        // {"account":6000,"bet":0,"btn":1024,"pid":"CO_SELECT_INFO","raise":0,"seat":7}
        // {"account":110326,"bet":1874,"btn":256,"pid":"CO_SELECT_INFO","raise":0,"seat":8}
        // {"account":34326,"bet":1874,"btn":256,"pid":"CO_SELECT_INFO","raise":0,"seat":1}
        // {"account":25157,"bet":937,"btn":256,"pid":"CO_SELECT_INFO","raise":0,"seat":3}
        // {"account":285923,"bet":937,"btn":256,"pid":"CO_SELECT_INFO","raise":0,"seat":4}
        // {"account":81526,"bet":0,"btn":64,"pid":"CO_SELECT_INFO","raise":0,"seat":5}
        // {"account":104441,"bet":5885,"btn":128,"pid":"CO_SELECT_INFO","raise":0,"seat":8}

        int seat = data["seat"].get<int>();
        int bet = data["bet"].get<int>();
        int raise = data["raise"].get<int>();
        int button = data["btn"].get<int>();

        std::string betType = game->getActionName(seat, bet, raise, button);
        game->sendPlayerAction(seat, betType, bet, raise);
    }
    else if (pid == "CO_SELECT_REQ") {
        // {"bet":400,"betPot":3000,"btns":6293248,"halfPot":1700,"maxRaise":12000,"pid":"CO_SELECT_REQ","raise":1200,"timeBank":30}
        // {"bet":11200,"betPot":0,"btns":3072,"halfPot":0,"maxRaise":0,"pid":"CO_SELECT_REQ","raise":0,"timeBank":30}
        // {"bet":400,"betPot":1400,"btns":2098944,"halfPot":0,"maxRaise":11200,"pid":"CO_SELECT_REQ","raise":800,"timeBank":30}
        // {"bet":10800,"betPot":0,"btns":34606080,"halfPot":0,"maxRaise":0,"pid":"CO_SELECT_REQ","raise":0,"timeBank":30}
        // {"bet":400,"betPot":1400,"btns":2098944,"halfPot":0,"maxRaise":10800,"pid":"CO_SELECT_REQ","raise":800,"timeBank":30}
        // {"bet":10400,"betPot":0,"btns":3072,"halfPot":0,"maxRaise":0,"pid":"CO_SELECT_REQ","raise":0,"timeBank":30}
        // {"bet":10000,"betPot":0,"btns":3072,"halfPot":0,"maxRaise":0,"pid":"CO_SELECT_REQ","raise":0,"timeBank":30}

        int raise = data["raise"].get<int>();
        int bet = data["bet"].get<int>();
        int btns = data["btns"].get<int>();

        game->tryToGetTask("select request", btns);
    }
    else if (pid == "CO_SELECT_SPEED_INFO") {
        // TODO: -------------- this part was not present in latest dump, but was in previous. Try to catch it again.
        // {"data":{"account":[0,0,0,0,2995,194,0,0,0],"bet":[0,0,0,0,0,0,0,0,0],"btn":[0,0,0,0,64,64,0,0,0],"firstSeat":5,"pid":"CO_SELECT_SPEED_INFO","raise":[0,0,0,0,0,0,0,0,0]},"seq":1302,"tDiff":5515}
        // {
        //     "pid":"CO_SELECT_SPEED_INFO",
        //     "firstSeat":5,
        //     "account":[0,0,0,0,2995,194,0,0,0],
        //     "bet":[0,0,0,0,0,0,0,0,0],
        //     "btn":[0,0,0,0,64,64,0,0,0],
        //     "raise":[0,0,0,0,0,0,0,0,0]
        // }

        logfff(thisptr, "----- Multiple actions follow\n");
        
        int firstSeat = data["firstSeat"].get<int>();
        std::vector<int> accounts = data["account"].get<std::vector<int>>();
        std::vector<int> bets     = data["bet"].get<std::vector<int>>();
        std::vector<int> buttons  = data["btn"].get<std::vector<int>>();
        std::vector<int> raises   = data["raise"].get<std::vector<int>>();

        game->DEBUGSendBulkSpeedActions(firstSeat, accounts, bets, buttons, raises);
    }
    else if (pid == "CO_SHOW_BTN") {
        // {"pid":"CO_SHOW_BTN","show":0}
    }
    else if (pid == "CO_LOCK_INFO") {
         // {"data":{"bLock":0,"pid":"CO_LOCK_INFO","seat":9,"time":30},"seq":129,"tDiff":0}
    }
    else if (pid == "CO_SHOW_INFO") {
        // {"btn":32768,"pid":"CO_SHOW_INFO","seat":4}
        // {"btn":32768,"pid":"CO_SHOW_INFO","seat":4}
    }
    else if (pid == "CO_SIT_PLAY") {
        // {"pid":"CO_SIT_PLAY","play":1,"seat":2}
        // {"pid":"CO_SIT_PLAY","play":0,"seat":2}
        //
        // TODO: what exactly does play state differ from seat state
        // NOTE: It looks like that this state describes if we are playing current hand. So if
        // we folden then it is 0. But we are not getting this information about other players.
        // We might not need it in the end and just can remove it later.
        int isPlaying = data["play"].get<int>();
        int seat = data["seat"].get<int>();
        game->setSeatPlayState(seat, isPlaying);
    }
    else if (pid == "CO_TABLE_STATE") {
        // {"pid":"CO_TABLE_STATE","tableState":16}
        // {"pid":"CO_TABLE_STATE","tableState":32}
        // {"pid":"CO_TABLE_STATE","tableState":64}
        // {"pid":"CO_TABLE_STATE","tableState":32768}
        // {"pid":"CO_TABLE_STATE","tableState":65536}

        int TABLESTATE_0 = 0;
        int TABLESTATE_1 = 1;
        int TABLESTATE_2 = 2;
        int TABLESTATE_4 = 4;
        int TABLESTATE_BIG_SMAL_BLINDS_DONE = 8;

        int tableState = data["tableState"].get<int>();

        game->setTableState(tableState);
    }
    else if (pid == "LATENCY_REPORT") {
        // {"id":"0d28c70cbde582eac0d29a5a4db8a082","nsDuration":47284759,"pid":"LATENCY_REPORT"}
        // {"id":"140bdd72feb5b03d8b73af817602a5ea","nsDuration":47452719,"pid":"LATENCY_REPORT"}
        // {"id":"8233cdfe7843f152302ae04e6ae2cb78","nsDuration":45650330,"pid":"LATENCY_REPORT"}
    }
    else if (pid == "PLAY_ACCOUNT_CASH_RES") {
        // {"cash":12000,"pid":"PLAY_ACCOUNT_CASH_RES","seat":2,"type":2}
        // {"cash":40000,"pid":"PLAY_ACCOUNT_CASH_RES","seat":6,"type":2}
        // {"cash":12569,"pid":"PLAY_ACCOUNT_CASH_RES","seat":1,"type":2}
        // {"cash":40000,"pid":"PLAY_ACCOUNT_CASH_RES","seat":1,"type":2}
        // {"cash":54219,"pid":"PLAY_ACCOUNT_CASH_RES","seat":6,"type":2}
        // {"cash":40000,"pid":"PLAY_ACCOUNT_CASH_RES","seat":6,"type":2}

        // NOTE: 
        // 2 ~ IncreasedBalance
        // 5 ~ RebuyCanceled

        int seat = data["seat"].get<int>();
        int cash = data["cash"].get<int>();
        int type = data["type"].get<int>();

        // NOTE: Seen values 2, 5, 6. Not sure what those mean but 2 looks like a correct one
        int TYPE_INCREASE_BALANCE = 2;
        int TYPE_REBUY_CANCEL = 5;
        
        // NOTE: test hack
        if (type == TYPE_REBUY_CANCEL){
            logfff(thisptr, "type is %d, skipping setting seat info\n", type);
        }
        else {
            game->setSeatInfo(seat, cash);
        }
    }
    else if (pid == "PLAY_ACCOUNT_INFO") {
        // {"account":11200,"pid":"PLAY_ACCOUNT_INFO"}
        // {"account":10800,"pid":"PLAY_ACCOUNT_INFO"}
        // {"account":10400,"pid":"PLAY_ACCOUNT_INFO"}
        // {"account":0,"pid":"PLAY_ACCOUNT_INFO"}
    }
    else if (pid == "PLAY_BUYIN_INFO") {
        // {"account":46200,"allowedMax":28000,"allowedMin":1,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}
        // {"account":46200,"allowedMax":40000,"allowedMin":12000,"defaultBuyin":12000,"displayMax":40000,"displayMin":12000,"pid":"PLAY_BUYIN_INFO","seat":2,"type":1}


    }
    else if (pid == "PLAY_CLEAR_INFO") {
        // {"pid":"PLAY_CLEAR_INFO"}
        // {"pid":"PLAY_CLEAR_INFO"}
        // {"pid":"PLAY_CLEAR_INFO"}
        // {"pid":"PLAY_CLEAR_INFO"}
        // {"pid":"PLAY_CLEAR_INFO"}
        // {"pid":"PLAY_CLEAR_INFO"}

        // game->setNextQuickFold(false);

        game->clearSmallBlinePlayer();
        game->clearBigBlindPlayer();
        game->clearErrorServerState();
        game->endStage();
    }
    else if (pid == "PLAY_SEAT_INFO") {
        // {"account":0     , "nickName":"550869473257200" , "pid":"PLAY_SEAT_INFO" , "seat":2 , "state":16 , "type":1}
        // {"account":0     , "nickName":""                , "pid":"PLAY_SEAT_INFO" , "seat":6 , "state":16 , "type":1}
        // {"account":81526 , "nickName":""                , "pid":"PLAY_SEAT_INFO" , "seat":5 , "state":32 , "type":1}
        // {"account":0     , "nickName":""                , "pid":"PLAY_SEAT_INFO" , "seat":5 , "state":16 , "type":0}
        // {"account":0     , "nickName":""                , "pid":"PLAY_SEAT_INFO" , "seat":6 , "state":16 , "type":0}

        // NOTE: Set new player or leaving player on table

        // NOTE: Using 'nickname' is not possible because they are not provided 
        // every time. Maybe it is just not nick/account info.

        // NOTE: if account is 0 then it is probably because somebody played all in and lost
        int account = data["account"].get<int>();
        int seat = data["seat"].get<int>();
        int state = data["state"].get<int>();
        int type = data["type"].get<int>();

        // TODO: It could be that state 32(sitout) means that player is fully leaving table. 
        // Verify that and if true just fully remove player from the table

        // TODO: type 0 or 1 COULD mean 1 - normal, 0 - reversed.

        game->setSeatInfo(seat, account);

        if(game->isMovingFromSitoutToSitin(seat, state, type)) {
            // NOTE: Player was sitting out and now moves into sitting in
            // it is likely they lost money and bought back or moved to sitout
            // and they decided to play again. Game will not allow to enter right
            // away, so they either will wait till big blind or post it right away.
            // So we 'postpone' allowing player to play until they post a blind where
            // we will know exactly that game allowed to play.
            // TODO: Verify this logic one more time
        } else {
            // NOTE: So basically here we allow only to sitout. Sitin is either
            // done on game start or through blinds.
            game->setSeatState(seat, state, type);
        }
    }
    else if (pid == "PLAY_SEAT_RESERVATION") {
        // {"add":0,"pid":"PLAY_SEAT_RESERVATION","seat":2}

        // TODO: there is 'add' field. I have not met it being used but it
        // could be that by reservation you can also reserve 'cash' and thus
        // wee need to add it to the seat total amount. Check it and try to verify

        int seatId = data["seat"].get<int>();

        // NOTE: This is a hack to not let the player into the table. The issue is that when
        // player entered the game it was set as playing but we don't know yet if they will play.
        // So we set state as if the player is not playing and will set it back during a blind.
        // If player decided to enter the game right away, then we will have an 'extra' blind
        // that will tell us that they are playing.
        bool isNormal = true;
        game->setSeatState(seatId, SeatState::Sitout, isNormal);
    }
    else if (pid == "PLAY_STAGE_END_REQ") {
        // NOTE: End of single table game
        // {"pid":"PLAY_STAGE_END_REQ"}
    }
    else if (pid == "PLAY_STAGE_INFO") {
        // {"pid":"PLAY_STAGE_INFO","stageNo":"4737318967"}
        // {"pid":"PLAY_STAGE_INFO","stageNo":"4737318967"}
    }
    else if (pid == "PLAY_TIME_INFO") {
        // {"pid":"PLAY_TIME_INFO","time":30}
        // {"pid":"PLAY_TIME_INFO","time":7}
     
    }
    else if (pid == "SYS_MSG_V2") {
        // {"chat":"_amIBlocked_","pid":"SYS_MSG_V2","seat":2}
        // {"chat":"_amIBlocked_","pid":"SYS_MSG_V2","seat":6}
        // {"chat":"_amIBlocked_","pid":"SYS_MSG_V2","seat":6}
        // {"chat":"_amIBlocked_","pid":"SYS_MSG_V2","seat":6}
    }
    else if (pid == "CO_LAST_HAND_NUMBER") {
        // {"pid":"CO_LAST_HAND_NUMBER","stageNo":"4753608092"}
    }
    else if (pid == "CO_OPTION_INFO") {
        // {"bblind":400,"gameType":2,"gameType2":1,"hiLow":0,"maxSeat":9,"maxStakes":0,"minStakes":0,"pid":"CO_OPTION_INFO","real":0,"sblind":200}
        // {
        //     "bblind":400,
        //     "gameType":2,
        //     "gameType2":1,
        //     "hiLow":0,
        //     "maxSeat":9,
        //     "maxStakes":0,
        //     "minStakes":0,
        //     "pid":"CO_OPTION_INFO",
        //     "real":0,
        //     "sblind":200
        // }

        int bblind = data["bblind"].get<int>();
        int sblind = data["sblind"].get<int>();
        int gameType = data["gameType"].get<int>();
        int maxSeats = data["maxSeat"].get<int>();
        int maxStakes = data["maxStakes"].get<int>();
        int minStakes = data["minStakes"].get<int>();

        game->setSmallBlind(sblind);
        game->setBigBlind(bblind);

        // int straddle = bblind * 2; // TODO: Straddle seems to be not provided anywhere. For now using x2 but needs more reasearch
        // game->setStraddle(bblind);

        // TODO: Ante is not provided and thus just 0 for now. Needs more research
    }
    else if (pid == "CO_TABLE_INFO") {
        // {"account":[0,0,0,260000,0,0,0,186800,12000],"curPot":[0,0,0,0,0,0,0,0,0],"curRake":[0,0,0,0,0,0,0,0,0],"currentSeat":8,"dealerSeat":8,"lockSeat":0,"pbet":[0,0,0,400,0,0,0,200,0],"pcard4":[32896,32896],"pcard8":[32896,32896],"pid":"CO_TABLE_INFO","potCount":0,"seatState":[0,0,0,80,0,0,0,80,16],"tableState":8,"time":28,"totalpot":0}
        // {
        //     "account":[0,0,0,260000,0,0,0,186800,12000],
        //     "curPot":[0,0,0,0,0,0,0,0,0],
        //     "curRake":[0,0,0,0,0,0,0,0,0],
        //     "currentSeat":8,
        //     "dealerSeat":8,
        //     "lockSeat":0,
        //     "pbet":[0,0,0,400,0,0,0,200,0],
        //     "pcard4":[32896,32896],
        //     "pcard8":[32896,32896],
        //     "pid":"CO_TABLE_INFO",
        //     "potCount":0,
        //     "seatState":[0,0,0,80,0,0,0,80,16],
        //     "tableState":8,
        //     "time":28,
        //     "totalpot":0
        // }
        
        // NOTE: This message should come if we start the game while our dll is running.
        // If the program starts later then it seems like it never comes. So there is secondary 
        // initialization mechanism just in case. It is done in CO_RESULT_INFO
        int currentSeat = data["currentSeat"].get<int>();
        int dealerSeat = data["dealerSeat"].get<int>();
        int lockSeat = data["lockSeat"].get<int>();
        int potCount = data["potCount"].get<int>();
        int tableState = data["tableState"].get<int>();
        int time = data["time"].get<int>();
        int totalpot = data["totalpot"].get<int>();

        std::vector<int> account = data["account"].get<std::vector<int>>();
        std::vector<int> curPot = data["curPot"].get<std::vector<int>>();
        std::vector<int> curRake = data["curRake"].get<std::vector<int>>();
        std::vector<int> pbet = data["pbet"].get<std::vector<int>>();
        std::vector<int> seatStates = data["seatState"].get<std::vector<int>>();

        // TODO: What is state 80? In game playing?
        int PROBABLY_SITTING_OUT = 48;
        int PROBABLY_CURRENTLY_PLAYING_MASK = 64;
        for(int i = 0; i < seatStates.size(); i++) {
            int seatNumber = i + 1;
            int state = seatStates[i];
            // NOTE: We are not setting state 80 because it looks like normal 
            // playing state is 16. But I am not sure. Here we set state 48 just 
            // to make sure it won't be sent in next game start.
            // TODO: Check and verify if state 16 or state 80 is proper state for 
            // currently paying player.
            // if(state == PROBABLY_SITTING_OUT) {
            //     bool isNormal = true;
            //     game->setSeatState(seatNumber, state, isNormal);
            // }

            // NOTE: So we change strategy here and on start we assume that all players 
            // that have mask indicating that they are playint as active and everybody else
            // no matter their state we set as 'sitout'. It will get set to correct state on
            // their first blind.
            bool isNormal = true;
            if(state & PROBABLY_CURRENTLY_PLAYING_MASK) {
                game->setSeatState(seatNumber, SeatState::Playing, isNormal);
            } else {
                game->setSeatState(seatNumber, SeatState::Sitout, isNormal);
            }
        }

        game->deinitialize();
        
        game->setTableState(tableState);
        // game->setMaxPlayerCount(account.size());

        // game->setPlayers(account);
        game->setPlayersWithTableInfo(account, seatStates);

        // NOTE: Since it is start of the table it could be that tables have changed. Thus
        // we set AI player to invalid (-1) so that later during player seat reservation it
        // will correctly reassign it. This should happen only once for each table entrance
        game->setAiPlayerId(-1);
        // game->checkSidebar();
    }
    else if (pid == "SYS_INFO") {
        // {"data":30,"dwData":30,"pid":"SYS_INFO","type":3}
    }
    else if (pid == "CO_SELECT_SPEED_BTN") {
        // {"bet":0,"btns":130,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":130,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":1,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":1600,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0 }
        // {"bet":0,"btns":9,"call":200,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":11400,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":400,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":1,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":1400,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":1,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":130,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":1,"call":0,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":1600,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}
        // {"bet":0,"btns":9,"call":6400,"pid":"CO_SELECT_SPEED_BTN","raise":0,"selected":0}

        // if(game->needToQuickFold()) {
        //     DEBUGDoQuickFoldSelection();
        // }
    }
    else if (pid == "CO_CARDHAND_INFO") {
        // {"CardHi":[2,0,4,1,3],"CardLo":[0,0,0,0,0],"bEnableHi":1,"bEnableLo":0,"pid":"CO_CARDHAND_INFO","seat":7,"type":0}
        // {"CardHi":[2,0,4,1,5],"CardLo":[0,0,0,0,0],"bEnableHi":1,"bEnableLo":0,"pid":"CO_CARDHAND_INFO","seat":7,"type":0}
    }
    else if (pid == "CO_SHOW_REQ") { 
        // {"btns":33595392,"pid":"CO_SHOW_REQ"}
        // {"btns":33595392,"pid":"CO_SHOW_REQ"}

        // DEBUGDoMuckCards(thisptr, game->getAreas(), game->getTableState()); // TODO: Do we need to do this? It looks like a straddle. @Research
    }
    //------------ ZONE
    else if (pid == "CO_ZONE_WAITING_PLAYERS_INFO") { 
        // {"pid":"CO_ZONE_WAITING_PLAYERS_INFO","waitingPlayers":0}
    }
    else if (pid == "PLAY_TABLE_NUMBER") { 
        // {"data":{"pid":"PLAY_TABLE_NUMBER","tableNo":2},"seq":204,"tDiff":0}
        // {"data":{"pid":"PLAY_TABLE_NUMBER","tableNo":6},"seq":246,"tDiff":0}
        // {"data":{"pid":"PLAY_TABLE_NUMBER","tableNo":3},"seq":293,"tDiff":0}
    }
    else if (pid == "CO_ZONE_CHANGE_TABLE_INFO") { 
        // {"data":{"pid":"CO_ZONE_CHANGE_TABLE_INFO"},"seq":294,"tDiff":0}
        // {"data":{"pid":"CO_ZONE_CHANGE_TABLE_INFO"},"seq":85,"tDiff":0}
        // {"data":{"pid":"CO_ZONE_CHANGE_TABLE_INFO"},"seq":124,"tDiff":0}

        // NOTE: In Zone poker we won't reach showdown. Then we can do that here.
        game->checkUpdateBalance();
    }
    else if (pid == "CO_ZONE_NO_INFO") { 
        // {"data":{"pid":"CO_ZONE_NO_INFO","zoneNo":2074}
    }
    else {
        logfff(thisptr, "---UNKNOWN---: %s: %s\n", pid.c_str(), json.dump().c_str());
    }


}


void processWebsocketJson(void* thisptr, nlohmann::json json, char* rawData, Game* game) {

    //if (jsonKeyExists(json, "type")) {
    if (json.contains("type")) {
        // TODO: Process types
        /* "tournament.status";
         "tournament.add";
         "tournament.delete";
         "tournament.update";
         "tournament.ticket";
         "lobby.message";
         "lobby.logout";
         "pbo.update";*/
         //....
         //logfff(".\n");
    }
    else if (json.contains("seq")) {
        // logfff("::: %s\n", json.dump().c_str());
        processGameSequence(thisptr, json, game);
    }
    else {
        // TODO: Process later. For now it is just some random tournament info
        //logfff("---- UNKNOWN: %s\n", rawData);
    }
}

// //int(__fastcall * detourTestFunctionOrig2)(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool final, std::span<const char> payload);
// //int __fastcall detourTestFunction2(void* thisptr, void* ignore,  const WebSocketFrameHeader::OpCode opcode, bool final, std::span<const char> payload)
// // int(__fastcall* detourReadWebsocketOrig)(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, int size, void* payload);
// // int __fastcall detourReadWebsocket(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, int size, void* payload)
// int(__fastcall* detourReadWebsocketOrig)(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, int size, base::span<const char> payload);
// int __fastcall detourReadWebsocket(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, int size, base::span<const char> payload)
// {
//     // logfff("size: %d, payload: %x\n", size, payload);
//
//     // if(payload == 0 || (*(char*)payload) == 0 ) {
//     //     if(payload == 0) { logfff("ERROR: payload was 0\n"); }
//     //     else { logfff("*payload = %x\n", *(char*)payload); }
//     // } else {
//     //     try {
//     //         char payloadCopy[16*1024];
//     //         memset(payloadCopy, '\0', sizeof(payloadCopy));
//     //         strncpy_s(payloadCopy, (const char*)payload, sizeof(payloadCopy));
//     //         // nlohmann::json json = probeNumber((char*)payload, size);
//     //         nlohmann::json json = probeNumber((char*)payloadCopy, size);
//     //         processWebsocketJson(json, (char*)payload, globalGame);
//     //     }
//     //     catch (const std::exception& e) {
//     //         logfff("Probe/Json error: %s: %s\n", e.what(), payload);
//     //     }
//     //     catch (...) {
//     //         logfff("Probe or json process error. %s\n", payload);
//     //     }
//     // }
//
//     return detourReadWebsocketOrig(thisptr, ignore, opcode, isfinal, size, payload);
// }



struct custom_span {
    int size;
    unsigned char* data;
};
int(__fastcall* detourReadWebsocketOrig)(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, custom_span payload);
int __fastcall detourReadWebsocket(void* thisptr, void* ignore, const WebSocketFrameHeader::OpCode opcode, bool isfinal, custom_span payload)
{
    Game* game = globalInjectionState->getOrAddGame(thisptr);

    if(opcode == WebSocketFrameHeader::kOpCodeText) {
        if(payload.size > 0) {
            try {
                if(payload.size > 0 && payload.data != 0) {

                    char payloadCopy[256*1024];
                    memset(payloadCopy, '\0', sizeof(payloadCopy));
                    if(payload.size > sizeof(payloadCopy)) {
                        logfff(thisptr, "Payload is too big: {%d-%d} \n", payload.size, sizeof(payloadCopy));
                    } else {
                        // strncpy_s(payloadCopy, sizeof(payloadCopy), (const char*)payload.data, payload.size);
                        strncpy_s(payloadCopy, sizeof(payloadCopy), (const char*)payload.data, payload.size);
                        nlohmann::json json = probeNumber((char*)payloadCopy, payload.size);
                        processWebsocketJson(thisptr, json, (char*)payloadCopy, game);
                    }

                    // nlohmann::json json = probeNumber((char*)payload.data, payload.size);
                    // processWebsocketJson(json, (char*)payload.data, globalGame);
                } else {
                    logfff(thisptr, "Payload error: Size: %d, Data: %x\n", payload.size, payload.data);
                }
            }
            catch (const std::exception& e) {
                logfff(thisptr, "Probe/Json error: %s: %s\n", e.what(), payload);
            }
            catch (...) {
                logfff(thisptr, "Probe or json process error. %s\n", payload);
            }
        } else {
            logfff(thisptr, "M: Zero length message\n");
        }
    } 
    else if(opcode == WebSocketFrameHeader::kOpCodeClose) {
        logfff(thisptr, "Opcode: %d, [Close]: %x\n", opcode, thisptr);
        globalInjectionState->removeGame(thisptr);
    } 
    else {
        logfff(thisptr, "---- [%x] Opcode: %d\n", thisptr, opcode);
    }

    return detourReadWebsocketOrig(thisptr, ignore, opcode, isfinal, payload);
}


int __fastcall detourWriteWebocket(void* thisptr, void* ignore, bool a, WebSocketFrameHeader::OpCode op_code, scoped_refptr<IOBuffer> buffer, size_t size)
{
    // logfff(thisptr, "Writing:[%d] p:%x\t", size, buffer->getData());
    if(size > 0) {
        // TODO: This part can create access violation. Check for data not being 0
        // logfff(thisptr, "Writing: [%d] %s\n", size, buffer->getData());
        //std::string newData = preprocessWritingRequests(thisptr, buffer->getData(), size);

        //int length = newData.size();
        //char* newbuffer = new char[length + 1];
        //std::memcpy(newbuffer, newData.c_str(), length);
        //newbuffer[length] = '\0';

        // int result = detourWriteWebocketOrig(thisptr, ignore, a, op_code, std::move(buffer), size);
        // // TODO: Think about memory management. We are will be leaking memory if not freeing but
        // // also need to verify that scoped_refptr is will reach 0 and deallocate. (in cases where we send 
        // // data ourselves
        //
        // // delete[] newbuffer;
        //
        // return result;
    } else {
        logfff(thisptr, "==== Writing: Buffer is 0\n");
    }

    int result = detourWriteWebocketOrig(thisptr, ignore, a, op_code, std::move(buffer), size);
    // logfff(thisptr, "Written Result: %d\n", result);

    return result;
}




int(__fastcall* detourUrlCreateOrig)(void* thisptr, void* ignore, void* someData);
int __fastcall detourUrlCreate(void* thisptr, void* ignore, void* someData)
{
    if(someData == 0) {
        logfff(thisptr, "URLData is 0\n");
    } else {
        char** url = (char**)((char**)someData + 2);
        if (*url != 0) {
            logfff(thisptr, "--------- URL: %x %s\n", someData, *url);
        }
        else {
            logfff(thisptr, "--------- URL: %x \n", someData);
        }
    }
    
    return detourUrlCreateOrig(thisptr, ignore, someData);
}

int(* detourSSLWriteOrig)(void* SSL, unsigned char* buffer, int length);
int detourSSLWrite(void* SSL, unsigned char* buffer, int length)
{
    // NOTE: gzip 0x1f 0x8b
    logfff(SSL, "\n--------------------------------: ");
    // TODO: Verify length > 2
    logfff(SSL, "SSLWrite: length: %d [%x %x]", length, buffer[0], buffer[1]);
    if(buffer[0] == 0x1f && buffer[1] == 0x8b) {
        for(int i = 0; i < length; i++) {
            logfff(-1, "%02x", buffer[i]);
        }
    } else {
        for(int i = 0; i < length; i++) {
            logfff(-1, "%c", buffer[i]);
        }
    }
    logfff(SSL, "\n");
    return detourSSLWriteOrig(SSL, buffer, length);
}


int(* detourSSLReadOrig)(void* SSL, unsigned char* buffer, int bufferTotalLength);
int detourSSLRead(void* SSL, unsigned char* buffer, int bufferTotalLength)
{
    int readBytes = detourSSLReadOrig(SSL, buffer, bufferTotalLength);

    if(readBytes > 0) {
        logfff(SSL, "\n================================: ");
        logfff(SSL, "SSLRead: length: %d [%x %x] ", readBytes, buffer[0], buffer[1]);
        if(buffer[0] == 0x81) {
            // NOTE: Websocket
            // NOTE: 0x81 indicates a frame: 0x8 - FIN bit, 0x1 - (Opcode) Textframe
            // Next byte is frame length. If first bit is 0 then it has extended length
            logfff(-1, "[Websocket frame ...]");
        }
        else if(buffer[0] == 0x1f && buffer[1] == 0x8b) {
            for(int i = 0; i < readBytes; i++) {
                logfff(-1, "%02x", buffer[i]);
            }
        } else {
            for(int i = 0; i < readBytes; i++) {
                logfff(-1, "%c", buffer[i]);
            }
        }
        logfff(SSL, " \n");
    }

    return readBytes;
}


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

// TODO: Merge these into a common format

void createReadingWebsocketHook() {
    const char* signature = "31 c0 ? ? ? ? ? ? ? ? ? ? ? ? 83 fb ? 0f 87 ? ? ? ? b8 ? ? ? ? ff 24 9d ? ? ? ? b8 ? ? ? ? e9";   // second
    int offsetToTop = -0xAD;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourReadWebsocket, reinterpret_cast<void**>(&detourReadWebsocketOrig));
    }
}

void createWritingWebsocketHook() {
    const char* signature = "E8 ? ? ? ? 89 D8 83 C4 1C 5E 5F 5B 5D C2 10 00 8B 55 14 8B 5D 0C 8A 4D 08 83 FB 01 74 15 85 DB";
    int offsetToTop = -0x4d;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourWriteWebocket, reinterpret_cast<void**>(&detourWriteWebocketOrig));
    }
}

void createUrlCreationHook() {
    const char* signature = "E8 ? ? ? ? 8A 47 0C 88 46 0C 8D 4E 10 8D 47 10 50 E8 ? ? ? ? C7 46 58 00 00 00 00 8B 7F 58 85 FF";
    int offsetToTop = -0xC;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    if (location != 0) { 
        // TODO: Log
    }

    void* HookFunctionName = (void*)(location);
    MH_CreateHook(HookFunctionName, &detourUrlCreate, reinterpret_cast<void**>(&detourUrlCreateOrig));
}


void createSSLWriteHook() {
    const char* signature = "83 7F 50 00 75 45 83 7F 14 00 74 5B 8D 5D EF C6 03 00 8B 47 18 8B 80 D8 00 00 00 85 C0 75 59 8B 07 FF 75 10 FF 75 0C 53 57 FF 50 24 83 C4 10 80 7D EF 00 75 DD 89 C6 8B 4D F0 31 E9 E8 ? ? ? ? 89 F0 83 C4 08 5E 5F 5B 5D C3 68 16 04 00 00 68 ? ? ? ? 6A 42 6A 00 6A 10 E8 ? ? ? ? 83 C4 14 31 F6 EB D0 68 1B 04 00 00 68 ? ? ? ? 68 E2 00 00 00 EB 30 F6 80 34 05 00 00 10 75 9E 80 B8 35 05 00 00 00 78 95 57 E8";
    int offsetToTop = -0x2D;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    if (location != 0) { 
        // TODO: ...
    }

    void* HookFunctionName = (void*)(location);
    MH_CreateHook(HookFunctionName, &detourSSLWrite, reinterpret_cast<void**>(&detourSSLWriteOrig));
}

void createSSLReadHook() {
    const char* signature = "8B 47 18 C7 80 9C 00 00 00 00 00 00 00 E8 ? ? ? ? E8 ? ? ? ? 83 7F 14 00 0F 84 ? ? ? ? 8B 47 18 83 B8 8C 00 00 00 02 0F 84 ? ? ? ? 83 78 74 00 0F 85 ? ? ? ? F6 80 B5 00 00 00 40 0F 85 ? ? ? ? 8B 80 D8 00 00 00 85 C0 0F 85 ? ? ? ? 66 0F 76 C0 66 0F 7F 44 24 10 C7 44 24 20 FF FF FF FF 8B 07 8D 4C 24 10 51 57 FF 50 0C 83 C4 08 84 C0 0F 85 ? ? ? ? C6 44 24 0F 32 8B 47 18 0F B7 48 56 8B 50 50 0F B7 58 54 C7 44 24 08 00 00 00 00 83 B8 8C 00 00 00 02 0F 84";
    int offsetToTop = -0x24;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    void* HookFunctionName = (void*)(location);
    MH_CreateHook(HookFunctionName, &detourSSLRead, reinterpret_cast<void**>(&detourSSLReadOrig));
}




void initGameObject(IniParser config) {
    char* ip = _strdup((char*)config.get("Server", "ServerIp", "").c_str());
    int port = atoi(config.get("Server", "ServerPort", "").c_str());
    gRebuyAtBB = atoi(config.get("Server", "RebuyAtBB", "100").c_str());

    logfff(1, "Initializing game to a client(%s, %d)\n", ip , port);

    globalInjectionState = new InjectionState(config);
}


void createWebsocketHooks(IniParser config) {
    // TODO: We might not need separate hooks for reading and writing
    // but for now it is easier and faster to do it this way.
    // In the future **MAYBE** go through single endpoint.


    // TODO: Game initialization
    try {
        logfff(0, "Initialized game\n");
        initGameObject(config);
    }
    catch (const std::exception& e) {
        logfff(0, "Game Initiazliation error: %s\n", e.what());
    }

    createReadingWebsocketHook();
    createWritingWebsocketHook();

    // createUrlCreationHook();

    // createSSLWriteHook();
    // createSSLReadHook();

    const char* signature = "83 C6 04 8B 46 FC 83 C6 FC 89 F1 6A 01 FF 10 8B 4D F0 31 E9 E8 ? ? ? ? 89 D8 83 C4 1C 5E 5F 5B 5D C2 10 00 8B 55 14 8B 5D 0C 8A 4D 08 83 FB 01 74 15 85 DB 0F 85 ? ? ? ? 80 BF FD 00 00 00 00 0F 84 ? ? ? ? 8D 8F FC 00 00 00 52 FF 76 08 E8 ? ? ? ? 8A 4D 08 83 F8 02 74 09 83 F8 01";
    int offsetToTop = -0x39;
    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;
    SendFrame = SendFramePtr(location);


}
