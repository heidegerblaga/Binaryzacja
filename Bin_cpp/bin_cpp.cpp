/*
* Temat projektu: Binaryzacja obraz�w
* Opis algorytmu:
* Binaryzacja to proces konwersji obrazu na dwukolorowy, czarno-bia�y.
* Algorytm obejmuje wyb�r progu, powy�ej kt�rego piksele s� przypisywane do bia�ego, a poni�ej do czarnego.
* U�ywana jest adaptacyjna binaryzacja, dostosowuj�ca pr�g lokalnie.
* Data wykonania projektu: 16.01.2024
* Semestr: 5
* Rok akademicki: 2023/24
* Autor: Pawe� Pluta
* v3 - v1(algorytm w asm oraz .cpp), v2(gui), v3(poprawki, komentarze oraz wyko�czenie)
*/

#include "pch.h"
#include "bin_cpp.h"

/**
 * BinaryCpp to funkcja napisana w C++ wykonuj�ca algorytm binaryzacji obrazu.
 * Opis wyniku funkcji:
 * Po wykonaniu funkcji BinaryCpp, tablica pikseli 'data' zostanie zaktualizowana zgodnie
 * z algorytmem binaryzacji. Wszystkie piksele zostan� zamienione na jednokolorowe piksele:
 * bia�e (255, 255, 255) dla pikseli, kt�rych �rednia warto�� koloru jest wi�ksza ni� warto�� progowa ('value'),
 * oraz czarne (0, 0, 0) dla pikseli o �redniej warto�ci poni�ej lub r�wnej warto�ci progowej.
 *
 * @param data  Wska�nik do tablicy surowych danych pikseli obrazu.
 * @param size  Liczba pikseli w tablicy.
 * @param value Warto�� progowa do binaryzacji pikseli (0-255).
 */
void BinaryCpp(uint8_t* data, int size, int value) {
    // P�tla przechodz�ca przez tablic� bajt�w reprezentuj�c� obraz.
    for (int i = 0; i < size; i += 3) {
        // Obliczenie �redniej warto�ci sk�adowych koloru (czerwonej, zielonej i niebieskiej) dla danego piksela.
        uint8_t avg = (data[i] + data[i + 1] + data[i + 2]) / 3;

        // Por�wnanie �redniej warto�ci z zadanym progiem binaryzacji (value).
        if (avg > value)
            data[i] = data[i + 1] = data[i + 2] = 255; // Je�li �rednia jest wi�ksza ni� pr�g, ustawia kolor na bia�y.
        else
            data[i] = data[i + 1] = data[i + 2] = 0; // W przeciwnym przypadku ustawia kolor na czarny.
    }
}