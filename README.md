# Aron Mandrella – Praca Inżynierska (2018)
## Opis
Projekt ten dotyczył algorytmów kwantyzacji obrazów wielobarwnych. Celem projektu była implementacja różnych algorytmów kwantyzacji w języku C++, oraz porównanie ich pod kątem jakości otrzymywanych wyników i czasu potrzebnego na wykonanie obliczeń. W ramach projektu stworzony został również program z prostym interfejsem graficznym pozwalający określić parametry wejściowe dla algorytmów.

* [Pełen tekst pracy inżynierskiej]( https://github.com/am-portfolio/QuantStudio/blob/main/AMandrella%20-%20Praca%20In%C5%BCynierska.pdf)
* [Pliki wykonywalne](https://github.com/am-portfolio/Praca-Inzynierska/releases/tag/1.0)
* [Wyniki w postaci wykresów (na dysku Google)]( https://drive.google.com/drive/folders/1Vtz8GABDCrWPLk_FXwrDxCHjYdSvlFe3?usp=sharing)

## 🧰 Wykorzystane technologie i narzędzia
* **C++17**: Interfejs graficzny (biblioteka *SFML*), algorytmy kwantyzacji, zarządzanie obliczeniami w wielu wątkach (*std::thread, std::mutex, std::future, std:: condition_variable*)
* **Visual Studio 2017**: Kompilacja C++, debugowanie kodu
* **MATLAB**: Generowanie wykresów, obliczenia niektórych wskaźników jakości
* **Adobe Photoshop**
## 🎓 Zdobyta bądź poszerzona wiedza
* Algorytmy klasteryzacji danych (np. algorytm k-średnich)
* Algorytmy interpolacji danych (metoda najbliższego sąsiada, interpolacja dwuliniowa)
* Analiza obrazów cyfrowych w różnych przestrzeniach barw i za pomocą różnych wskaźników jakości (RGB, sRGB, CIE XYZ, RMSE, PSNR, MAE, DE76, DSCSI, HPSI)
* Umiejętność zrozumienia i implementacji algorytmów matematycznych
* Akademickie metody statystycznej analizy zebranych danych (wykresy pudełkowe, aproksymowanie za pomocą wielomianów, tabele, itp.)
## Interfejs graficzny
![](https://github.com/am-portfolio/Praca-Inzynierska/blob/main/GUI.PNG)
