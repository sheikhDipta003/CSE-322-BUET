// C++ program for the above approach

#include <bits/stdc++.h>
#include <algorithm>
using namespace std;

// Function to generate hamming code
vector<int> generateHammingCode(
	vector<int> msgBits, int m, int r)
{
	// Stores the Hamming Code
	vector<int> hammingCode(r + m);

	// Find positions of redundant bits
	for (int i = 0; i < r; ++i) {

		// Placing -1 at redundant bits
		// place to identify it later
		hammingCode[pow(2, i) - 1] = -1;
	}

	int j = 0;

	// Iterate to update the code
	for (int i = 0; i < (r + m); i++) {

		// Placing msgBits where -1 is
		// absent i.e., except redundant
		// bits all positions are msgBits
		if (hammingCode[i] != -1) {
			hammingCode[i] = msgBits[j];
			j++;
		}
	}

	for (int i = 0; i < (r + m); i++) {

		// If current bit is not redundant
		// bit then continue
		if (hammingCode[i] != -1)
			continue;

		int x = log2(i + 1);
		int one_count = 0;

		// Find msg bits containing
		// set bit at x'th position
		for (int j = i + 2;
			j <= (r + m); ++j) {

			if (j & (1 << x)) {
				if (hammingCode[j - 1] == 1) {
					one_count++;
				}
			}
		}

		// Generating hamming code for
		// even parity
		if (one_count % 2 == 0) {
			hammingCode[i] = 0;
		}
		else {
			hammingCode[i] = 1;
		}
	}

	// Return the generated code
	return hammingCode;
}

// Function to find the hamming code
// of the given message bit msgBit[]
vector<int> findHammingCode(vector<int>& msgBit)
{

	// Message bit size
	int m = msgBit.size();

	// r is the number of redundant bits
	int r = 1;

	// Find no. of redundant bits
	while (pow(2, r) < (m + r + 1)) {
		r++;
	}

	// Generating Code
	vector<int> ans = generateHammingCode(msgBit, m, r);
	return ans;

	// Print the code
	// cout << "Message bits are: ";
	// for (int i = 0; i < msgBit.size(); i++)
	// 	cout << msgBit[i] << " ";

	// cout << "\nHamming code is: ";
	// for (int i = 0; i < ans.size(); i++)
	// 	cout << ans[i] << " ";
}

vector<int> recover(vector<int>& rxBits, int m, int r){
	// cout << "\nreceived bits: ";
	// for(int b : rxBits){
	// 	cout << b << " ";
	// }
	// cout << endl;

	// cout << "\nm = " << m << ", r = " << r << endl;

	//this ans will store the redundant bits, if they were right then according to even parity they will store 0 else if some error was made in a bit it will store 1
	vector<char> ans;
	int bit = 0;
	//this loop corresponds to the logic used in set redundant bits function
	for(int i = 1 ; i  <= m+r ; i*=2){
		int count = 0;

		// cout << "\n";

		for(int j = i+1 ; j<=m+r ; j++){
			if(j & (1 << bit)){
				if(rxBits[j-1] == 1) count++;
				// cout << j << ", ";
			}
		}
		//incrementing the ans variable with the parity of redundant bit
		// if it was right then add 0 else 1
		if(count & 1){		//count of 1s in the message bits are odd
			if(rxBits[i-1] == 1) ans.push_back('0');		//the redundant bit at i-th position is 1, so parity even
			else ans.push_back('1');
		}
		else{
			if(rxBits[i-1]==0) ans.push_back('0');
			else ans.push_back('1');
		}
		bit++;
	}

	// cout << endl;
	// for(char ch: ans){
	// 	cout << ch << " ";
	// }
	// cout << endl;

	// if the ans had any occurrence of 1 then there is some fault
	if(find(ans.begin(), ans.end(), '1') != ans.end()){
		int power = 1;
		int wrongbit = 0;
		//evaluating the binary expression of ans variable
		for(int i = 0 ; i < ans.size() ; i++){
			if(ans[i]=='1') wrongbit+=power;
			power*=2;
			// cout << "wrong bit = " << wrongbit << ", power = " << power << endl;
		}
		// cout << "\nbit number " << wrongbit << " is wrong and having error " << endl;
		rxBits[wrongbit-1] = 1 - rxBits[wrongbit-1];
	}
	// if the ans dont have any occurrence of 1 then it is correct
	// else{
	// 	cout << "\ncorrect data packet received " << endl;
	// }

	// cout << "\ncorrected bits: ";
	// for(int b : rxBits){
	// 	cout << b << " ";
	// }
	// cout << endl;

	// cout << "\noriginal bit stream with checkbits removed: ";
	vector<int> recovered;
	for(int i = 0; i < rxBits.size(); i++){
		if( (i & (i+1)) != 0 ){
			cout << rxBits[i];
			recovered.push_back(rxBits[i]);
		}
	}
	// cout << endl;
	return recovered;
}

// Driver Code
// int main()
// {
// 	// Given message bits
// 	vector<int> msgBit = {0,0,1,0,1,1,0,0};		//m=16,r=5

// 	// Function Call
// 	findHammingCode(msgBit);

// 	vector<int> rxBits = {1,0,0,1,0,1,0,0,1,0,0,0};
// 	recover(rxBits,8,4);

// 	return 0;
// }
