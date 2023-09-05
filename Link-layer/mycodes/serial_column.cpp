#include <iostream>
#include <vector>
#include <string>

std::string serialize2DVector(const std::vector<std::vector<int>>& inputVector) {
    std::string serializedString;
    
    // Get the number of rows and columns
    size_t numRows = inputVector.size();
    size_t numCols = (numRows > 0) ? inputVector[0].size() : 0;
    
    // Serialize column-wise
    for (size_t col = 0; col < numCols; col++) {
        for (size_t row = 0; row < numRows; row++) {
            serializedString += std::to_string(inputVector[row][col]);
        }
    }
    
    return serializedString;
}

// int main() {
//     // Example 2D vector
//     std::vector<std::vector<int>> msgBit = {
//         {1, 0, 1},
//         {0, 1, 0},
//         {1, 1, 0}
//     };
    
//     std::string serialized = serialize2DVector(msgBit);
//     std::cout << "Serialized String: " << serialized << std::endl;
    
//     return 0;
// }
