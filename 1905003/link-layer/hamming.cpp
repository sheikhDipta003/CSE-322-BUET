#include <vector>
#include <algorithm>
using namespace std;

// Function to generate hamming code
vector<int> generateHammingCode(vector<int> msgBits, int m, int r)
{
	vector<int> hammingCode(r + m);

	// Find positions of redundant bits
	for (int i = 0; i < r; i++) {
		hammingCode[pow(2, i) - 1] = -1;
	}

	int j = 0;
	for (int i = 0; i < (r + m); i++) {
		if (hammingCode[i] != -1) {
			hammingCode[i] = msgBits[j];
			j++;
		}
	}

	for (int i = 0; i < (r + m); i++) {
		// if its not redundant bit, continue
		if (hammingCode[i] != -1)
			continue;

		int k = log2(i + 1);
		int count = 0;		//counts the number of 1s necessary for calculating i-th redundant bit

		// Find msg bits containing
		// set bit at k-th position
		for (int j = i + 2; j <= (r + m); j++) {
			if (j & (1 << k)) {
				if (hammingCode[j - 1] == 1) {
					count++;
				}
			}
		}

		// set i-th rbit's parity accordingly
		if (count % 2 == 0) {
			hammingCode[i] = 0;
		}
		else {
			hammingCode[i] = 1;
		}
	}

	return hammingCode;
}

// Function to find the hamming code of the given message msgBit
vector<int> findHammingCode(vector<int>& msgBit)
{
	int m = msgBit.size();
	int r = 1;
	while (pow(2, r) < (m + r + 1)) {
		r++;
	}

	return generateHammingCode(msgBit, m, r);
}

// Function to recover the transmitted message on the receiver end
//When a codeword arrives, the receiver redoes the check bit computations including the values of the received check bits. We call these the check results. If the check bits are correct then, for even parity sums, each check result should be zero. In this case the codeword is accepted as valid. If the check results are not all zero, however, an error has been detected. The set of check results forms the error syndrome that is used to pinpoint and correct the error.
vector<int> recover(vector<int>& rxBits, int m, int r){
	vector<char> checkResults;		//store check results

	int bit = 0;
	for(int i = 1 ; i  <= m+r ; i*=2){
		int count = 0;
		for(int j = i+1 ; j<=m+r ; j++){
			if(j & (1 << bit)){
				if(rxBits[j-1] == 1) count++;
			}
		}
		// now consider the parity of redundant bit
		if(count & 1){										//count of 1s in the message bits are odd
			if(rxBits[i-1] == 1) checkResults.push_back('0');		//the redundant bit at i-th position is 1, so parity even; set check result as 0
			else checkResults.push_back('1');
		}
		else{
			if(rxBits[i-1]==0) checkResults.push_back('0');
			else checkResults.push_back('1');
		}
		bit++;
	}

	// if the checkResults had any occurrence of 1 then there is some error
	if(find(checkResults.begin(), checkResults.end(), '1') != checkResults.end()){
		int power = 1;
		int wrongbit = 0;
		//evaluating the binary expression of checkResults variable
		for(int i = 0 ; i < checkResults.size() ; i++){
			if(checkResults[i]=='1') wrongbit+=power;
			power*=2;
		}
		rxBits[wrongbit-1] = 1 - rxBits[wrongbit-1];
	}

	vector<int> recovered;
	for(int i = 0; i < rxBits.size(); i++){
		if( (i & (i+1)) != 0 ){
			cout << rxBits[i];
			recovered.push_back(rxBits[i]);
		}
	}
	return recovered;
}
