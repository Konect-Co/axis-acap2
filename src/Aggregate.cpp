#include "../include/Aggregate.h"

/*
 * Generates the aggregate data displayed on SpaSpect dashboard
 * @params CVOutput, filename
 * @return aggregate data
*/
map<> Aggregate::genAggData(map<> &CVOutput, map<> &aggData){
	vector<auto> hoursList;

	for(std::map<>::iterator it = m.begin(); it != m.end(); ++it)
		hoursList.push_back(it->first);
	
	sort(hoursList.begin(), hoursList.end(), greater<int>());
	map<> currHour = aggData.at(hoursList.at(0));

	vector<auto> currTracked = currHour.at("currTracked");
	auto tracked = CVOutput.at("tracked");


}
