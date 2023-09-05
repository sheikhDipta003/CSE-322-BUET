#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <math.h>
#include <windows.h>
#include "hamming.cpp"
#include "serial_column.cpp"
#include "crc.cpp"

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
vector<int> bitsetToIntVector(const bitset<8>& bitset);
void showElements(vector<int> vect);
string padDataString(const string& data, int m);

int main() {
    string data;
    int m;
    double p;
    string generator;
    
    cout << "enter data string : ";
    getline(cin, data);
    // cin.ignore();
    
    cout << "enter number of data bytes in a row <m>: "; cin >> m;
    cout << "enter probability <p> : "; cin >> p;
    cout << "enter generator polynomial : "; cin >> generator;
    // cin.ignore();
    
    // Pad the data string if needed and print the updated data string
    string paddedData = padDataString(data, m);
    cout << "\ndata string after padding: " << paddedData << endl;
    
    int dataSize = paddedData.size();
    int numRows = dataSize / m;
    
    // Create the data block and print it in binary format
    cout << "\ndata block (ascii code of m characters per row) : " <<endl;
    vector<vector<int>> msgBit(numRows);
    for (int i = 0; i < numRows; i++) {
        string row = paddedData.substr(i * m, m);
        for (char c : row) {
            bitset<8> binary(c);
            cout << binary;
            vector<int> result = bitsetToIntVector(binary);
            msgBit[i].insert(msgBit[i].end(), result.begin(), result.end());
        }
        cout << endl;
    }

    // cout << "\nverify ascii conversion: \n";
    // for(vector<int> row : msgBit){
    //     for(int b : row){
    //         cout << b ;
    //     }
    //     cout << endl;
    // }

    cout << "\ndata block after adding check bits: "<< endl;
    vector<vector<int>> hammingCode;
    for(int i = 0; i < numRows; i++){
        hammingCode.push_back(findHammingCode(msgBit[i]));
        showElements(hammingCode.back());
        cout << endl;
    }

    cout << "\ndata bits after column-wise serialization: "<<endl;
    string serialized = serialize2DVector(hammingCode);
    cout << serialized << endl;

    cout << "\ndata bits after appending CRC checksum <sent frame>: "<<endl;
    cout << encodeData(serialized, generator) << endl;

    return 0;
}

vector<int> bitsetToIntVector(const bitset<8>& bitset) {
    vector<int> intVector;
    
    // Iterate through the bits in the bitset and push them into the vector
    for (int i = 7; i >= 0; i--) {  // Start from the most significant bit (bit 7) down to the least significant (bit 0)
        intVector.push_back(bitset[i]);
    }
    
    return intVector;
}

// prints all elements of any container in given outputstream
void showElements(vector<int> vect) {
    for(int i = 0; i < vect.size(); i++){
        if(i==0 || (i & (i + 1)) == 0)   SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        else SetConsoleTextAttribute(hConsole, 7);      //white
        cout << vect[i];
    }
}

// Function to pad the data string with '~' if needed
string padDataString(const string& data, int m) {
    if( !(data.size()%m) ) return data;
    
    int f = ceil(data.size()*1.0 / m);
    int paddingSize = m * f - data.size();
    string paddedData = data;
    
    for (int i = 0; i < paddingSize; i++) {
        paddedData += '~';
    }
    
    return paddedData;
}
