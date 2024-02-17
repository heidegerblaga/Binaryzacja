; Temat projektu: Binaryzacja obrazów
; Opis algorytmu:
; Binaryzacja to proces konwersji obrazu na dwukolorowy, czarno-bia³y. 
; Algorytm obejmuje wybór progu, powy¿ej którego piksele s¹ przypisywane do bia³ego, a poni¿ej do czarnego. 
; U¿ywana jest adaptacyjna binaryzacja, dostosowuj¹ca próg lokalnie. 
; Data wykonania projektu: 16.01.2024
; Semestr: 5
; Rok akademicki: 2023/24
; Autor: Pawe³ Pluta
; v3 - v1(algorytm w asm oraz .cpp), v2(gui), v3(poprawki, komentarze oraz wykoñczenie)

.model flat, stdcall
.data

; BinaryAsm to funkcja napisana w asemblerze wykonuj¹ca algorytm binaryzacji obrazu.
;
; Parametry wejœciowe:
; - data - WskaŸnik do tablicy surowych danych pikseli obrazu (ka¿de pole to jeden ze sk³adników RGB).
; - size_ - Liczba wszystkich pikseli na obrazie pomno¿ona przez 3 (3 wartoœci RGB).
; - value - Wartoœæ progowa podana przez u¿ytkownika do binaryzacji pikseli (wartoœci: 0-255).
;
; Parametry wyjœciowe:
; - data - Tablica z pikselami nowo utworzonej bitmapy.
;
; Po wykonaniu funkcji, tablica pikseli 'data' zostanie zaktualizowana zgodnie z algorytmem binaryzacji.
; Wszystkie piksele zostan¹ zamienione na jednokolorowe piksele:
; bia³e (255, 255, 255) dla pikseli, których œrednia wartoœæ koloru jest wiêksza ni¿ wartoœæ progowa (value),
; czarne (0, 0, 0) dla pikseli o œredniej wartoœci poni¿ej lub równej wartoœci progowej.

.code
BinaryAsm proc data: DWORD, size_: DWORD, value: DWORD
    XOR eax, eax               ; Zerowanie rejestru eax (u¿ywanego jako tymczasowy rejestr)
    MOV ebx, data              ; Ustawienie ebx na adres danych wejœciowych
    MOV ecx, size_             ; Ustawienie ecx na rozmiar danych wejœciowych
    MOV edx, value             ; Wczytanie wartoœci progowej do edx
    MOV edi, 3                 ; Ustawienie edi na wartoœæ 3 (do dzielenia)

main_loop:
    XORPS xmm0, xmm0           ; Zerowanie xmm0 przy u¿yciu instrukcji XORPS dla SSE2

    MOV eax, [ebx]             ; Wczytanie pierwszej wartoœci RGB pixela do eax
    PINSRB xmm0, eax, 0        ; Wprowadzenie pierwszego bajtu do xmm0 na pozycji 0
    MOV eax, [ebx + 1]         ; Wczytanie drugiej wartoœci RGB pixela do eax
    PINSRB xmm0, eax, 1        ; Wprowadzenie drugiego bajtu do xmm0 na pozycji 1
    MOV eax, [ebx + 2]         ; Wczytanie trzeciej wartoœci RGB pixela do eax
n    PINSRB xmm0, eax, 2        ; Wprowadzenie trzeciego bajtu do xmm0 na pozycji 2

    MOVD eax, xmm0             ; Skopiowanie ca³ego s³owa do eax

    DIV edi                    ; Dzielenie eax przez edi (3), wynik do eax
    CMP eax, edx               ; Porównanie wyniku dzielenia z wartoœci¹ progow¹
    JBE below                  ; Skok do etykiety below, jeœli mniejsze lub równe

above:
    MOV eax, 255               ; Ustawienie eax na 255 (bia³y piksel)
    JMP finish                 ; Skok do etykiety end

below:
    MOV eax, 0                 ; Ustawienie eax na 0 (czarny piksel)

finish:
    MOV [ebx], al              ; Zapisanie wartoœci do pierwszego bajtu danych wyjœciowych
                               ; (al to najmniejsza czêœæ rejestru ogólnego eax)
    MOV [ebx + 1], al          ; Zapisanie wartoœci do drugiego bajtu danych wyjœciowych
    MOV [ebx + 2], al          ; Zapisanie wartoœci do trzeciego bajtu danych wyjœciowych

    ADD ebx, 3                 ; Przesuniêcie wskaŸnika danych wejœciowych o 3 bajty
    SUB ecx, 3                 ; Zmniejszenie licznika rozmiaru danych wejœciowych o 3
    JG main_loop               ; Skok do etykiety loop, jeœli rozmiar danych wejœciowych jest wiêkszy od 0
    ret                        ; Zakoñczenie procedury

BinaryAsm endp
end