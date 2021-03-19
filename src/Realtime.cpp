#include "../include/Realtime.h"

map<> Realtime::genCoordinates(PixelMapper::PixelMapperConfig &pm, map<> &CVOutput, double score_threshold = 0.60){

	vector<double> X3D_vals;
	vector<double> Y3D_vals;
	vector<double> Z3D_vals;
	vector<double> lat_vals;
	vector<double> lon_vals;

	map<string, double> allCoordinates;

	int i;
	for(i = 0 ; i < sizeof(CVOutput.at("scores")[0]/CVOutput.at("scores")) ; ++i){
		if(CVOutput.at(scores)[i] < score_threshold)
			break;
		if(!CVOutput.at("classes")[i] == "person")
			continue;
		
		int box[] = CVOutput.at("boxes")[i];
		double midpoint[2] = {static_cast<int>(box[0]+box[2]/2), box[3]};

		double lonlat[2] = pm.pixel_to_lonlat(midpoint)[0];
		double coord3D[3] = pm.lonlat_to_3D(lonlat);

		lat_vals.push_back(lonlat[0]);
		lon_vals.push_back(lonlat[1]);
		X3D_vals.push_back(coord3D[0]);
		Y3D_vals.push_back(coord3D[1]); 
		Z3D_vals.push_back(coord3D[2]); 
	}

	allCoordinates.insert({"X3D_vals", X3D_vals});
	allCoordinates.insert({"Y3D_vals", Y3D_vals});
	allCoordinates.insert({"Z3D_vals", Z3D_vals});
	allCoordinates.insert({"lat_vals", lat_vals});
	allCoordinates.insert({"lon_vals", lon_vals});

	return allCoordinates;
}

vector<int>* Realtime::genMaskData(map<> &CVOutput){
	vector<int>* masked;

	for(auto box : CVOutput.at("boxes")){
		int wearingMask = 0;
		for(auto maskOut : CVOutput.at("masks")){
			//What's the datatype for this?
			int face_box[4] = maskOut[0];
			auto mask = maskOut[1];
			
			double IOA = CVUtils::compute_IOA(face_box, box);

			if(IOA > 0.9){
				if(mask > 0.7)
					wearingMask = 1;
				else
					wearingMask = 2;
				break;
			}
		}
		masked.push_back(wearingMask);
	}
	return masked;
}

int* Realtime::genDistanceData(map<string, double> &coordinatesData, int distance_threshold = 2){
	double X3D_vals[] = coordinatesData.at("X3D_vals");
	double Y3D_vals[] = coordinatesData.at("Y3D_vals");
	double Z3D_vals[] = coordinatesData.at("Z3D_vals");
	int distanced[];

	for(int i = 0 ; i < sizeof(X3D_vals[0])/sizeof(X3D_vals) ; ++i){
		distanced[i] = 1;
	}
	for(int j = 0 ; j < sizeof(X3D_vals[0])/sizeof(X3D_vals) ; ++j){
		for(int x = j+1 ; x < sizeof(X3D_vals[0])/sizeof(X3D_vals) ; ++x){
			if(distanced[j] == 0 && distanced[x] == 0)
				continue;
			int distance[3] = {X3D_vals[j]-X3D_vals[x], Y3D_vals[j]-Y3D_vals[x], Z3D_vals[j]-Z3D_vals[x]};
			int sum = 0;
			double distanceNum;
			for(int element : distance){
				sum += pow(element, 2);
			}
			distanceNum = sqrt(sum);

			if(distanceNum < distance_threshold){
				distanced[j] = 0;
				distanced[x] = 0;
			}
		}
		if(find(begin(distanced), end(distanced), 1) != end(distanced))
			break;
	}
	return distanced;
}

map<> genTrackingData(int* boxes, map<string, double> &coordinatesData, int* masked, int* distanced){
	double X3D_vals[] = coordinatesData["X3D_vals"];
	double Y3D_vals[] = coordinatesData["Y3D_vals"];
	double Z3D_vals[] = coordinatesData["Z3D_vals"];

	cout << "NEW Objects " << sizeof(X3D_vals[0])/sizeof(X3D_vals) << endl;

	//Continue...

}

map<> genRealData(PixelMapper::PixelMapperConfig &pm, map<> &CVOutput, string &filename, int distance_threshold = 2, double score_threshold = 0.60){
	map<> realData = genCoordinates(pm, CVOutput);
	
	int distanced[] = genDistanceData(realData)
	realData.at("distanced") = distanced;
	
	vector<int>* masked = genMaskData(CVOutput);
	realData.at("masked") = *masked;

	map<> tracked = genTrackingData(CVOutput.at("boxes"), realData, masked, distanced);
	realData.at("tracked") = tracked;

	return realData;
}
