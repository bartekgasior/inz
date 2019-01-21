Aby uruchomi� program jako parametry nale�y poda�:
- Baz� danych:
  - "db1";
  - "db2".
- Deskryptor obrazu:
  - "hog" - histogram zorientowanych gradient�w;
  - "shape context";
  - "wsp" - wsp�czynniki kszta�tu.
- �cie�k� zapisu pliku z wynikami.
- Opcjonalnie, w przypadku wybrania wsp�czynnik�w kszta�tu, kombinacje wsp�czynnik�w:
  - wsp_1 - W1, W3, W5, W6, W7;
  - wsp_2 - W3, W5, W7, W9;
  - wsp_3 - W5, W6.

W celu wywo�ania menu pomocy nale�y wywo�a� funkcj� "help".

Przyk�adowe wywo�anie programu:
- shapeContext1 "db2" "wsp" "wsp_1Results.txt" "wsp_2"
- shapeContext1 "help"

###########################################################

Zmienne globalne:
- dataPathDB1, dataPathDB2 - �cie�ki do folder�w z plikami nale��cymi do baz danych.
- dataBase1Path, dataBase2Path -  Foldery zapisu plik�w z poszczeg�lnymi wynikami.
- validExtensions - Wektor z akceptowalnymi rozszerzeniami zdj��.
- angleBins, radiusBins - zmienne niezb�dne do metody shape context, przechowuj�ce ilo�� bin�w wzgl�dem k�ta (angle) oraz promienia (radius).
- k - parametr k wykorzystywany w klasyfikaotrze k-NN.
- wsp - Zmienna przechowuj�ca wybran� kombinacj� wsp�czynnik�w kszta�tu.
- P1Data-P10Data - Wektory przechowuj�ce nazwy plik�w klas obu baz danych.
- daneUczace, daneTestowe - Wektory zawieraj�ce pliki danych s�u��ce do zbudowania klasyfikatora oraz klasyfikacji.

Zmienne lokalne w poszczeg�lnych funkcjach:

a) Funkcja zapiszPlikiDanychDB1 - Zapis naz plik�w bazy danych "p�otkarze":
  - folders - Foldery P1,P2 itd.
  - P1, P2, P3, P4, P5 - Foldery wewn�trz folder�w P1,P2 ..., np. P1/z1.t2/.

b) Funkcja zapiszDaneUczaceTestoweDB1 oraz zapiszDaneUczaceTestoweDB2 - Zapis danych ucz�cych oraz testowych dla bazy danych "p�otkarze" oraz "Actions as Space-Time Shapes" w zale�no�ci od kroku walidacji:
  - daneTestoweTMP - Wektor przechowuj�cy dane testowe danej klasy.
  - data - Wektor macierzy, w kt�rego sk�ad wchodz�:
    - data[0] - Macierz, w kt�rej ka�dy z wierszy jest zestawem deskryptor�w opisuj�cych obiekt;
    - data[1] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    - data[2] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    - data[3] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by� wykorzystany.
  - dataTMP - Wektor macierzy, kt�rego elementy s� rozmiar�w odpowiadaj�cych element�w w macierzy data. Przechowuje dane ucz�ce.
  
  Funkcje zwracaj� macierz z warto�ciami dla danych ucz�cych.

c) Funkcja morphology - Operacje closingu oraz openingu:
  - element - element strukturalny wykorzystywany do powy�szych operacji.

d) Funkcja detectObject - Znalezenie na obrazie najwi�kszego obiektu:
  - dst - Zmienna, do kt�rej przypisywany jest obraz, na kt�rym wykonywane s� przekszta�cenia.
  - largest_area - Zmienna przechowuj�ca warto�� pola najwi�kszego obiektu.
  - largest_contour_index - Indeks najwi�kszego konturu.

  Funkcja zwraca contur najwi�kszego znalezionego obiektu.

e) Funkcje do obliczania wsp�czynnik�w kszta�tu:
  - contourTmp - Kontur obiektu, dla kt�rego obliczany b�dzie wsp�czynnik kszta�tu.
  - tmpx, tmpy - Zmienne tymczasowe s�u��ce do przechowywania wsp�rz�dnych sprawdzanego punktu. 
  - L - Obw�d obiektu.
  - S - Pole obiektu.
  - Lh - Maksymalna �rednica poziomo.
  - Lv - Maksymalna �rednica obiektu pionowo.
  - mc, mu - Zmienne do obliczenia �rodka ci�ko�ci obiektu.

  Funkcje zwracaj� obliczon� warto�� wsp�czynnik�w.

f) Funkcja zapiszWsp - Obliczanie kombinacji wsp�czynnik�w:
  - row - Wektor, w kt�rym zapisywane s� warto�ci poszczeg�lnych wsp�czynnik�w.
  - img - Obraz, dla kt�rego obliczane s� deskryptory.

  Funkcja zwraca obliczone wsp�czynniki.

g) Funkcje zapiszWspDoPliku, zapiszShapeContextDoPliku oraz zapiszHOGDoPliku - Zapisanie wspolczynnikow do pliku:
  - rows - Wektor, w kt�rym zapisywane s� obliczone wsp�czynniki kszta�tu.
  - img - zmienna tymczasowa, przechowuj�ca aktualnie wczytany obraz.
  - row - liczba wierszy (zdj��).
  - col - liczba kolumn (wsp�czynnik�w). 
  - M - Macierz o rozmiarze (row,col) przechowuj�ca wszystkie wsp�czynniki.

h) Funkcja DescriptorMTX_wsp_DB1 oraz DescriptorMTX_wsp_DB2 - Odczyt wsp�czynnik�w kszta�tu z plik�w oraz zapis klas poszczeg�lnych zdj��:
  - descriptors_labels - Wektor macierzy zaweraj�cy wsp�czynniki oraz klasy zdj��.
    Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.
  - pMat_1-pMat_10 - Macierz, w kt�rej zapisywane s� odczytane wcze�niej z plik�w warto�ci wsp�czynnik�w.
  - pRow1-pRow10 oraz pCol1-pCol10 - ilo�� wierszy oraz kolumn odpowiednich macierzy pMat.
  - Descriptor_mtx - Macierz zawieraj�ca deskryptory zdj��, ka�dy wiersz to osobne zdj�cie.
  - labels - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu k-NN oraz SVM.
  - trainClasses - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
  - labelsLogisticRegression - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by� wykorzystany.

  Funkcje zwracaj� wektor macierzy descriptor_labels.

i) Funkcje s�u��ce do treningu klasyfikator�w SVM - nauczSVM_wsp, nauczSVM_ShapeContext, nauczSVM_HOG:
  - data - Wektor macierzy zaweraj�cy wsp�czynniki oraz klasy zdj��.
    -- data[0] - Macierz, w kt�rej ka�dy z wierszy jest zestawem deskryptor�w opisuj�cych obiekt;
    -- data[1] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by� wykorzystany.
  - dataTmp - Wektor macierzy, kt�rego elementy przechowuj� dane ucz�ce oraz ich klasy. Poszczeg�lne macierze przechowuj� elementy, wspomnianego wy�ej, wektora macierzy data. 

j) Funkcje s�u��ce do treningu klasyfikator�w MLP (perceptron wielowarstwowy) - nauczANN_wsp, nauczANN_ShapeContext, nauczANN_HOG:
  - data - Wektor macierzy zaweraj�cy wsp�czynniki oraz klasy zdj��.
    -- data[0] - Macierz, w kt�rej ka�dy z wierszy jest zestawem deskryptor�w opisuj�cych obiekt;
    -- data[1] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by� wykorzystany.
  - dataTmp - Wektor macierzy, kt�rego elementy przechowuj� dane ucz�ce oraz ich klasy. Poszczeg�lne macierze przechowuj� elementy, wspomnianego wy�ej, wektora macierzy data. 
  - layers - Macierz, w kt�rej przehowywane s� rozmiary warstw perceptronu, w programie perceptron zbudowany jest z:
    -- layers.row(0) - warstwa wej�ciowa - liczba wsp�czynnik�w (deskryptor�w, warto�ci) opisuj�cych dany obiekt.
    -- layers.row(1) - warstwa ukryta - w programie warstwa ta zbudowana jest z 7 neuron�w.
    -- layers.row(2) - warstwa wyj�ciowa - liczba klas bazy danych.

k) Funkcje s�u��ce do treningu oraz klasyfikacji z u�yciem klasyfikatora k-NN - KNN_wsp, KNN_ShapeContext, KNN_HOG:
  - img - zmienna tymczasowa, przechowuj�ca aktualnie wczytany obraz, kt�ry b�dzie testowany.
  - data - Wektor macierzy zaweraj�cy wsp�czynniki oraz klasy zdj��.
    -- data[0] - Macierz, w kt�rej ka�dy z wierszy jest zestawem deskryptor�w opisuj�cych obiekt;
    -- data[1] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w kt�rej przechowywane s� klasy zdj��. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by� wykorzystany.
  - dataTmp - Wektor macierzy, kt�rego elementy przechowuj� dane ucz�ce oraz ich klasy. Poszczeg�lne macierze przechowuj� elementy, wspomnianego wy�ej, wektora macierzy data. 
  - result - Macierz, w kt�rej zapisywany jest wynik klasyfikacji.

  Funkcja zwraca numer klasy, do kt�rej sklasyfikowany zosta� testowany obiekt.

l) Funkcje s�u��ce do klasyfikacji wykorzystuj�ce klasyfikator SVM - sprawdzImgSVM_wsp ,sprawdzImgSVM_ShapeContext:
  - img - Zmienna przechowuj�ca wybrany obraz, kt�ry b�dzie klasyfikowany.
  - wspImg - Wektor w kt�rym przechowywane s� warto�ci wsp�czynnik�w kszta�tu wybranego zdj�cia.
  - shape - Wektor przechowuj�cy deksryptor shape context wczytanego zdj�cia.
  - result - Wynik klasyfikacji.

m) Funkcja sprawdzImgSVM_HOG s�u��ca do klasyfikacji HOG wykorzystuj�ca SVM:
  - contourTmp - Kontur badanego obiektu.
  - mask - Macierz o rozmiarze zdj�cia, wype�niona zerami.
  - imgTmp - Zdj�cie na kt�re nak�adany jest kontur obiektu.
  - fm1 - Wektor przechowuj�cy deskryptor hog wczytanego zdj�cia.
  - result - wynik klasyfikacji.

n) Funkcje s�u��ce do klasyfikacji wykorzystuj�ce klasyfikator MLP - sprawdzImgANN_wsp ,sprawdzImgANN_ShapeContext:
  - img - Zmienna przechowuj�ca wybrany obraz, kt�ry b�dzie klasyfikowany.
  - wspImg - Wektor w kt�rym przechowywane s� warto�ci wsp�czynnik�w kszta�tu wybranego zdj�cia.
  - shape - Wektor przechowuj�cy deksryptor shape context wczytanego zdj�cia.
  - cls - Wynik klasyfikacji.

o) Funkcja sprawdzImgANN_HOG s�u��ca do klasyfikacji HOG wykorzystuj�ca MLP:
  - contourTmp - Kontur badanego obiektu.
  - mask - Macierz o rozmiarze zdj�cia, wype�niona zerami.
  - imgTmp - Zdj�cie na kt�re nak�adany jest kontur obiektu.
  - fm1 - Wektor przechowuj�cy deskryptor hog wczytanego zdj�cia.
  - cls - Wynik klasyfikacji.

Zmienne funkcji g��wnej main:

- resultANN, resultSVM, resultKNN - Zmienne przechowuj�ce wynik klasyfikacji.
- ANN[10][5], SVM[10][5], KNN[10][5] oraz ANN[10][10], SVM[10][10], KNN[10][10] - Tablice, w kt�rych przechowywana jest liczba przyporz�dkowanych obiekt�w do poszczeg�lnych klas.
- skutecznoscKNN[10][5], skutecznoscSVM[10][5], skutecznoscANN[10][5] oraz skutecznoscKNN[10][10], skutecznoscSVM[10][10], skutecznoscANN[10][10] - Skuteczno�� klasyfikacji dla poszczeg�lnych krok�w walidacji.
- svmSrednia[5], knnSrednia[5], annSrednia[5] oraz svmSrednia[10], knnSrednia[10], annSrednia[10] - �rednia skuteczno�� klasyfikacji ka�dej pozy.