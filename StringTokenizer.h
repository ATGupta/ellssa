#include <string>

char* s2ca(std::string s){
	char* a = (char*)malloc((s.length()+1) * sizeof(char));
	a[s.length()] = 0;
	for(int i=0;i<s.length();i++){
		a[i] = s[i];
	}
	return a;
}

int s2i(std::string s){
	int i = 0;
	for(int j = 0; j < s.length(); j++){
		int r = s[j] - 48;
		i = i*10 + r;
	}
	return i;
}

std::string i2s(int n){
	int i = 0;
	while(n > 0) {
		int r = n % 10;
		i = i * 10 + r;
		n = n / 10;
	}
	
	std::string s = "";
	while(i > 0) {
		int r = i % 10;
		s.push_back((char)(r+48));
		i = i / 10;
	}
	return s;
}

class StringTokenizer{
	char* papa;
	std::string breakCharacters = "\n\t ";
	int pos = 0;
	int length = 0;
	
	public:
	void tokenize(std::string s){
		papa = s2ca(s);
		length = s.length();
		pos = 0;
	}
	void setBreakCharacters(std::string bC){
		breakCharacters = bC;
	}
	std::string nextToken(){
		std::string nT = "";
		bool breakFound = false;
		for(; pos < length; pos++){
			for(int i=0; i < breakCharacters.length(); i++){
				if(papa[pos] == breakCharacters[i]){
					breakFound = true;
					pos++;
					break;
				}
			}
			if(breakFound){
				break;
			}
			nT.push_back(papa[pos]);
		}
		return nT;
	}
};
