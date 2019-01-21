#include "opencv2/shape.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/utility.hpp>
#include < opencv2\opencv.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include < stdio.h>
#include < conio.h >
#include "dirent.h"
#include <float.h>

#define M_PI 3.14159265358979323846

using namespace std;
using namespace cv;
using namespace cv::ml;

/*#####################*/

/*Scie¿ki do folderów z plikami nale¿¹cymi do baz danych.*/
const string dataPathDB1 = "..\\IMG\\baza1\\";
const string dataPathDB2 = "..\\IMG\\baza2\\";

/*Foldery zapisu plików z poszczególnymi wynikami.*/
const string dataBase1Path = "..\\DB1\\";
const string dataBase2Path = "..\\DB2\\";

/*Wektor z akceptowalnymi rozszerzeniami zdjêæ.*/
static vector<string> validExtensions; //jpg,png

/*SHAPE CONTEXT - zmienne przechowuj¹ce iloœæ binów wzglêdem k¹ta (angle) oraz promienia (radius).*/
int angleBins = 10;
int radiusBins = 5;

/*k-NN - parametr k*/
int k = 7;

/*Zmienna przechowuj¹ca wybran¹ kombinacjê wspó³czynników kszta³tu.*/
string wsp;

/*#####################*/

/*Wektory P1Data-P10Data przechowuj¹ce nazwy plików klas obu baz danych.*/
vector<string> P1Data, P2Data, P3Data, P4Data, P5Data;
vector<string> P6Data, P7Data, P8Data, P9Data, P10Data;

/*Wektory zawieraj¹ce pliki danych s³u¿¹ce do zbudowania klasyfikatora oraz klasyfikacji.*/
vector<vector<string>> daneUczace, daneTestowe;

/*Funkcja zapisuj¹ca nazwy folderów np. /z1.t2/, /z1t4/ itp.*/
void getFoldernames(const char * nazwa_sciezki, vector<string>& folderNames) {
	DIR *dir = opendir(nazwa_sciezki);

	struct dirent *entry = readdir(dir);

	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR)
			folderNames.push_back(entry->d_name);

		entry = readdir(dir);
	}
	folderNames.erase(folderNames.begin());
	folderNames.erase(folderNames.begin());
	closedir(dir);
}

/*Funkcja zapisuj¹ca nazwy plików z podanymi rozszerzeniami (wektor validExtensions) z wybranej lokalizacji.*/
void getFilenames(const char * nazwa_sciezki, const vector<string>& validExtensions, vector<string>& fileNames) {
	struct dirent * plik;
	DIR * sciezka;
	size_t extensionLocation;

	if ((sciezka = opendir(nazwa_sciezki))) {
		while ((plik = readdir(sciezka))) {
#ifdef __MINGW32__	
			stat(ep->d_name, &s);
			if (s.st_mode & S_IFDIR) {
				continue;
			}
#else
			if (plik->d_type & DT_DIR) {
				continue;
			}
#endif

			extensionLocation = string(plik->d_name).find_last_of("."); // Assume the last point marks beginning of extension like file.ext
			if (find(validExtensions.begin(), validExtensions.end(), string(plik->d_name).substr(extensionLocation + 1)) != validExtensions.end()) {
				//printf("Found matching data file '%s'\n", plik->d_name);
				fileNames.push_back((string)nazwa_sciezki + plik->d_name);
			}
			else {
				printf("Plik ma z³e rozszerzenie: '%s'\n", plik->d_name);
			}
		}
		closedir(sciezka);
	}
	else
		printf("! wywo³uj¹c funkcjê opendir(%s) pojawi³ siê b³¹d otwarcia\n", nazwa_sciezki);

}

/*Zapis naz plików bazy danych "p³otkarze".*/
void zapiszPlikiDanychDB1(string dataPath) {
	vector<string> folders; /*foldery P1, P2 itp*/
	vector<string> P1, P2, P3, P4, P5; /*Foldery wewn¹trz folderów P1,P2 ..., np. P1/z1.t2/.*/

	/*W przypadku kilkukrotnego uruchomienia dzia³ania programu nale¿y usun¹æ dane zapisane w zmiennych P1-P5Data.*/
	P1Data.clear(); P2Data.clear(); P3Data.clear(); P4Data.clear(); P5Data.clear();
	getFoldernames(dataPath.c_str(), folders);

	getFoldernames((dataPath + folders[0]).c_str(), P1);
	getFoldernames((dataPath + folders[1]).c_str(), P2);
	getFoldernames((dataPath + folders[2]).c_str(), P3);
	getFoldernames((dataPath + folders[3]).c_str(), P4);
	getFoldernames((dataPath + folders[4]).c_str(), P5);

	/*Zapis nazw plików do wektorów P1Data-P5Data. Baza danych ma piêæ klas.*/
	for (int j = 0; j < P1.size(); j++) {
		getFilenames((dataPath + folders[0] + "\\" + P1[j] + "\\").c_str(), validExtensions, P1Data);
	}

	for (int j = 0; j < P1.size(); j++) {
		getFilenames((dataPath + folders[1] + "\\" + P2[j] + "\\").c_str(), validExtensions, P2Data);
	}

	for (int j = 0; j < P1.size(); j++) {
		getFilenames((dataPath + folders[2] + "\\" + P3[j] + "\\").c_str(), validExtensions, P3Data);
	}

	for (int j = 0; j < P1.size(); j++) {
		getFilenames((dataPath + folders[3] + "\\" + P4[j] + "\\").c_str(), validExtensions, P4Data);
	}

	for (int j = 0; j < P1.size(); j++) {
		getFilenames((dataPath + folders[4] + "\\" + P5[j] + "\\").c_str(), validExtensions, P5Data);
	}
}

/*Zapis naz plików bazy danych "Actions as Space-Time Shapes". Obrazy nie znajduj¹ siê w podfolderach tak jak w poprzedniej bazie danych.*/
void zapiszPlikiDanychDB2(string dataPath) { 
	/*W przypadku kilkukrotnego uruchomienia dzia³ania programu nale¿y usun¹æ dane zapisane w zmiennych P1-P10Data.*/
	P1Data.clear(); P2Data.clear(); P3Data.clear(); P4Data.clear(); P5Data.clear(); P6Data.clear(); P7Data.clear(); P8Data.clear(); P9Data.clear(); P10Data.clear();

	/*Zapis nazw plików z folderów P1, P2 ... do wektorów P1Data, P2Data itd.*/
	getFilenames((dataPath + "P1\\").c_str(), validExtensions, P1Data);
	getFilenames((dataPath + "P2\\").c_str(), validExtensions, P2Data);
	getFilenames((dataPath + "P3\\").c_str(), validExtensions, P3Data);
	getFilenames((dataPath + "P4\\").c_str(), validExtensions, P4Data);
	getFilenames((dataPath + "P5\\").c_str(), validExtensions, P5Data);
	getFilenames((dataPath + "P6\\").c_str(), validExtensions, P6Data);
	getFilenames((dataPath + "P7\\").c_str(), validExtensions, P7Data);
	getFilenames((dataPath + "P8\\").c_str(), validExtensions, P8Data);
	getFilenames((dataPath + "P9\\").c_str(), validExtensions, P9Data);
	getFilenames((dataPath + "P10\\").c_str(), validExtensions, P10Data);
}

/*Zapis danych ucz¹cych oraz testowych dla bazy danych "p³otkarze" w zale¿noœci od kroku walidacji.*/
vector<Mat> zapiszDaneUczaceTestoweDB1(vector<Mat> dataTmp, vector<Mat> data, int index) {
	daneTestowe.clear();
	vector<string> daneTestoweTMP;  /*Wektor przechowuj¹cy dane testowe danej klasy.*/

	/*Wektor pustych macierzy, którego elementy s¹ rozmiarów odpowiadaj¹cych elementów w macierzy data.*/
	dataTmp.push_back(Mat(0, data[0].cols, CV_32F)); /*Macierz o liczbie kolumn równej liczbie obliczonych deskryptorów zdjêæ.*/
	dataTmp.push_back(Mat(0, 1, CV_32S)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.*/
	dataTmp.push_back(Mat(0, 1, CV_32F)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.*/
	dataTmp.push_back(Mat(0, 1, CV_32F)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.*/

	int suma = 0;

	/*Zapis 10% danych pierwszej klasy bazy danych "p³otkarze" wykorzystanych do testowania klasyfikatora.*/
	for (int i = 0; i < P1Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P1Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P1Data.size())) {
			daneTestoweTMP.push_back(P1Data[i]);
		}
		/*Zapis obliczonych deskryptorów oraz klas danych s³u¿¹cych do zbudowania klasyfikatora.*/
		else {
			dataTmp[0].push_back(data[0].row(i));
			dataTmp[1].push_back(data[1].row(i));
			dataTmp[2].push_back(data[2].row(i));
			dataTmp[3].push_back(data[3].row(i));
		}
	}

	suma += P1Data.size();

	/*daneTestowe - wektor wektorów typu string.*/

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();
	
	for (int i = 0; i < P2Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P2Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P2Data.size())) {
			daneTestoweTMP.push_back(P2Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P2Data.size();

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();
	
	for (int i = 0; i < P3Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P3Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P3Data.size())) {
			daneTestoweTMP.push_back(P3Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P3Data.size();

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();
	
	for (int i = 0; i < P4Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P4Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P4Data.size())) {
			daneTestoweTMP.push_back(P4Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P4Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();
	
	for (int i = 0; i < P5Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P5Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P5Data.size())) {
			daneTestoweTMP.push_back(P5Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P5Data.size();

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	/*Zwracane s¹ wartoœci dla danych ucz¹cych.*/
	return dataTmp;
}

/*Zapis danych ucz¹cych oraz testowych dla bazy danych "Actions as Space-Time Shapes" w zale¿noœci od kroku walidacji.*/
vector<Mat> zapiszDaneUczaceTestoweDB2(vector<Mat> dataTmp, vector<Mat> data, int index) {
	daneTestowe.clear();
	vector<string> daneTestoweTMP; /*Wektor przechowuj¹cy dane testowe danej klasy.*/

	/*Wektor pustych macierzy, którego elementy s¹ rozmiarów odpowiadaj¹cych elementów w macierzy data.*/
	dataTmp.push_back(Mat(0, data[0].cols, CV_32F)); /*Macierz o liczbie kolumn równej liczbie obliczonych deskryptorów zdjêæ.*/
	dataTmp.push_back(Mat(0, 1, CV_32S)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.*/
	dataTmp.push_back(Mat(0, 1, CV_32F)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.*/
	dataTmp.push_back(Mat(0, 1, CV_32F)); /*Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.*/

	int suma = 0;
	/*Zapis 10% danych pierwszej klasy bazy danych "Actions as Space-Time Shapes" wykorzystanych do testowania klasyfikatora.*/
	for (int i = 0; i < P1Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P1Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P1Data.size())) {
			daneTestoweTMP.push_back(P1Data[i]);
		}
		/*Zapis obliczonych deskryptorów oraz klas danych s³u¿¹cych do zbudowania klasyfikatora.*/
		else {
			dataTmp[0].push_back(data[0].row(i));
			dataTmp[1].push_back(data[1].row(i));
			dataTmp[2].push_back(data[2].row(i));
			dataTmp[3].push_back(data[3].row(i));
		}

	}
	suma += P1Data.size();

	/*daneTestowe - wektor wektorów typu string.*/
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P2Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P2Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P2Data.size())) {
			daneTestoweTMP.push_back(P2Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P2Data.size();

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P3Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P3Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P3Data.size())) {
			daneTestoweTMP.push_back(P3Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P3Data.size();

	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P4Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P4Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P4Data.size())) {
			daneTestoweTMP.push_back(P4Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P4Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P5Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P5Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P5Data.size())) {
			daneTestoweTMP.push_back(P5Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P5Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P6Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P6Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P6Data.size())) {
			daneTestoweTMP.push_back(P6Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P6Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P7Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P7Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P7Data.size())) {
			daneTestoweTMP.push_back(P7Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P7Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P8Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P8Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P8Data.size())) {
			daneTestoweTMP.push_back(P8Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P8Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P9Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P9Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P9Data.size())) {
			daneTestoweTMP.push_back(P9Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P9Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	for (int i = 0; i < P10Data.size(); i++) {
		if (i >= ((0.1 * (index - 1)) * P10Data.size()) && i < ((0.1 * (index - 1) + 0.1)*P10Data.size())) {
			daneTestoweTMP.push_back(P10Data[i]);
		}
		else {
			dataTmp[0].push_back(data[0].row(i + suma));
			dataTmp[1].push_back(data[1].row(i + suma));
			dataTmp[2].push_back(data[2].row(i + suma));
			dataTmp[3].push_back(data[3].row(i + suma));
		}
	}
	suma += P10Data.size();
	daneTestowe.push_back(daneTestoweTMP);
	daneTestoweTMP.clear();

	/*Zwracane s¹ wartoœci dla danych ucz¹cych.*/
	return dataTmp;
}

/*Operacje closingu oraz openingu.*/
Mat morphology(Mat img) {
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(img, img, MORPH_CLOSE, element, Point(-1, -1));
	morphologyEx(img, img, MORPH_OPEN, element, Point(-1, -1));
	return img;
}

/*Znalezenie na obrazie najwiêkszego obiektu.*/
vector<Point> detectObject(Mat img) {
	Mat dst = img;
	int largest_area = 0; /*zmienna przechowuj¹ca wartoœæ pola najwiêkszego obiektu.*/
	int largest_contour_index = 0; /*Indeks najwiêkszego konturu.*/
	Rect bounding_rect;
	cvtColor(dst, dst, CV_BGR2GRAY);
	threshold(dst, dst, 25, 255, THRESH_BINARY);

	vector<vector<Point>> contours;

	findContours(dst, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE); /*Znalezienie na obrazie konturów wszystkich obiektów.*/

	for (int i = 0; i < contours.size(); i++) {
		double a = contourArea(contours[i], false); 
		if (a > largest_area) {
			largest_area = a;
			largest_contour_index = i;
			bounding_rect = boundingRect(contours[i]);
		}
	}

	return contours[largest_contour_index];
}
/*############################################################################################################################################################################*/


/*WSPÓ£CZYNNIKI KSZTA£TU*/


//bezwymiarowy wspó³czynnik kszta³tu do iloœciowego charakteryzowania kszta³tu cz¹ste
double wspBezwymiarowy(Mat img) {
	double L; //obwod
	double S; //pole
	double wsp;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	L = arcLength(contours[0], true);
	S = abs(contourArea(contours[0], true));

	return wsp = (L*L) / (4 * M_PI*S);;
}

//wspó³czynnik Fereta (charakteryzuje wyd³u¿enie cz¹stki):
double wspFereta(Mat img) {
	double wsp,tmpx,tmpy;
	double Lh=0.0; //maksymalna œrednica poziomo
	double Lv=0.0; //maksymalna œrednica pionowo
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	for (int i = 0; i < contours.size(); i++) {
		for (int j = 0; j < contours[i].size(); j++) {

			tmpx = contours[i][j].x;
			tmpy = contours[i][j].y;

			for (int ii = 0; ii < contours.size(); ii++) {
				for (int jj= 0; jj < contours[ii].size(); jj++) {

					if (tmpx == contours[ii][jj].x) { /*Sprawdzenie czy punktu konturu le¿¹ na prostej równoleg³ej do osi y.*/
						if (abs(contours[ii][jj].y - tmpy) > Lv) {
							Lv = abs(contours[ii][jj].y - tmpy);
						}
					}

					if (tmpy == contours[ii][jj].y) { /*Sprawdzenie czy punktu konturu le¿¹ na prostej równoleg³ej do osi x.*/
						if (abs(contours[ii][jj].x - tmpx) > Lh) {
							Lh = abs(contours[ii][jj].x - tmpx);
						}
					}
				}
			}
		}
	}

	//cout << Lh << '\n';
	//cout << Lv << '\n';

	wsp = Lh / Lv;
	return wsp;
}

//wspó³czynniki cyrkularnoœci
double W1(Mat img) {
	double S; //pole
	double wsp;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	S = abs(contourArea(contours[0], true));

	return wsp = 2 * (sqrt(S / M_PI));
}

double W2(Mat img) {
	double L; //obwod
	double S; //pole
	double wsp;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	L = arcLength(contours[0], true); 
	S = abs(contourArea(contours[0], true));
	return wsp = L / M_PI;
}

//wspó³czynnik Malinowskiej
double W3(Mat img) {
	double L; //obwod
	double S; //pole
	double wsp;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	L = arcLength(contours[0], true); 
	S = abs(contourArea(contours[0], true));

	return wsp=((L/(2*sqrt(M_PI*S)))-1);
}

//wspó³czynnik Blaira - Blissa
double W4(Mat img) {
	double S; //pole
	double tmp,tmpx,tmpy,suma = 0, wsp = 0;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);
	
	S = abs(contourArea(contourTmp, true));

	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mu[i] = moments(contours[i], false);
	}
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00); //srodek ciezkosci obiektu
	}

	Point p;
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			p.x = x;
			p.y = y;
			if (pointPolygonTest(contours[0], p, false) > 0){
				tmpx = (p.x - mc[0].x);
				tmpy = (p.y - mc[0].y);
				tmp = sqrt((tmpx*tmpx) + (tmpy*tmpy));
				suma = suma + (tmp*tmp);
			}
		}
	}

	return wsp=(S/(sqrt(2*M_PI*suma)));
}

//wspó³czynnik Danielssona
double W5(Mat img) {
	double S; //pole
	double wsp,suma=0,tmpx,tmpy,min,odl;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);
	S = abs(contourArea(contours[0], true));

	Point p;
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			p.x = x;
			p.y = y;
			min = 100000;
			if (pointPolygonTest(contours[0], p, false) > 0) {
				for (int i = 0; i < contours.size(); i++) {
					for (int j = 0; j < contours[i].size(); j++) {
						tmpx = contours[i][j].x;
						tmpy = contours[i][j].y;
						odl = sqrt((tmpx - p.x)*(tmpx -p.x) + (tmpy - p.y)*(tmpy - p.y));
						if (odl < min) { /*Minimalna odleg³oœæ piksela obiektu od konturu obiektu.*/
							min = odl;
						}
					}
				}
				suma = suma + min;
			}
		}
	}

	return wsp = ((S*S*S) / (suma*suma));
}

//wspó³czynnik Haralicka
double W6(Mat img) {
	double wsp, tmpx, tmpy, sumaLicznik = 0, sumaMianownik = 0, ilePxKonturu = 0, ilePxObiektu = 0, odl;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mu[i] = moments(contours[i], false);
	}
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00); //srodek ciezkosci obiektu
	}

	for (int i = 0; i < contours.size(); i++) {
		for (int j = 0; j < contours[i].size(); j++) {
			ilePxKonturu++;
			tmpx = contours[i][j].x-mc[0].x;
			tmpy = contours[i][j].y-mc[0].y;
			odl = sqrt((tmpx*tmpx)+(tmpy*tmpy));
			sumaLicznik = sumaLicznik + odl;
			sumaMianownik = sumaMianownik + (odl*odl);
		}
	}
	return wsp=(sqrt((sumaLicznik*sumaLicznik)/((ilePxKonturu*sumaMianownik)-1)));
}

//wspó³czynnik Lp1
double W7(Mat img) {
	double wsp;
	double rmin, rmax, tmpx ,tmpy ,odl;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mu[i] = moments(contours[i], false);
	}
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00); //srodek ciezkosci obiektu
	}

	rmin = sqrt(pow(contours[0][0].x - mc[0].x+ contours[0][0].y - mc[0].y,2.0));
	rmax = rmin;
	for (int i = 0; i < contours.size(); i++) {
		for (int j = 0; j < contours[i].size(); j++) {
			tmpx = contours[i][j].x - mc[0].x;
			tmpy = contours[i][j].y - mc[0].y;
			odl = sqrt((tmpx*tmpx) + (tmpy*tmpy));
			if (odl < rmin) rmin = odl;
			if (odl > rmax) rmax = odl;
		}
	}

	return wsp = rmin / rmax;
}

//wspó³czynnik Mz
double W9(Mat img) {
	double wsp;
	double S,L;
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	L = arcLength(contours[0], true); 
	S = abs(contourArea(contours[0], true));

	return wsp = ((2 * sqrt(M_PI*S)) / L);
}

/*Obliczenie oraz zapisanie wybranych wspó³czynników.*/
vector<float> zapiszWsp(Mat imgTmp) {
	vector<float> row;
	Mat img;
	resize(imgTmp, img, Size(128, 96));
	if (wsp == "wsp_1") {
		row.push_back(W1(img));
		row.push_back(W3(img));
		row.push_back(W5(img));
		row.push_back(W6(img));
		row.push_back(W7(img));
	}
	else if (wsp == "wsp_2") {
		row.push_back(W3(img));
		row.push_back(W5(img));
		row.push_back(W7(img));
		row.push_back(W9(img));
	}
	else if (wsp == "wsp_3") {
		row.push_back(W5(img));
		row.push_back(W6(img));
	}
	return row;
}

/*Zapisanie wspolczynnikow do pliku. Nale¿y podaæ œcie¿kê oraz wektor z nazwami plików np. P1Data.*/
void zapiszWspDoPliku(const char *xml, vector<string> samples) {
	vector<vector<float>> rows; /*Wektor, w którym zapisywane s¹ obliczone wspó³czynniki kszta³tu.*/
	Mat img;
	for (int i = 0; i < samples.size(); i++) {
		img = imread(samples[i]);
		//img = morphology(img);
		rows.push_back(zapiszWsp(img));	/*Ka¿dy wiersz wektora rows zawiera wspó³czynniki dla jednego obrazu.*/
	}
	/*Stworzenie pliku xml, w którym zapisane bêd¹ obliczone wartoœci.*/
	FileStorage hogXml(xml, FileStorage::WRITE); //FileStorage::READ

	/*row - liczba wierszy (zdjêæ); col - liczba kolumn (wspó³czynników).*/
	int row = rows.size(), col = rows[0].size();
	std::printf("row=%d, col=%d\n", row, col);
	Mat M(row, col, CV_32F);
	/*Zapisanie Mat do XML.*/
	for (int i = 0; i< row; ++i)
		memcpy(&(M.data[col * i * sizeof(float)]), rows[i].data(), col * sizeof(float));
	/*Zapis pliku.*/
	cv::write(hogXml, "Wspolczynniki_Ksztaltu", M);
	hogXml.release();
}

/*Odczyt wspó³czynników kszta³tu z plików oraz zapis klas poszczególnych zdjêæ bazy danych "p³otkarze".*/
vector<Mat> DescriptorMTX_wsp_DB1() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/ /*Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.*/

	FileStorage read_1(dataBase1Path + "wspKsztaltuP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase1Path + "wspKsztaltuP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase1Path + "wspKsztaltuP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase1Path + "wspKsztaltuP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase1Path + "wspKsztaltuP5.xml", FileStorage::READ);

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5;;
	read_1["Wspolczynniki_Ksztaltu"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Wspolczynniki_Ksztaltu"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Wspolczynniki_Ksztaltu"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Wspolczynniki_Ksztaltu"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Wspolczynniki_Ksztaltu"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);

	/*######################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32S, 5);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32F, 5);
	labelsLogisticRegression.rowRange(0, pRow_1) = 1;
	labelsLogisticRegression.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 5, CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;
		else
			trainClasses.at<float>(i, 4) = 1.0;
	}

	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*Odczyt wspó³czynników kszta³tu z plików oraz zapis klas poszczególnych zdjêæ bazy danych "Actions as Space-Time Shapes".*/
vector<Mat> DescriptorMTX_wsp_DB2() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/

	FileStorage read_1(dataBase2Path + "wspKsztaltuP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase2Path + "wspKsztaltuP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase2Path + "wspKsztaltuP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase2Path + "wspKsztaltuP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase2Path + "wspKsztaltuP5.xml", FileStorage::READ);
	FileStorage read_6(dataBase2Path + "wspKsztaltuP6.xml", FileStorage::READ);
	FileStorage read_7(dataBase2Path + "wspKsztaltuP7.xml", FileStorage::READ);
	FileStorage read_8(dataBase2Path + "wspKsztaltuP8.xml", FileStorage::READ);
	FileStorage read_9(dataBase2Path + "wspKsztaltuP9.xml", FileStorage::READ);
	FileStorage read_10(dataBase2Path + "wspKsztaltuP10.xml", FileStorage::READ);

	/*##################################################################*/

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5, pMat_6, pMat_7, pMat_8, pMat_9, pMat_10;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5, pRow_6, pCol_6, pRow_7, pCol_7, pRow_8, pCol_8, pRow_9, pCol_9, pRow_10, pCol_10;
	read_1["Wspolczynniki_Ksztaltu"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Wspolczynniki_Ksztaltu"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Wspolczynniki_Ksztaltu"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Wspolczynniki_Ksztaltu"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Wspolczynniki_Ksztaltu"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_6["Wspolczynniki_Ksztaltu"] >> pMat_6;
	pRow_6 = pMat_6.rows; pCol_6 = pMat_6.cols;

	read_7["Wspolczynniki_Ksztaltu"] >> pMat_7;
	pRow_7 = pMat_7.rows; pCol_7 = pMat_7.cols;

	read_8["Wspolczynniki_Ksztaltu"] >> pMat_8;
	pRow_8 = pMat_8.rows; pCol_8 = pMat_8.cols;

	read_9["Wspolczynniki_Ksztaltu"] >> pMat_9;
	pRow_9 = pMat_9.rows; pCol_9 = pMat_9.cols;

	read_10["Wspolczynniki_Ksztaltu"] >> pMat_10;
	pRow_10 = pMat_10.rows; pCol_10 = pMat_10.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();
	read_6.release();
	read_7.release();
	read_8.release();
	read_9.release();
	read_10.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);
	start = start + sizeof(float) * pMat_5.cols * pMat_5.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_6.data, sizeof(float) * pMat_6.cols * pMat_6.rows);
	start = start + sizeof(float) * pMat_6.cols * pMat_6.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_7.data, sizeof(float) * pMat_7.cols * pMat_7.rows);
	start = start + sizeof(float) * pMat_7.cols * pMat_7.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_8.data, sizeof(float) * pMat_8.cols * pMat_8.rows);
	start = start + sizeof(float) * pMat_8.cols * pMat_8.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_9.data, sizeof(float) * pMat_9.cols * pMat_9.rows);
	start = start + sizeof(float) * pMat_9.cols * pMat_9.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_10.data, sizeof(float) * pMat_10.cols * pMat_10.rows);

	/*######################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32S, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32F, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 10, CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5))
			trainClasses.at<float>(i, 4) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6))
			trainClasses.at<float>(i, 5) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7))
			trainClasses.at<float>(i, 6) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8))
			trainClasses.at<float>(i, 7) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9))
			trainClasses.at<float>(i, 8) = 1.0;

		else
			trainClasses.at<float>(i, 9) = 1.0;
	}

	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*-----------------------------------------------------------------------------------------------*/

/*Wspó³czynniki kszta³tu trening klasyfikatorów.*/

void nauczSVM_wsp(string db, string path,int index) {
	vector<Mat> data;
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_wsp_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_wsp_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::SVM> svm = ml::SVM::create();
	svm->setType(ml::SVM::C_SVC);
	svm->setKernel(ml::SVM::LINEAR);
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

	svm->train(dataTmp[0], ROW_SAMPLE, dataTmp[1]);
	path = path + string("wspKsztaltuTrainedSVM.xml");
	svm->save(path);
	cout << "SVM zapisany" << '\n';
}

void nauczANN_wsp(string db, string path, int index) {
	vector<Mat> data;
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_wsp_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_wsp_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr< ANN_MLP >  ann = ANN_MLP::create();
	Mat layers = cv::Mat(3, 1, CV_32F);
	layers.row(0) = Scalar(dataTmp[0].cols);
	layers.row(1) = Scalar(7);
	if (db == std::string("db1")) {
		layers.row(2) = Scalar(5);
	}
	else if (db == std::string("db2")) {
		layers.row(2) = Scalar(10);
	}

	ann->setLayerSizes(layers);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM, 1.0, 1.0);
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
	ann->setBackpropMomentumScale(0.0001);
	ann->setBackpropWeightScale(0.03);
	ann->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 3, 0.00001));
	ann->train(dataTmp[0], ROW_SAMPLE, dataTmp[2]);
	path = path + string("wspKsztaltuTrainedANN.xml");
	ann->save(path);
	cout << "ANN zapisany" << '\n';
}

void nauczLogisticRegression_wsp(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_wsp_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_wsp_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	cv::Ptr<cv::ml::LogisticRegression> logisticRegression = cv::ml::LogisticRegression::create();
	logisticRegression->setLearningRate(0.5);
	logisticRegression->setIterations(10);
	logisticRegression->setRegularization(cv::ml::LogisticRegression::REG_L1);
	logisticRegression->setTrainMethod(cv::ml::LogisticRegression::BATCH);
	logisticRegression->setMiniBatchSize(1);
	logisticRegression->train(dataTmp[0], cv::ml::ROW_SAMPLE, dataTmp[3]);
	path = path + string("wspKsztaltuTrainedLogisticRegression.xml");
	logisticRegression->save(path);
	cout << "zapisano logistic regression" << '\n';;
}

void nauczRTrees_wsp(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_wsp_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_wsp_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::RTrees> RTrees = ml::RTrees::create();

	RTrees->setMaxDepth(10);
	RTrees->setMinSampleCount(10);
	RTrees->setRegressionAccuracy(0);
	RTrees->setUseSurrogates(false);
	RTrees->setMaxCategories(15);
	RTrees->setPriors(Mat());
	RTrees->setCalculateVarImportance(true);
	RTrees->setActiveVarCount(4);
	RTrees->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 1, 1e-6));

	RTrees->train(dataTmp[0], ml::ROW_SAMPLE, dataTmp[1]);
	path = path + string("shapeContextTrainedRTrees.xml");
	RTrees->save(path);
	cout << "zapisano random trees" << endl;
}

float KNN_wsp(Mat imgTmp,string db, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	Mat img;
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_wsp_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_wsp_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	resize(imgTmp, img, Size(128, 96));
	//img = morphology(img);
	cv::Ptr<cv::ml::KNearest> knn(cv::ml::KNearest::create());
	Ptr<TrainData> trainingData;
	trainingData = TrainData::create(dataTmp[0], ml::ROW_SAMPLE, dataTmp[1]);
	knn->setAlgorithmType(ml::KNearest::BRUTE_FORCE);
	knn->setIsClassifier(1);
	knn->setDefaultK(k);

	knn->train(trainingData);

	Mat result(0, 0, CV_32FC1);

	knn->findNearest(zapiszWsp(img), knn->getDefaultK(), result);
	return result.at<float>(Point(0, 0));
}

/*-----------------------------------------------------------------------------------------------*/

/*Wspó³czynniki kszta³tu klasyfikacja.*/
/*imgTmp - testowane zdjêcie, xml - œcie¿ka do pliku z zapisanym nauczonym klasyfikatorem.*/
int sprawdzImgSVM_wsp(Mat imgTmp,string xml) {
	Ptr<ml::SVM> svmTmp = cv::Algorithm::load<ml::SVM>(xml);
	Mat img;
	resize(imgTmp, img, Size(128, 96));
	//img = morphology(img);
	vector<float> wspImg = zapiszWsp(img);
	float result = svmTmp->predict(wspImg);
	return result;
}

int sprawdzImgANN_wsp(Mat imgTmp, string xml,string db) {
	Mat img;
	resize(imgTmp, img, Size(128, 96));
	//img = morphology(img);
	Ptr<ml::ANN_MLP> annTmp = cv::Algorithm::load<ml::ANN_MLP>(xml);
	vector<float> wspImg = zapiszWsp(img);
	int cls = -1;
	if (db == std::string("db1")) {
			Mat response = Mat::zeros(1, 5, CV_32F);

			annTmp->predict(wspImg, response);
			float max = -FLT_MAX;
			for (int i = 0; i < 5; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	else if (db == std::string("db2")) {
		Mat response = Mat::zeros(1, 10, CV_32F);

		annTmp->predict(wspImg, response);
		float max = -FLT_MAX;
		for (int i = 0; i < 10; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	
	return cls;
}

int sprawdzImgLogisticRegression_wsp(Mat imgTmp, string xml) {
	cv::Ptr<cv::ml::LogisticRegression> logisticRegressionTmp = cv::Algorithm::load<cv::ml::LogisticRegression>(xml);
	Mat img;
	resize(imgTmp, img, Size(128, 96));
	//img = morphology(img);
	vector<float> wspImg = zapiszWsp(img);
	float result = logisticRegressionTmp->predict(wspImg);
	return result;
}

int sprawdzImgRTrees_wsp(Mat imgTmp, string xml) {
	Ptr<ml::RTrees> RTreesTmp = cv::Algorithm::load<cv::ml::RTrees>(xml);
	Mat img;
	resize(imgTmp, img, Size(128, 96));
	//img = morphology(img);
	vector<float> wspImg = zapiszWsp(img);
	float result = RTreesTmp->predict(wspImg);
	return result;
}

/*############################################################################################################################################################################*/

/*SHAPE CONTEXT*/

/* http://www.cs.utexas.edu/~grauman/courses/spring2008/slides/ShapeContexts425.pdf 
   http://www.slideshare.net/VanyaVabrina/shape-context */

/*Wybranie 40 punktów konturu.*/
static vector<Point> simpleContour(vector<Point> contourQuery, int n = 40){
	vector<Point> contour;
	int tmp;
	
	for (int add = 0; add < n; add++) {
		tmp = (contourQuery.size() / n) * (add);
		contour.push_back(contourQuery[tmp]);
	}	
	return contour;
}

/*Shape context zdjêcia, argumenty: zdjêcie, liczba binów k¹ta, liczba binów odlg³oœci*/
vector<float> shapeContext(Mat img, int nbins, int rbins) {
	//odleglosc euklidesowa
	vector<vector<double>> odl;
	//katy miedzy punktami konturu
	vector<vector<double>> katy;
	double	r_inner = 10;	//minimalna odleg³oœæ
	double	r_outer = 250;		//maksymalna odleg³oœæ
	double r_bins_edges[5];		//liczba "granic" binów odleg³oœci

	Point tmpPoint;
	double odlEuklidesowa, odlSrednia,kat;
	double sumaOdl=0, tmpx, tmpy,ilePkt=0;

	vector<Point>contours;
	contours = detectObject(img);
	contours = simpleContour(contours);

	/*Wektor z odleg³oœciami miêdzy wszystkimi punktami konturu*/
	for (int i = 0; i < contours.size(); i++) {
			tmpPoint = contours[i];
			odlEuklidesowa = 0;
			vector<double> tmpOdl;
			for (int ii = 0; ii < contours.size(); ii++) {
					tmpx = contours[ii].x - tmpPoint.x;
					tmpy = contours[ii].y - tmpPoint.y;
					odlEuklidesowa = sqrt((tmpx*tmpx) + (tmpy*tmpy));
					tmpOdl.push_back(odlEuklidesowa);
			}
		odl.push_back(tmpOdl);
	}
			

	/*Œrednia odleg³oœæ miêdzy punktami*/ 
	for (int i = 0; i < odl.size(); i++) {
		for (int j = 0; j < odl[i].size(); j++) {
			if (odl[i][j] == -1)
				continue;
			sumaOdl = sumaOdl + odl[i][j];
			ilePkt++;
		}
	}

	odlSrednia = (sumaOdl / (ilePkt - odl.size()));
	double stopnie;

	/*K¹t miêdzy punktami konturu w stopniach*/
	for (int i = 0; i < contours.size(); i++) {
			tmpPoint = contours[i];
			vector<double> tmpKaty;
			for (int ii = 0; ii < contours.size(); ii++) {
					kat = atan2(contours[ii].y - tmpPoint.y, contours[ii].x - tmpPoint.x);
					kat = fmod(fmod(kat + 1e-5, 2 * M_PI) + 2 * M_PI, 2 * M_PI);
					kat = floor(kat * nbins / (2 * M_PI)); // + 1;
					tmpKaty.push_back(kat);
			}
		katy.push_back(tmpKaty);
	}
	
	double	nDist = (log10(r_outer) - log10(r_inner)) / (rbins - 1);
	for (int i = 0; i<rbins; i++)
		r_bins_edges[i] = pow(10, log10(r_inner) + nDist*i);

	/*Normalizacja promienia*/
	for (int i = 0; i < odl.size(); i++) {
		for (int j = 0; j < odl[i].size(); j++) {
			if(odl[i][j]!=0)
				odl[i][j] = odl[i][j] / odlSrednia;
		}
	}

	for (int i = 0; i < odl.size(); i++) {
		for (int j = 0; j < odl[i].size(); j++) {

			if (odl[i][j] == -1) 
				continue;
			for (int ki = 0; ki < rbins; ki++) {
				if (odl[i][j] <= r_bins_edges[ki]) {
					odl[i][j] = ki;
					break;
				}
			}
		}
	}

	vector<vector<float>> binPoints;
	vector<float> bins;

	for (int i = 0; i < odl.size(); i++) {
		vector<float> bin;
		for (int j = 0; j < nbins*rbins; j++) {
			bin.push_back(0);
		}
		binPoints.push_back(bin);
	}

	/*Zliczanie punktów w poszczególnych binach.*/
	for (int i = 0; i < odl.size(); i++) {
		for (int j = 0; j < odl[i].size(); j++) {
			if (odl[i][j] == -1) 
				continue;
			if (i == j)
				continue;
			int index =  odl[i][j] * nbins + katy[i][j];
			binPoints[i].at(index)++;
		}
	}
	
	for (int i = 0; i < binPoints.size(); i++) {
		bins.insert(bins.end(),binPoints[i].begin(),binPoints[i].end());
	}

	return bins;
}

/*Zapisanie shape context do pliku. Nale¿y podaæ œcie¿kê oraz wektor z nazwami plików np. P1Data.*/
void zapiszShapeContextDoPliku(const char *xml, vector<string> samples) {
	vector<vector<float>> rows; /*Wektor, w którym zapisywane s¹ obliczone deskryptory shape context.*/
	Mat img;
	for (int i = 0; i < samples.size(); i++) {
		img = imread(samples[i]);
		resize(img, img, Size(128,96));
		//img = morphology(img);
		rows.push_back(shapeContext(img, angleBins, radiusBins)); /*Ka¿dy wiersz wektora rows zawiera shape context dla jednego obrazu.*/
	}
	/*Stworzenie pliku xml, w którym zapisane bêd¹ obliczone wartoœci.*/
	FileStorage shapeContextXml(xml, FileStorage::WRITE); //FileStorage::READ
				
	/*row - liczba wierszy (zdjêæ); col - liczba kolumn (deskryptorów).*/
	int row = rows.size(), col = rows[0].size();
	std::printf("row=%d, col=%d\n", row, col);
	Mat M(row, col, CV_32F);

	/*Zapisanie Mat do XML.*/
	for (int i = 0; i< row; ++i)
		memcpy(&(M.data[col * i * sizeof(float)]), rows[i].data(), col * sizeof(float));

	/*Zapis pliku.*/
	cv::write(shapeContextXml, "Shape_Context", M);
	shapeContextXml.release();
	cout << "zakonczono" << '\n';
}

/*Odczyt shape context z plików oraz zapis klas poszczególnych zdjêæ bazy danych "p³otkarze".*/
vector<Mat> DescriptorMTX_ShapeContext_DB1() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/ /*Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.*/

	FileStorage read_1(dataBase1Path + "shapeContextP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase1Path + "shapeContextP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase1Path + "shapeContextP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase1Path + "shapeContextP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase1Path + "shapeContextP5.xml", FileStorage::READ);

	/*######################################################################*/

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5;;
	read_1["Shape_Context"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Shape_Context"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Shape_Context"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Shape_Context"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Shape_Context"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);

	/*######################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32S, 5);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32F, 5);
	labelsLogisticRegression.rowRange(0, pRow_1) = 1;
	labelsLogisticRegression.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 5, CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;
		else
			trainClasses.at<float>(i, 4) = 1.0;
	}

	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*Odczyt shape context z plików oraz zapis klas poszczególnych zdjêæ bazy danych "Actions as Space-Time Shapes".*/
vector<Mat> DescriptorMTX_ShapeContext_DB2() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/ /*Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.*/

	FileStorage read_1(dataBase2Path + "shapeContextP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase2Path + "shapeContextP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase2Path + "shapeContextP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase2Path + "shapeContextP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase2Path + "shapeContextP5.xml", FileStorage::READ);
	FileStorage read_6(dataBase2Path + "shapeContextP6.xml", FileStorage::READ);
	FileStorage read_7(dataBase2Path + "shapeContextP7.xml", FileStorage::READ);
	FileStorage read_8(dataBase2Path + "shapeContextP8.xml", FileStorage::READ);
	FileStorage read_9(dataBase2Path + "shapeContextP9.xml", FileStorage::READ);
	FileStorage read_10(dataBase2Path + "shapeContextP10.xml", FileStorage::READ);

	/*######################################################################*/

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5, pMat_6, pMat_7, pMat_8, pMat_9, pMat_10;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5, pRow_6, pCol_6, pRow_7, pCol_7, pRow_8, pCol_8, pRow_9, pCol_9, pRow_10, pCol_10;
	read_1["Shape_Context"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Shape_Context"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Shape_Context"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Shape_Context"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Shape_Context"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_6["Shape_Context"] >> pMat_6;
	pRow_6 = pMat_6.rows; pCol_6 = pMat_6.cols;

	read_7["Shape_Context"] >> pMat_7;
	pRow_7 = pMat_7.rows; pCol_7 = pMat_7.cols;

	read_8["Shape_Context"] >> pMat_8;
	pRow_8 = pMat_8.rows; pCol_8 = pMat_8.cols;

	read_9["Shape_Context"] >> pMat_9;
	pRow_9 = pMat_9.rows; pCol_9 = pMat_9.cols;

	read_10["Shape_Context"] >> pMat_10;
	pRow_10 = pMat_10.rows; pCol_10 = pMat_10.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();
	read_6.release();
	read_7.release();
	read_8.release();
	read_9.release();
	read_10.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);
	start = start + sizeof(float) * pMat_5.cols * pMat_5.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_6.data, sizeof(float) * pMat_6.cols * pMat_6.rows);
	start = start + sizeof(float) * pMat_6.cols * pMat_6.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_7.data, sizeof(float) * pMat_7.cols * pMat_7.rows);
	start = start + sizeof(float) * pMat_7.cols * pMat_7.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_8.data, sizeof(float) * pMat_8.cols * pMat_8.rows);
	start = start + sizeof(float) * pMat_8.cols * pMat_8.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_9.data, sizeof(float) * pMat_9.cols * pMat_9.rows);
	start = start + sizeof(float) * pMat_9.cols * pMat_9.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_10.data, sizeof(float) * pMat_10.cols * pMat_10.rows);
	
	/*######################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32S, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32F, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 10 , CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5))
			trainClasses.at<float>(i, 4) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6))
			trainClasses.at<float>(i, 5) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7))
			trainClasses.at<float>(i, 6) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8))
			trainClasses.at<float>(i, 7) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9))
			trainClasses.at<float>(i, 8) = 1.0;

		else
			trainClasses.at<float>(i, 9) = 1.0;
	}

	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*-----------------------------------------------------------------------------------------------*/

/*Shape context trening klasyfikatorów.*/

void nauczSVM_ShapeContext(string db,string path, int index) {
	vector<Mat> data;
	vector<Mat> dataTmp;
	
	if (db == std::string("db1")) {
		data = DescriptorMTX_ShapeContext_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_ShapeContext_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::SVM> svm = ml::SVM::create();
	svm->setType(ml::SVM::C_SVC);
	svm->setKernel(ml::SVM::LINEAR);
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
	svm->train(dataTmp[0], ROW_SAMPLE, dataTmp[1]);
	path = path + string("ShapeContextTrainedSVM.xml");
	svm->save(path);
	cout << "SVM zapisany" << '\n';
}

void nauczANN_ShapeContext(string db, string path, int index) {
	vector<Mat> data; /*indeks 0 - descrptor_mtx, 1-labels, 2-trainingClasses*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_ShapeContext_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_ShapeContext_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr< ANN_MLP >  ann = ANN_MLP::create();
	Mat layers = cv::Mat(3, 1, CV_32F);
	layers.row(0) = Scalar(dataTmp[0].cols);
	layers.row(1) = Scalar(7);

	if (db == std::string("db1")) {
		layers.row(2) = Scalar(5);
	}
	else if (db == std::string("db2")) {
		layers.row(2) = Scalar(10);
	}
	
	ann->setLayerSizes(layers);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM, 1.0, 1.0);
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
	ann->setBackpropMomentumScale(0.0001);
	ann->setBackpropWeightScale(0.03);
	ann->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, (int)3, 0.00001));
	ann->train(dataTmp[0], ROW_SAMPLE, dataTmp[2]);
	path = path + string("ShapeContextTrainedANN.xml");
	ann->save(path);
	cout << "ANN zapisany" << '\n';
}

void nauczLogisticRegression_ShapeContext(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;
	if (db == std::string("db1")) {
		data = DescriptorMTX_ShapeContext_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_ShapeContext_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	cv::Ptr<cv::ml::LogisticRegression> logisticRegression = cv::ml::LogisticRegression::create();
	logisticRegression->setLearningRate(0.5);
	logisticRegression->setIterations(100);
	logisticRegression->setRegularization(cv::ml::LogisticRegression::REG_L2);
	logisticRegression->setTrainMethod(cv::ml::LogisticRegression::BATCH);
	logisticRegression->setMiniBatchSize(1);
	logisticRegression->train(data[0], cv::ml::ROW_SAMPLE, data[3]);
	path = path + string("shapeContextTrainedLogisticRegression.xml");
	logisticRegression->save(path);
	cout << "zapisano logistic regression" << '\n';
}

void nauczRTrees_ShapeContext(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels, 2-trainingClasses*/
	vector<Mat> dataTmp;
	if (db == std::string("db1")) {
		data = DescriptorMTX_ShapeContext_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_ShapeContext_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::RTrees> RTrees = ml::RTrees::create();

	RTrees->setMaxDepth(10);
	RTrees->setMinSampleCount(10);
	RTrees->setRegressionAccuracy(0);
	RTrees->setUseSurrogates(false);
	RTrees->setMaxCategories(15);
	RTrees->setPriors(Mat());
	RTrees->setCalculateVarImportance(true);
	RTrees->setActiveVarCount(4);
	RTrees->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

	RTrees->train(data[0], ml::ROW_SAMPLE, data[1]);
	path = path + string("shapeContextTrainedRTrees.xml");
	RTrees->save(path);
	cout << "zapisano random trees" << endl;
}

int KNN_ShapeContext(Mat img, string db,int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels, 2-trainingClasses*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_ShapeContext_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_ShapeContext_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	cv::Ptr<cv::ml::KNearest> knn(cv::ml::KNearest::create());
	Ptr<TrainData> trainingData;
	trainingData = TrainData::create(dataTmp[0], ml::ROW_SAMPLE, dataTmp[1]);
	knn->setAlgorithmType(ml::KNearest::BRUTE_FORCE);
	knn->setIsClassifier(1);
	knn->setDefaultK(k);

	knn->train(trainingData);

	Mat result(0, 0, CV_32FC1);
	resize(img, img, Size(128,96));
	//img = morphology(img);
	knn->findNearest(shapeContext(img, angleBins, radiusBins), knn->getDefaultK(), result);
	return result.at<float>(Point(0,0));
}

/*-----------------------------------------------------------------------------------------------*/

/*Shape context klasyfikacja.*/
/*imgTmp - testowane zdjêcie, xml - œcie¿ka do pliku z zapisanym nauczonym klasyfikatorem.*/

float sprawdzImgSVM_ShapeContext(Mat img, string xml) {
	Ptr<ml::SVM> svmTmp = cv::Algorithm::load<ml::SVM>(xml);
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<float> shape = shapeContext(img, angleBins, radiusBins);
	float result = svmTmp->predict(shape);
	return result;
}

int sprawdzImgANN_ShapeContext(Mat img, string xml,string db) {
	Ptr<ml::ANN_MLP> annTmp = cv::Algorithm::load<ml::ANN_MLP>(xml);
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<float> shape = shapeContext(img, angleBins, radiusBins);
	int cls = -1;
	if (db == std::string("db1")) {
		Mat response = Mat::zeros(1, 5, CV_32F);

		annTmp->predict(shape, response);
		float max = -FLT_MAX;
		for (int i = 0; i < 5; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	else if (db == std::string("db2")) {
		Mat response = Mat::zeros(1, 10, CV_32F);

		annTmp->predict(shape, response);
		float max = -FLT_MAX;
		for (int i = 0; i < 10; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	return cls;
}

int sprawdzImgLogisticRegression_ShapeContext(Mat img, string xml) {
	cv::Ptr<cv::ml::LogisticRegression> logisticRegressionTmp = cv::Algorithm::load<cv::ml::LogisticRegression>(xml);
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<float> shape = shapeContext(img, angleBins, radiusBins);
	float result = logisticRegressionTmp->predict(shape);
	return result;
}

int sprawdzImgRTrees_ShapeContext(Mat img, string xml) {
	Ptr<ml::RTrees> RTreesTmp = cv::Algorithm::load<cv::ml::RTrees>(xml);
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<float> shape = shapeContext(img, angleBins, radiusBins);
	float result = RTreesTmp->predict(shape);
	return result;
}

/*############################################################################################################################################################################*/


/*HOG*/

/*Zapisanie HOG do pliku. Nale¿y podaæ œcie¿kê oraz wektor z nazwami plików np. P1Data.*/
void zapiszHogDoPliku(const char *xml, vector<string> samples) {
	vector< vector < float> > v_descriptorsValues; /*Wektor, w którym zapisywane s¹ obliczone deskryptory hog.*/
	vector< vector < Point> > v_locations;

	for (int i = 0; i< samples.size(); ++i) {
		Mat img, img_gray;
		Mat imgTmp = imread(samples[i]);

		resize(imgTmp, imgTmp, Size(128, 96));
		//img = morphology(imgTmp);
		img = imgTmp;
		vector<Point> contourTmp;
		vector<vector<Point>> contours;

		contourTmp = detectObject(img);
		contours.push_back(contourTmp);

		Mat mask = Mat::zeros(img.size(), CV_8UC1);
		drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
		img.copyTo(imgTmp, mask);

		HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
		vector< float> descriptorsValues;
		vector< Point> locations;
		d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

		v_descriptorsValues.push_back(descriptorsValues); /*Ka¿dy wiersz wektora v_descriptorsValues zawiera hog dla jednego obrazu.*/
		v_locations.push_back(locations);
	}
	/*Stworzenie pliku xml, w którym zapisane bêd¹ obliczone wartoœci.*/
	FileStorage hogXml(xml, FileStorage::WRITE);

	/*row - liczba wierszy (zdjêæ); col - liczba kolumn (deskryptorów).*/
	int row = v_descriptorsValues.size(), col = v_descriptorsValues[0].size();
	Mat M(row, col, CV_32F);

	/*Zapisanie Mat do XML.*/
	for (int i = 0; i< row; ++i)
		memcpy(&(M.data[col * i * sizeof(float)]), v_descriptorsValues[i].data(), col * sizeof(float));

	/*Zapis pliku.*/
	cv::write(hogXml, "Descriptor_of_images", M);
	hogXml.release();
	cout << "zapisano" << endl;
}

/*Odczyt HOG z plików oraz zapis klas poszczególnych zdjêæ bazy danych "p³otkarze".*/
vector<Mat> DescriptorMTX_HOG_DB1() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/ /*Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.*/

	FileStorage read_1(dataBase1Path + "HOGP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase1Path + "HOGP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase1Path + "HOGP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase1Path + "HOGP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase1Path + "HOGP5.xml", FileStorage::READ);

	/*##################################################################*/

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5;;
	read_1["Descriptor_of_images"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Descriptor_of_images"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Descriptor_of_images"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Descriptor_of_images"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Descriptor_of_images"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);

	/*##################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32S, 5);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, 1, CV_32F, 5);
	labelsLogisticRegression.rowRange(0, pRow_1) = 1;
	labelsLogisticRegression.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labelsLogisticRegression.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 5, CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;
		else
			trainClasses.at<float>(i, 4) = 1.0;
	}

	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*Odczyt HOG z plików oraz zapis klas poszczególnych zdjêæ bazy danych "Actions as Space-Time Shapes".*/
vector<Mat> DescriptorMTX_HOG_DB2() {
	vector<Mat> descriptors_labels; /*Macierz zawieraj¹ca wspó³czynniki, oraz klasy.*/ /*Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.*/

	FileStorage read_1(dataBase2Path + "HOGP1.xml", FileStorage::READ);
	FileStorage read_2(dataBase2Path + "HOGP2.xml", FileStorage::READ);
	FileStorage read_3(dataBase2Path + "HOGP3.xml", FileStorage::READ);
	FileStorage read_4(dataBase2Path + "HOGP4.xml", FileStorage::READ);
	FileStorage read_5(dataBase2Path + "HOGP5.xml", FileStorage::READ);
	FileStorage read_6(dataBase2Path + "HOGP6.xml", FileStorage::READ);
	FileStorage read_7(dataBase2Path + "HOGP7.xml", FileStorage::READ);
	FileStorage read_8(dataBase2Path + "HOGP8.xml", FileStorage::READ);
	FileStorage read_9(dataBase2Path + "HOGP9.xml", FileStorage::READ);
	FileStorage read_10(dataBase2Path + "HOGP10.xml", FileStorage::READ);

	/*######################################################################*/

	Mat pMat_1, pMat_2, pMat_3, pMat_4, pMat_5, pMat_6, pMat_7, pMat_8, pMat_9, pMat_10;
	int pRow_1, pCol_1, pRow_2, pCol_2, pRow_3, pCol_3, pRow_4, pCol_4, pRow_5, pCol_5, pRow_6, pCol_6, pRow_7, pCol_7, pRow_8, pCol_8, pRow_9, pCol_9, pRow_10, pCol_10;
	read_1["Descriptor_of_images"] >> pMat_1;
	pRow_1 = pMat_1.rows; pCol_1 = pMat_1.cols;

	read_2["Descriptor_of_images"] >> pMat_2;
	pRow_2 = pMat_2.rows; pCol_2 = pMat_2.cols;

	read_3["Descriptor_of_images"] >> pMat_3;
	pRow_3 = pMat_3.rows; pCol_3 = pMat_3.cols;

	read_4["Descriptor_of_images"] >> pMat_4;
	pRow_4 = pMat_4.rows; pCol_4 = pMat_4.cols;

	read_5["Descriptor_of_images"] >> pMat_5;
	pRow_5 = pMat_5.rows; pCol_5 = pMat_5.cols;

	read_6["Descriptor_of_images"] >> pMat_6;
	pRow_6 = pMat_6.rows; pCol_6 = pMat_6.cols;

	read_7["Descriptor_of_images"] >> pMat_7;
	pRow_7 = pMat_7.rows; pCol_7 = pMat_7.cols;

	read_8["Descriptor_of_images"] >> pMat_8;
	pRow_8 = pMat_8.rows; pCol_8 = pMat_8.cols;

	read_9["Descriptor_of_images"] >> pMat_9;
	pRow_9 = pMat_9.rows; pCol_9 = pMat_9.cols;

	read_10["Descriptor_of_images"] >> pMat_10;
	pRow_10 = pMat_10.rows; pCol_10 = pMat_10.cols;

	read_1.release();
	read_2.release();
	read_3.release();
	read_4.release();
	read_5.release();
	read_6.release();
	read_7.release();
	read_8.release();
	read_9.release();
	read_10.release();

	Mat Descriptor_mtx(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, pCol_1, CV_32F);

	memcpy(&(Descriptor_mtx.data[0]), pMat_1.data, sizeof(float) * pMat_1.cols * pMat_1.rows);
	int start = sizeof(float) * pMat_1.cols * pMat_1.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_2.data, sizeof(float) * pMat_2.cols * pMat_2.rows);
	start = start + sizeof(float) * pMat_2.cols * pMat_2.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_3.data, sizeof(float) * pMat_3.cols * pMat_3.rows);
	start = start + sizeof(float) * pMat_3.cols * pMat_3.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_4.data, sizeof(float) * pMat_4.cols * pMat_4.rows);
	start = start + sizeof(float) * pMat_4.cols * pMat_4.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_5.data, sizeof(float) * pMat_5.cols * pMat_5.rows);
	start = start + sizeof(float) * pMat_5.cols * pMat_5.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_6.data, sizeof(float) * pMat_6.cols * pMat_6.rows);
	start = start + sizeof(float) * pMat_6.cols * pMat_6.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_7.data, sizeof(float) * pMat_7.cols * pMat_7.rows);
	start = start + sizeof(float) * pMat_7.cols * pMat_7.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_8.data, sizeof(float) * pMat_8.cols * pMat_8.rows);
	start = start + sizeof(float) * pMat_8.cols * pMat_8.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_9.data, sizeof(float) * pMat_9.cols * pMat_9.rows);
	start = start + sizeof(float) * pMat_9.cols * pMat_9.rows;

	memcpy(&(Descriptor_mtx.data[start]), pMat_10.data, sizeof(float) * pMat_10.cols * pMat_10.rows);

	/*######################################################################*/

	Mat labels(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32S, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat labelsLogisticRegression(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9 + pRow_10, 1, CV_32F, 10);
	labels.rowRange(0, pRow_1) = 1;
	labels.rowRange(pRow_1, pRow_1 + pRow_2) = 2;
	labels.rowRange(pRow_1 + pRow_2, pRow_1 + pRow_2 + pRow_3) = 3;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3, pRow_1 + pRow_2 + pRow_3 + pRow_4) = 4;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) = 5;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) = 6;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) = 7;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) = 8;
	labels.rowRange(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8, pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9) = 9;

	/*######################################################################*/

	Mat trainClasses = Mat::zeros(Descriptor_mtx.rows, 10, CV_32F);

	for (int i = 0; i < trainClasses.rows; i++) {
		if (i < pRow_1)
			trainClasses.at<float>(i, 0) = 1.0;

		else if (i >= pRow_1 && i<(pRow_1 + pRow_2))
			trainClasses.at<float>(i, 1) = 1.0;

		else if (i >= (pRow_1 + pRow_2) && i<(pRow_1 + pRow_2 + pRow_3))
			trainClasses.at<float>(i, 2) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4))
			trainClasses.at<float>(i, 3) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5))
			trainClasses.at<float>(i, 4) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6))
			trainClasses.at<float>(i, 5) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7))
			trainClasses.at<float>(i, 6) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8))
			trainClasses.at<float>(i, 7) = 1.0;

		else if (i >= (pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8) && i<(pRow_1 + pRow_2 + pRow_3 + pRow_4 + pRow_5 + pRow_6 + pRow_7 + pRow_8 + pRow_9))
			trainClasses.at<float>(i, 8) = 1.0;

		else
			trainClasses.at<float>(i, 9) = 1.0;
	}
	/*######################################################################*/

	descriptors_labels.push_back(Descriptor_mtx);
	descriptors_labels.push_back(labels);
	descriptors_labels.push_back(trainClasses);
	descriptors_labels.push_back(labelsLogisticRegression);
	return descriptors_labels;
}

/*-----------------------------------------------------------------------------------------------*/

/*HOG trening klasyfikatorów.*/

void nauczSVM_HOG(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_HOG_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_HOG_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::SVM> svm = ml::SVM::create();
	svm->setType(ml::SVM::C_SVC);
	svm->setKernel(ml::SVM::LINEAR);
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

	svm->train(dataTmp[0], ROW_SAMPLE, dataTmp[1]);
	path = path + string("HOGTrainedSVM.xml");
	svm->save(path);
	cout << "SVM zapisany" << '\n';
}

void nauczANN_HOG(string db, string path,int index) {
	vector<Mat>data ;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_HOG_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_HOG_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr< ANN_MLP >  ann = ANN_MLP::create();
	Mat layers = cv::Mat(3, 1, CV_32F);
	layers.row(0) = Scalar(dataTmp[0].cols);
	layers.row(1) = Scalar(7);
	if (db == std::string("db1")) {
		layers.row(2) = Scalar(5);
	}
	else if (db == std::string("db2")) {
		layers.row(2) = Scalar(10);
	}

	ann->setLayerSizes(layers);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM,1.0,1.0);
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
	ann->setBackpropMomentumScale(0.0001);
	ann->setBackpropWeightScale(0.03);
	ann->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, (int)3, 0.00001));
	ann->train(dataTmp[0], ROW_SAMPLE, dataTmp[2]);
	path = path + string("HOGTrainedANN.xml");
	ann->save(path);
	cout << "ANN zapisany" << '\n';
}

void nauczLogisticRegression_HOG(string db, string path,int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_HOG_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_HOG_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	cv::Ptr<cv::ml::LogisticRegression> logisticRegression = cv::ml::LogisticRegression::create();
	logisticRegression->setLearningRate(0.5);
	logisticRegression->setIterations(10);
	logisticRegression->setRegularization(cv::ml::LogisticRegression::REG_L2);
	logisticRegression->setTrainMethod(cv::ml::LogisticRegression::BATCH);
	logisticRegression->setMiniBatchSize(1);
	logisticRegression->train(dataTmp[0], cv::ml::ROW_SAMPLE, dataTmp[3]);
	path = path + string("HOGTrainedLogisticRegression.xml");
	logisticRegression->save(path);
	cout << "zapisano logistic regression" << '\n';
}

void nauczRTrees_HOG(string db, string path, int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_HOG_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_HOG_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	Ptr<ml::RTrees> RTrees = ml::RTrees::create();

	RTrees->setMaxDepth(1);
	RTrees->setMinSampleCount(10);
	RTrees->setRegressionAccuracy(0);
	RTrees->setUseSurrogates(false);
	RTrees->setMaxCategories(15);
	RTrees->setPriors(Mat());
	RTrees->setCalculateVarImportance(true);
	RTrees->setActiveVarCount(4);
	RTrees->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 1, 0.01f));

	RTrees->train(dataTmp[0], ml::ROW_SAMPLE, dataTmp[1]);
	path = path + string("HOGTrainedRTrees.xml");
	RTrees->save(path);
	cout << "zapisano random trees" << endl;
}

int KNN_HOG(Mat img, string db,int index) {
	vector<Mat> data;/*indeks 0 - descrptor_mtx, 1-labels int, 2-trainingClasses, 3 - labels float*/
	vector<Mat> dataTmp;

	if (db == std::string("db1")) {
		data = DescriptorMTX_HOG_DB1();
		dataTmp = zapiszDaneUczaceTestoweDB1(dataTmp, data, index);
	}
	else if (db == std::string("db2")) {
		data = DescriptorMTX_HOG_DB2();
		dataTmp = zapiszDaneUczaceTestoweDB2(dataTmp, data, index);
	}

	cv::Ptr<cv::ml::KNearest> knn(cv::ml::KNearest::create());
	Ptr<TrainData> trainingData;
	trainingData = TrainData::create(dataTmp[0], ml::ROW_SAMPLE, dataTmp[1]);
	knn->setAlgorithmType(ml::KNearest::BRUTE_FORCE);
	knn->setIsClassifier(1);
	knn->setDefaultK(k);

	knn->train(trainingData);

	Mat result(0, 0, CV_32FC1);

	Mat imgGray;
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	Mat mask = Mat::zeros(img.size(), CV_8UC1);
	drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
	Mat imgTmp;
	img.copyTo(imgTmp, mask);

	HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;

	d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

	Mat fm = Mat(descriptorsValues).reshape(1, 1260);
	Mat fm1 = fm.t();
	fm1.convertTo(fm, CV_32F);

	knn->findNearest(fm1, knn->getDefaultK(), result);
	return result.at<float>(Point(0, 0));
}

/*-----------------------------------------------------------------------------------------------*/

/*Shape context klasyfikacja.*/
/*imgTmp - testowane zdjêcie, xml - œcie¿ka do pliku z zapisanym nauczonym klasyfikatorem.*/

float sprawdzImgSVM_HOG(Mat img, string xml) {
	Ptr<ml::SVM> svmTmp = cv::Algorithm::load<ml::SVM>(xml);
	Mat imgGray;
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	Mat mask = Mat::zeros(img.size(), CV_8UC1);
	drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
	Mat imgTmp;
	img.copyTo(imgTmp, mask);

	HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;

	d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

	Mat fm = Mat(descriptorsValues).reshape(1, 1260);
	Mat fm1 = fm.t();
	fm1.convertTo(fm, CV_32F);

	float result = svmTmp->predict(fm1);
	return result;
}

int sprawdzImgANN_HOG(Mat img, string xml, string db) {
	Ptr<ml::ANN_MLP> annTmp = cv::Algorithm::load<ml::ANN_MLP>(xml);
	Mat imgGray;
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	Mat mask = Mat::zeros(img.size(), CV_8UC1);
	drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
	Mat imgTmp;
	img.copyTo(imgTmp, mask);

	HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;

	d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

	Mat fm = Mat(descriptorsValues).reshape(1, 1260);

	Mat fm1 = fm.t();
	fm1.convertTo(fm1, CV_32F);

	int cls = -1;
	if (db == std::string("db1")) {
		Mat response = Mat::zeros(1, 5, CV_32F);

		annTmp->predict(fm1, response);
		float max = -FLT_MAX;
		for (int i = 0; i < 5; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	else if (db == std::string("db2")) {
		Mat response = Mat::zeros(1, 10, CV_32F);

		annTmp->predict(fm1, response);
		float max = -FLT_MAX;
		for (int i = 0; i < 10; i++) {
			float value(response.at<float>(0, i));
			if (value > max) {
				max = value;
				cls = i + 1;
			}
		}
	}
	return cls;
}

int sprawdzImgLogisticRegression_HOG(Mat img, string xml) {
	cv::Ptr<cv::ml::LogisticRegression> logisticRegressionTmp = cv::Algorithm::load<cv::ml::LogisticRegression>(xml);
	Mat imgGray;
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	Mat mask = Mat::zeros(img.size(), CV_8UC1);
	drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
	Mat imgTmp;
	img.copyTo(imgTmp, mask);

	HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;

	d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

	Mat fm = Mat(descriptorsValues).reshape(1, 1260);
	Mat fm1 = fm.t();
	fm1.convertTo(fm1, CV_32F);

	int result = logisticRegressionTmp->predict(fm1);
	return result;
}

int sprawdzImgRTrees_HOG(Mat img, string xml) {
	Ptr<ml::RTrees> RTreesTmp = cv::Algorithm::load<cv::ml::RTrees>(xml);
	Mat imgGray;
	resize(img, img, Size(128, 96));
	//img = morphology(img);
	vector<Point> contourTmp;
	vector<vector<Point>> contours;

	contourTmp = detectObject(img);
	contours.push_back(contourTmp);

	Mat mask = Mat::zeros(img.size(), CV_8UC1);
	drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
	Mat imgTmp;
	img.copyTo(imgTmp, mask);

	HOGDescriptor d(imgTmp.size(), Size(32, 32), Size(16, 16), Size(16, 16), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;

	d.compute(imgTmp, descriptorsValues, Size(8, 8), Size(0, 0), locations);

	Mat fm = Mat(descriptorsValues).reshape(1, 1260);
	Mat fm1 = fm.t();
	fm1.convertTo(fm1, CV_32F);
	float result = RTreesTmp->predict(fm1);
	return result;
}

/*############################################################################################################################################################################*/

/*Zapis do plików obliczonych deskryptorów shape context bazy danych "p³otkarze".*/
void zapiszPlikiShapeContext_DB1() {
	zapiszShapeContextDoPliku((dataBase1Path + "shapeContextP1.xml").c_str(), P1Data);
	zapiszShapeContextDoPliku((dataBase1Path + "shapeContextP2.xml").c_str(), P2Data);
	zapiszShapeContextDoPliku((dataBase1Path + "shapeContextP3.xml").c_str(), P3Data);
	zapiszShapeContextDoPliku((dataBase1Path + "shapeContextP4.xml").c_str(), P4Data);
	zapiszShapeContextDoPliku((dataBase1Path + "shapeContextP5.xml").c_str(), P5Data);
}

/*Zapis do plików obliczonych deskryptorów hog bazy danych "p³otkarze".*/
void zapiszPlikiHOG_DB1() {
	zapiszHogDoPliku((dataBase1Path + "HOGP1.xml").c_str(), P1Data);
	zapiszHogDoPliku((dataBase1Path + "HOGP2.xml").c_str(), P2Data);
	zapiszHogDoPliku((dataBase1Path + "HOGP3.xml").c_str(), P3Data);
	zapiszHogDoPliku((dataBase1Path + "HOGP4.xml").c_str(), P4Data);
	zapiszHogDoPliku((dataBase1Path + "HOGP5.xml").c_str(), P5Data);
}

/*Zapis do plików obliczonych wspó³czynników kszta³tu bazy danych "p³otkarze".*/
void zapiszPlikiWspKsztaltu_DB1() {
	zapiszWspDoPliku((dataBase1Path + "wspKsztaltuP1.xml").c_str(), P1Data);
	zapiszWspDoPliku((dataBase1Path + "wspKsztaltuP2.xml").c_str(), P2Data);
	zapiszWspDoPliku((dataBase1Path + "wspKsztaltuP3.xml").c_str(), P3Data);
	zapiszWspDoPliku((dataBase1Path + "wspKsztaltuP4.xml").c_str(), P4Data);
	zapiszWspDoPliku((dataBase1Path + "wspKsztaltuP5.xml").c_str(), P5Data);
}

/*Zapis do plików obliczonych deskryptorów shape context bazy danych "Actions as Space-Time Shapes".*/
void zapiszPlikiShapeContext_DB2() {
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP1.xml").c_str(), P1Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP2.xml").c_str(), P2Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP3.xml").c_str(), P3Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP4.xml").c_str(), P4Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP5.xml").c_str(), P5Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP6.xml").c_str(), P6Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP7.xml").c_str(), P7Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP8.xml").c_str(), P8Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP9.xml").c_str(), P9Data);
	zapiszShapeContextDoPliku((dataBase2Path + "shapeContextP10.xml").c_str(), P10Data);
}

/*Zapis do plików obliczonych deskryptorów hog bazy danych "Actions as Space-Time Shapes".*/
void zapiszPlikiHOG_DB2() {
	zapiszHogDoPliku((dataBase2Path + "HOGP1.xml").c_str(), P1Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP2.xml").c_str(), P2Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP3.xml").c_str(), P3Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP4.xml").c_str(), P4Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP5.xml").c_str(), P5Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP6.xml").c_str(), P6Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP7.xml").c_str(), P7Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP8.xml").c_str(), P8Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP9.xml").c_str(), P9Data);
	zapiszHogDoPliku((dataBase2Path + "HOGP10.xml").c_str(), P10Data);
}

/*Zapis do plików obliczonych wspó³czynników kszta³tu bazy danych "Actions as Space-Time Shapes".*/
void zapiszPlikiWspKsztaltu_DB2() {
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP1.xml").c_str(), P1Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP2.xml").c_str(), P2Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP3.xml").c_str(), P3Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP4.xml").c_str(), P4Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP5.xml").c_str(), P5Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP6.xml").c_str(), P6Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP7.xml").c_str(), P7Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP8.xml").c_str(), P8Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP9.xml").c_str(), P9Data);
	zapiszWspDoPliku((dataBase2Path + "wspKsztaltuP10.xml").c_str(), P10Data);
}

int main(int argc, char *argv[]){
	cout << -FLT_MAX << endl;
	validExtensions.push_back("jpg");
	validExtensions.push_back("png");
	validExtensions.push_back("JPG");
	validExtensions.push_back("PNG");

	if (argv[1] == std::string("help")) {
		cout << "Do uruchomienia programu potrzebne jest polecenie w postaci \"baza_danych\" \"deskryptor\" \"sciezka pliku zapisu\"." << "\n";
		cout << "Przykladowy schemat : shapeContext1 \"db1\" \"hog\" \"hog.txt\"." << "\n";
		cout << "\n";
		cout << "W celu uruchomienia pomocy nalezy uzyc polecenia \"help\":" << "\n";
		cout << "shapeContex1 \"help\"" << "\n";
		return 1;
	}
	else if (argv[1] == std::string("db1")) {
			float resultANN, resultSVM, resultKNN;
			float ANN[10][5], SVM[10][5], KNN[10][5];
			double skutecznoscKNN[10][5], skutecznoscSVM[10][5], skutecznoscANN[10][5];
			if (argv[2] == std::string("shape context")) {

				ofstream outfile(dataBase1Path + std::string(argv[3]));
				fstream file;
				file.open(dataBase1Path + std::string(argv[3]), fstream::app);
				zapiszPlikiDanychDB1(dataPathDB1);
				zapiszPlikiShapeContext_DB1();
				for (int j = 0; j < 10; j++) {

					nauczSVM_ShapeContext(std::string(argv[1]), dataBase1Path.c_str(), j + 1);
					nauczANN_ShapeContext(std::string(argv[1]), dataBase1Path.c_str(), j + 1);

					for (int k = 0; k < 5; k++) {
						for (int i = 0; i < 5; i++) {
							ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
						}
						for (int i = 0; i < daneTestowe[k].size(); i++) {
							Mat img = imread(daneTestowe[k][i]);

							resultKNN = KNN_ShapeContext(img, std::string(argv[1]), j + 1);
							resultANN = sprawdzImgANN_ShapeContext(img, dataBase1Path + "ShapeContextTrainedANN.xml", std::string(argv[1]));
							resultSVM = sprawdzImgSVM_ShapeContext(img, dataBase1Path + "ShapeContextTrainedSVM.xml");

							if (resultKNN == 1) KNN[j][0]++;
							else if (resultKNN == 2) KNN[j][1]++;
							else if (resultKNN == 3) KNN[j][2]++;
							else if (resultKNN == 4) KNN[j][3]++;
							else if (resultKNN == 5) KNN[j][4]++;

							if (resultANN == 1) ANN[j][0]++;
							else if (resultANN == 2) ANN[j][1]++;
							else if (resultANN == 3) ANN[j][2]++;
							else if (resultANN == 4) ANN[j][3]++;
							else if (resultANN == 5) ANN[j][4]++;

							if (resultSVM == 1) SVM[j][0]++;
							else if (resultSVM == 2) SVM[j][1]++;
							else if (resultSVM == 3) SVM[j][2]++;
							else if (resultSVM == 4) SVM[j][3]++;
							else if (resultSVM == 5) SVM[j][4]++;
						}
						file << "numer iteracji - " << j + 1 << endl;
						file << "indeks sprawdzanej pozy - " << k + 1 << endl;
						file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

						file << "KNN" << endl;
						file << "P1 - " << KNN[j][0] << endl;
						file << "P2 - " << KNN[j][1] << endl;
						file << "P3 - " << KNN[j][2] << endl;
						file << "P4 - " << KNN[j][3] << endl;
						file << "P5 - " << KNN[j][4] << endl;
						file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

						file << "ANN" << endl;
						file << "P1 - " << ANN[j][0] << endl;
						file << "P2 - " << ANN[j][1] << endl;
						file << "P3 - " << ANN[j][2] << endl;
						file << "P4 - " << ANN[j][3] << endl;
						file << "P5 - " << ANN[j][4] << endl;
						file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());
						file << "SVM" << endl;
						file << "P1 - " << SVM[j][0] << endl;
						file << "P2 - " << SVM[j][1] << endl;
						file << "P3 - " << SVM[j][2] << endl;
						file << "P4 - " << SVM[j][3] << endl;
						file << "P5 - " << SVM[j][4] << endl;
						file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
						file << endl << endl;
					}
					file << endl << endl;
				}
				float svmSrednia[5], knnSrednia[5], annSrednia[5];
				for (int jj = 0; jj < 5; jj++) {
					svmSrednia[jj] = 0;
					knnSrednia[jj] = 0;
					annSrednia[jj] = 0;
				}

				for (int ii = 0; ii < 5; ii++) {
					double svmSuma = 0, knnSuma = 0, annSuma = 0;
					for (int jj = 0; jj < 10; jj++) {
						svmSuma = svmSuma + skutecznoscSVM[jj][ii];
						annSuma = annSuma + skutecznoscANN[jj][ii];
						knnSuma = knnSuma + skutecznoscKNN[jj][ii];
					}
					file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
					file << "SVM - " << (svmSuma / 10) << endl;
					file << "KNN - " << (knnSuma / 10) << endl;
					file << "ANN - " << (annSuma / 10) << endl;
				}
				file.close();
			}

			else if (argv[2] == std::string("hog")) {
				ofstream outfile(dataBase1Path + std::string(argv[3]));
				fstream file;
				file.open(dataBase1Path + std::string(argv[3]), fstream::app);
				zapiszPlikiDanychDB1(dataPathDB1);
				zapiszPlikiHOG_DB1();
				for (int j = 0; j < 10; j++) {
					nauczSVM_HOG(std::string(argv[1]), dataBase1Path.c_str(), j + 1);
					nauczANN_HOG(std::string(argv[1]), dataBase1Path.c_str(), j + 1);

					for (int k = 0; k < 5; k++) {
						for (int i = 0; i < 5; i++) {
							ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
						}

						for (int i = 0; i < daneTestowe[k].size(); i++) {
							Mat img = imread(daneTestowe[k][i]);

							resultKNN = KNN_HOG(img, std::string(argv[1]), j + 1);
							resultANN = sprawdzImgANN_HOG(img, dataBase1Path + "HOGTrainedANN.xml", std::string(argv[1]));
							resultSVM = sprawdzImgSVM_HOG(img, dataBase1Path + "HOGTrainedSVM.xml");
							//cout << resultSVM << endl;
							if (resultKNN == 1) KNN[j][0]++;
							else if (resultKNN == 2) KNN[j][1]++;
							else if (resultKNN == 3) KNN[j][2]++;
							else if (resultKNN == 4) KNN[j][3]++;
							else if (resultKNN == 5) KNN[j][4]++;

							if (resultANN == 1) ANN[j][0]++;
							else if (resultANN == 2) ANN[j][1]++;
							else if (resultANN == 3) ANN[j][2]++;
							else if (resultANN == 4) ANN[j][3]++;
							else if (resultANN == 5) ANN[j][4]++;

							if (resultSVM == 1) SVM[j][0]++;
							else if (resultSVM == 2) SVM[j][1]++;
							else if (resultSVM == 3) SVM[j][2]++;
							else if (resultSVM == 4) SVM[j][3]++;
							else if (resultSVM == 5) SVM[j][4]++;
						}
						file << "numer iteracji - " << j + 1 << endl;
						file << "indeks sprawdzanej pozy - " << k + 1 << endl;
						file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

						file << "KNN" << endl;
						file << "P1 - " << KNN[j][0] << endl;
						file << "P2 - " << KNN[j][1] << endl;
						file << "P3 - " << KNN[j][2] << endl;
						file << "P4 - " << KNN[j][3] << endl;
						file << "P5 - " << KNN[j][4] << endl;
						file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

						file << "ANN" << endl;
						file << "P1 - " << ANN[j][0] << endl;
						file << "P2 - " << ANN[j][1] << endl;
						file << "P3 - " << ANN[j][2] << endl;
						file << "P4 - " << ANN[j][3] << endl;
						file << "P5 - " << ANN[j][4] << endl;
						file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());
						file << "SVM" << endl;
						file << "P1 - " << SVM[j][0] << endl;
						file << "P2 - " << SVM[j][1] << endl;
						file << "P3 - " << SVM[j][2] << endl;
						file << "P4 - " << SVM[j][3] << endl;
						file << "P5 - " << SVM[j][4] << endl;
						file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
						file << endl << endl;
					}
					file << endl << endl;
				}
				float svmSrednia[5], knnSrednia[5], annSrednia[5];
				for (int jj = 0; jj < 5; jj++) {
					svmSrednia[jj] = 0;
					knnSrednia[jj] = 0;
					annSrednia[jj] = 0;
				}

				for (int ii = 0; ii < 5; ii++) {
					double svmSuma = 0, knnSuma = 0, annSuma = 0;
					for (int jj = 0; jj < 10; jj++) {
						svmSuma = svmSuma + skutecznoscSVM[jj][ii];
						annSuma = annSuma + skutecznoscANN[jj][ii];
						knnSuma = knnSuma + skutecznoscKNN[jj][ii];
					}
					file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
					file << "SVM - " << (svmSuma / 10) << endl;
					file << "KNN - " << (knnSuma / 10) << endl;
					file << "ANN - " << (annSuma / 10) << endl;
				}
				file.close();
			}

			else if (argv[2] == std::string("wsp")) {
				wsp = std::string(argv[4]);
				ofstream outfile(dataBase1Path + std::string(argv[3]));
				fstream file;
				file.open(dataBase1Path + std::string(argv[3]), fstream::app);
				zapiszPlikiDanychDB1(dataPathDB1);
				zapiszPlikiWspKsztaltu_DB1();
				for (int j = 0; j < 10; j++) {
					nauczSVM_wsp(std::string(argv[1]), dataBase1Path.c_str(), j + 1);
					nauczANN_wsp(std::string(argv[1]), dataBase1Path.c_str(), j + 1);

					for (int k = 0; k < 5; k++) {
						for (int i = 0; i < 5; i++) {
							ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
						}

						for (int i = 0; i < daneTestowe[k].size(); i++) {
							Mat img = imread(daneTestowe[k][i]);
							cout << daneTestowe[k][i] << endl;
							resultKNN = KNN_wsp(img, std::string(argv[1]), j + 1);
							resultANN = sprawdzImgANN_wsp(img, dataBase1Path + "wspKsztaltuTrainedANN.xml", std::string(argv[1]));
							resultSVM = sprawdzImgSVM_wsp(img, dataBase1Path + "wspKsztaltuTrainedSVM.xml");
							//cout << resultKNN << endl;
							if (resultKNN == 1) KNN[j][0]++;
							else if (resultKNN == 2) KNN[j][1]++;
							else if (resultKNN == 3) KNN[j][2]++;
							else if (resultKNN == 4) KNN[j][3]++;
							else if (resultKNN == 5) KNN[j][4]++;

							if (resultANN == 1) ANN[j][0]++;
							else if (resultANN == 2) ANN[j][1]++;
							else if (resultANN == 3) ANN[j][2]++;
							else if (resultANN == 4) ANN[j][3]++;
							else if (resultANN == 5) ANN[j][4]++;

							if (resultSVM == 1) SVM[j][0]++;
							else if (resultSVM == 2) SVM[j][1]++;
							else if (resultSVM == 3) SVM[j][2]++;
							else if (resultSVM == 4) SVM[j][3]++;
							else if (resultSVM == 5) SVM[j][4]++;
						}
						file << "numer iteracji - " << j + 1 << endl;
						file << "indeks sprawdzanej pozy - " << k + 1 << endl;
						file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

						file << "KNN" << endl;
						file << "P1 - " << KNN[j][0] << endl;
						file << "P2 - " << KNN[j][1] << endl;
						file << "P3 - " << KNN[j][2] << endl;
						file << "P4 - " << KNN[j][3] << endl;
						file << "P5 - " << KNN[j][4] << endl;
						file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

						file << "ANN" << endl;
						file << "P1 - " << ANN[j][0] << endl;
						file << "P2 - " << ANN[j][1] << endl;
						file << "P3 - " << ANN[j][2] << endl;
						file << "P4 - " << ANN[j][3] << endl;
						file << "P5 - " << ANN[j][4] << endl;
						file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());
						file << "SVM" << endl;
						file << "P1 - " << SVM[j][0] << endl;
						file << "P2 - " << SVM[j][1] << endl;
						file << "P3 - " << SVM[j][2] << endl;
						file << "P4 - " << SVM[j][3] << endl;
						file << "P5 - " << SVM[j][4] << endl;
						file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
						skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
						file << endl << endl;
					}
					file << endl << endl;
				}
				for (int ii = 0; ii < 5; ii++) {
					double svmSuma=0, knnSuma=0, annSuma=0;
					for (int jj = 0; jj < 10; jj++) {
						svmSuma = svmSuma + skutecznoscSVM[jj][ii];
						annSuma = annSuma + skutecznoscANN[jj][ii];
						knnSuma = knnSuma + skutecznoscKNN[jj][ii];
					}
					file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
					file << "SVM - " << (svmSuma / 10) << endl;
					file << "KNN - " << (knnSuma / 10) << endl;
					file << "ANN - " << (annSuma / 10) << endl;
				}
				file.close();
			}

			else {
				cout << "Niepoprawna nazwa deskryptora. " << "\n";
				cout << "mozliwe opcje to : " << "\n";
				cout << "shape context \nhog \nwsp" << "\n";
			}
	}
	else if (argv[1] == std::string("db2")){
		float resultANN, resultSVM, resultKNN;
		float ANN[10][10], SVM[10][10], KNN[10][10];
		double skutecznoscKNN[10][10], skutecznoscSVM[10][10], skutecznoscANN[10][10];
		if (argv[2] == std::string("shape context")) {
			ofstream outfile(dataBase2Path + std::string(argv[3]));
			fstream file;
			file.open(dataBase2Path + std::string(argv[3]), fstream::app);
			zapiszPlikiDanychDB2(dataPathDB2);
			zapiszPlikiShapeContext_DB2();
			for (int j = 0; j < 10; j++) {
				nauczSVM_ShapeContext(std::string(argv[1]), dataBase2Path.c_str(), j + 1);
				nauczANN_ShapeContext(std::string(argv[1]), dataBase2Path.c_str(), j + 1);

				for (int k = 0; k < 10; k++) {
					for (int i = 0; i < 10; i++) {
						ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
					}

					for (int i = 0; i < daneTestowe[k].size(); i++) {
						Mat img = imread(daneTestowe[k][i]);

						resultKNN = KNN_ShapeContext(img, std::string(argv[1]), j + 1);
						resultANN = sprawdzImgANN_ShapeContext(img, dataBase2Path + "ShapeContextTrainedANN.xml", std::string(argv[1]));
						resultSVM = sprawdzImgSVM_ShapeContext(img, dataBase2Path + "ShapeContextTrainedSVM.xml");
						//cout << resultSVM << endl;
						if (resultKNN == 1) KNN[j][0]++;
						else if (resultKNN == 2) KNN[j][1]++;
						else if (resultKNN == 3) KNN[j][2]++;
						else if (resultKNN == 4) KNN[j][3]++;
						else if (resultKNN == 5) KNN[j][4]++;
						else if (resultKNN == 6) KNN[j][5]++;
						else if (resultKNN == 7) KNN[j][6]++;
						else if (resultKNN == 8) KNN[j][7]++;
						else if (resultKNN == 9) KNN[j][8]++;
						else if (resultKNN == 10) KNN[j][9]++;

						if (resultANN == 1) ANN[j][0]++;
						else if (resultANN == 2) ANN[j][1]++;
						else if (resultANN == 3) ANN[j][2]++;
						else if (resultANN == 4) ANN[j][3]++;
						else if (resultANN == 5) ANN[j][4]++;
						else if (resultANN == 6) ANN[j][5]++;
						else if (resultANN == 7) ANN[j][6]++;
						else if (resultANN == 8) ANN[j][7]++;
						else if (resultANN == 9) ANN[j][8]++;
						else if (resultANN == 10) ANN[j][9]++;

						if (resultSVM == 1) SVM[j][0]++;
						else if (resultSVM == 2) SVM[j][1]++;
						else if (resultSVM == 3) SVM[j][2]++;
						else if (resultSVM == 4) SVM[j][3]++;
						else if (resultSVM == 5) SVM[j][4]++;
						else if (resultSVM == 6) SVM[j][5]++;
						else if (resultSVM == 7) SVM[j][6]++;
						else if (resultSVM == 8) SVM[j][7]++;
						else if (resultSVM == 9) SVM[j][8]++;
						else if (resultSVM == 10) SVM[j][9]++;
					}
					file << "numer iteracji - " << j + 1 << endl;
					file << "indeks sprawdzanej pozy - " << k + 1 << endl;
					file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

					file << "KNN" << endl;
					file << "P1 - " << KNN[j][0] << endl;
					file << "P2 - " << KNN[j][1] << endl;
					file << "P3 - " << KNN[j][2] << endl;
					file << "P4 - " << KNN[j][3] << endl;
					file << "P5 - " << KNN[j][4] << endl;
					file << "P6 - " << KNN[j][5] << endl;
					file << "P7 - " << KNN[j][6] << endl;
					file << "P8 - " << KNN[j][7] << endl;
					file << "P9 - " << KNN[j][8] << endl;
					file << "P10 - " << KNN[j][9] << endl;
					file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

					file << "ANN" << endl;
					file << "P1 - " << ANN[j][0] << endl;
					file << "P2 - " << ANN[j][1] << endl;
					file << "P3 - " << ANN[j][2] << endl;
					file << "P4 - " << ANN[j][3] << endl;
					file << "P5 - " << ANN[j][4] << endl;
					file << "P6 - " << ANN[j][5] << endl;
					file << "P7 - " << ANN[j][6] << endl;
					file << "P8 - " << ANN[j][7] << endl;
					file << "P9 - " << ANN[j][8] << endl;
					file << "P10 - " << ANN[j][9] << endl;
					file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());

					file << "SVM" << endl;
					file << "P1 - " << SVM[j][0] << endl;
					file << "P2 - " << SVM[j][1] << endl;
					file << "P3 - " << SVM[j][2] << endl;
					file << "P4 - " << SVM[j][3] << endl;
					file << "P5 - " << SVM[j][4] << endl;
					file << "P6 - " << SVM[j][5] << endl;
					file << "P7 - " << SVM[j][6] << endl;
					file << "P8 - " << SVM[j][7] << endl;
					file << "P9 - " << SVM[j][8] << endl;
					file << "P10 - " << SVM[j][9] << endl;
					file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
					file << endl << endl;
				}
				file << endl << endl;
			}
			for (int ii = 0; ii < 10; ii++) {
				double svmSuma = 0, knnSuma = 0, annSuma = 0;
				for (int jj = 0; jj < 10; jj++) {
					svmSuma = svmSuma + skutecznoscSVM[jj][ii];
					annSuma = annSuma + skutecznoscANN[jj][ii];
					knnSuma = knnSuma + skutecznoscKNN[jj][ii];
				}
				file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
				file << "SVM - " << (svmSuma / 10) << endl;
				file << "KNN - " << (knnSuma / 10) << endl;
				file << "ANN - " << (annSuma / 10) << endl;
			}
			file.close();
		}

		else if (argv[2] == std::string("hog")) {
			ofstream outfile(dataBase2Path + std::string(argv[3]));
			fstream file;
			file.open(dataBase2Path + std::string(argv[3]), fstream::app);
			zapiszPlikiDanychDB2(dataPathDB2);
			zapiszPlikiHOG_DB2();
			for (int j = 0; j < 10; j++) {
				nauczSVM_HOG(std::string(argv[1]), dataBase2Path.c_str(), j + 1);
				nauczANN_HOG(std::string(argv[1]), dataBase2Path.c_str(), j + 1);
				for (int k = 0; k < 10; k++) {
					for (int i = 0; i < 10; i++) {
						ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
					}

					for (int i = 0; i < daneTestowe[k].size(); i++) {
						Mat img = imread(daneTestowe[k][i]);

						resultKNN = KNN_HOG(img, std::string(argv[1]), j + 1);
						resultANN = sprawdzImgANN_HOG(img, dataBase2Path + "HOGTrainedANN.xml", std::string(argv[1]));
						resultSVM = sprawdzImgSVM_HOG(img, dataBase2Path + "HOGTrainedSVM.xml");
						//cout << resultSVM << endl;
						if (resultKNN == 1) KNN[j][0]++;
						else if (resultKNN == 2) KNN[j][1]++;
						else if (resultKNN == 3) KNN[j][2]++;
						else if (resultKNN == 4) KNN[j][3]++;
						else if (resultKNN == 5) KNN[j][4]++;
						else if (resultKNN == 6) KNN[j][5]++;
						else if (resultKNN == 7) KNN[j][6]++;
						else if (resultKNN == 8) KNN[j][7]++;
						else if (resultKNN == 9) KNN[j][8]++;
						else if (resultKNN == 10) KNN[j][9]++;
						
						if (resultANN == 1) ANN[j][0]++;
						else if (resultANN == 2) ANN[j][1]++;
						else if (resultANN == 3) ANN[j][2]++;
						else if (resultANN == 4) ANN[j][3]++;
						else if (resultANN == 5) ANN[j][4]++;
						else if (resultANN == 6) ANN[j][5]++;
						else if (resultANN == 7) ANN[j][6]++;
						else if (resultANN == 8) ANN[j][7]++;
						else if (resultANN == 9) ANN[j][8]++;
						else if (resultANN == 10) ANN[j][9]++;
						
						if (resultSVM == 1) SVM[j][0]++;
						else if (resultSVM == 2) SVM[j][1]++;
						else if (resultSVM == 3) SVM[j][2]++;
						else if (resultSVM == 4) SVM[j][3]++;
						else if (resultSVM == 5) SVM[j][4]++;
						else if (resultSVM == 6) SVM[j][5]++;
						else if (resultSVM == 7) SVM[j][6]++;
						else if (resultSVM == 8) SVM[j][7]++;
						else if (resultSVM == 9) SVM[j][8]++;
						else if (resultSVM == 10) SVM[j][9]++;
					}
					file << "numer iteracji - " << j + 1 << endl;
					file << "indeks sprawdzanej pozy - " << k + 1 << endl;
					file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

					file << "KNN" << endl;
					file << "P1 - " << KNN[j][0] << endl;
					file << "P2 - " << KNN[j][1] << endl;
					file << "P3 - " << KNN[j][2] << endl;
					file << "P4 - " << KNN[j][3] << endl;
					file << "P5 - " << KNN[j][4] << endl;
					file << "P6 - " << KNN[j][5] << endl;
					file << "P7 - " << KNN[j][6] << endl;
					file << "P8 - " << KNN[j][7] << endl;
					file << "P9 - " << KNN[j][8] << endl;
					file << "P10 - " << KNN[j][9] << endl;
					file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

					file << "ANN" << endl;
					file << "P1 - " << ANN[j][0] << endl;
					file << "P2 - " << ANN[j][1] << endl;
					file << "P3 - " << ANN[j][2] << endl;
					file << "P4 - " << ANN[j][3] << endl;
					file << "P5 - " << ANN[j][4] << endl;
					file << "P6 - " << ANN[j][5] << endl;
					file << "P7 - " << ANN[j][6] << endl;
					file << "P8 - " << ANN[j][7] << endl;
					file << "P9 - " << ANN[j][8] << endl;
					file << "P10 - " << ANN[j][9] << endl;
					file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());

					file << "SVM" << endl;
					file << "P1 - " << SVM[j][0] << endl;
					file << "P2 - " << SVM[j][1] << endl;
					file << "P3 - " << SVM[j][2] << endl;
					file << "P4 - " << SVM[j][3] << endl;
					file << "P5 - " << SVM[j][4] << endl;
					file << "P6 - " << SVM[j][5] << endl;
					file << "P7 - " << SVM[j][6] << endl;
					file << "P8 - " << SVM[j][7] << endl;
					file << "P9 - " << SVM[j][8] << endl;
					file << "P10 - " << SVM[j][9] << endl;
					file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
					file << endl << endl;
				}
				file << endl << endl;
			}
			for (int ii = 0; ii < 10; ii++) {
				double svmSuma = 0, knnSuma = 0, annSuma = 0;
				for (int jj = 0; jj < 10; jj++) {
					svmSuma = svmSuma + skutecznoscSVM[jj][ii];
					annSuma = annSuma + skutecznoscANN[jj][ii];
					knnSuma = knnSuma + skutecznoscKNN[jj][ii];
				}
				file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
				file << "SVM - " << (svmSuma / 10) << endl;
				file << "KNN - " << (knnSuma / 10) << endl;
				file << "ANN - " << (annSuma / 10) << endl;
			}
			file.close();
		}

		else if (argv[2] == std::string("wsp")) {
			wsp = std::string(argv[4]);
			ofstream outfile(dataBase2Path + std::string(argv[3]));
			fstream file;
			file.open(dataBase2Path + std::string(argv[3]), fstream::app);
			zapiszPlikiDanychDB2(dataPathDB2);
			zapiszPlikiWspKsztaltu_DB2();
			for (int j = 0; j < 10; j++) {
				nauczSVM_wsp(std::string(argv[1]), dataBase2Path.c_str(), j + 1);
				nauczANN_wsp(std::string(argv[1]), dataBase2Path.c_str(), j + 1);
				for (int k = 0; k < 10; k++) {
					for (int i = 0; i < 10; i++) {
						ANN[j][i] = 0; KNN[j][i] = 0; SVM[j][i] = 0;
					}

					for (int i = 0; i < daneTestowe[k].size(); i++) {
						Mat img = imread(daneTestowe[k][i]);
						resultKNN = KNN_wsp(img, std::string(argv[1]), j + 1);
						resultANN = sprawdzImgANN_wsp(img, dataBase2Path + "wspKsztaltuTrainedANN.xml", std::string(argv[1]));
						resultSVM = sprawdzImgSVM_wsp(img, dataBase2Path + "wspKsztaltuTrainedSVM.xml");
					//	cout << resultANN << endl;
						if (resultKNN == 1) KNN[j][0]++;
						else if (resultKNN == 2) KNN[j][1]++;
						else if (resultKNN == 3) KNN[j][2]++;
						else if (resultKNN == 4) KNN[j][3]++;
						else if (resultKNN == 5) KNN[j][4]++;
						else if (resultKNN == 6) KNN[j][5]++;
						else if (resultKNN == 7) KNN[j][6]++;
						else if (resultKNN == 8) KNN[j][7]++;
						else if (resultKNN == 9) KNN[j][8]++;
						else if (resultKNN == 10) KNN[j][9]++;

						if (resultANN == 1) ANN[j][0]++;
						else if (resultANN == 2) ANN[j][1]++;
						else if (resultANN == 3) ANN[j][2]++;
						else if (resultANN == 4) ANN[j][3]++;
						else if (resultANN == 5) ANN[j][4]++;
						else if (resultANN == 6) ANN[j][5]++;
						else if (resultANN == 7) ANN[j][6]++;
						else if (resultANN == 8) ANN[j][7]++;
						else if (resultANN == 9) ANN[j][8]++;
						else if (resultANN == 10) ANN[j][9]++;

						if (resultSVM == 1) SVM[j][0]++;
						else if (resultSVM == 2) SVM[j][1]++;
						else if (resultSVM == 3) SVM[j][2]++;
						else if (resultSVM == 4) SVM[j][3]++;
						else if (resultSVM == 5) SVM[j][4]++;
						else if (resultSVM == 6) SVM[j][5]++;
						else if (resultSVM == 7) SVM[j][6]++;
						else if (resultSVM == 8) SVM[j][7]++;
						else if (resultSVM == 9) SVM[j][8]++;
						else if (resultSVM == 10) SVM[j][9]++;
					}
					file << "numer iteracji - " << j + 1 << endl;
					file << "indeks sprawdzanej pozy - " << k + 1 << endl;
					file << "iloœæ danych testowych - " << daneTestowe[k].size() << endl;

					file << "KNN" << endl;
					file << "P1 - " << KNN[j][0] << endl;
					file << "P2 - " << KNN[j][1] << endl;
					file << "P3 - " << KNN[j][2] << endl;
					file << "P4 - " << KNN[j][3] << endl;
					file << "P5 - " << KNN[j][4] << endl;
					file << "P6 - " << KNN[j][5] << endl;
					file << "P7 - " << KNN[j][6] << endl;
					file << "P8 - " << KNN[j][7] << endl;
					file << "P9 - " << KNN[j][8] << endl;
					file << "P10 - " << KNN[j][9] << endl;
					file << "skutecznoœæ KNN: " << (KNN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscKNN[j][k] = (KNN[j][k] / daneTestowe[k].size());

					file << "ANN" << endl;
					file << "P1 - " << ANN[j][0] << endl;
					file << "P2 - " << ANN[j][1] << endl;
					file << "P3 - " << ANN[j][2] << endl;
					file << "P4 - " << ANN[j][3] << endl;
					file << "P5 - " << ANN[j][4] << endl;
					file << "P6 - " << ANN[j][5] << endl;
					file << "P7 - " << ANN[j][6] << endl;
					file << "P8 - " << ANN[j][7] << endl;
					file << "P9 - " << ANN[j][8] << endl;
					file << "P10 - " << ANN[j][9] << endl;
					file << "skutecznoœæ ANN: " << (ANN[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscANN[j][k] = (ANN[j][k] / daneTestowe[k].size());

					file << "SVM" << endl;
					file << "P1 - " << SVM[j][0] << endl;
					file << "P2 - " << SVM[j][1] << endl;
					file << "P3 - " << SVM[j][2] << endl;
					file << "P4 - " << SVM[j][3] << endl;
					file << "P5 - " << SVM[j][4] << endl;
					file << "P6 - " << SVM[j][5] << endl;
					file << "P7 - " << SVM[j][6] << endl;
					file << "P8 - " << SVM[j][7] << endl;
					file << "P9 - " << SVM[j][8] << endl;
					file << "P10 - " << SVM[j][9] << endl;
					file << "skutecznoœæ SVM: " << (SVM[j][k] / daneTestowe[k].size()) << endl;
					skutecznoscSVM[j][k] = (SVM[j][k] / daneTestowe[k].size());
					file << endl << endl;
				}
				file << endl << endl;
			}
				for (int ii = 0; ii < 10; ii++) {
				double svmSuma = 0, knnSuma = 0, annSuma = 0;
				for (int jj = 0; jj < 10; jj++) {
					svmSuma = svmSuma + skutecznoscSVM[jj][ii];
					annSuma = annSuma + skutecznoscANN[jj][ii];
					knnSuma = knnSuma + skutecznoscKNN[jj][ii];
				}
				file << "œrednia skutecznoœæ pozy:" << ii + 1 << endl;
				file << "SVM - " << (svmSuma / 10) << endl;
				file << "KNN - " << (knnSuma / 10) << endl;
				file << "ANN - " << (annSuma / 10) << endl;
			}
			file.close();
		}
		
		else {
			cout << "Niepoprawna nazwa deskryptora. " << "\n";
			cout << "mozliwe opcje to : " << "\n";
			cout << "shape context \nhog \nwsp" << "\n";
		}
	}
	
	else {
		cout << "Mozliwe opcje to: \"db1\" \"db2\" \"help\""<<"\n";
	}
	
	cout << "ZAKONCZONO DZIALANIE PROGRAMU" << endl;
	getchar();
}
