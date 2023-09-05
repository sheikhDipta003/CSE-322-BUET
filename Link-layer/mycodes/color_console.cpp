// color your text in Windows console mode
// colors are 0=black 1=blue 2=green and so on to 15=white  
// colorattribute = foreground + background * 16
// to get red text on yellow use 4 + 14*16 = 228
// light red on yellow would be 12 + 14*16 = 236
// a Dev-C++ tested console application by  vegaseat  07nov2004

#include <iostream>
#include <windows.h>   // WinApi header

using namespace std;    // std::cout, std::cin

int main()
{
    HANDLE  hConsole;
    int k;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // you can loop k higher to see more color choices
    // FOREGROUND_BLUE	Text color contains blue.
    // FOREGROUND_GREEN	Text color contains green.
    // FOREGROUND_RED	Text color contains red.
    for(k = 0; k < 8; k++)
    {
        // pick the colorattribute k you want
        SetConsoleTextAttribute(hConsole, k);
        cout << k << " I want to be nice today!" << endl;
    }

    // SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    // cout << 1 << " I want to be nice today!" << endl;
    // SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    // cout << 1 << " I want to be nice today!" << endl;
    // SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    // cout << 1 << " I want to be nice today!" << endl;

    // cin.get(); // wait
    return 0;
}