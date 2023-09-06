#include <iostream>
#include <vector>
#include <string>

string serialize2DVector(const vector<vector<int>>& inputVector) {
    string serializedString;
    
    // Get the number of rows and columns
    size_t numRows = inputVector.size();
    size_t numCols = (numRows > 0) ? inputVector[0].size() : 0;
    
    // Serialize column-wise
    for (size_t col = 0; col < numCols; col++) {
        for (size_t row = 0; row < numRows; row++) {
            serializedString += to_string(inputVector[row][col]);
        }
    }
    
    return serializedString;
}

int find(vector<int> vect, int x){
    for(int i = 0; i < vect.size(); i++){
        if(vect[i] == x) return i;
    }
    return -1;
}

vector<vector<int>> deserializeInto2DVector(string str, vector<int>& errorBits, int nRows) {
    vector<vector<int>> block(nRows);

    int r = 0;
    for(int i = 0; i < str.size(); i++){
        if(find(errorBits, i) >= 0){
            
        }
    }

    // string serializedString;
    
    // // Get the number of rows and columns
    // size_t numRows = inputVector.size();
    // size_t numCols = (numRows > 0) ? inputVector[0].size() : 0;
    
    // // Serialize column-wise
    // for (size_t col = 0; col < numCols; col++) {
    //     for (size_t row = 0; row < numRows; row++) {
    //         serializedString += to_string(inputVector[row][col]);
    //     }
    // }
    
    // return serializedString;
}

// int main() {
//     // Example 2D vector
//     vector<vector<int>> msgBit = {
//         {1, 0, 1},
//         {0, 1, 0},
//         {1, 1, 0}
//     };
    
//     string serialized = serialize2DVector(msgBit);
//     cout << "Serialized String: " << serialized << endl;
    
//     return 0;
// }
