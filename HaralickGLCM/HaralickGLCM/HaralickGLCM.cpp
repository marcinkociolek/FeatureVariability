// HaralickGLCM.cpp : Defines the entry point for the console application.
//

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include <boost\filesystem.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <fstream>

//#include "..\..\..\ProjectsLib\LibMarcin\Functions.h"
//#include "..\..\..\ProjectsLib\LibMarcin\NormalizationLib.h"
#include "..\..\..\ProjectsLib\LibMarcin\HaralickLib.h"
#include "..\..\..\ProjectsLib\LibMarcin\RegionU16Lib.h"

//#include "..\..\..\ProjectsLib\LibMarcin\ParamFromXML.h"
#include "..\..\..\ProjectsLib\LibMarcin\DispLib.h"
#include "..\..\..\ProjectsLib\LibMarcin\StringFcLib.h"

//#include <tinyxml.h>
//#include <tinystr.h>

#define PI 3.14159265

using namespace cv;
using namespace std;
using namespace boost;
using namespace boost::filesystem;


//const int stepNr = 180;


int main(int argc, char* argv[])
{
	bool displayInput = false;
	path PathToProcess("C:\\Data\\FeatureVariabilityTest\\InData\\BarsA0-180F16T08N00000"); //"C:\\Data\\FeatureVariabilityTest\\InData\\BarsA0-90F08T04N00000"
	path PathROIFile("C:\\Data\\FeatureVariabilityTest\\InData\\ROIs512x512c51x51xCount001\\ROIRectSize59x59Nr000.tif");

	string OutFolderName = "C:\\Data\\FeatureVariabilityTest\\";
	string OutFilerName = "BarsA0-180F16T08N00000.txt";
	int ofset = 3;
	int angleNr = 0;

	bool symetricCom = true;
	int binCount = 16;
	float maxNorm = 65535.0;
	float minNorm = 0.0;

	int waitTime = 50;

	regex FilePattern(".+.tif");
	if (!exists(PathToProcess))
	{
		cout << PathToProcess << " not exists " << '\n';
		return 0;
	}
	if (!is_directory(PathToProcess))
	{
		cout << PathToProcess << " This is not a directory path " << '\n';
		//return 0;
	}
	if (!exists(PathToProcess))
	{
		cout << PathToProcess << " not exists " << '\n';
		return 0;
	}
	if (!is_directory(PathToProcess))
	{
		cout << PathToProcess << " This is not a directory path " << '\n';
		//return 0;
	}

	if (!exists(PathROIFile))
	{
		cout << PathToProcess << " not exists " << '\n';
		return 0;
	}
	Mat ImRoi = imread(PathROIFile.string(), CV_LOAD_IMAGE_ANYDEPTH);
	
	if (!ImRoi.cols || !ImRoi.rows)
	{
		cout << "No vailid Roi file";
		return 0;
	}

	int *maxRoiX = new int[65536];
	int *minRoiX = new int[65536];
	int *maxRoiY = new int[65536];
	int *minRoiY = new int[65536];

	int maxRoiNr = 0;

	for (int i = 0; i < 65536; i++)
	{
		maxRoiX[i] = 0;
		minRoiX[i] = 65536;
		maxRoiY[i] = 0;
		minRoiY[i] = 65536;
	}

	if (ImRoi.depth() != CV_16U)
		ImRoi.convertTo(ImRoi, CV_16U);
	unsigned short *wImRoi = (unsigned short *)ImRoi.data;
	for (int y = 0; y < ImRoi.rows; y++)
	{
		for (int x = 0; x < ImRoi.cols; x++)
		{
			if (*wImRoi)
			{
				if (maxRoiX[*wImRoi] < x)
					maxRoiX[*wImRoi] = x;
				if (minRoiX[*wImRoi] > x)
					minRoiX[*wImRoi] = x;
				if (maxRoiY[*wImRoi] < y)
					maxRoiY[*wImRoi] = y;
				if (minRoiY[*wImRoi] > y)
					minRoiY[*wImRoi] = y;

				if (maxRoiNr < *wImRoi)
					maxRoiNr = *wImRoi;
			}
			wImRoi++;
		}
	}
	
	cout << "Roi file  - " << PathROIFile.filename().string() << "\n";
	

	if (displayInput)
	{
		namedWindow("Image", WINDOW_AUTOSIZE);
		namedWindow("ROI", WINDOW_AUTOSIZE);
		namedWindow("SmallImage", WINDOW_AUTOSIZE);
		namedWindow("SmallROI", WINDOW_AUTOSIZE);
	}

	string OutString;
	OutString += "Image\tRoi\tContrast\tenergy\thomogeneity\tCorelation\n";
	for (directory_entry& FileToProcess : directory_iterator(PathToProcess))
	{
		path InPath = FileToProcess.path();

		//string OutString = ProcOptionsStr;


		if (!regex_match(InPath.filename().string().c_str(), FilePattern))
			continue;

		if (!exists(InPath))
		{
			cout << InPath.filename().string() << " File not exists" << "\n";
			continue;
		}


		cout << "In file  - " << InPath.filename().string() << "\n";

		Mat ImIn = imread(InPath.string(), CV_LOAD_IMAGE_ANYDEPTH);

		if (!ImIn.size)
		{
			cout << "this is not a valid image file";
			continue;
		}

		int maxX, maxY, maxXY;
		maxX = ImIn.cols;
		maxY = ImIn.rows;
		maxXY = maxX * maxY;


		if (displayInput)
		{
			Mat ImRGB = ShowImage16PseudoColor(ImIn, 0.0, 65535.0);
			imshow("Image", ImRGB);
			imshow("ROI", ShowSolidRegionOnImage(ImRoi, ImRGB));
			waitKey(waitTime);
		}
		

		for (unsigned short roiNr = 1; roiNr <= maxRoiNr; roiNr++)
		{
			Mat SmallIm;
			Mat Roi;
			ImIn(Rect(minRoiX[roiNr], minRoiY[roiNr], maxRoiX[roiNr] - minRoiX[roiNr], maxRoiY[roiNr] - minRoiY[roiNr])).copyTo(SmallIm);
			ImRoi(Rect(minRoiX[roiNr], minRoiY[roiNr], maxRoiX[roiNr] - minRoiX[roiNr], maxRoiY[roiNr] - minRoiY[roiNr])).copyTo(Roi);

			if (displayInput)
			{
				Mat ImRGBSmall = ShowImage16PseudoColor(SmallIm, 0.0, 65535.0);
				imshow("SmallImage", ImRGBSmall);
				imshow("SmallROI", ShowSolidRegionOnImage(Roi, ImRGBSmall));
				waitKey(waitTime);
			}
			Mat COM = COMStd(SmallIm, Roi, ofset, angleNr, roiNr, symetricCom, binCount, maxNorm,minNorm);
			float contrast, energy, homogenity, correlation;
			COMParams(COM, &contrast, &energy, &homogenity, &correlation);
			OutString += InPath.filename().string() + "\t";
			OutString += to_string(roiNr) + "\t"; 
			OutString += to_string(contrast) + "\t";
			OutString += to_string(energy) + "\t";
			OutString += to_string(homogenity) + "\t";
			OutString += to_string(correlation) + "\n";
		}
	}
	string TextFileName = OutFolderName +OutFilerName;
	std::ofstream out(TextFileName);
	out << OutString;
	out.close();
	return 0;
}

