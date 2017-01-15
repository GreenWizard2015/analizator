#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

void resetInput(){
	cin.clear();
	cin.sync();
}
//
struct TArrayDefinition{
	string name;
	long DIMS[2];
};

const long MAX_ARRAYS = 100;
struct TDefinitions{
	int Count;
	TArrayDefinition Defs[MAX_ARRAYS];

	void add(TArrayDefinition& def){
		Defs[Count] = def;
		Count++;
	}

	bool checkName(const string& name){
		for(int i = 0; i < Count; ++i)
			if(Defs[i].name.compare(name) == 0)
				return false;
		return true;
	}

	TDefinitions() {
		Count = 0;
	}

	void printAll(){
		cout << "Defined arrays:\n";
		for(int i = 0; i < Count; ++i){
			cout << "Name: " << Defs[i].name << "\tDims: " << Defs[i].DIMS[0] <<
				" " << Defs[i].DIMS[1] << endl;
		}
	}

	void clear(){
		Count = 0;
	}
};

TDefinitions Arrays;

int selectAction(char* query, int maxN){
	int res;
	do{
		resetInput();
		cout << query;
		cin >> res;
	} while ((res < 0) || (maxN < res));
	return res;
}

const long MAX_CODE_SIZE = 8 * 1024; // 8kb
struct TCode{
	string Source;
	size_t Pos;

	string Error;
	//
	char nextChr(){
		Pos++;
		return Pos < Source.length();
	}

	bool isChar(char ch){
		return hasSymbols() && (ch == Source[Pos]);
	}

	bool isCharRange(char ch1, char ch2){
		return hasSymbols() && (ch1 <= Source[Pos]) && (Source[Pos] <= ch2);
	}

	char getCharAndGoNext(){
		char res = Source[Pos];
		nextChr();
		return res;
	}

	bool atEnd(){
		return Source.length() <= Pos;
	}
	
	bool hasSymbols(){
		return !atEnd();
	}

	bool hasError(){
		return !Error.empty();
	}

	bool allGood(){
		return !hasError();
	}
};

TCode currentCode;

string inputLine(char* msg){
	resetInput();
	cout << msg;

	string res;
	getline(cin, res);
	return res;
}

void inputCode(){
	currentCode.Source = inputLine("Enter code:\n");
}

bool readCodeFromFile(){
	string filename = inputLine("File name: \n");
	ifstream file(filename);
	if(file.is_open()){
		ostringstream buffer;
		buffer << file.rdbuf();
		currentCode.Source = buffer.str();
		cout << "Loaded code:\n" << currentCode.Source << endl << endl;
		return true;
	} else {
		cout << "Can't read file " << filename << endl;
		return false;
	}
}

string createCode(){
	ostringstream res;
	// header
	res	<< "#include <iostream>\n"
		<< "using namespace std;\n"
		<< "\n"
		<< "int main(){\n";

	for(int i = 0; i < Arrays.Count; ++i){
		TArrayDefinition& arr = Arrays.Defs[i];

		res	<< "\tint " << arr.name << "[" << arr.DIMS[0] << "][" 
			<< arr.DIMS[1] << "];\n";

		res << "\tfor(int i = 0; i < " << arr.DIMS[0] << "; ++i)\n";
		res << "\t\tfor(int j = 0; j < " << arr.DIMS[1] << "; ++j)\n";
		res << "\t\t\t" << arr.name << "[i][j] = rand() % 100;\n";

		res << "\tcout << \"Array arr:\\n\";\n";

		res << "\tfor(int i = 0; i < " << arr.DIMS[0] << "; ++i){\n";
		res << "\t\tfor(int j = 0; j < " << arr.DIMS[1] << "; ++j)\n";
		res << "\t\t\tcout << " << arr.name << "[i][j] << \"\\t\";\n";
		res << "\t\tcout << endl;\n";
		res << "\t}\n";
		res << "\n\n";
	}

	// tail
	res	<< "\treturn 0;\n"
		<< "}\n";

	return res.str();
}

void beginGeneration(){
	string code = createCode();
	cout << code << endl << endl;

	if(selectAction("Save to file? [0 - no, 1 - yes]: ", 2) == 1){
		string filename = inputLine("File name:\n");
		ofstream file(filename);
		file << code;
		file.close();
		cout << "Saved!\n";
	}
}

void showResults(){
	if(!currentCode.Error.empty()){
		cout << "Error:\n" << currentCode.Source << endl;
		cout << setw(currentCode.Pos + 2) << "^\n";
		cout << currentCode.Error << endl;
	} else {
		cout << "All ok\n";
		Arrays.printAll();
		cout << endl;
		if(selectAction("Generate C++ code? [0 - no, 1 - yes]: ", 2) == 1){
			beginGeneration();
		}
	}
}
///////////////////////////////////////////////////////
void Error(const char* msg){
	currentCode.Error = msg;
}

bool isAlpha(){
	return	currentCode.isCharRange('a', 'z') || 
		currentCode.isCharRange('A', 'Z') ||
		currentCode.isChar('_');
}

bool isNumber(){
	return currentCode.isCharRange('0', '9');
}

bool  isSpaces(){
	return	currentCode.isChar(' ') || 
			currentCode.isChar('\r') ||
			currentCode.isChar('\n');
}

void skipSpaces(){
	while(isSpaces())
		currentCode.nextChr();
}

char readAlpha(){
	if (isAlpha()){
		return currentCode.getCharAndGoNext();
	} else {
		Error("Expected letter or '_'");
		return '\0';
	}
}

int readNumber(){
	skipSpaces();
	if(!isNumber()){
		Error("Expected number");
		return 0;
	}
	int res = 0;
	while(isNumber()){
		res = (res * 10) + (currentCode.getCharAndGoNext() - '0');
	}
	return res;
}

bool expectChar(char ch){
	skipSpaces();
	if(currentCode.isChar(ch)) {
		currentCode.getCharAndGoNext();
		return true;
	}

	string s = "Expected symbol '";
	s += ch;
	s += "'";
	Error(s.c_str());
	return false;
}

int readDim(){
	if(!expectChar('[')) return 0;
	int dim = readNumber();
	if(currentCode.hasError()) return 0;

	if ((dim < 1) || (1000 < dim)){
		Error("Dimension must be in range 1..1000");
		return 0;
	}

	if(!expectChar(']')) return 0;
	return dim;
}

string readName(){
	skipSpaces();
	string res;
	res += readAlpha();
	if(currentCode.hasError()) return res;

	while(isAlpha() || isNumber()){
		res += currentCode.getCharAndGoNext();
	}
	return res;
}

void readDelimer(){
	skipSpaces();
	if(currentCode.isChar(';'))	{
		currentCode.nextChr();
		return;
	} 
	if(currentCode.atEnd()) return;

	Error("Expected ';'");
}

TArrayDefinition loadArrayDef(){
	TArrayDefinition res;
	
	res.name = readName();
	if(currentCode.hasError()) return res;
	if(!Arrays.checkName(res.name)){
		Error("Array redefinition");	
		return res;
	}
	
	res.DIMS[0] = readDim();
	if(currentCode.hasError()) return res;

	res.DIMS[1] = readDim();
	if(currentCode.hasError()) return res;
	readDelimer();

	return res;
}

void analyze(){
	currentCode.Pos = 0;
	currentCode.Error = "";
	Arrays.clear();

	while(currentCode.hasSymbols() && currentCode.allGood()){
		TArrayDefinition def = loadArrayDef();
		if(currentCode.allGood()){
			Arrays.add(def);
			skipSpaces();
		}
	}
}

///////////////////////////////////////////////////////

int main(){
	while(true){
		switch(selectAction("Action [0 - input code, 1 - load file, 2 - quit]: ", 3)){
		case 0:
			inputCode();
			analyze();
			showResults();
			break;

		case 1:
			if(readCodeFromFile()){
				analyze();
				showResults();
			}
			break;

		default:
			return 0;
		}
	}

	return 0;
}