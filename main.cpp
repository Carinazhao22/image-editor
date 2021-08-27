#define OPEN_FILE_BUTTON 1
#define EXIT 2

#ifndef UNICODE
#define UNICODE
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types.hpp>

#include <iostream>
#include <windows.h>
#include <fstream>

using namespace cv;
using namespace std;

Mat quantization = (Mat_<int>(8, 8) << 1, 1, 2, 4, 8, 16, 32, 64,
                    1, 1, 2, 4, 8, 16, 32, 64,
                    2, 2, 2, 4, 8, 16, 32, 64,
                    4, 4, 4, 4, 8, 16, 32, 64,
                    8, 8, 8, 8, 8, 16, 32, 64,
                    16, 16, 16, 16, 16, 16, 32, 64,
                    32, 32, 32, 32, 32, 32, 32, 64,
                    64, 64, 64, 64, 64, 64, 64, 64);

HBITMAP hImageL, hImageR;
HWND hIMGL, hIMGR;
Mat Img, res;
char Count = '1';

// file path in different types
const char *fileName;
wchar_t originPath[100];

// GUI
char *wchar_to_char(const wchar_t *);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedure1(HWND, UINT, WPARAM, LPARAM);
void open_file(HWND);
void AddControls(HWND);
void loadImage2win(Mat);
void createChildWin();
void writeMatToFile(Mat, const char *);

// Encoding
void cal2DCT(Mat);
void quantized(Mat);
void cali2DCT(Mat);
void encode(Mat);

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

// Main Function for GUI: refer to https://www.youtube.com/watch?v=8GCvZs55mEM&list=PLWzp0Bbyy_3i750dsUj7yq4JrPOIUR_NK
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

// open file dialog
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

// create a subwindow to display images after loading files
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
    {
        cout<<"cannot create sub"<<endl;
        exit(0);
    }

    HWND hDlg1 = CreateWindowW(L"myWindowSubClass" + Count, L"My Child Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 2000, 800, NULL, NULL, NULL, NULL);
    Count++;

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
    case WM_CREATE:
        Img = imread(fileName);
        loadImage2win(Img);
        AddControls(hWnd);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wp, lp);
    }

    return 0;
}

void AddControls(HWND hWnd)
{
    hIMGL = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 0, 70, 100, 50, hWnd, NULL, NULL, NULL);
    hIMGR = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, Img.cols + 50, 70, 100, 50, hWnd, NULL, NULL, NULL);
    SendMessageW(hIMGL, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageL);
    SendMessageW(hIMGR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImageR);
}

void loadImage2win(Mat target)
{
    // encoding image
    encode(target);
    // reference to :https://stackoverflow.com/questions/26159489/loadimage-with-qrcode-bitmap-failing-unless-file-is-opened-saved-with-ms-paint
    imwrite("output.bmp", target);
    hImageL = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols, target.rows, LR_LOADFROMFILE);
    imwrite("output.bmp", res);
    hImageR = (HBITMAP)LoadImageW(0, L"output.bmp", IMAGE_BITMAP, target.cols, target.rows, LR_LOADFROMFILE);
}

// store quantized results to txt files
void writeMatToFile(Mat m, const char *filename)
{
    ofstream fout(filename);
    if (!fout)
        cout << "File Not Opened" << endl;

    for (int i = 0; i < m.rows; i++)
    {
        for (int j = 0; j < m.cols; j++)
            fout << m.at<int>(i, j) << "\t";
        fout << endl;
    }
    fout.close();
}

// encoding images
void encode(Mat target)
{
    Mat Y, U, V, YUV;

    // convert RGB2YUV
    if (target.channels() == 3)
        cvtColor(target, YUV, CV_RGB2YUV);

    // split each channel
    vector<Mat> channels(3);
    split(YUV, channels);
    Y = channels[0];
    U = channels[1];
    V = channels[2];

    // apply 2DCT to 8x8 block over images
    Y.convertTo(Y, CV_64F);
    U.convertTo(U, CV_64F);
    V.convertTo(V, CV_64F);
    cal2DCT(Y);
    cal2DCT(U);
    cal2DCT(V);
    Y.convertTo(Y, CV_16SC1);
    U.convertTo(U, CV_16SC1);
    V.convertTo(V, CV_16SC1);

    // start quantization
    Y.convertTo(Y, CV_64F);
    U.convertTo(U, CV_64F);
    V.convertTo(V, CV_64F);
    quantization.convertTo(quantization, CV_64F);
    quantized(Y);
    quantized(U);
    quantized(V);

    //rounded results
    Y.convertTo(Y, CV_16SC1);
    U.convertTo(U, CV_16SC1);
    V.convertTo(V, CV_16SC1);

    // store the final quantized result
    writeMatToFile(Y, "quantizedChannel1.txt");
    writeMatToFile(U, "quantizedChannel2.txt");
    writeMatToFile(V, "quantizedChannel3.txt");

    // apply invert dct to convert back images

    Y.convertTo(Y, CV_64F);
    U.convertTo(U, CV_64F);
    V.convertTo(V, CV_64F);
    cali2DCT(Y);
    cali2DCT(U);
    cali2DCT(V);

    Y.convertTo(Y, CV_8UC1);
    U.convertTo(U, CV_8UC1);
    V.convertTo(V, CV_8UC1);
    channels[0] = Y;
    channels[1] = U;
    channels[2] = V;

    // store final result to res
    merge(channels, YUV);
    cvtColor(YUV, res, CV_YUV2RGB);
}

void cal2DCT(Mat channel)
{
    for (int r = 0; r < channel.rows; r += 8)
        for (int c = 0; c < channel.cols; c += 8)
            dct(channel(Range(r, r + 8), Range(c, c + 8)), channel(Range(r, r + 8), Range(c, c + 8)));
}
void quantized(Mat channel)
{
    for (int r = 0; r < channel.rows; r += 8)
        for (int c = 0; c < channel.cols; c += 8)
            channel(Range(r, r + 8), Range(c, c + 8)) = channel(Range(r, r + 8), Range(c, c + 8)) / quantization;
}
void cali2DCT(Mat channel)
{
    for (int r = 0; r < channel.rows; r += 8)
        for (int c = 0; c < channel.cols; c += 8)
            idct(channel(Range(r, r + 8), Range(c, c + 8)), channel(Range(r, r + 8), Range(c, c + 8)));
}
