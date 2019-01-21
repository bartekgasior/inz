Aby uruchomiæ program jako parametry nale¿y podaæ:
- Bazê danych:
  - "db1";
  - "db2".
- Deskryptor obrazu:
  - "hog" - histogram zorientowanych gradientów;
  - "shape context";
  - "wsp" - wspó³czynniki kszta³tu.
- Œcie¿kê zapisu pliku z wynikami.
- Opcjonalnie, w przypadku wybrania wspó³czynników kszta³tu, kombinacje wspó³czynników:
  - wsp_1 - W1, W3, W5, W6, W7;
  - wsp_2 - W3, W5, W7, W9;
  - wsp_3 - W5, W6.

W celu wywo³ania menu pomocy nale¿y wywo³aæ funkcjê "help".

Przyk³adowe wywo³anie programu:
- shapeContext1 "db2" "wsp" "wsp_1Results.txt" "wsp_2"
- shapeContext1 "help"

###########################################################

Zmienne globalne:
- dataPathDB1, dataPathDB2 - Œcie¿ki do folderów z plikami nale¿¹cymi do baz danych.
- dataBase1Path, dataBase2Path -  Foldery zapisu plików z poszczególnymi wynikami.
- validExtensions - Wektor z akceptowalnymi rozszerzeniami zdjêæ.
- angleBins, radiusBins - zmienne niezbêdne do metody shape context, przechowuj¹ce iloœæ binów wzglêdem k¹ta (angle) oraz promienia (radius).
- k - parametr k wykorzystywany w klasyfikaotrze k-NN.
- wsp - Zmienna przechowuj¹ca wybran¹ kombinacjê wspó³czynników kszta³tu.
- P1Data-P10Data - Wektory przechowuj¹ce nazwy plików klas obu baz danych.
- daneUczace, daneTestowe - Wektory zawieraj¹ce pliki danych s³u¿¹ce do zbudowania klasyfikatora oraz klasyfikacji.

Zmienne lokalne w poszczególnych funkcjach:

a) Funkcja zapiszPlikiDanychDB1 - Zapis naz plików bazy danych "p³otkarze":
  - folders - Foldery P1,P2 itd.
  - P1, P2, P3, P4, P5 - Foldery wewn¹trz folderów P1,P2 ..., np. P1/z1.t2/.

b) Funkcja zapiszDaneUczaceTestoweDB1 oraz zapiszDaneUczaceTestoweDB2 - Zapis danych ucz¹cych oraz testowych dla bazy danych "p³otkarze" oraz "Actions as Space-Time Shapes" w zale¿noœci od kroku walidacji:
  - daneTestoweTMP - Wektor przechowuj¹cy dane testowe danej klasy.
  - data - Wektor macierzy, w którego sk³ad wchodz¹:
    - data[0] - Macierz, w której ka¿dy z wierszy jest zestawem deskryptorów opisuj¹cych obiekt;
    - data[1] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    - data[2] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    - data[3] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.
  - dataTMP - Wektor macierzy, którego elementy s¹ rozmiarów odpowiadaj¹cych elementów w macierzy data. Przechowuje dane ucz¹ce.
  
  Funkcje zwracaj¹ macierz z wartoœciami dla danych ucz¹cych.

c) Funkcja morphology - Operacje closingu oraz openingu:
  - element - element strukturalny wykorzystywany do powy¿szych operacji.

d) Funkcja detectObject - Znalezenie na obrazie najwiêkszego obiektu:
  - dst - Zmienna, do której przypisywany jest obraz, na którym wykonywane s¹ przekszta³cenia.
  - largest_area - Zmienna przechowuj¹ca wartoœæ pola najwiêkszego obiektu.
  - largest_contour_index - Indeks najwiêkszego konturu.

  Funkcja zwraca contur najwiêkszego znalezionego obiektu.

e) Funkcje do obliczania wspó³czynników kszta³tu:
  - contourTmp - Kontur obiektu, dla którego obliczany bêdzie wspó³czynnik kszta³tu.
  - tmpx, tmpy - Zmienne tymczasowe s³u¿¹ce do przechowywania wspó³rzêdnych sprawdzanego punktu. 
  - L - Obwód obiektu.
  - S - Pole obiektu.
  - Lh - Maksymalna œrednica poziomo.
  - Lv - Maksymalna œrednica obiektu pionowo.
  - mc, mu - Zmienne do obliczenia œrodka ciê¿koœci obiektu.

  Funkcje zwracaj¹ obliczon¹ wartoœæ wspó³czynników.

f) Funkcja zapiszWsp - Obliczanie kombinacji wspó³czynników:
  - row - Wektor, w którym zapisywane s¹ wartoœci poszczególnych wspó³czynników.
  - img - Obraz, dla którego obliczane s¹ deskryptory.

  Funkcja zwraca obliczone wspó³czynniki.

g) Funkcje zapiszWspDoPliku, zapiszShapeContextDoPliku oraz zapiszHOGDoPliku - Zapisanie wspolczynnikow do pliku:
  - rows - Wektor, w którym zapisywane s¹ obliczone wspó³czynniki kszta³tu.
  - img - zmienna tymczasowa, przechowuj¹ca aktualnie wczytany obraz.
  - row - liczba wierszy (zdjêæ).
  - col - liczba kolumn (wspó³czynników). 
  - M - Macierz o rozmiarze (row,col) przechowuj¹ca wszystkie wspó³czynniki.

h) Funkcja DescriptorMTX_wsp_DB1 oraz DescriptorMTX_wsp_DB2 - Odczyt wspó³czynników kszta³tu z plików oraz zapis klas poszczególnych zdjêæ:
  - descriptors_labels - Wektor macierzy zaweraj¹cy wspó³czynniki oraz klasy zdjêæ.
    Indeksy: 0 - deskryptory, 1 - klasy int, 2 - klasy float, 3 - klasy dla logistic regression.
  - pMat_1-pMat_10 - Macierz, w której zapisywane s¹ odczytane wczeœniej z plików wartoœci wspó³czynników.
  - pRow1-pRow10 oraz pCol1-pCol10 - iloœæ wierszy oraz kolumn odpowiednich macierzy pMat.
  - Descriptor_mtx - Macierz zawieraj¹ca deskryptory zdjêæ, ka¿dy wiersz to osobne zdjêcie.
  - labels - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.
  - trainClasses - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
  - labelsLogisticRegression - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.

  Funkcje zwracaj¹ wektor macierzy descriptor_labels.

i) Funkcje s³u¿¹ce do treningu klasyfikatorów SVM - nauczSVM_wsp, nauczSVM_ShapeContext, nauczSVM_HOG:
  - data - Wektor macierzy zaweraj¹cy wspó³czynniki oraz klasy zdjêæ.
    -- data[0] - Macierz, w której ka¿dy z wierszy jest zestawem deskryptorów opisuj¹cych obiekt;
    -- data[1] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.
  - dataTmp - Wektor macierzy, którego elementy przechowuj¹ dane ucz¹ce oraz ich klasy. Poszczególne macierze przechowuj¹ elementy, wspomnianego wy¿ej, wektora macierzy data. 

j) Funkcje s³u¿¹ce do treningu klasyfikatorów MLP (perceptron wielowarstwowy) - nauczANN_wsp, nauczANN_ShapeContext, nauczANN_HOG:
  - data - Wektor macierzy zaweraj¹cy wspó³czynniki oraz klasy zdjêæ.
    -- data[0] - Macierz, w której ka¿dy z wierszy jest zestawem deskryptorów opisuj¹cych obiekt;
    -- data[1] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.
  - dataTmp - Wektor macierzy, którego elementy przechowuj¹ dane ucz¹ce oraz ich klasy. Poszczególne macierze przechowuj¹ elementy, wspomnianego wy¿ej, wektora macierzy data. 
  - layers - Macierz, w której przehowywane s¹ rozmiary warstw perceptronu, w programie perceptron zbudowany jest z:
    -- layers.row(0) - warstwa wejœciowa - liczba wspó³czynników (deskryptorów, wartoœci) opisuj¹cych dany obiekt.
    -- layers.row(1) - warstwa ukryta - w programie warstwa ta zbudowana jest z 7 neuronów.
    -- layers.row(2) - warstwa wyjœciowa - liczba klas bazy danych.

k) Funkcje s³u¿¹ce do treningu oraz klasyfikacji z u¿yciem klasyfikatora k-NN - KNN_wsp, KNN_ShapeContext, KNN_HOG:
  - img - zmienna tymczasowa, przechowuj¹ca aktualnie wczytany obraz, który bêdzie testowany.
  - data - Wektor macierzy zaweraj¹cy wspó³czynniki oraz klasy zdjêæ.
    -- data[0] - Macierz, w której ka¿dy z wierszy jest zestawem deskryptorów opisuj¹cych obiekt;
    -- data[1] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu k-NN oraz SVM.
    -- data[2] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu perceptronu wielowarstwowego.
    -- data[3] - Macierz, w której przechowywane s¹ klasy zdjêæ. Wykorzystywana przy tworzeniu logicznej regresji. W pracy klasyfikator ten nie by³ wykorzystany.
  - dataTmp - Wektor macierzy, którego elementy przechowuj¹ dane ucz¹ce oraz ich klasy. Poszczególne macierze przechowuj¹ elementy, wspomnianego wy¿ej, wektora macierzy data. 
  - result - Macierz, w której zapisywany jest wynik klasyfikacji.

  Funkcja zwraca numer klasy, do której sklasyfikowany zosta³ testowany obiekt.

l) Funkcje s³u¿¹ce do klasyfikacji wykorzystuj¹ce klasyfikator SVM - sprawdzImgSVM_wsp ,sprawdzImgSVM_ShapeContext:
  - img - Zmienna przechowuj¹ca wybrany obraz, który bêdzie klasyfikowany.
  - wspImg - Wektor w którym przechowywane s¹ wartoœci wspó³czynników kszta³tu wybranego zdjêcia.
  - shape - Wektor przechowuj¹cy deksryptor shape context wczytanego zdjêcia.
  - result - Wynik klasyfikacji.

m) Funkcja sprawdzImgSVM_HOG s³u¿¹ca do klasyfikacji HOG wykorzystuj¹ca SVM:
  - contourTmp - Kontur badanego obiektu.
  - mask - Macierz o rozmiarze zdjêcia, wype³niona zerami.
  - imgTmp - Zdjêcie na które nak³adany jest kontur obiektu.
  - fm1 - Wektor przechowuj¹cy deskryptor hog wczytanego zdjêcia.
  - result - wynik klasyfikacji.

n) Funkcje s³u¿¹ce do klasyfikacji wykorzystuj¹ce klasyfikator MLP - sprawdzImgANN_wsp ,sprawdzImgANN_ShapeContext:
  - img - Zmienna przechowuj¹ca wybrany obraz, który bêdzie klasyfikowany.
  - wspImg - Wektor w którym przechowywane s¹ wartoœci wspó³czynników kszta³tu wybranego zdjêcia.
  - shape - Wektor przechowuj¹cy deksryptor shape context wczytanego zdjêcia.
  - cls - Wynik klasyfikacji.

o) Funkcja sprawdzImgANN_HOG s³u¿¹ca do klasyfikacji HOG wykorzystuj¹ca MLP:
  - contourTmp - Kontur badanego obiektu.
  - mask - Macierz o rozmiarze zdjêcia, wype³niona zerami.
  - imgTmp - Zdjêcie na które nak³adany jest kontur obiektu.
  - fm1 - Wektor przechowuj¹cy deskryptor hog wczytanego zdjêcia.
  - cls - Wynik klasyfikacji.

Zmienne funkcji g³ównej main:

- resultANN, resultSVM, resultKNN - Zmienne przechowuj¹ce wynik klasyfikacji.
- ANN[10][5], SVM[10][5], KNN[10][5] oraz ANN[10][10], SVM[10][10], KNN[10][10] - Tablice, w których przechowywana jest liczba przyporz¹dkowanych obiektów do poszczególnych klas.
- skutecznoscKNN[10][5], skutecznoscSVM[10][5], skutecznoscANN[10][5] oraz skutecznoscKNN[10][10], skutecznoscSVM[10][10], skutecznoscANN[10][10] - Skutecznoœæ klasyfikacji dla poszczególnych kroków walidacji.
- svmSrednia[5], knnSrednia[5], annSrednia[5] oraz svmSrednia[10], knnSrednia[10], annSrednia[10] - Œrednia skutecznoœæ klasyfikacji ka¿dej pozy.