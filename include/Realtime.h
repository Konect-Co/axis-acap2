#include "CVUtils.h"
#include "Track.h"
#include "PixelMapper.h"

#include <cmath>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

namespace Realtime{
	map<> genCoordinates(PixelMapper::PixelMapperConfig &pm, map<> &CVOutput, double score_threshold = 0.60);
	vector<int>* genMaskData(map<> &CVOutput);
	int* genDistanceData(map<string, double> &coordinatesData, int distance_threshold = 2);
	map<> genTrackingData(int* boxes, map<string, double> &coordinatesData, int* masked, int* distanced);
	map<> genRealData(PixelMapper::PixelMapperConfig &pm, map<> &CVOutput, string &filename, int distance_threshold = 2, double score_threshold = 0.60);
}

