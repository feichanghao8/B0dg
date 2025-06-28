#pragma once
#include <string>
#include "../nlohmann/json.hpp"


// "type":"tournament.add"
// "type" : "tournament.delete"
// "type" : "tournament.idxupdate"
// "type" : "tournament.status"
// "type" : "tournament.update"

class Tournament {
	
	Tournament(std::string rawString) {
		//nlohmann::json;
		const auto a = nlohmann::json::parse(rawString);
	}

	Tournament(nlohmann::json rawData) {
	}
};