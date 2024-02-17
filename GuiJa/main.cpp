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

#include <iostream>
#include <vector>
#include <thread>
#include <shlobj.h> 
#include <map>
#include "bin_cpp.h"

using namespace std;

// Define constants for different controls
#define BUTTONINPUTFILE 1
#define BUTTONOUTPUTFILE 2
#define BUTTONRUN 3
#define EDITNINPUTFILE 4
#define EDITONOUTPUTFILE 5
#define CHECKBOXOUTPUTFILEENABLE 6
#define RADIOBUTTONCPP 7
#define RADIOBUTTONASM 8
#define TOGGLEVALUE 9
#define PROGRESSBARR 10
#define THREADS 11
#define TIME 12
#define HISTOGRAM 13

// Global variables for GUI controls
MSG Message;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND ButtonBrowseInputFile;
HWND ButtonBrowseOutputFile;
HWND ButtonRun;
HWND HistogramButton;
HWND EditBrowseInputFile;
HWND EditBrowseOutputFile;
HWND CheckBoxOutputFileEnable;
HWND RadioButtonCPP;
HWND RadioButtonASM;
bool isAsm = false;  // Flag indicating whether the ASM algorithm is selected
bool isGood = false;
HWND valSlider;
HWND hThreadSlider;
HWND TimeLabel;
HWND ProgressBar;

// Strings to store file and directory names
wstring filename;
wstring dirName;
wstring prevfilename;

// Bitmap handle for displaying images
HBITMAP hBitmap;

// Function to display a single-color histogram
void DisplaySingleColorHistogram(HDC hdc, int xPosition, int yPosition, int width, int height, int histogram[], COLORREF color);

// Function to display a full color histogram
void DisplayColorHistogram(HDC hdc, HDC hdcMem, int xPosition, int yPosition, int width, int height);

// Function to normalize the histogram data
void NormalizeHistogram(int histogram[], int size, int maxHeight);

// Function to clear a specified region in the device context
void clear(HDC hdc, int xPosition, int yPosition, int width, int height);

// Function to add controls to the window
void Controllers(HWND hWnd);

// Function to open a file dialog and return the selected file path
wstring InputFile(HWND hWnd);

// Function to browse for a folder and return the selected directory path
wstring OutputFile(string saved_path);

// Function to run the image processing algorithm based on user input
void algorithm(int toggleVal, int threads);

// Function to display a bitmap image
void DisplayBitmap(HDC hdc, LPCWSTR bmpFileName, int type, int side, int histo);

// External C function declaration (BinaryAsm) - potentially implemented in assembly language
extern "C" int _stdcall BinaryAsm(DWORD data, DWORD size_, DWORD value);

// Entry point for the Windows application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Window class structure
    WNDCLASSEX wc;

    // Initialize the window class structure
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"Bmp file modyficator";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Register the window class
    if (!RegisterClassEx(&wc))
        return 0;

    // Create the main window without maximize button (without WS_MAXIMIZEBOX)
    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"bmp file modyficator", L"Projekt JA",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800, NULL, NULL, hInstance, NULL);

    // Check if window creation is successful
    if (hwnd == NULL)
        return 0;

    // Show and update the main window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Initialize common controls (progress bar, etc.)
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // Message loop
    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    // Return the wParam from the last message received
    return Message.wParam;
}

// Window procedure for handling messages related to the main window
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        switch (wParam) {
        case BUTTONINPUTFILE:
            // Open file dialog and set the selected file name in the input edit control
            filename = InputFile(hwnd);
            SetWindowText(EditBrowseInputFile, filename.c_str());

            // Display the selected bitmap in the window
            {
                HDC hdc = GetDC(hwnd);
                DisplayBitmap(hdc, filename.c_str(), 1, 2, 0);
                ReleaseDC(hwnd, hdc);
            }
            break;
        case BUTTONOUTPUTFILE:
            // Browse for a folder and set the selected directory in the output edit control
            dirName = OutputFile("C:\\");
            SetWindowText(EditBrowseOutputFile, dirName.c_str());
            break;
        case BUTTONRUN:
            // Run the algorithm with user-defined parameters
            SendMessage(ProgressBar, PBM_SETPOS, (WPARAM)0, 0);
            {
                Sleep(100);
                int val = SendMessageW(valSlider, TBM_GETPOS, 0, 0);
                if (!IsWindow(valSlider) || (val < 0 || val > 255)) {
                    MessageBoxA(hwnd, "Could not get first number!" + val, "Error", MB_OK | MB_ICONERROR);
                    return 0;
                }

                int threads = SendMessageW(hThreadSlider, TBM_GETPOS, 0, 0);
                if (!IsWindow(hThreadSlider) || (threads < 1 || threads > 64)) {
                    MessageBoxA(hwnd, "Could not get first number!", "Error", MB_OK | MB_ICONERROR);
                    return 0;
                }

                // Run the algorithm with the specified parameters
                algorithm(val, threads);

                // Display the resulting bitmap if the algorithm execution was successful
                if (isGood) {
                    HDC hdc = GetDC(hwnd);
                    std::wstring bmpFileName = dirName + L"\\output.bmp";
                    DisplayBitmap(hdc, bmpFileName.c_str(), 2, 0, 0);
                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        case HISTOGRAM:
            // Display the histogram for the currently loaded bitmap
        {
            HDC hdc = GetDC(hwnd);
            DisplayBitmap(hdc, filename.c_str(), 2, 0, 1);
            ReleaseDC(hwnd, hdc);
        }

        // Display the resulting bitmap if the algorithm execution was successful
        if (isGood) {
            HDC hdc = GetDC(hwnd);
            std::wstring bmpFileName = dirName + L"\\output.bmp";
            DisplayBitmap(hdc, bmpFileName.c_str(), 2, 0, 2);
            ReleaseDC(hwnd, hdc);
        }
        break;
        case RADIOBUTTONCPP:
            // Set the algorithm type to CPP
            isAsm = false;
            CheckRadioButton(hwnd, RADIOBUTTONCPP, RADIOBUTTONASM, RADIOBUTTONCPP);
            break;
        case RADIOBUTTONASM:
            // Set the algorithm type to ASM
            isAsm = true;
            CheckRadioButton(hwnd, RADIOBUTTONCPP, RADIOBUTTONASM, RADIOBUTTONASM);
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
        // Handle painting messages
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        int newWidth = 400;
        int newHeight = 300;
        int xPosition = 30;
        int yPosition = 10;

        // Clear specific regions in the window for display
        clear(hdc, xPosition, yPosition, newWidth, newHeight);
        clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth , newHeight + 20);
        xPosition = 575;
        clear(hdc, xPosition, yPosition, newWidth, newHeight);
        clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_CREATE:
        // Initialize and add controls to the window
        Controllers(hwnd);
        break;
    case WM_CLOSE:
        // Close the window
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        // Post a quit message to terminate the application
        PostQuitMessage(0);
        break;
    default:
        // Handle other messages using the default window procedure
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Function to display a bitmap with optional histogram
void DisplayBitmap(HDC hdc, LPCWSTR bmpFileName, int type, int side, int histo) {
    // Load the bitmap from file
    hBitmap = (HBITMAP)LoadImage(NULL, bmpFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (hBitmap != NULL) {
        // Retrieve bitmap information
        BITMAP bm;
        GetObject(hBitmap, sizeof(BITMAP), &bm);

        // Set the dimensions of the display area
        int newWidth = 400;
        int newHeight = 300;

        int xPosition;
        int yPosition = 10;
        if (type == 1)
            xPosition = 30;
        else
            xPosition = 575;

        // Create compatible device contexts and bitmaps
        HDC hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, hBitmap);

        HBITMAP hNewBitmap = CreateCompatibleBitmap(hdc, newWidth, newHeight);
        HDC hdcScaled = CreateCompatibleDC(hdc);
        SelectObject(hdcScaled, hNewBitmap);

        // Clear the specified regions based on the 'side' parameter
        if (side == 1) {
            xPosition = 30;
            clear(hdc, xPosition, yPosition, newWidth, newHeight);
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
            xPosition = 575;
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
        }
        else if (side == 2) {
            xPosition = 575;
            clear(hdc, xPosition, yPosition, newWidth, newHeight);
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
            xPosition = 30;
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
        }
        else if (side == 3) {
            xPosition = 30;
            clear(hdc, xPosition, yPosition, newWidth, newHeight);
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
            xPosition = 575;
            clear(hdc, xPosition, yPosition, newWidth, newHeight);
            clear(hdc, xPosition, (yPosition + newHeight + 10), newWidth, newHeight + 20);
        }

        // Display histogram or the stretched bitmap
        if (histo != 0) {
            if (histo == 1)
                xPosition = 30;
            else
                xPosition = 575;

            // Display color histogram
            DisplayColorHistogram(hdc, hdcMem, xPosition, (yPosition + newHeight + 10), newWidth, newHeight);
        }
        else {
            // Stretch and display the bitmap
            SetStretchBltMode(hdcScaled, HALFTONE);
            StretchBlt(hdcScaled, 0, 0, newWidth, newHeight, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            BitBlt(hdc, xPosition, yPosition, newWidth, newHeight, hdcScaled, 0, 0, SRCCOPY);
        }

        // Clean up resources
        DeleteDC(hdcMem);
        DeleteDC(hdcScaled);
        DeleteObject(hBitmap);
        DeleteObject(hNewBitmap);
    }
}

// Function to display a color histogram for an image
void DisplayColorHistogram(HDC hdc, HDC hdcMem, int xPosition, int yPosition, int width, int height) {
    const int NUM_BINS = 256;
    int redHistogram[NUM_BINS] = { 0 };
    int greenHistogram[NUM_BINS] = { 0 };
    int blueHistogram[NUM_BINS] = { 0 };
    int totalPixels = 0;
    int totalRed = 0;
    int totalGreen = 0;
    int totalBlue = 0;

    // Retrieve bitmap information
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);

    // Calculate color histograms and accumulate total values
    for (int y = 0; y < bm.bmHeight; y++) {
        for (int x = 0; x < bm.bmWidth; x++) {
            COLORREF pixelColor = GetPixel(hdcMem, x, y);
            int red = GetRValue(pixelColor);
            int green = GetGValue(pixelColor);
            int blue = GetBValue(pixelColor);

            redHistogram[red]++;
            greenHistogram[green]++;
            blueHistogram[blue]++;

            totalPixels++;
            totalRed += red;
            totalGreen += green;
            totalBlue += blue;
        }
    }

    // Normalize the histograms
    NormalizeHistogram(redHistogram, NUM_BINS, height);
    NormalizeHistogram(greenHistogram, NUM_BINS, height);
    NormalizeHistogram(blueHistogram, NUM_BINS, height);

    // Display individual color histograms with text labels
    DisplaySingleColorHistogram(hdc, xPosition, yPosition, width, height, redHistogram, RGB(255, 0, 0));
    DisplaySingleColorHistogram(hdc, xPosition, yPosition, width, height, greenHistogram, RGB(0, 255, 0));
    DisplaySingleColorHistogram(hdc, xPosition, yPosition, width, height, blueHistogram, RGB(0, 0, 255));

    // Calculate average RGB values
    int avgRed = totalRed / totalPixels;
    int avgGreen = totalGreen / totalPixels;
    int avgBlue = totalBlue / totalPixels;

    // Display text label
    SetTextColor(hdc, RGB(0, 0, 0)); // Set text color to black
    SetBkColor(hdc, RGB(255, 255, 255)); // Set background color to white

    char labelText[50]; // Adjust the size based on your needs
    sprintf_s(labelText, "R: %d G: %d B: %d", avgRed, avgGreen, avgBlue); // Format the label text

    TextOutA(hdc, xPosition + 130, yPosition + height + 5, labelText, strlen(labelText));
}

// Function to normalize a histogram to fit within a specified height
void NormalizeHistogram(int histogram[], int size, int maxHeight) {
    int maxValue = 0;
    for (int i = 0; i < size; i++)
        if (histogram[i] > maxValue)
            maxValue = histogram[i];

    for (int i = 0; i < size; i++)
        histogram[i] = static_cast<int>((static_cast<double>(histogram[i]) / maxValue) * maxHeight);
}

// Function to display a single-color histogram bar with text labels
void DisplaySingleColorHistogram(HDC hdc, int xPosition, int yPosition, int width, int height, int histogram[], COLORREF color) {
    xPosition += 70;
    for (int i = 0; i < 256; i++) {
        int barHeight = histogram[i];
        int barWidth = width / 256;

        if (barHeight > 0) {
            int startX = xPosition + i * barWidth;
            int startY = yPosition + height - barHeight;
            int endX = startX + barWidth;
            int endY = yPosition + height;

            // Draw histogram bar
            SelectObject(hdc, CreatePen(PS_SOLID, 1, color));
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, startX, startY, endX, endY);
        }
    }
}

// Function to clear a specified region in the device context
void clear(HDC hdc, int xPosition, int yPosition, int width, int height) {
    // Create a solid white brush
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));

    // Set background color and ROP2 mode
    SetBkColor(hdc, RGB(255, 255, 255));
    SetROP2(hdc, R2_COPYPEN);

    // Fill the specified rectangle with the white brush
    RECT rect = { xPosition, yPosition, xPosition + width, yPosition + height };
    FillRect(hdc, &rect, hBrush);

    // Delete the created brush
    DeleteObject(hBrush);
}

// Function to add controls to the window
void Controllers(HWND hWnd) {
    // Program name
    CreateWindowEx(NULL, L"Static", L"Binaryzacja         obrazów",
        WS_CHILD | WS_VISIBLE, 460, 20, 100, 60, hWnd, NULL, NULL, NULL);

    // Create "Algorytm" button
    ButtonRun = CreateWindowEx(WS_EX_CLIENTEDGE, L"Button", L"Algorytm",
        WS_CHILD | WS_VISIBLE, 452, 250, 100, 40, hWnd, (HMENU)BUTTONRUN, NULL, NULL);

    // Create "Histogram" button
    HistogramButton = CreateWindowEx(WS_EX_CLIENTEDGE, L"Button", L"Histogram",
        WS_CHILD | WS_VISIBLE, 452, 570, 100, 40, hWnd, (HMENU)HISTOGRAM, NULL, NULL);

    // Create input file edit control and browse button
    EditBrowseInputFile = CreateWindowEx(NULL, L"Edit", L"Plik wejœciowy (.bmp): ",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 80, 665, 225, 25, hWnd, (HMENU)EDITNINPUTFILE, NULL, NULL);
    ButtonBrowseInputFile = CreateWindowEx(WS_EX_CLIENTEDGE, L"Button", L"Wybierz",
        WS_CHILD | WS_VISIBLE, 315, 665, 80, 25, hWnd, (HMENU)BUTTONINPUTFILE, NULL, NULL);

    // Create output file edit control and browse button
    EditBrowseOutputFile = CreateWindowEx(NULL, L"Edit", L"Katalog wyjœciowy: ",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 80, 705, 225, 25, hWnd, (HMENU)EDITONOUTPUTFILE, NULL, NULL);
    ButtonBrowseOutputFile = CreateWindowEx(WS_EX_CLIENTEDGE, L"Button", L"Wybierz",
        WS_CHILD | WS_VISIBLE, 315, 705, 80, 25, hWnd, (HMENU)BUTTONOUTPUTFILE, NULL, NULL);

    // Create radio buttons for algorithm type selection
    RadioButtonCPP = CreateWindowEx(NULL, L"Button", L"Biblioteka cpp",
        WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 815, 675, 125, 30, hWnd, (HMENU)RADIOBUTTONCPP, NULL, NULL);
    RadioButtonASM = CreateWindowEx(NULL, L"Button", L"Biblioteka asm",
        WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 815, 710, 125, 30, hWnd, (HMENU)RADIOBUTTONASM, NULL, NULL);
    CheckRadioButton(hWnd, RADIOBUTTONCPP, RADIOBUTTONASM, RADIOBUTTONCPP);

    // Create threshold value slider
    CreateWindowW(L"Static", L"Próg wejœcia:", WS_CHILD | WS_VISIBLE, 450, 668, 225, 25, hWnd, (HMENU)20, NULL, NULL);
    valSlider = CreateWindowW(TRACKBAR_CLASSW, NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS,
        540, 665, 256, 25, hWnd, (HMENU)TOGGLEVALUE, NULL, NULL);
    SendMessageW(valSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 255));
    SendMessageW(valSlider, TBM_SETPOS, TRUE, 127);

    

    // Retrieve the optimal number of threads based on the hardware
    unsigned int optimalThreads = std::thread::hardware_concurrency();
    // Limit to the maximum value of the slider
    unsigned int threadsToSet = (optimalThreads > 64) ? 64 : optimalThreads;

    // Create a static control with the text "Number of threads"
    CreateWindowW(L"Static", L"W¹tki: ", WS_CHILD | WS_VISIBLE, 560, 708, 225, 25, hWnd, (HMENU)20, NULL, NULL);

    // Create a slider control
    hThreadSlider = CreateWindowW(TRACKBAR_CLASSW, NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS,
        600, 705, 200, 25, hWnd, (HMENU)THREADS, NULL, NULL); // Adjusted the position of the slider to the left

    // Set the range of the slider from 1 to 64
    SendMessageW(hThreadSlider, TBM_SETRANGE, TRUE, MAKELPARAM(1, 64));

    // Set the initial position of the slider to the optimal number of threads or 64 if the optimal number exceeds 64
    SendMessageW(hThreadSlider, TBM_SETPOS, TRUE, threadsToSet);





    // Create label for execution time display
    TimeLabel = CreateWindowExA(NULL, "Static", "0.00s", WS_CHILD | WS_VISIBLE, 477, 308, 70, 25, hWnd, (HMENU)TIME, NULL, NULL);

    // Create progress bar
    ProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        440, 340, 120, 20, hWnd, (HMENU)PROGRESSBARR, NULL, NULL);
    SendMessage(ProgressBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
}

// Function to open a file dialog and return the selected file path
wstring InputFile(HWND hWnd) {
    OPENFILENAME ofn;
    TCHAR szFile[260] = { 0 };

    // Initialize the structure for the file dialog
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Bmp\0*.bmp\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the file dialog and return the selected file path
    if (GetOpenFileName(&ofn) == TRUE)
        return wstring(ofn.lpstrFile);

    return wstring(L"");
}

// Callback function for browsing folders
static int CALLBACK CallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {
        // Set the initial folder selection for the folder browser
        wstring tmp = wstring((const wchar_t*)lpData);
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)tmp.c_str());
    }

    return 0;
}

// Function to browse for a folder and return the selected folder path
wstring OutputFile(string saved_path) {
    TCHAR path[MAX_PATH];

    // Convert the saved_path to a wstring
    wstring wsaved_path(saved_path.begin(), saved_path.end());
    const wchar_t* path_param = wsaved_path.c_str();

    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Browse for folder...";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = CallbackProc;
    bi.lParam = (LPARAM)path_param;

    // Display the folder browser and return the selected folder path
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0) {
        SHGetPathFromIDList(pidl, path);

        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        return path;
    }

    return wstring(L"");
}

// Function to load a bitmap file and return the image data
unsigned char* loadBitmapFile(char* filename, BITMAPFILEHEADER* bitmapFileHeader, BITMAPINFOHEADER* bitmapInfoHeader) {
    FILE* filePtr;
    unsigned char* bitmapImage;
    int imageIdx = 0;

    // Open the bitmap file for reading
    fopen_s(&filePtr, filename, "rb");
    if (filePtr == NULL)
        return NULL;

    // Read the bitmap file and info headers
    fread(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    // Check if the file is a valid BMP file
    if (bitmapFileHeader->bfType != 0x4D42) {
        fclose(filePtr);
        return NULL;
    }

    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    // Set the file pointer to the beginning of the image data
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage + 1024);

    // Check if memory allocation is successful
    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    // Read the image data
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

    // Check if the image data is successfully read
    if (bitmapImage == NULL) {
        fclose(filePtr);
        return NULL;
    }

    fclose(filePtr);
    return bitmapImage;
}

// Function to write a bitmap file with the given data
void writeBmp(char* filename, BITMAPFILEHEADER* bitmapFileHeader, BITMAPINFOHEADER* bitmapInfoHeader, unsigned char* data) {
    FILE* filePtr;

    // Open the bitmap file for writing
    fopen_s(&filePtr, filename, "wb");
    if (filePtr == NULL)
        return;

    // Write the file and info headers
    fwrite(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    fwrite(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    int widthInBytes = bitmapInfoHeader->biWidth * 3;
    unsigned char padding[3] = { 0, 0, 0 };

    // Write the image data
    fwrite(data, sizeof(char), bitmapInfoHeader->biSizeImage, filePtr);
    fclose(filePtr);
}

// Function to update the progress bar based on the number of threads
void updateStatus(int threads) {
    // Calculate the progress delta and update the progress bar
    int delta = 100 / threads;
    SendMessage(ProgressBar, PBM_DELTAPOS, (WPARAM)delta, 0);
}

// Function to check if a directory exists
bool dirExists(char* dirName_in) {
    // Check if the given path corresponds to an existing directory
    DWORD ftyp = GetFileAttributesA(dirName_in);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;

    return false;
}

// Function to run the binaryization algorithm based on user input
void algorithm(int toggleVal, int threads) {
    // Get the input file path from the Edit control
    char filename[1024];
    GetWindowTextA(EditBrowseInputFile, filename, 1024);

    // Initialize variables to store bitmap file information
    unsigned char* data;
    BITMAPFILEHEADER* bitmapFileHeader = (BITMAPFILEHEADER*)malloc(sizeof(BITMAPFILEHEADER));
    BITMAPINFOHEADER* bitmapInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));
    data = loadBitmapFile(filename, bitmapFileHeader, bitmapInfoHeader);

    // Check if the input file exists
    if (data == NULL) {
        MessageBoxA(NULL, "Input file does not exist!", "Error", MB_ICONINFORMATION | MB_OK);
        isGood = false;
        return;
    }

    // Get the output directory from the Edit control
    char output[1024];
    GetWindowTextA(EditBrowseOutputFile, output, 1024);

    // Check if the output directory exists
    if (!dirExists(output)) {
        MessageBoxA(NULL, "Output directory does not exist!", "Error", MB_ICONINFORMATION | MB_OK);
        isGood = false;
        return;
    }

    // Get the size of the image data
    int len = bitmapInfoHeader->biSizeImage;

    // Run the binaryization algorithm based on the specified number of threads
    if (threads == 1) {
        if (isAsm) {
            // Record the start time for performance measurement
            auto start = chrono::high_resolution_clock::now();
            BinaryAsm((DWORD)data, len, toggleVal);
            // Record the stop time and calculate the duration for performance measurement
            auto stop = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
            // Display the execution time on the window
            char buff[128];
            _snprintf_s(buff, 128, "%.3f ms", static_cast<double>(duration.count()) / 1000.0);
            SetWindowTextA(TimeLabel, buff);
        } else {
            // Record the start time for performance measurement
            auto start = chrono::high_resolution_clock::now();
            BinaryCpp(data, len, toggleVal);
            // Record the stop time and calculate the duration for performance measurement
            auto stop = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
            // Display the execution time on the window
            char buff[128];
            _snprintf_s(buff, 128, "%.3f ms", static_cast<double>(duration.count()) / 1000.0);
            SetWindowTextA(TimeLabel, buff);
        }

        // Update the progress bar
        updateStatus(threads);
    } else {
        int partialLen = len / threads;
        vector<thread> threadsList;

        // Declare the start time variable
        decltype(chrono::high_resolution_clock::now()) start;

        if (isAsm) {
            // Record the start time for performance measurement
            start = chrono::high_resolution_clock::now();
            // Create threads and run the algorithm in parallel
            for (int t = 0; t < threads; t++)
                threadsList.push_back(thread(BinaryAsm, (DWORD)&data[t * partialLen], partialLen, toggleVal));
        } else {
            // Record the start time for performance measurement
            start = chrono::high_resolution_clock::now();
            // Create threads and run the algorithm in parallel
            for (int t = 0; t < threads; t++)
                threadsList.push_back(thread(BinaryCpp, &data[t * partialLen], partialLen, toggleVal));
        }

        // Wait for all threads to complete and update the progress bar
        for (size_t i = 0; i < threadsList.size(); i++) {
            threadsList[i].join();
            updateStatus(threads);
        }

        // Record the stop time and calculate the duration for performance measurement
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        // Display the execution time on the window
        char buff[128];
        _snprintf_s(buff, 128, "%.3f ms", static_cast<double>(duration.count()) / 1000.0);
        SetWindowTextA(TimeLabel, buff);
    }

    // Generate the output file path and write the result to a new bitmap file
    strcat_s(output, "\\output.bmp");
    writeBmp(output, bitmapFileHeader, bitmapInfoHeader, data);

    // Free allocated memory
    free(data);
    free(bitmapFileHeader);
    free(bitmapInfoHeader);

    // Set progress bar to 100% and indicate success
    SendMessage(ProgressBar, PBM_DELTAPOS, (WPARAM)100, 0);
    isGood = true;
}