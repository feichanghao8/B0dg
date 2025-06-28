#pragma once
#include <vector>
#include <string>
#include <deque>
#include "nlohmann/json.hpp"
#include "Client.h"
#include "JobQueue.h"

using json = nlohmann::json;

typedef int SeatIndex;  // This is index in array
typedef int SeatNumber; // This is what bodog gives us

std::vector<std::string> globalCardTable = {
	"Ac", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc",
	"Ad", "2d", "3d", "4d", "5d", "6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd",
	"Ah", "2h", "3h", "4h", "5h", "6h", "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh",
	"As", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s", "Ts", "Js", "Qs", "Ks"

};

// TODO: Extract these values into signle location
int REQ_BUTTON_CHECK       = 64;
int REQ_BUTTON_BET         = 128;
int REQ_BUTTON_CALL        = 256;
int REQ_BUTTON_RAISE       = 512;
int REQ_BUTTON_FOLD        = 1024;
int REQ_BUTTON_ALLIN       = 2048;
int REQ_BUTTON_ALLIN_RAISE = 4096;


int TEST_SEND_BUTTONs_AUTO = REQ_BUTTON_FOLD |  REQ_BUTTON_CALL | REQ_BUTTON_RAISE;

int TIME_TO_WAIT_ACTION = 1*1000;
int MAXIMUM_TASK_WAIT_TIME = 18;
int MAX_TASK_GET_RETRY_AMOUNT = (int)((MAXIMUM_TASK_WAIT_TIME*1000) / TIME_TO_WAIT_ACTION);

std::string getCardFromIndex(int index) {
    if(index <= globalCardTable.size()) {
        return globalCardTable[index];
    }

    int CLOSED_CARD_INDEX = 32896;
    if(index == CLOSED_CARD_INDEX) {
        return "*";
    } else {
        return "-";
    }
}




namespace ActionType {
enum ActionType { 
    call,
    bet,
    check,
    fold,
    raise,
    allin,
    allinraise,

    wrongAction
};
}
class InputTask {
public:
    InputTask(std::string type, double amount):
        type(type), amount(amount) {}

    std::string type;
    double amount;


    /// ----------------
    int _raise = 0;
    int _bet = 0;

    long long startTime = 0;
    ActionType::ActionType actionType = ActionType::wrongAction;

    void setActionType(ActionType::ActionType actionType) {
        this->actionType = actionType;
        this->startTime = getCurrentTimeMillis();
        // logfff(0, "===========: start time: %d\n", this->startTime);
    }
};


class SeatState {
// TODO: Should we use enum instead?
public:
    static const int Rejoin = 4; // TODO: Implement

    static const int Playing = 16; // TODO: Rename to Sitdown
    static const int Sitout = 32;

    static int calculateStateFromServer(int state, int isNormal) {
        if(isNormal) {
            return state;
        } else {
            // NOTE: Sitdown
            if(state == SeatState::Playing)     { return SeatState::Sitout; }
            else if(state == SeatState::Sitout) { return SeatState::Playing; }
            else if(state == SeatState::Rejoin) { return SeatState::Rejoin; }
            else {
                return state; // TODO: Log unknown
            }
        }
    }
};



class Seat {
	// NOTE: Id of the user. Currently it is just the table number because it is 
	// the only information available every time.
	int pid;

	int stack; // sum
    int roundBet; // Amount currently in front of the player

	std::vector<int> cards;
    bool isPlaying = false;

    // int state = SeatState::Playing;
    int state = -1;
	
public:
	Seat(int pid = -1, int stack = 0) : pid(pid), stack(stack), roundBet(0) {};

	void setPid(int pid) { this->pid = pid; };
	void setStack(int stack) { 
        //logfff("Update seat %d from %d to %d\n", pid, this->stack, stack);
        this->stack = stack; 

        // TODO: Is this correct place to move? For now it looks like Websocket does not send
        // 'sitout' information in this case but player is not playing for the next round as
        // they don't have 'cash'. So I guess we can assume that game automatically changes state
        // to 'sitout' and thus we also emulate it here.
        if(stack == 0) {
            this->state = SeatState::Sitout;
            // TODO: Do we also need to clear 'playing" state? To verify that try to catch
            // what happens when user buys more money after reaching 0.
        }
    };
    void setRoundBet(int amount) noexcept {
        this->roundBet = amount;
    }

    void clearCards() { cards.clear(); }

	int getPid() { return pid; }
	int getStack() { return stack; }
    int getRoundBet() { return roundBet; }

    int getSeatIndex() { return pid - 1; }
    
	void setPlayState(bool state) { 
        // logfff(streamPtr, "set seat %d to GAME PLAY state %d\n", pid, state);
        this->isPlaying = state; 
    }
	bool getPlayState() { return isPlaying;  }

	void setState(int state) {
        // logfff(streamPtr, "set seat %d to state %d\n", pid, state);
        this->state = state;  
    }

    int getState() {
        return state;
    }

	std::vector<int> getCards() {
		return cards;
	}

    std::vector<std::string> getExternalCards() {
        std::vector<std::string> result;
        for(int i = 0; i < cards.size(); i++) {
            result.push_back(
                getCardFromIndex(cards[i])
            );
        }
        return result;
    }

	void setCards(std::vector<int> newCards) {
		this->cards = newCards;
	}

	void clear() {
		pid = -1;
		stack = 0;
	}

    double getExternalStack() {
        return (double)stack / 100;
    }


	json toJson() {
		return json::object({
			{"pid", pid},
			// {"stack", getExternalStack() },
			{"stack", stack },
		});
	}

    json dump() {
		return json::object({
			{"pid", pid},
			{"stack", stack },
			{"cards", cards },
			{"isPlaying", isPlaying },
			{"state", state },
		});
    }
};

enum MessageType {
	start_game,
	action,
	player_move,
	private_hand,
	showdown,
};

class Message {

};

class Game {
public:

    Game(Client client): client(client),
                         seats(9)
    {
        sendToAiMutex = CreateMutexA(NULL, FALSE, NULL);
        actionMutex = CreateEvent(NULL, TRUE, FALSE, NULL);
        logfff(streamPtr, "lockAction: %d\n", actionMutex);
    };

    ~Game() {
        // TODO: Make proper cleanup
    }

	void setSmallBlind(int amount) {
		smallBlind = amount;
	}

	void setBigBlind(int amount) {
		this->bigBlind = amount;
	}

	void setStraddle(int amount) {
		this->straddle = amount;
	}

    // double getExternalBigBlind() { return ((double)bigBlind / 100); }
    // double getExternalSmallBlind() { return ((double)smallBlind / 100); }
    // double getExternalAnte() { return ((double)ante / 100); }
    // double getExternalStraddle() { return ((double)straddle / 100); }

    int getExternalBigBlind() { return bigBlind;  }
    int getExternalSmallBlind() { return smallBlind; }
    int getExternalAnte() { return ante; }
    int getExternalStraddle() { return straddle; }

	void setMaxPlayerCount(int amount) {
		this->maxNumberOfPlayers = amount;
        // NOTE: We are sizing to to one more than then players to try to
        // use 1 base indexing. This will should greatly reduce index manipulation
        // in the code.
		seats.resize(maxNumberOfPlayers);
	}

    bool isMovingFromSitoutToSitin(int seatNumber, int newState, bool isNormal) {
        // NOTE: Are we moving from Sitout to Siting in the table game.
        int seatIndex = seatNumber - 1;
        newState = SeatState::calculateStateFromServer(newState, isNormal);
        int currentState = seats[seatIndex].getState();

        if((currentState == SeatState::Sitout) && (newState == SeatState::Playing)) {
            return true;
        } else {
            return false;
        }
    }

    void setSeatPlayState(int seatNumber, bool state) {
        int seatIndex = seatNumber - 1;
        logfff(streamPtr, "Set seat %d PLAY state to %d\n", seatNumber, state);
        if(seatIndex < seats.size()) {
            // Seat& seat = seats[seatIndex];
            // seat.setPlayState(state);    
            seats[seatIndex].setPlayState(state);

            int AIPlayerId = getAiPlayerId();
            if(AIPlayerId != -1 && state == 1) {
                // NOTE: This ia a little hack to allow us check game state base 
                // on regular state but still not send any information to the server
                // which have not started playing our first game
                initialize();
            }
        }
    }


    void setSeatState(int seatNumber, int state, bool isNormal) {
        state = SeatState::calculateStateFromServer(state, isNormal);

        int seatIndex = seatNumber - 1;
        logfff(streamPtr, "Set seat %d state to %d\n", seatNumber, state);
        if(seatIndex < seats.size()) {
            // Seat& seat = seats[seatIndex];
            // seat.setState(state);
            seats[seatIndex].setState(state);
        }
    }

	// void setPlayers(std::vector<int> playerAmounts) {
    //     setMaxPlayerCount(playerAmounts.size());
    //
	// 	if (playerAmounts.size() > seats.size()) {
	// 		logfff("Error: Size of seats[%d] is smaller than size of players [%d]\n", seats.size(), playerAmounts.size());
	// 		// TODO: Should we just abort current game?
	// 		return;
	// 	}
	// 	for (int i = 0; i < seats.size(); i++) {
    //         if(i >= playerAmounts.size() ) { 
    //             // TODO: check
    //             logfff("playerAmounts size %d is smaller than index %d, seats size\n", playerAmounts.size(), i, seats.size());
    //             continue;
    //         }
	// 		int amount = playerAmounts[i];
    //
    //         if(seats[i].getPid() == getAiPlayerId()) {
    //             // NOTE: This is a weird edge case where we were sitting in on table and then
    //             // switched to another. In initialization it will get info about table a set
    //             // it's seats
    //         }
    //
    //         logfff("---: user %d  amount %d\n", i + 1, amount);
	// 		if (amount == 0) {
	// 			// NOTE: There is no player here.
    //             // TODO: Clearing here will be probably destructive. Maybe don't do clearing here
	// 			// seats[i].clear();
    //             
    //             // TODO: Maybe player is present but just lost the money due to all in? Check it.
	// 			seats[i] = Seat(-1, 0);
	// 		}
	// 		else {
	// 			int userId = i + 1; // Bodog counts users starting from 1
	// 			int amount = playerAmounts[i]; // TODO: Do we need to convert amounts?
	// 			seats[i] = Seat(userId, amount);
    //             seats[i].setPlayState(true);
	// 		}
	// 	}
    //
    //     logfff("____:\n");
    //     _dumpSeats();
	// 	// for (Seat seat : seats) {
    //     //     logfff("___ %d %d: %d %d\n", 
    //     //         seat.getPid(),
    //     //         seat.getSeatIndex(),
    //     //         seat.getStack(),
    //     //         seat.getPlayState()
    //     //     );
    //     // }
	// }


	void setPlayersWithTableInfo(std::vector<int> playerAmounts, std::vector<int> seatStates) {
        setMaxPlayerCount(playerAmounts.size());

        for(int i = 0; i < playerAmounts.size(); i++) {
            int amount = playerAmounts[i];
            int state = seatStates[i];

            int userId = i + 1;

            seats[i] = Seat(userId, amount);
            bool isSitting = (state & SeatState::Playing) && (!(state & SeatState::Sitout)); // TODO: Check this logic
            if(isSitting) {
                seats[i].setState(SeatState::Playing);
                seats[i].setPlayState(true); // TODO: Do we need to set it now?
            } else {
                // TODO: Maybe they are not sitting out. There are multiple states. Check what else it could be.
                seats[i].setState(SeatState::Sitout);
            }
        }
        logfff(streamPtr, "setPlayersWithTableInfo dump\n");
        // _dumpSeats();
	}

    void _dumpSeats() {
        logfff(streamPtr, "--- Dump Seats:\n");
        for(Seat seat : seats) {
            logfff(streamPtr, " == %s\n", seat.dump().dump().c_str());
        }
    }

	void setSeatInfo(int seatNumber, int cashAmount) {
        int seatIndex = seatNumber - 1; // NOTE: checkk
		if (seatIndex >= seats.size()) {
			logfff(streamPtr, "ERROR: SeatNumber[%d] is larger than total seats [%d]", seatNumber, seats.size());
            return;
		}
		//seats[seatNumber].clear();
		seats[seatIndex].setPid(seatNumber);
		seats[seatIndex].setStack(cashAmount);
        seats[seatIndex].setRoundBet(0);
        logfff(streamPtr, "SetSeatInfo: seat number: %d, cashAmount: %d\n", seatNumber, cashAmount);
        // logfff(streamPtr, "%s\n", seats[seatIndex].dump().dump(4).c_str());
        logfff(streamPtr, "%s\n", seats[seatIndex].dump().dump().c_str());
        // _dumpSeats();
	}

	// int getNumberOfPlayers() {
	// 	// NOTE: We could store total 'active' seats and just return that number
	// 	// but it is premature optimization and this is fast enough.
	// 	// int total = 0;
	// 	// for (Seat& seat : seats) {
	// 	// 	if (seat.getPid() != -1) {
	// 	// 		total += 1;
	// 	// 	}
	// 	// }
	// 	return total;
	// }

	void setCardsForSeat(int seatId, std::vector<int> cards) {
        int seatIndex = seatId - 1; // NOTE: checkk
		// TODO: Verify in range
        if(seatIndex >= seats.size()) {
			logfff(streamPtr, "ERROR: setCardsForSeat: SeatIndex[%d] is larger than total seats [%d]", seatIndex, seats.size());
            return;
        }

        logfff(streamPtr, "--> Set seat number %d to have cards %s\n", seatId, vectorToString(cards).c_str());

		// getActiveSeats()[seatIndex].setCards(cards);
		seats[seatIndex].setCards(cards);
	}
	


	void setDealerSeat(int seatId) {
        // seatId -=  1; // NOTE: checkk
		this->dealderSeatId = seatId;
	}
	int getDealerSeat() {
		return this->dealderSeatId;
    }

	void setTableState(int tableState) {
		this->tableState = tableState;
	}

	int getTableState() {
		return this->tableState;
	}

    void checkSidebar() {
        if(!_isSidebarChecked) {
            _isSidebarChecked = true;
            areas.isSidebarPresent = checkIfHasSidebar(); 
        }
    }
	void initialize() {
        this->_isInitialized = true;
    }
    // void lockAction() {
        // NOTE: Lock mutex without waiting for it
        // DWORD waitResult = WaitForSingleObject(actionMutex, 0);
        // if (waitResult == WAIT_OBJECT_0) { logfff(streamPtr, "lockAction: [%d] Locked [%d]\n", actionMutex, waitResult); } 
        // else if (waitResult == WAIT_TIMEOUT) { logfff(streamPtr, "lockAction: [%d] Timeout [%d]\n", actionMutex, waitResult); } 
        // else { logfff(streamPtr, "lockAction: [%d] Error [%d]\n", actionMutex, waitResult); }
    // }

    void releaseAction(HANDLE event) {
        bool result = SetEvent(event);
        logfff(streamPtr, "lockAction: Releasing action: %d: %d\n", event, result);
    }
    int waitForAction() {
        logfff(streamPtr, "%x: Wait for action: %d\n", actionMutex, TIME_TO_WAIT_ACTION);

        long long start = getCurrentTimeMillis();
        DWORD waitResult = WaitForSingleObject(actionMutex, TIME_TO_WAIT_ACTION);
        int timeDiff = (int)getTimeDifferenceMillis(start);


        logfff(streamPtr, "%x: lockAction:  wait result [%d], took %d ms\n", actionMutex, waitResult, timeDiff);
        if(waitResult == WAIT_OBJECT_0) {
            bool resetResult = ResetEvent(actionMutex);
            logfff(streamPtr, "%x: lockAction: reset event\n", actionMutex, resetResult);
        }

        return waitResult;
    }
	void deinitialize() { this->_isInitialized = false; }
    bool isInitialzed() {
        // TODO: Maybe we don't need getter here. We needed before when 
        // we were exporting this function across class boundary but now we don't.
        return _isInitialized;
    }

	bool isGamePlaying() {
        int AIPlayerId = getAiPlayerId();
        if(AIPlayerId == -1) {
            logfff(streamPtr, "_ai_player_id_: %d\n", AIPlayerId);
            return false;
        }
        // NOTE: Switching from game state management to adding/removing ai seat on each initial
        // card calculation. It should be the most reliable way to get that information
        else 
        {
            return true;
        }


        // // NOTE: There are different state. There is a state for the seat in general 
        // // like we are sitting out or playing and etc. But there is also a state that
        // // show if we are in a stage game or not. It looks like it sets it to false at the
        // // end of stage and to true at the start. For now we use this indicator.
        // int AIPlayerSeatIndex = AIPlayerId - 1;
        // Seat aiSeat = seats[AIPlayerSeatIndex];
        //
        // // logfff("isInitialized: %d, aiseat[%d][%d] play state:% d\n",
        // //         isInitialzed(), 
        // //         aiSeat.getPid(),
        // //         aiSeat.getStack(),
        // //         aiSeat.getPlayState());
        //
        //     // NOTE: gamestate does not work here as it break logic as soon as we fold our cards,
        //     // so we have to use regular state
        //     bool isStateInGame = aiSeat.getState() == SeatState::Playing;
        //     logfff(streamPtr, "game state. init: %d, aiPlayerState: %d\n", aiSeat.getState());
        //
        // return isInitialzed() && isStateInGame;
	}

	void setAiPlayerId(int playerId) {
        logfff(streamPtr, "Set aiPlayerPid to %d\n", playerId);
		aiPlayerPid = playerId;

        // NOTE: Correctly initialize this. 
        // {
        //     int aiSeatIndex = aiPlayerPid - 1;
        //     int TEMP_SEAT_COUNT = 9;
        //     if(aiSeatIndex < TEMP_SEAT_COUNT) {
        //         seats[aiSeatIndex].setState(SeatState::Playing);
        //     }
        // }
	}

	int getAiPlayerId() {
		return aiPlayerPid;
	}

	std::vector<Seat> &getSeats() {
        return seats;
    }

	std::vector<Seat> getActiveSeats() {
		std::vector<Seat> result = {};
		for (Seat seat : seats) {
			if (seat.getPid() != -1) {
				result.push_back(seat);
			}
		}
		return result;
	}

	// std::vector<Seat> getActiveInGameSeatsFromSb() {
	// 	std::vector<Seat> result = {};
    //
    //     // int dealerSeatIndex = getDealerSeat() - 1;
    //     // int smallBlindStart = dealerSeatIndex;
    //     //
    //     // int countActiveInGame = 0;
    //     // for(int i = 0; i < seats.size(); i++) {
    //     //     if(seats[i].getPlayState() == true && seats[i].getPid() != -1 && seats[i].getState() == SeatState::Playing) {
    //     //         countActiveInGame++;
    //     //     }
    //     // }
    //     //
    //     // if(countActiveInGame== 2) {
    //     //     // NOTE: If there are only two players, then does not set small blind as next after dealer.
    //     //     // It isets is too be the dealer. For normal case we just find the next player after dealer
    //     // } else {
    //     //     for(int i = 0; i < seats.size(); i++) {
    //     //         int currentIndex = (i + dealerSeatIndex) % seats.size();
    //     //         Seat seat = seats[currentIndex];
    //     //
    //     //         // TODO: Verify this logic and move this calculation into Seat class
    //     //         if(seat.getPlayState() == true && seat.getPid() != -1 && seat.getState() == SeatState::Playing) {
    //     //             smallBlindStart = seat.getPid();
    //     //             break;
    //     //         }
    //     //     }
    //     // }
    //
    //     int smallBlindStart = smallBlindPlayerNumber - 1; // TODO: Verify if correct?
    //
    //     for(int i = 0; i < seats.size(); i++) {
    //         int currentIndex = (i + smallBlindStart) % seats.size();
    //         Seat seat = seats[currentIndex];
    //
	// 		// if (seat.getPid() != -1 && seat.getPlayState() == true && seat.getState() == SeatState::Playing) {
	// 		if (seat.getPid() != -1 &&  seat.getState() == SeatState::Playing) {
	// 			result.push_back(seat);
	// 		}
    //     }
    //     logfff("getActiveInGameSeatsFromSb: dump\n");
    //     _dumpSeats();
    //
	// 	return result;
	// }

	std::vector<Seat> getPlayingSeats() {
		std::vector<Seat> result = {};

        for(int i = 0; i < seats.size(); i++) {
            int currentIndex = i;
            Seat seat = seats[currentIndex];

            int state = seat.getState();
            bool isSittingInGame = ((state & SeatState::Playing) != 0)
                && ((state & SeatState::Sitout) == 0);
			if (seat.getPid() != -1 && isSittingInGame) {
				result.push_back(seat);
			}
        }
		return result;
	}

    void saveActivePlayers() {
        storedStartSeats = getPlayingSeats();
    }
    void setActivePlayers(std::vector<Seat> seatsToSave) {
        storedStartSeats = seatsToSave;
    }
    std::vector<Seat> getActivePlayers() {
        return storedStartSeats;
    }

    std::vector<Seat> getActivePlayerFromSb() {
        // int smallBlindStart = smallBlindPlayerNumber - 1; // TODO: Verify if correct?
        std::vector<Seat> result;

        // FIND blind index
        int blindIndex = -1;
        if(smallBlindPlayerNumber != -1) {
            for(int i = 0; i < storedStartSeats.size(); i++) {
                if(storedStartSeats[i].getPid() == smallBlindPlayerNumber) {
                    blindIndex = i;
                }
            }
        } else {
            // NOTE: There are some weird cases where small blind does not come with a websocket
            // message. This is a hack to overcome that.
            int bigBlindIndex = -1;
            for(int i = 0; i < storedStartSeats.size(); i++) {
                if(storedStartSeats[i].getPid() == bigBlindPlayerNumber) {
                    bigBlindIndex = i;
                }
            }
            if(bigBlindIndex == 0) {
                blindIndex = (storedStartSeats.size() - 1);
            } else {
                blindIndex = bigBlindIndex - 1;
            }
        }

        for(int i = 0; i < storedStartSeats.size(); i++) {
            int currentIndex = (i + blindIndex) % storedStartSeats.size();
            Seat seat = storedStartSeats[currentIndex];

			// if (seat.getPid() != -1 &&  seat.getState() == SeatState::Playing) {
			// 	result.push_back(seat);
			// }

            // NOTE: No need to check as they should have been checked at 'storage' time
            result.push_back(seat);
        }

        int startNumber = -1;
        if(result.size() > 0) {
            startNumber = result[0].getPid();
        }
        logfff(streamPtr, "getActivePlayerFromSb. smallBlindNumber: %d, result size:%d, start number: %d\n",
            smallBlindPlayerNumber,
            result.size(),
            startNumber
        );
        // _dumpSeats();

        return result;
    }

	std::vector<Seat> getAllSeats() {
        return seats;
	}

    bool isRequestAllin(int seatNumber, int amount) {
        // TODO: Don't need this anymore? Can remove?
        int seatIndex = seatNumber;
        if(seatIndex < seats.size()) {
            Seat seat = seats[seatIndex];
            int difference = seat.getStack() - amount;
            if(difference <= 0) {
                return true;
            }
        }
        return false;
    }
    
    void addTask(InputTask task) {
		inputTasks.emplace_back(task);
    }

    Areas getAreas() {
        return areas;
    }

    int _AIRestoreAvailableMoney = -1;
    int _AIRestoreSeatNumber = -1;
    long long _AIRestoreTimeStart = 0;
    void setRestoreSitin(int availableMoney, int seatNumber) {
        _AIRestoreAvailableMoney = availableMoney;
        _AIRestoreSeatNumber = seatNumber;
        _AIRestoreTimeStart = getCurrentTimeMillis();
        logfff(streamPtr, "Save money: %d, seat: %d\n", _AIRestoreAvailableMoney, _AIRestoreSeatNumber);
    }

    void checkRestoreSitin() {
        if(_AIRestoreAvailableMoney > 0 && _AIRestoreSeatNumber > 0) {
            // logfff(streamPtr, "=== Restore sitin wait\n");
            int timeDiff = (int)getTimeDifferenceMillis(_AIRestoreTimeStart);

            int waitTime = 3000;
            if(timeDiff > waitTime) {
                // logfff(streamPtr, "=== Restore sitin actual\n");
                DEBUGDoSitin(streamPtr, _AIRestoreAvailableMoney, _AIRestoreSeatNumber);

                _AIRestoreAvailableMoney = -1;
                _AIRestoreSeatNumber = -1;
                _AIRestoreTimeStart = 0;
            }
        }
    }

    void checkAndSendDelayedInput() {
        // checkRestoreSitin();

        if(taskWaitRetryAmount >= MAX_TASK_GET_RETRY_AMOUNT) {
            logfff(streamPtr, "ERROR: Amount of retries [%d] is larger than max allowed [%d]. Will Stop here.\n", taskWaitRetryAmount, MAX_TASK_GET_RETRY_AMOUNT);
            clearWaitingTask("checkAndSendDelayedInput Repeat overflow");

            setAsFailed("Number of task retries is too big. Will fail and try to check/fold.");
            DEBUGDoSendPing(streamPtr);
        }

        // logfff(streamPtr, "___: requestButtonBits: %d\n", requestButtonBits);
        if(requestButtonBits > 0) {
            tryToGetTask("regular response", requestButtonBits);
            
        }

        // logfff(streamPtr, "___: inputTasks.size(): %d\n", inputTasks.size());
        if(inputTasks.size() == 0) {
            return;
        }

        InputTask task = inputTasks.front();

        // logfff(streamPtr, "___: task.startTime: %d\n", task.startTime);
        if(task.startTime == 0) {
            return;
        }

        clearWaitingTask("checkAndSendDelayedInput about to start executing task");
        int timeDiff = (int)getTimeDifferenceMillis(task.startTime);

        int timeLimit = 0.5*1000;
        if(timeDiff < timeLimit) {
            int sleepTime = timeLimit - timeDiff;
            logfff(streamPtr, ":s: %d\n", sleepTime);
            Sleep(sleepTime);
        }

        inputTasks.pop_front(); // Remove task from queue

        int amountInt = task.amount; 

        logfff(streamPtr, "Doing delayed action %d. Start: %d, Diff: %d [%lu]\n", task.actionType, task.startTime, timeDiff, task.startTime);

        switch(task.actionType) {
            case ActionType::call: {
                DEBUGDoCall(streamPtr, areas, tableState, amountInt); 
            } break;

            case ActionType::bet: {
                DEBUGDoBet(streamPtr, areas, tableState, amountInt);
            } break;

            case ActionType::check: {
                DEBUGDoCheck(streamPtr, areas, tableState); 
            } break;

            case ActionType::fold: {
                DEBUGDoFold(streamPtr, areas, tableState); 
            } break;

            case ActionType::raise: {
                DEBUGDoRaise(streamPtr, areas, tableState, amountInt); 
            } break;

            case ActionType::allin: {
                DEBUGDoAllIn(streamPtr, areas, tableState, amountInt);
            } break;

            case ActionType::allinraise: {
                DEBUGDoAllInRaise(streamPtr, areas, tableState, amountInt);
            } break;

            default: {
                logfff(streamPtr, "ERROR: sendDelayedInput: requested action type [%d] is unknown\n", task.actionType);
            } break;
        }
        requestAlreadySent = true;
    }

    int requestButtonBits = -1;
    bool requestAlreadySent = false;
    int taskWaitRetryAmount = 0;
    void setRequestButtons(int buttonBits) {
        requestButtonBits = buttonBits;
    }
    void clearWaitingTask(const char* message) {
        logfff(streamPtr, "clear waiting task: %s\n", message);
        requestButtonBits = -1;
        taskWaitRetryAmount = 0;
    }

    void tryToGetTask(const char* message, int buttonBits) {
        logfff(streamPtr, "tryToGetTask: %s\n", message);
        if(requestAlreadySent == true) {
            logfff(streamPtr, "WARNING: We already did process this tryToGetTask. Won't do anything;\n");
            clearWaitingTask("repeated request");
            return;
        }
        setRequestButtons(buttonBits);
        getTaskAndSendInput(buttonBits);
    }

    void getTaskAndSendInput(int buttonBits) {
        // TODO: This is just a test but these threads are better of going through a separate thread
        // queue system instead creating/destroying like this. But this is the fastest approach to test
        // and idea and good enough for now. But change it later.
        // std::thread t([this, buttonBits]() {

            taskWaitRetryAmount++;

            // // NOTE: We need to wait for mutex because server should return action in previous response and we
            // // can get ahead of it.
            if(getAiPlayerId() == -1) {
                // NOTE: We were requested an action but it looks like we are not seated. This sometimes happens when
                // we are in ZONE room and they either don't send regular card initialization or send it too late.
                // In this case we for now just set error state and check/fold until start of next round
                inErrorServerState = true;
                logfff(streamPtr, "We were requested action but AI id is: %d\n", getAiPlayerId());
            } else {
                logfff(streamPtr, "___: wait\n");
                DWORD result = waitForAction();
                if(result == WAIT_TIMEOUT) {
                    // We timed out and should try later. 
                    logfff(streamPtr, "Task not ready. Will try a bit later.\n");
                    DEBUGDoSendPing(streamPtr);
                    return;
                }
            }


            // TODO: Discuss with the team AGAIN and REMOVE later if we decided not to go this route.
            // NOTE: This is is a temporary solution to try to fold right away if small blind 
            // were not posted in proper way.
            if(smallBlindPlayerNumber == -1) {
                logfff(streamPtr, "WARNING: smallBlindPlayerNumber is -1. Will error out and try to fold.\n");
                logfff(streamPtr, "MARK1: error small blind missing\n");
                setAsFailed("Missing small blind. Will fail and try to check/fold.");
            }

            // NOTE: Yet another hack to bypass a case where we didn'g get 'CO_CARDTABLE_INFO' and were requested
            // an action. Since we didn't get CO_CARDTABLE_INFO then it means that we have not initialized and
            // can't do anything. Here the fastest approach is to fail and fold (it is game start so we won't lose money)
            // and go to another round.
            // TODO: Find where and how they card info. For now it is good enough solution so that we can bypass this and
            // search for other bugs.
            if(getAiPlayerId() == -1) {
                // logfff(streamPtr, "ERROR: We were requested for action but we are not playing. AI id is %d\n", getAiPlayerId());
                logfff(streamPtr, "MARK1: error CO_CARDTABLE_INFO missing\n");
                setAsFailed("We were requested for action but we are not playing. Will check/fold.");
            }


            if(inErrorServerState) {
                // TODO: A quick hack to not do anything if we are in error state
                // DEBUGDoFoldOrCheck(streamPtr, areas, tableState);

                // if(isBitPresent(buttonBits, REQ_BUTTON_CHECK)) {
                //     DEBUGDoFoldOrCheck(streamPtr, areas, tableState);
                // }
                // else if(isBitPresent(buttonBits, REQ_BUTTON_FOLD)) {
                //     DEBUGDoFold(streamPtr, areas, tableState); 
                // }
                // else {
                // }

                if(inputTasks.size() > 0) {
                    // NOTE: We have an action but won't be processing it. Just pop it and continue
                    // InputTask task = inputTasks.front();
                    inputTasks.pop_front();
                }

                if(isBitPresent(buttonBits, REQ_BUTTON_FOLD)) {
                    logfff(streamPtr, "ERROR: we are in error state. We can fold and try to fold right away.\n");
                    DEBUGDoFold(streamPtr, areas, tableState); 
                }
                else if(isBitPresent(buttonBits, REQ_BUTTON_CHECK)) {
                    logfff(streamPtr, "ERROR: we are in error state. Cannot fold, so will try to check.\n");
                    DEBUGDoCheck(streamPtr, areas, tableState); 

                    // NOTE: If we were in error state and had to call check, then we need to release action because we won't 
                    // recieve any action from server. We need to do that until we fold or round ends.
                    releaseAction(this->actionMutex);
                } else {
                    logfff(streamPtr, "ERROR: we are in error state but for now there is nothing we can do. Probably will timeout. Actions: %d\n", buttonBits);
                }

                requestAlreadySent = true;
                clearWaitingTask("getTaskAndSendInput in error state");
                return;
            }

            if(inputTasks.size() > 1) { 
                logfff(streamPtr, "ERROR: Input tasks count is %d but it must always be 1 when action is requested\n", inputTasks.size());
                setAsFailed("More than 1 task in the task list. It must not happen.");
            } else if(inputTasks.size() == 0) {
                // NOTE: In ZONE poker it is possible that we are requested action without getting all info
                // about previous actions. So this becomes one of the correct states insteads of error state.
                // Thus we remove folding here.

                logfff(streamPtr, "Input task size: %d\n", inputTasks.size());
                DEBUGDoSendPing(streamPtr);

                {
                    // logfff(streamPtr, "Error: Input task size is 0. Will fold hand.\n");
                    // setAsFailed("No inputs in the task list. This must not happen.");
                    // DEBUGDoFoldOrCheck(streamPtr, areas, tableState);
                    // if(inputTasks.size() > 0) { inputTasks.pop_front(); }
                }

            } else {
                InputTask& task = inputTasks.front();
                // inputTasks.pop_front();
                std::string betType = task.type;

                logfff(streamPtr, "TASK: %s\n", task.type.c_str());

                // NOTE: No need to convert from double to multiplied ints here as this is user seen value and it is similar to AI.
                int amountInt = task.amount; 
                double amount = (double)amountInt/(double)100;

                // logfff("action: %s, amountInt: %d, amountD: %f\n", betType.c_str(), amountInt, amount);

                if(betType == "calls") {
                    if(isBitPresent(buttonBits, REQ_BUTTON_CALL)) {
                        task.setActionType(ActionType::call);
                        // DEBUGDoCall(streamPtr, areas, tableState, amountInt); 
                    } else if(isBitPresent(buttonBits, REQ_BUTTON_ALLIN)) {
                        // TODO: It looks like server returns '0' amount here. We might need to do cash management to send correct amount here
                        logfff(streamPtr, "Server requested calls but we will call all-in\n");
                        task.setActionType(ActionType::allin);
                        // DEBUGDoAllIn(streamPtr, areas, tableState, amountInt);
                    } else {
                        logfff(streamPtr, "ERROR: Server requested '%s' but there is nothing we can do. Here is our options %d\n", betType.c_str(), buttonBits);
                    }
                }
                else if(betType == "checks") { 
                    task.setActionType(ActionType::check);
                    // DEBUGDoCheck(streamPtr, areas, tableState); 
                }
                else if(betType == "folds") { 
                    // NOTE: We were told to fold but from here we cannot know if we can fold or not
                    // because when there is possiblity to check bodog 'prohibits' folds. So we try to 
                    // fold and at the same time set quick fold. If we can fold we will do that and if not
                    // then we will try do it in the next round
                    task.setActionType(ActionType::fold);
                    // DEBUGDoFold(streamPtr, areas, tableState); 
                }
                else if(betType == "raises")  {
                    if(isBitPresent(buttonBits, REQ_BUTTON_RAISE)) {
                        // NOTE: Server request to raise and we have button raise. So we raise.
                        logfff(streamPtr, "Server requested '%s' and we will call 'raise'\n", betType.c_str());
                        task.setActionType(ActionType::raise);
                    } else if (isBitPresent(buttonBits, REQ_BUTTON_BET)) {
                        // NOTE: If raise was requested from AI server but we don't have that button
                        // displayed then we need to 'bet'.
                        logfff(streamPtr, "Server requested '%s' but we will call 'bet'\n", betType.c_str());
                        task.setActionType(ActionType::bet);
                    } else if (isBitPresent(buttonBits, REQ_BUTTON_CALL)) {
                        logfff(streamPtr, "Server requested '%s' but we will call 'bet'\n", betType.c_str());
                        task.setActionType(ActionType::call);
                    } else if(isBitPresent(buttonBits, REQ_BUTTON_ALLIN_RAISE)) {
                        // TODO: The same with REQ_BUTTON_RAISE. Should we merge them?
                        logfff(streamPtr, "Server requested '%s' but we will call 'all-in raise'\n", betType.c_str());
                        task.setActionType(ActionType::allinraise);
                    } else if (isBitPresent(buttonBits, REQ_BUTTON_ALLIN)) {
                        logfff(streamPtr, "Server requested '%s' but we will call 'all-in'\n", betType.c_str());
                        task.setActionType(ActionType::allin);
                    } else {
                        logfff(streamPtr, "ERROR: Server requested '%s' but there is nothing we can do. Here is our options %d\n", betType.c_str(), buttonBits);
                    }
                }
                else if(betType == "bets") {
                    task.setActionType(ActionType::bet);
                    DEBUGDoBet(streamPtr, areas, tableState, amountInt); 
                }
                else if(betType == "allins") {
                    task.setActionType(ActionType::allin);
                    DEBUGDoAllIn(streamPtr, areas, tableState, amountInt); 
                }
                else {
                    // NOTE: do nothing fo now
                    logfff(streamPtr, "Recieved %s action. Don't know how to process, so skip any action\n", betType.c_str());
                }
                DEBUGDoSendPing(streamPtr);

                clearWaitingTask("getTaskAndSendInput done");
            }

            // lockAction();
        // });
        // t.detach();
    }

	void updateSeatTotals(std::vector<int> account) {
		for (Seat& seat : seats) {
            int seatIndex = seat.getSeatIndex();
            if(seatIndex < account.size()) {
                int amount = account[seatIndex];
                seat.setStack(amount);
            }
        }
        // _dumpSeats();
    }

    void endStage() {
        client.closeConnection(streamPtr);
        logfff(streamPtr, "Clearing all cards\n");
        for(Seat& seat: seats) {
            seat.clearCards();
            seat.setRoundBet(0);
        }
        clearPostBigBlinds();
        clearWaitingTask("end stage");

        inputTasks.clear();
        requestAlreadySent = false;

        setAiPlayerId(-1);

        // TODO: clear actions and other information. Think this through.
    }

    bool getErrorServerState () { return inErrorServerState; }
    void setErrorServerState (bool value) { inErrorServerState = value; }
    void clearErrorServerState () { inErrorServerState = false; }

	// ------------------------------------------------------------
	// ------------------------------------------------------------
	// ------------------------------------------------------------
    void DEBUGProcessServerResultAction(nlohmann::json action) {
        // TODO: make some getter that will check if value exists and
        // set a default value if it doesn't (for json access)
        std::string betType = action["bet_type"].get<std::string>();
        logfff(streamPtr, "action: %s\n", betType.c_str());
        // std::string playerPid = action["player_pid"].get<std::string>();

        // double amount = action["amount"].get<double>() / 100; // TODO: Do we actually devide by 100 here? Ask devs
        int amount = action["amount"].get<int>();

        // NOTE: We can do 'action' only after we recieved "CO_SELECT_REQ" but we will get response
        // from server after one of "CO_SELECT_INFO". So There is unspecified delay between we send one
        // of "CO_SELECT_INFO" to the server (which might give us what action to do) and getting "CO_SELECT_REQ"
        // which will show buttons and allows us to actually commit request. So we should store the action
        // and wait for program to ask us to do something and the do that base on saved action. Clear action
        // after we are done.



        InputTask task = InputTask(betType, amount);
        logfff(streamPtr, "== Add task. %d %s\n", amount, betType.c_str());
        addTask(task);

        // if(betType == "calls")        { DEBUGDoCall(areas); }
        // else if(betType == "checks")  { DEBUGDoCheck(areas); }
        // else if(betType == "folds")   { DEBUGDoFold()areas; }
		// else if (betType == "raises") { DEBUGDoRaise(areas, amount); }
        // else if(betType == "bets")    { DEBUGDoBet(areas, amount); }
        // else if(betType == "allins")  { DEBUGAllIn(areas); }
        // else {
        //     // NOTE: do nothing fo now
        //     logfff("Recieved %s action. Don't know how to process, so skip any action\n", betType.c_str());
        // }
    }

    void setAsFailed(const char* message) {
        inErrorServerState = true;
        logfff(streamPtr, "ERROR happened. Info: %s\n", message);
    }

    bool checkJsonKeyValue(nlohmann::json data, std::string key, std::string value) {
        if(data.contains(key)) {
            if(data[key] == value) {
                return true;
            }
        }
        return false;
    }

    void setErrorConditional(nlohmann::json data, std::string key, std::string value, const char* message) {
        if(!checkJsonKeyValue(data, key, value)) {
            setAsFailed(message);
        } else {
            logfff(streamPtr, "Skip _error_ setting. %s %s\n", key.c_str(), value.c_str());
        }
    }

    

    bool isAutoAction = false;
    void setRequestButtonsManuallyFullAndGetTask() {
        // TODO: This is an estimation of avaialble actions right after bb. Do more testing and verify that
        // // NOTE: removed raise as we cannot do raise in this situationremoved raise as we cannot do raise in this situation
        isAutoAction = true;
        int availableButtonBits = TEST_SEND_BUTTONs_AUTO;
        logfff(streamPtr, "Set available buttons manually. Buttons: %d\n", availableButtonBits);
        setRequestButtons(availableButtonBits);
    }
    // void clearAutoAction() {
    //     isAutoAction = false;
    //     logfff(streamPtr, "Clear autoaction\n");
    // }
    // void autoActionHack() {
    //     if(isAutoAction == true) {
    //         logfff(streamPtr, "Executing autoaction\n");
    //         clearAutoAction();
    //         if(requestButtonBits > 0) {
    //
    //             // // NOTE: It looks like we are too fast for the program. Since we don't know exactly how much time 
    //             // // we need between sending private cards and this action we test it with 4 seconds for now.
    //             // int sleepTime = 4*1000;
    //             // Sleep(sleepTime);
    //
    //             tryToGetTask(requestButtonBits);
    //             // clearAutoAction();
    //         } else {
    //             logfff(streamPtr, "ERROR: request autoaction, but requestButtonBits is not set\n");
    //         }
    //     }
    // }

    void DEBUGsendMessage(const char* name, json data) {
        if(inErrorServerState) {
            // TODO: A quick hack to not do anything if we are in error state
            logfff(streamPtr, "We want to send '%s', but we are in error state from AI server. Won't be sending anything. AI: %d\n", name, getAiPlayerId());
            return;
        }

        HANDLE mutex = this->sendToAiMutex;
        // logfff(streamPtr, "SendingToAI: %s\n", data.dump().c_str());

        std::string message_type = data.value("message_type", "");

        // std::thread t([this, name, data, mutex]() {
        jobQueue.addJob([this, name, data, message_type, mutex]() {
            // WaitForSingleObject(mutex, INFINITE);
            WaitForSingleObject(mutex, 10*1000);

            logfff(streamPtr, "------------------------- SEND: %s\n", name);
            logfff(streamPtr, "SendingToAI: %s\n", data.dump().c_str());
            logfff(streamPtr, "-------------------------\n");


            try {
                nlohmann::json result;

                long long start = getCurrentTimeMillis();
                json resultJson = this->client.sendMessage(streamPtr, data.dump(4), &result);
                int timeDiff = (int)getTimeDifferenceMillis(start);
                logfff(streamPtr, "[%s]: server responded in %d ms\n", name, timeDiff);

                if(resultJson.contains("success")) {
                    bool isSuccess = resultJson["success"];
                    if(!isSuccess) {
                        // setAsFailed("'success' key from server is set to 'false'");
                        setErrorConditional(data, "message_type", "showdown", "'success' key from server is set to 'false'");
                    }
                } else {
                    // logfff(streamPtr, "ERROR: Didn't get correct response from server. Probably timed out or error\n");
                    // setAsFailed("result from request to server does not contain 'success' key, response is empty or it failed/timedout");
                    setErrorConditional(data, "message_type", "showdown", "result from request to server does not contain 'success' key, response is empty or it failed/timedout");
                }

                if(checkJsonKeyValue(data, "message_type", "showdown")) {
                    // NOTE: this is a temporary solution to not set state to failed when we are in showdown.
                    logfff(streamPtr, "Restoring _error_ state from _error_ in showdown.\n");
                    clearErrorServerState();
                }

                if(resultJson.contains("ai_action")) {
                    nlohmann::json action = resultJson["ai_action"];
                    logfff(streamPtr, " :: %s\n", action.dump().c_str());

                    // int timeLimit = 1*1000;
                    // if(timeDiff < timeLimit) {
                    //     int sleepTime = timeLimit - timeDiff;
                    //     logfff(streamPtr, ":ss: %d\n", sleepTime);
                    //     Sleep(sleepTime);
                    // }

                    DEBUGProcessServerResultAction(action);

                    releaseAction(this->actionMutex);

                    if(message_type == "private_hand") {
                        // NOTE: This is a hack. Every time when bodog wants us to do something they send us "CO_SELECT_REQ"
                        // but in Zone poker (it looks like) they don't send that when we are right after BB seat.
                        // I assume they think that at this stage it is obvious that it is our turn to play and no need
                        // to send message. So here set requestButton which is an indicator to later game stages that it is
                        // our turn to act.
                        logfff(streamPtr, "bbNext: %d, ai: %d\n", nextAfterBigBlind, getAiPlayerId() );
                        if(getAiPlayerId() == nextAfterBigBlind) {
                            setRequestButtonsManuallyFullAndGetTask();
                        }
                    }
                } else {
                    // logfff("No action provided.\n");
                }

                if(inErrorServerState == true) {
                    // NOTE: if we are in error state then we will need to send fold/check later. For that
                    // we need to release action and let it pass through later
                    releaseAction(this->actionMutex);
                }
            }
            catch (const std::exception& e) {
                logfff(streamPtr, "Error[sendMessage] %s\n", e.what());
            }
            ReleaseMutex(mutex);
        });


        if(message_type == "private_hand") {
            if(getAiPlayerId() == nextAfterBigBlind) {
                int availableButtonBits = TEST_SEND_BUTTONs_AUTO;
                // TODO: We need a better way to slow down this action. It used to be 2 seconds but 'looks' too fast.
                // Now it is 4 but let's try to think about a better approach.

                // TODO: Instead of sleeping set condition to resolve it on next message (maybe with ping).
                Sleep(0.5*1000);
                tryToGetTask("private hand pre-send", availableButtonBits);
            }
        }

        // t.detach();
    }


    void checkUpdateBalance() {
        if (getAiPlayerId() != -1)
        {
            Seat aiSeat = seats[getAiPlayerId() - 1];

            const int heroBalance = aiSeat.getStack();
            const int bb100 = getExternalBigBlind() * 100;
            const int bottomHeroStackLimit = getExternalBigBlind() * 90;

            if (heroBalance < bottomHeroStackLimit)
            {
                // NOTE: Rounding to nearest dollar + 1
                const int addCashAmount = (((bb100 - heroBalance) / 100) + 1) * 100;

                if(addCashAmount <= 0 ) {
                    logfff(streamPtr, "Won't be sending cash-add request. balance:%d, bb100: %d, addCashAmoun: %d\n", heroBalance, bb100, addCashAmount);
                    return;
                }

                logfff(streamPtr, "Hero's stack is %fBB, initiating re-buyin to 100BB\n",
                    static_cast<float>(heroBalance) / static_cast<float>(getExternalBigBlind()));

                const std::string buyinMessage = std::format(R"({{"availableMoney":5282,"cash":{},"pid":"PLAY_ACCOUNT_CASH_REQ","type":1}})",
                    bb100 - heroBalance);

                setSingleStack(getAiPlayerId(), heroBalance + addCashAmount);

                logfff(streamPtr, "Buyin message: %s\n", buyinMessage.data());

                scoped_refptr buffer {new IOBuffer(buyinMessage)};
                SendFrame(streamPtr, true, WebSocketFrameHeader::kOpCodeText, std::move(buffer), buffer->size());
            }
        }
    }


	void sendRequestAction(int bet, int raise, int btns) {
		// TODO: Test if game correctly initialized
		// json data;
		// data["do"] = json::object({
		// 	{"bet", bet},
		// 	{"raise", raise},
		// 	{"btns", btns},
		// });
        // DEBUGsendMessage("request_action", data);
    }

    void activateAllValidPlayers() {
        // NOTE: We do not get information which players are activated but
        // we can infer that from their state. If their state is SeatState::Playing
        // and the round is about to start we can change their GAME playing state to true.
        // It is a bit confusing but good enough solution for now.
        for(Seat& seat : seats) {
            logfff(streamPtr, "Seat: %d, state: %d | ", seat.getPid(), seat.getState());
            if(seat.getState() != SeatState::Sitout && seat.getPid() != -1) {
                seat.setPlayState(true);
            } else {
                seat.setPlayState(false);
            }
        }
    }

	void sendTableGameInit() {
		if (!isGamePlaying()) {
            logfff(streamPtr, "No init. Skip. [table init]\n");
			return;
		}
        if(inErrorServerState == true) {
            logfff(streamPtr, "Won't be sending [table init] because we are in error state.\n");
            return;
        }

        // activateAllValidPlayers();

        // auto activePlayers = getActiveInGameSeatsFromSb();
        // storedStartSeats = activePlayers;

        auto activePlayers = getActivePlayerFromSb();
        
		// TODO: Test if game correctly initialized
		json data;
		data["message_type"] = "start_game";
		data["game_type"] = "cash";
        data["platform_name"] = "bodog";

		data["n_players"] = activePlayers.size();
		data["sb"] = getExternalSmallBlind();
		data["bb"] = getExternalBigBlind();
		data["ante"] = getExternalAnte();
		data["straddle"] = getExternalStraddle();
		data["ai_player_pid"] = getAiPlayerId();
        

		data["player_info"] = json::array();
        data["raise_to"] = true;



        if(activePlayers.size() > 2) {
            nextAfterBigBlind = activePlayers[2].getPid();
        } else {
            nextAfterBigBlind = activePlayers[0].getPid();
        }
        logfff(streamPtr, "Next player after bb: %d\n", nextAfterBigBlind);


        bool isThereZeroStackUsers = false;
		// for (Seat& seat : getActiveSeats()) {
		for (Seat& seat : activePlayers) {
			data["player_info"].push_back(seat.toJson());
            if(seat.getStack() <= 0) {
                isThereZeroStackUsers = true;
            }
		}

        // NOTE: This is temporary hack for Zone poker to fold hand if users have not been initialized yet.
        // TODO: Remove this or discuss with team one more time for a proper solution
        if(isThereZeroStackUsers) {
            logfff(streamPtr, "ERROR: There are 0 stack users in game initialization: %s\n", data.dump().c_str());
            inErrorServerState = true;

            logfff(streamPtr, "MARK1: error zero_stack_users\n");

            // NOTE: Usually we leave this action to be done after we get response from AI server but 
            // since we won't be able to get it due to this error and hack we have to do it here also.
            releaseAction(this->actionMutex); 

            // NOTE: This is continuation of zero stack users and being right after the bb.
            // So if we are at that position then we will need to check/fold right now but we wont'
            // get reqeust from server. So we need to set action buttons so that it will get picked and
            // do its processing
            // TODO: This is copy-pasta and if we end up using it we need to extract it single place
            if(nextAfterBigBlind > 0 && getAiPlayerId() == nextAfterBigBlind) {
                logfff(streamPtr, "We are first to act at game start.\n");
                setRequestButtonsManuallyFullAndGetTask();

                int availableButtonBits = TEST_SEND_BUTTONs_AUTO;
                Sleep(0.5*1000);
                tryToGetTask("zero stack users", availableButtonBits);
            }
            return;
        }

        if(postedBigBlindUserIds.size() > 0) {
            data["post_a_blind_pids"] = json::array();
            for (int seatNumber : postedBigBlindUserIds) {
                // NOTE: We need to do this second loop because there is a case where somebody posted blind, blind
                // was accepted but player was not added to the game. This will cause error from AI server and here
                // we try to remove this ids to make AI server happy.
                for(Seat seat : activePlayers) {
                    if(seat.getPid() == seatNumber) {
                        data["post_a_blind_pids"].push_back(seatNumber);
                    }
                }
            }
            if(data["post_a_blind_pids"] != postedBigBlindUserIds) {
                logfff(streamPtr, "HACK: postedBigBlindUserIds count: %d, final count: %d\n", postedBigBlindUserIds.size(), data["post_a_blind_pids"].size());
            }
        }

        logfff(streamPtr, "MARK1: normal\n");
        DEBUGsendMessage((char*)"init", data);
	}

	void sendStartingHand(std::vector<int> cards) {
		if (!isGamePlaying()) {
            logfff(streamPtr, "No init. Skip. [starting hand]\n");
			return;
		}
        if(inErrorServerState == true) {
            logfff(streamPtr, "Won't be sending [starting hand] because we are in error state.\n");
            return;
        }

		json data;

		data["message_type"] = "private_hand";
		data["private_hand"] = json::array();
		for (int cardId: cards) {
			//assert(cardId < globalCardTable.size()); // TODO: 
			// if (cardId < globalCardTable.size()) {
			// 	data["private_hand"].push_back(globalCardTable[cardId]);
			// } else{
			// 	data["private_hand"].push_back("__"); // TODO: ...
			// 	// logfff("------------- Error CardID: %d, tableSize: %d\n", cardId, globalCardTable.size());
            //
			// }

            data["private_hand"].push_back(getCardFromIndex(cardId));
		}

        DEBUGsendMessage("private_hand", data);
	}

    int getActiveInGameUserCount() {
        int count = 0;
        for(Seat seat : seats) {
            if(seat.getPid() != -1 && seat.getPlayState() == true) {
                count++;
            }
        }
        return count;
    }


    std::string getActionName(int seat, int bet, int raise, int button) {
        int BUTTON_ALL_IN = 2048;
        int BUTTON_ALL_IN_RAISE = 4096;
        int BUTTON_BET = 128;
        int BUTTON_CALL = 256;
        int BUTTON_CHECK = 64;
        int BUTTON_FOLD = 1024;
        int BUTTON_RAISE = 512;
        int BUTTON_FOLD_AND_SHOW = 1048576;

        std::string betType;

        if(button == BUTTON_ALL_IN)            { betType = "allins"; }
        else if(button == BUTTON_ALL_IN_RAISE) { betType = "allins"; }
        else if(button == BUTTON_BET)          { betType = "bets"; }
        else if(button == BUTTON_CALL)         { betType = "calls"; }
        else if(button == BUTTON_CHECK)        { betType = "checks"; }
        else if(button == BUTTON_FOLD)         { betType = "folds"; }
        else if(button == BUTTON_FOLD_AND_SHOW){ betType = "folds"; }
        else if(button == BUTTON_RAISE)        { betType = "raises"; }
        else {
            logfff(streamPtr, "Not known action button: %d\n", button);
            betType = "[UNKNOWN]";
        }

        logfff(streamPtr, "action selection: bet: %d, raise: %d, button: %d, result: %s\n", bet, raise, button, betType.c_str());

        return betType;
    }

    void DEBUGSendBulkSpeedActions(
            int firstSeatNumber,
            std::vector<int> accounts,
            std::vector<int> bets,
            std::vector<int> buttons,
            std::vector<int> raises)
    {
        int firstSeatIndex = firstSeatNumber - 1; 
        if(firstSeatIndex < 0 && firstSeatIndex >= accounts.size()) {
            logfff(streamPtr, "Wrong firstSeatNumber %d. Max seats: %d\n", firstSeatNumber, accounts.size());
        }

        logfff(streamPtr, "Sending parts of bulk speed message:\n");
        for(int i = 0; i < accounts.size(); i++) {
            int currentIndex = (firstSeatIndex + i) % accounts.size();
            int currentSeatNumber = currentIndex + 1;


            int availableCache = accounts[currentIndex];
            int bet = bets[currentIndex];
            int raise = raises[currentIndex];
            int button = buttons[currentIndex];

            logfff(streamPtr, "i: %d, currentIndex: %d, currentSeatNumber: %d, button:%d\n", i, currentIndex, currentSeatNumber, button);

            if(button > 0) {
                std::string betType = getActionName(currentSeatNumber, bet, raise, button);
                logfff(streamPtr, " :-: Seat:%d, Bet:%d, Raise:%d, Button:%d, betType: %s\n", currentSeatNumber, bet, raise, button, betType.c_str());
                sendPlayerAction(currentSeatNumber, betType, bet, raise);
            }

        }
    }

    void decreaseSingleStack(int seatNumber, int amount) {
        int seatIndex = seatNumber - 1;
        int stack = seats[seatIndex].getStack();
        int newStack = stack - amount;
        seats[seatIndex].setStack(newStack);
    }

    void setSingleStack(int seatNumber, int newStack) {
        int seatIndex = seatNumber - 1;
        seats[seatIndex].setStack(newStack);
    }

	void sendPlayerAction(int playerId, std::string betType, int bet, int raise)
    {
		if (!isGamePlaying())
        {
            logfff(streamPtr, "No init. Skip. [send player action]\n");
			return;
        }
        else if (inErrorServerState == true)
        {
            logfff(streamPtr, "Won't be sending [send player action] because we are in error state.\n");
            return;
        }

        checkSidebar();
        decreaseSingleStack(playerId, bet + raise);

        // Reference this player
        auto &player = seats[playerId - 1];

        if (playerId != getAiPlayerId() && requestAlreadySent == true) {
            requestAlreadySent = false;
        }

        int amount = raise > 0 ? raise : bet;

        if (betType == "raises") {
            // In this case, `raise` represents the amount above the chips already in front
            // of the player. Adjust to the absolute raise amount.
            amount += player.getRoundBet();

            logfff(streamPtr, "Player %d (%d) adding round bet of %d to amount\n",
                playerId, player.getPid(), player.getRoundBet());
        }

        // Update the round bet, if appropriate
        if (betType == "bets" || betType == "raises" || betType == "calls" || betType == "allins")
        {
            player.setRoundBet(amount);

            logfff(streamPtr, "Player %d (%d) round bet changed to %d (%fbb)\n",
                playerId, player.getPid(), amount, (float)(amount) / (float)bigBlind);
        }

		json data;
		data["message_type"] = "player_move";
		data["action"] = json::object({
			{"player_pid", playerId},
			{"bet_type", betType},
			{"amount", amount},
		});

        DEBUGsendMessage("player_move", data);
	}

	void sendDealCommunityCards(std::vector<int> cardIds) {
        requestAlreadySent = false;
        isPreflop = false;

		if (!isGamePlaying()) { logfff(streamPtr, "No init. Skip. [deal community cards]\n"); return; }
        if(inErrorServerState == true) {
            logfff(streamPtr, "Won't be sending [deal community cards] because we are in error state.\n");
            return;
        }

		json data;
		data["message_type"] = "deal_community_cards";
		data["deal_community_cards"] = json::array();
		for (int cardId : cardIds) {
			// if (cardId < globalCardTable.size()) {
			// 	data["deal_community_cards"].push_back(globalCardTable[cardId]);
			// }
			// else {
			// 	data["private_hand"].push_back("__"); // TODO: ...
			// 	// logfff("------------- Error CardID: %d, tableSize: %d\n", cardId, globalCardTable.size());
            //
			// }
            data["deal_community_cards"].push_back(getCardFromIndex(cardId));
		}

        DEBUGsendMessage("deal_community_cards", data);
	}

	void sendFinalResults(std::vector<int> account) {
		if (!isGamePlaying()) {
            logfff(streamPtr, "No init. Skip. [send final results]\n");
			return;
		}
        // if(inErrorServerState == true) {
        //     logfff(streamPtr, "Won't be sending [send final results] because we are in error state.\n");
        //     return;
        // }

		json data;
		data["message_type"] = "showdown";
		data["showdown_info"] = json::array();

		// for (Seat& seat : getActiveSeats()) {
		for (Seat& seat : storedStartSeats) {
			try {
                int seatId = seat.getSeatIndex(); // NOTE: checkk
                if(seatId >= seats.size()) {
                    logfff(streamPtr, "seatId[%d] is larger than seats.size()[%d]", seatId, seats.size());
                    continue; // TODO: CHeck
                }
                if(seatId >= account.size()) {
                    logfff(streamPtr, "seatId[%d] is larger than account.size()[%d]", seatId, account.size());
                    continue; // TODO: CHeck
                }

                Seat freshSeat = seats[seatId];

				// int payoff = (account[seatId] - seat.getStack()) / 100;
				int payoff = (account[seatId] - seat.getStack());
				data["showdown_info"].push_back({
					{"pid", seat.getPid()},
					{"payoff", payoff},
					{"rake", 0},  // TODO: Get rake?
					{"showdown_cards", freshSeat.getExternalCards()} // NOTE: Getting card data from fresh seats
				});
			}
			catch (const std::exception& e) {
				logfff(streamPtr, "Error:... %s\n", e.what());
			}

		}

        DEBUGsendMessage("showdown", data);

        storedStartSeats.clear();

        // TODO: Do we need to deactivate state 32 players?
	}

    // bool needToQuickFold() {
    //     return doNextSpeedFold;
    // }

    // void setNextQuickFold(bool status) {
    //     doNextSpeedFold = status;
    // }
	
    void setClient(Client client) {
        this->client = client;
    }

    void setSmallBlindPlayerNumber(int seatNumber) { smallBlindPlayerNumber = seatNumber; }
    void clearSmallBlinePlayer()                   { smallBlindPlayerNumber = -1; }

    void setBigBlindPlayerNumber(int seatNumber) { bigBlindPlayerNumber = seatNumber; }
    void clearBigBlindPlayer()             { bigBlindPlayerNumber = -1; }

    void addPostBigBlindNumber(int seatNumber) {
        logfff(streamPtr, "add post blind: seat number %d\n", seatNumber);
        postedBigBlindUserIds.push_back(seatNumber);
    }
    void clearPostBigBlinds() {
        postedBigBlindUserIds.clear();
    }
    // void setMaxRaise(int value) {
    //     gameTurnCurrentMaxRaise = value;
    // }
    // void clearMaxRaise() {
    //     gameTurnCurrentMaxRaise = 0;
    // }

    void setStream(void* streamPtr) {
        this->streamPtr = streamPtr;
    }
    void* getStream() {
        return this->streamPtr;
    }
private:
	int smallBlind = 0; // TODO:
	int bigBlind = 0; // TODO:
	int ante = 0;
	int straddle = 0;
	int aiPlayerPid = -1;

    bool isPreflop = true;
	

	int numberOfPlayers = -1;
	int maxNumberOfPlayers = -1;

	std::string gameType = "cash";
	// std::vector<Seat> seats;
	std::vector<Seat> seats;
	std::vector<Seat> storedStartSeats;

	std::vector<int> postedBigBlindUserIds;

    std::deque<InputTask> inputTasks;

    // NOTE: AI server expect to get raise as increment to current max raise
    // int gameTurnCurrentMaxRaise = 0;

	int tableState = 0;
	int dealderSeatId = -1;
	bool _isInitialized = false;
	bool _isSidebarChecked = false;
    Client client;
    Areas areas;

    void* streamPtr = 0;

//---------------
    // bool doNextSpeedFold = false;
    int smallBlindPlayerNumber = -1;
    int bigBlindPlayerNumber = -1;
    int nextAfterBigBlind = -2;
    int inErrorServerState = false;

    HANDLE sendToAiMutex;
    HANDLE actionMutex;
    JobQueue jobQueue;
};
