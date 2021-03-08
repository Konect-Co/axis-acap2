#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	 Mat image = imread("./zebra.png");

	  if (image.empty()) 
		   {
			     cout << "Could not open or find the image" << endl;
			       cin.get();
			         return -1;
				  }

	   String windowName = "The Guitar";

	    namedWindow(windowName);

	     imshow(windowName, image);

	      waitKey(0);

	       destroyWindow(windowName);

	        return 0;
}
