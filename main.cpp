#define OPEN_FILE_BUTTON 1
#define EXIT 2
#define NEXT 3

#ifndef UNICODE
#define UNICODE
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <windows.h>
#include <map>

using namespace cv;
using namespace std;

// GUI
char *wchar_to_char(const wchar_t *);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedure1(HWND, UINT, WPARAM, LPARAM);
void open_file(HWND);
void AddControls(HWND);
void loadImage2win(Mat);
void createChildWin();

// image operation
void rgb2grey();
void ordDither();
void autoLevel();
void insertKey(map<int, double> &, int);
void probabillty(map<int, double> &, int);
void cdf(map<int, double> &);

// Global Variables
HBITMAP hImageL, hImageR;
HWND hIMGL, hIMGR;
Mat Img, Grey, Dither, AutoLeveled;
int number;
char Count = '1';

// file path in different types
const char *fileName;
wchar_t originPath[100];

const int matSize = 8;
int dither[8][8] = {
    {0, 32, 8, 40, 2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44, 4, 36, 14, 46, 6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    {3, 35, 11, 43, 1, 33, 9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47, 7, 39, 13, 45, 5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}};

// int dither[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
// int dither[2][2] = {{0, 2}, {3, 1}};

// GUI: refer to https://www.youtube.com/watch?v=8GCvZs55mEM&list=PLWzp0Bbyy_3i750dsUj7yq4JrPOIUR_NK
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrerInst, LPSTR args, int ncmdshow)
{

    WNDCLASS wc = {};
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
    {
        return -1;
    }

    HWND hDlg = CreateWindowW(L"myWindowClass", L"My Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 500, 500, NULL, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Open File", WS_VISIBLE | WS_CHILD, 10, 100, 150, 36, hDlg, (HMENU)OPEN_FILE_BUTTON, NULL, NULL);
    CreateWindowW(L"Button", L"EXIT", WS_VISIBLE | WS_CHILD, 200, 100, 150, 36, hDlg, (HMENU)EXIT, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// helping with conversion
char *wchar_to_char(const wchar_t *pwchar)
{
    int currIdx = 0;
    char currChar = pwchar[currIdx];

    while (currChar != '\0')
        currChar = pwchar[++currIdx];

    // allocate char *
    char *res = (char *)malloc(sizeof(char) * (++currIdx));
    res[currIdx] = '\0';
    for (int i = 0; i < currIdx; i++)
        res[i] = char(pwchar[i]);

    return res;
}

// working procedure for main window
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (wp)
        {
        case OPEN_FILE_BUTTON:
            open_file(hWnd);
            break;
        case EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_DESTROY:
        exit(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}

void open_file(HWND hWnd)
{
    // define structure

    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = originPath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 100;
    ofn.lpstrFilter = L"All files\0*.*\0Image Files\0*.bmp\0";
    ofn.nFilterIndex = 1;

    // should link to lib
    GetOpenFileName(&ofn);
    fileName = static_cast<const char *>(wchar_to_char(ofn.lpstrFile));
    createChildWin();
}

// create a subwindow after loading files
void createChildWin()
{
    HINSTANCE hInst1;
    HINSTANCE hPrerInst1;
    LPSTR args1;
    int ncmdshow1;
    WNDCLASS wc1 = {};
    wc1.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc1.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc1.hInstance = hInst1;
    wc1.lpszClassName = L"myWindowSubClass" + Count;
    wc1.lpfnWndProc = WindowProcedure1;

    if (!RegisterClassW(&wc1))
        exit(0);
    number = 0;
    HWND hDlg1 = CreateWindowW(L"myWindowSubClass" + Count, L"My Child Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 2000, 800, NULL, NULL, NULL, NULL);
    Count++;
    CreateWindowW(L"Button", L"Next", WS_VISIBLE | WS_CHILD, 10, 10, 150, 36, hDlg1, (HMENU)NEXT, NULL, NULL);
    CreateWindowW(L"Button", L"EXIT", WS_VISIBLE | WS_CHILD, 200, 10, 150, 36, hDlg1, (HMENU)EXIT, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// working procedure for subwindow
LRESULT CALLBACK WindowProcedure1(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (wp)
        {
        case NEXT:
            if (number % 4 == 0)
            {
                loadImage2win(Img);
                AddControls(hWnd);
                number += 1;
            }
            else if (number % 4 == 1)
            {
                if (number == 1)
                    rgb2grey();
                loadImage2win(Grey);
                AddControls(hWnd);
                number += 1;
            }

            else if (number % 4 == 2)
            {
                if (number == 2)
                    ordDither();
                loadImage2win(Dither);
                AddControls(hWnd);
                number += 1;
            }
            else if (number % 4 == 3)
            {
                if (number == 3)
                    autoLevel();
                loadImage2win(AutoLeveled);
                AddControls(hWnd);
                number += 1;
            }
            break;
        case EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_CREATE:
        Img = imread(fileName);
        loadImage2win(Img);
        AddControls(hWnd);
        number += 1;
        break;
    default:
        return DefWindowProcW(hWnd, msg, wp, lp);
    }

    return 0;
}

// inspired by https://stackoverflow.com/questions/42802294/c-opencv-iterate-through-pixels-in-a-mat-which-is-roi-of-another-mat
void rgb2grey()
{
    Mat grayScaled(Img.rows, Img.cols, CV_8UC1);
    Vec3b *currRow;
    for (int y = 0; y < Img.rows; y++)
    {
        currRow = Img.ptr<Vec3b>(y);
        uchar *grey = grayScaled.ptr(y);
        for (int x = 0; x < Img.cols; x++)
        {
            // stored as BGR and grayscaled value = Y
            *grey = 0.114 * currRow[x][0] + 0.587 * currRow[x][1] + 0.299 * currRow[x][2];
            grey++;
        }
    }
    Grey = grayScaled;
}

// inspired by CMPT365 Lec3P47 algorithm
void ordDither()
{
    Mat ditherImg(Grey.rows, Grey.cols, CV_8UC1);
    const double N = 256 / (2 * matSize + 1);
    for (int y = 0; y < Grey.rows; y++)
    {
        uchar *gp = Grey.ptr(y);
        uchar *dp = ditherImg.ptr(y);
        int Y = y % matSize;
        for (int x = 0; x < Grey.cols; x++)
        {
            int X = x % matSize;
            if (*gp / N > dither[Y][X])
                *dp = 255;
            else
                *dp = 0;
            gp++;
            dp++;
        }
    }
    Dither = ditherImg;
}

void insertKey(map<int, double> &channel, int key)
{
    if (channel.find(key) != channel.end())
        channel[key] += 1;
    else
        channel[key] = 1;
}
void probabillty(map<int, double> &channel, int total)
{
    map<int, double>::iterator itr;
    for (itr = channel.begin(); itr != channel.end(); ++itr)
        itr->second /= double(total);
}
void cdf(map<int, double> &channel)
{
    for (int i = 0; i < 256; i++)
        if (channel.find(i) == channel.end())
            channel[i] = 0;
    for (int i = 1; i < 256; i++)
        channel[i] = channel[i] + channel[i - 1];
    for (int i = 0; i < 256; i++)
        channel[i] = int(channel[i] * 256);
}
// inspired by https://en.wikipedia.org/wiki/Histogram_equalization
void autoLevel()
{
    Img.copyTo(AutoLeveled);
    Vec3b *currRow;
    map<int, double> R, G, B;
    // step 1: frequency
    for (int y = 0; y < AutoLeveled.rows; y++)
    {
        currRow = AutoLeveled.ptr<Vec3b>(y);
        for (int x = 0; x < AutoLeveled.cols; x++)
        {
            // stored as BGR
            insertKey(R, currRow[x][2]);
            insertKey(G, currRow[x][1]);
            insertKey(B, currRow[x][0]);
        }
    }
    int total = AutoLeveled.rows * AutoLeveled.cols;

    // step2 probability
    probabillty(R, total);
    probabillty(G, total);
    probabillty(B, total);

    // step3 cdf
    cdf(R);
    cdf(G);
    cdf(B);

    // step4 back to array
    for (int y = 0; y < AutoLeveled.rows; y++)
    {
        currRow = AutoLeveled.ptr<Vec3b>(y);
        for (int x = 0; x < AutoLeveled.cols; x++)
        {
            currRow[x][0] = B[currRow[x][0]];
            currRow[x][1] = G[currRow[x][1]];
            currRow[x][2] = R[currRow[x][2]];
        }
    }
}

void AddControls(HWND hWnd)
{
    hIMGL = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 0, 70, 100, 50, hWnd, NULL, NULL, NULL);
    hIMGR = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, Img.cols * 0.9, 70, 100, 50, hWnd, NULL, NULL, NULL);
    SendMessageW(hIMGL, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageL);
    SendMessageW(hIMGR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageR);
}

void loadImage2win(Mat target)
{
    // delete L and R images
    hImageL = 0;
    hImageR = 0;
    SendMessageW(hIMGL, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageL);
    SendMessageW(hIMGR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageR);
    double r = 0.9;

    // display L or R images based on its number
    // reference to :https://stackoverflow.com/questions/26159489/loadimage-with-qrcode-bitmap-failing-unless-file-is-opened-saved-with-ms-paint
    // LoadImage cannot read original bmp files but can recognize the 24-bit bmp after imwrite.
    if (number % 4 == 0)
    {
     imwrite("output.bmp", target);
     hImageL = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
    }

    else if (number % 4 == 1)
    {
        imwrite("output.bmp", Img);
        hImageL = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
        imwrite("output.bmp", target);
        hImageR = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
    }
    else if (number % 4 == 2)
    {
        hImageL = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, Grey.cols * r, Grey.rows, LR_LOADFROMFILE);
        imwrite("output.bmp", target);
        hImageR = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
    }
    else if (number % 4 == 3)
    {
        imwrite("output.bmp", Img);
        hImageL = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
        imwrite("output.bmp", target);
        hImageR = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols * r, target.rows, LR_LOADFROMFILE);
    }
}
