#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <math.h>
#include <windows.h>
#include <random>
#include "hamming.cpp"
#include "crc.cpp"
#define FOREGROUND_WHITE 7

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

vector<int> bitsetToIntVector(const bitset<8>& bitset);
void showElements(vector<int> vect);
string padDataString(const string& data, int m);
string serialize2DVector(const vector<vector<int>>& inputVector);
vector<vector<int>> deserializeInto2DVector(string str, vector<int>& errorBits, int nRows);
int find(vector<int> vect, int x);
string toggleBits(const string& inputString, double probability, vector<int>& errorBits);
int binaryStringToAscii(const string& binaryString);
void print2DVectorInRowMajor(const vector<vector<int>>& inputVector);

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
    int M = msgBit[0].size();
    for(int i = 0; i < numRows; i++){
        hammingCode.push_back(findHammingCode(msgBit[i]));
        showElements(hammingCode.back());
        cout << endl;
    }

    cout << "\ndata bits after column-wise serialization: "<<endl;
    string serialized = serialize2DVector(hammingCode);
    cout << serialized << endl;

    cout << "\ndata bits after appending CRC checksum <sent frame>: "<<endl;
    pair<string,string> crcFrame = encodeData(serialized, generator);       //<data,remainder>
    cout << crcFrame.first;
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << crcFrame.second << endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);

    cout << "\nreceived frame:\n";
    string txFrame = crcFrame.first + crcFrame.second;
    vector<int> errorBits;
    string rxFrame = toggleBits(txFrame, p, errorBits);
    cout << endl;

    cout << "\nresult of CRC checksum matching: " << receiver(rxFrame, generator) << endl;

    cout << "\ndata block after removing CRC checksum bits:\n";
    int lenChecksum = crcFrame.second.length();
    string noChecksum = rxFrame.substr(0, rxFrame.size()-lenChecksum);
    // cout << noChecksum << endl;
    vector<vector<int>> block = deserializeInto2DVector(noChecksum, errorBits, numRows);
    for(int i = 0; i < block.size(); i++){
        for(int j = 0; j < block[i].size(); j++){
            // Check if the current index is in errorBits and set text color to red
            if (find(errorBits.begin(), errorBits.end(), j * numRows + i) != errorBits.end()) {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            }
            cout << block[i][j];
            // Reset text color
            SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
        }
        cout << endl;
    }

    cout << "\ndata block after removing check bits:\n";
    vector<vector<int>> recovered;
    // r is the number of redundant bits
	int r = 1;

	// Find no. of redundant bits
	while (pow(2, r) < (M + r + 1)) {
		r++;
	}

    for(vector<int> row : block){
        recovered.push_back(recover(row, M, r));
        cout << endl;
    }

    cout << "\noutput frame: ";
    print2DVectorInRowMajor(recovered);
    cout << endl;

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
        else SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
        cout << vect[i];
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
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

// Function to toggle each bit of inputString eith prob probability
string toggleBits(const string& inputString, double probability, vector<int>& errorBits) {
    string result = inputString;
    random_device rd;
    default_random_engine generator(rd());
    uniform_real_distribution<double> distribution(0.0, 1.0);

    int i = 0;
    for (char& bit : result) {
        double randomValue = distribution(generator);
        if (randomValue < probability) {
            // Toggle the bit if the random value is less than the specified probability
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            bit = (bit == '0') ? '1' : '0';
            cout << bit;
            errorBits.push_back(i);
        }
        else{
            SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
            cout << bit;
        }
        i++;
    }

    return result;
}

string serialize2DVector(const vector<vector<int>>& inputVector) {
    string serializedString;
    
    // Get the number of rows and columns
    int numRows = inputVector.size();
    int numCols = (numRows > 0) ? inputVector[0].size() : 0;
    
    // Serialize column-wise
    for (int col = 0; col < numCols; col++) {
        for (int row = 0; row < numRows; row++) {
            serializedString += to_string(inputVector[row][col]);
        }
    }
    
    return serializedString;
}

vector<vector<int>> deserializeInto2DVector(string serializedString, vector<int>& errorBits, int numRows) {
    vector<vector<int>> result;

    // Calculate the number of columns based on the string length and the specified number of rows
    int numCols = (numRows > 0) ? (serializedString.length() + numRows - 1) / numRows : 0;

    if (numCols == 0) {
        cerr << "Error: Invalid number of rows or empty serialized string." << endl;
        return result;
    }

    // Initialize the result vector with the specified number of rows and columns
    result.resize(numRows, vector<int>(numCols, 0));

    int index = 0; // Current index in the serialized string

    // Deserialize column-major
    for (int col = 0; col < numCols; col++) {
        for (int row = 0; row < numRows; row++) {
            if (index < serializedString.length()) {
                // Convert the character to an integer and store it in the vector
                result[row][col] = serializedString[index] - '0';
                index++;
            }
        }
    }

    return result;
}

// Function to convert a binary string to ASCII code
int binaryStringToAscii(const std::string& binaryString) {
    // Convert the binary string to an integer
    int asciiValue = std::stoi(binaryString, nullptr, 2);
    return asciiValue;
}

// Function to traverse and print a 2D vector in row-major order
void print2DVectorInRowMajor(const std::vector<std::vector<int>>& matrix) {
    for (const std::vector<int>& row : matrix) {
        std::string binaryString;
        
        // Concatenate the elements in the current row as a binary string
        for (int bit : row) {
            binaryString += std::to_string(bit);
        }

        // Ensure that the binary string is a multiple of 8
        while (binaryString.length() % 8 != 0) {
            binaryString.insert(binaryString.begin(), '0');
        }

        // Print the ASCII codes by grouping subsequent 8 bits
        for (size_t i = 0; i < binaryString.length(); i += 8) {
            std::string eightBits = binaryString.substr(i, 8);
            int asciiCode = binaryStringToAscii(eightBits);
            std::cout << static_cast<char>(asciiCode);
        }
    }
    std::cout << std::endl;  // Print a newline after processing all rows
}