// Copyright 2020 The Konect SpaSpect Authors. All Rights Reserved.
// This file is part of Konect SpaSpect technology.
//  ___            ___              _   
// / __|_ __  __ _/ __|_ __  ___ __| |_ 
// \__ \ '_ \/ _` \__ \ '_ \/ -_) _|  _|
// |___/ .__/\__,_|___/ .__/\___\__|\__|
//     |_|            |_|               

#include "CVUtils.h"

int CVUtils::compute_area(int box[4]) {
	//TODO
	int area;
	int box_reformed[4] = {box[0], box[1], box[0]+box[2], box[1]+box[3]};
	
	area = (box_reformeid[2]-box_reformed[0])*(box_reformed[3]-box_reformed[1]);
	return area;
}
int CVUtils::compute_intersection(int boxA[4], int boxB[4]) {
	//TODO
	int intersection;
	int boxA_reformed[4] = {boxA[0], boxA[1], boxA[0]+boxA[2], boxA[1]+boxA[3]};
	int boxB_reformed[4] = {boxB[0], boxB[1], boxB[0]+boxB[2], boxB[1]+boxB[3]};
	
	if(boxB_reformed[0] > boxA_reformed[2] || boxA_reformed[0] > boxB_reformed[2] || boxB_reformed[1] > boxA_reformed[3] || boxA_reformed[1] > boxB_reformed[3]){
		return 0;
	}
	int xVals[4] = {boxA_reformed[0], boxA_reformed[2], boxB_reformed[0], boxB_reformed[2]};
	int yVals[4] = {boxA_reformed[1], boxA_reformed[3], boxB_reformed[1], boxB_reformed[3]};
	
	sort(xVals, xVals+4);
	sort(yVals, yVals+4);
	
	intersection = (xVals[2]-xVals[1])*(yVals[2]-yVals[1]);
	
	return intersection;
}
double CVUtils::compute_IoU (int boxA[4], int boxB[4]) {
	//TODO
	double iou;
	double intersection = compute_intersection(boxA,boxB);
	double the_union = compute_area(boxA) + compute_area(boxB) - intersection;
	
	iou = intersection / the_union;
	
	return iou;
}
double CVUtils::compute_IoA (int boxA[4], int boxB[4]) {
	//TODO
	double ioa;
	double intersection = compute_intersection(boxA,boxB);
	double aArea = compute_area(boxA);
	
	ioa = intersection / aArea;
	
	return ioa;
}
