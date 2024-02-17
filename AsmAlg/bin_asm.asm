; Temat projektu: Binaryzacja obraz�w
; Opis algorytmu:
; Binaryzacja to proces konwersji obrazu na dwukolorowy, czarno-bia�y. 
; Algorytm obejmuje wyb�r progu, powy�ej kt�rego piksele s� przypisywane do bia�ego, a poni�ej do czarnego. 
; U�ywana jest adaptacyjna binaryzacja, dostosowuj�ca pr�g lokalnie. 
; Data wykonania projektu: 16.01.2024
; Semestr: 5
; Rok akademicki: 2023/24
; Autor: Pawe� Pluta
; v3 - v1(algorytm w asm oraz .cpp), v2(gui), v3(poprawki, komentarze oraz wyko�czenie)

.model flat, stdcall
.data

; BinaryAsm to funkcja napisana w asemblerze wykonuj�ca algorytm binaryzacji obrazu.
;
; Parametry wej�ciowe:
; - data - Wska�nik do tablicy surowych danych pikseli obrazu (ka�de pole to jeden ze sk�adnik�w RGB).
; - size_ - Liczba wszystkich pikseli na obrazie pomno�ona przez 3 (3 warto�ci RGB).
; - value - Warto�� progowa podana przez u�ytkownika do binaryzacji pikseli (warto�ci: 0-255).
;
; Parametry wyj�ciowe:
; - data - Tablica z pikselami nowo utworzonej bitmapy.
;
; Po wykonaniu funkcji, tablica pikseli 'data' zostanie zaktualizowana zgodnie z algorytmem binaryzacji.
; Wszystkie piksele zostan� zamienione na jednokolorowe piksele:
; bia�e (255, 255, 255) dla pikseli, kt�rych �rednia warto�� koloru jest wi�ksza ni� warto�� progowa (value),
; czarne (0, 0, 0) dla pikseli o �redniej warto�ci poni�ej lub r�wnej warto�ci progowej.

.code
BinaryAsm proc data: DWORD, size_: DWORD, value: DWORD
    XOR eax, eax               ; Zerowanie rejestru eax (u�ywanego jako tymczasowy rejestr)
    MOV ebx, data              ; Ustawienie ebx na adres danych wej�ciowych
    MOV ecx, size_             ; Ustawienie ecx na rozmiar danych wej�ciowych
    MOV edx, value             ; Wczytanie warto�ci progowej do edx
    MOV edi, 3                 ; Ustawienie edi na warto�� 3 (do dzielenia)

main_loop:
    XORPS xmm0, xmm0           ; Zerowanie xmm0 przy u�yciu instrukcji XORPS dla SSE2

    MOV eax, [ebx]             ; Wczytanie pierwszej warto�ci RGB pixela do eax
    PINSRB xmm0, eax, 0        ; Wprowadzenie pierwszego bajtu do xmm0 na pozycji 0
    MOV eax, [ebx + 1]         ; Wczytanie drugiej warto�ci RGB pixela do eax
    PINSRB xmm0, eax, 1        ; Wprowadzenie drugiego bajtu do xmm0 na pozycji 1
    MOV eax, [ebx + 2]         ; Wczytanie trzeciej warto�ci RGB pixela do eax
n    PINSRB xmm0, eax, 2        ; Wprowadzenie trzeciego bajtu do xmm0 na pozycji 2

    MOVD eax, xmm0             ; Skopiowanie ca�ego s�owa do eax

    DIV edi                    ; Dzielenie eax przez edi (3), wynik do eax
    CMP eax, edx               ; Por�wnanie wyniku dzielenia z warto�ci� progow�
    JBE below                  ; Skok do etykiety below, je�li mniejsze lub r�wne

above:
    MOV eax, 255               ; Ustawienie eax na 255 (bia�y piksel)
    JMP finish                 ; Skok do etykiety end

below:
    MOV eax, 0                 ; Ustawienie eax na 0 (czarny piksel)

finish:
    MOV [ebx], al              ; Zapisanie warto�ci do pierwszego bajtu danych wyj�ciowych
                               ; (al to najmniejsza cz�� rejestru og�lnego eax)
    MOV [ebx + 1], al          ; Zapisanie warto�ci do drugiego bajtu danych wyj�ciowych
    MOV [ebx + 2], al          ; Zapisanie warto�ci do trzeciego bajtu danych wyj�ciowych

    ADD ebx, 3                 ; Przesuni�cie wska�nika danych wej�ciowych o 3 bajty
    SUB ecx, 3                 ; Zmniejszenie licznika rozmiaru danych wej�ciowych o 3
    JG main_loop               ; Skok do etykiety loop, je�li rozmiar danych wej�ciowych jest wi�kszy od 0
    ret                        ; Zako�czenie procedury

BinaryAsm endp
end