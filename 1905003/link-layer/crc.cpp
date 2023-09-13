#include <bits/stdc++.h>
using namespace std;

// returns bitwise xor of the strings a and b, of the same length
string xor1(string a, string b)
{
	string ans = "";

	int n = b.length();

	for (int i = 1; i < n; i++) {
		if (a[i] == b[i])
			ans += "0";
		else
			ans += "1";
	}
	return ans;
}

// Performs Modulo-2 division
string mod2div(string dividend, string divisor)
{
	// Number of bits to be XORed at a time.
	int nXOR = divisor.length();

	// Slicing the dividend to appropriate
	// length for particular step
	string D = dividend.substr(0, nXOR);

	int n = dividend.length();

	while (nXOR < n) {
		if (D[0] == '1')	D = xor1(divisor, D) + dividend[nXOR];
		else	D = xor1(std::string(nXOR, '0'), D) + dividend[nXOR];
		nXOR += 1;
	}

	if (D[0] == '1')	D = xor1(divisor, D);
	else	D = xor1(std::string(nXOR, '0'), D);

	return D;
}

// Function used at the sender side to encode message with CRC checksum, returns both the original message and CRC checksum
pair<string,string> encodeData(string message, string generator)
{
	int len = generator.length();
	string appended_data = (message + std::string(len - 1, '0'));
	string remainder = mod2div(appended_data, generator);
	return pair<string,string>(message,remainder);
}

// checking if the message received by receiver is correct or not. If the remainder is 0 then it is correct, else wrong.
string receiver(string message, string generator)
{
	string currxor = mod2div(message.substr(0, generator.size()), generator);
	int curr = generator.size();
	while (curr != message.size()) {
		if (currxor.size() != generator.size()) {
			currxor.push_back(message[curr++]);
		}
		else {
			currxor = mod2div(currxor, generator);
		}
	}
	if (currxor.size() == generator.size()) {
		currxor = mod2div(currxor, generator);
	}
	if (currxor.find('1') != string::npos) {
		return "error detected";
	}
	
	return "no error detected";
}

