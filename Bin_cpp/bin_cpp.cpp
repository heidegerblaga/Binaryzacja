/*
* Temat projektu: Binaryzacja obrazów
* Opis algorytmu:
* Binaryzacja to proces konwersji obrazu na dwukolorowy, czarno-bia³y.
* Algorytm obejmuje wybór progu, powy¿ej którego piksele s¹ przypisywane do bia³ego, a poni¿ej do czarnego.
* U¿ywana jest adaptacyjna binaryzacja, dostosowuj¹ca próg lokalnie.
* Data wykonania projektu: 16.01.2024
* Semestr: 5
* Rok akademicki: 2023/24
* Autor: Pawe³ Pluta
* v3 - v1(algorytm w asm oraz .cpp), v2(gui), v3(poprawki, komentarze oraz wykoñczenie)
*/

#include "pch.h"
#include "bin_cpp.h"

/**
 * BinaryCpp to funkcja napisana w C++ wykonuj¹ca algorytm binaryzacji obrazu.
 * Opis wyniku funkcji:
 * Po wykonaniu funkcji BinaryCpp, tablica pikseli 'data' zostanie zaktualizowana zgodnie
 * z algorytmem binaryzacji. Wszystkie piksele zostan¹ zamienione na jednokolorowe piksele:
 * bia³e (255, 255, 255) dla pikseli, których œrednia wartoœæ koloru jest wiêksza ni¿ wartoœæ progowa ('value'),
 * oraz czarne (0, 0, 0) dla pikseli o œredniej wartoœci poni¿ej lub równej wartoœci progowej.
 *
 * @param data  WskaŸnik do tablicy surowych danych pikseli obrazu.
 * @param size  Liczba pikseli w tablicy.
 * @param value Wartoœæ progowa do binaryzacji pikseli (0-255).
 */
void BinaryCpp(uint8_t* data, int size, int value) {
    // Pêtla przechodz¹ca przez tablicê bajtów reprezentuj¹c¹ obraz.
    for (int i = 0; i < size; i += 3) {
        // Obliczenie œredniej wartoœci sk³adowych koloru (czerwonej, zielonej i niebieskiej) dla danego piksela.
        uint8_t avg = (data[i] + data[i + 1] + data[i + 2]) / 3;

        // Porównanie œredniej wartoœci z zadanym progiem binaryzacji (value).
        if (avg > value)
            data[i] = data[i + 1] = data[i + 2] = 255; // Jeœli œrednia jest wiêksza ni¿ próg, ustawia kolor na bia³y.
        else
            data[i] = data[i + 1] = data[i + 2] = 0; // W przeciwnym przypadku ustawia kolor na czarny.
    }
}