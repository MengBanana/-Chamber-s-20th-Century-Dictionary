/*
 * chambers_20th_century_dictionary.cpp
 *
 * This program relies on the availability of the 'curl' utility
 * for reading input from a URL. Curl is available with MacOS and
 * most Linux distros. Curl is also available under CygWin. See
 * http://www.oracle.com/webfolder/technetwork/tutorials/obe/cloud/
 * 	objectstorage/installing_cURL/installing_cURL_on_Cygwin_on_Windows.html
 * for instructions. There are also a number of curl ports
 * available on Windows. See https://curl.haxx.se/download.html.
 *
 * @author Xinmeng Zhang
 */

/**
 * The program instead of storing word as key and definition as value
 * in the map, it will store word as key, and a pair of definition statrin pointer
 * and definition length as value.
 * 1. need a function to store definition in the tempfile and return a
 * pair of definition starting position and length
 * 2. need a function to return a string based on the giving tempfile
 * pointer and definition information of starting point and length
 * 3. change dictionary structure to <string, definition information>
 * 4. change whenever "?"
 * 5. return a pair of dictionary and file when parsing because we need
 * to get the pointer to the tempfile where we store definition
 */


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <iterator>

using namespace std;


/**
 * Define DefInfo represents information of definition that holds an
 * starting position of the definition string in the file and
 * the length of the definition
 */
typedef pair<long, size_t> DefInfo;


/**
 * Define Dictionary as a map of word to a pair of defnition information of start
 * pointer and length
 */
typedef map<string, DefInfo> Dictionary;
//have to change dictionary structure, otherwise can insert



/**
 * Normalize a "\r\n" newline sequence to "\n".
 *
 * @param str the input string to be normaized
 * @return the new length of the input string
 */
static int normalizeEoln(char* str) { //end of line
	char* eoln = strstr(str, "\r\n");
	if (eoln != (char*)NULL) {
		strcpy(eoln++, "\n");
	}
	return eoln-str;
}

/**
 * Read the definition into the defStr buffer up to defSize characters.
 * Subsequent characters are ignored.
 *
 * @param file the input File
 * @param defStr the input string for the definition with initial text
 * @return true of the definition was read, false if there was an error
 */
static bool readDefinition(FILE* file, string& defStr) {
	bool status;
	char line[256];

    while ((status = (fgets(line, sizeof(line), file) != NULL))) {
    	normalizeEoln(line);

    	// definition ends with empty line
    	if (line[0] == '\n') {
        	break;
        }
    	defStr.append(line);
    }
    return status;
}

/**
 * Store the definition to a tempfile
 * @param file the pointer of the tempfile built when parsing the dictionary
 * @param defStr the input string for the definition with initial text
 * @return DefInfo which is a pair of start pointer of definition in the file
 * and the length of definition
 */
static DefInfo inputDefinition(FILE *file, string& defStr) {
	long start = ftell(file); // find the position of the pointer
	//convert string to charArray
	//so we can block copy array to file
	//https://www.geeksforgeeks.org/convert-string-char-array-cpp/
	size_t length = defStr.length();
	char temp[length];
	strcpy(temp, defStr.c_str());
	//write the definition to the file
	//size_t nwrite =
	fwrite(temp, sizeof(char), length, file);
	return make_pair(start, length);
}

/**
 * Using the pair of start pointer of definition and length to read the
 * characters from the tempfile and return a string
 * @param file the pointer of the tempfile
 * @param info the pair of start pointer of the definition and length
 * @return a string of the definition
 */
static string outputDefinition(FILE *file, DefInfo& info) {
	long start = info.first;
	fseek(file, start, SEEK_SET); // move ptr to the position before start
	size_t length = info.second;
	char temp[length+1];
	//need to make it null terminated
	//https://stackoverflow.com/questions/2911089/null-terminating-a-string
	temp[length] = '\0';
	//store definition to temp chararray;
	//size_t nread =
	fread(temp, sizeof(char), length, file);
	return string(temp);
}

/**
 * Read definitions from input stream, starting with the firstWord
 * and ending with the lastWord in the dictionary and build a HashMap
 * with the words and their definitions.
 *
 * Chambers 20th Century Dictionary entries are of the form:
 *
 * SAKE, sak'e, _n._ a Japanese fermented liquor made from rice: a generic
 * name for all spirituous liquors.\r\n
 *
 * The first line has the defined word followed by a comma. Definitions can
 * have multiple lines. Entries are separated by an empty line. Input Lines
 * are terminated by the newline sequence "\r\n" which is replace by '\n'
 * in the tree.
 *
 * @param file the input FILE
 * @param firstWord the first word whose definition is added to the map
 * @param lastWord the last word whose definition is added to the map
 * @return a map with the words and definitions as entries also a pointer
 * to temfile which stores the definition
 */
pair<Dictionary*, FILE*> readChambers_20th_CenturyDictionary(
	FILE* file, string const& firstWord, string const& lastWord) {
	//need to return serveral things, use pair
	// result map
	Dictionary* dict = new Dictionary();

	// line buffer
    char line[256];

    // find line with first word to process
	int firstLen = firstWord.length();
	bool found = false;
    while (!found && fgets(line, sizeof(line), file)) {
    	// found if line begins with first word followed by a ', '
        if (firstWord.compare(0, firstLen, line, 0, firstLen) == 0) {
			// definition valid if it begins with word followed by ", "
        	found = (strncmp(line+firstLen, ", ", 2) == 0);
        }
    }
    FILE* tfile = tmpfile(); // the file to save definition
    if (found) {
		do {
			// get definition of current word
			normalizeEoln(line);
			string defStr = line;
			if (!readDefinition(file, defStr)) {
				break;
			}

			// definition valid if it begins with word followed by ", "
			size_t wordLen = defStr.find(", ");
			if (wordLen != string::npos) {
				// extract the word being defined
				string word = defStr.substr(0, wordLen);
				string def = defStr.substr(wordLen+2);  //skip over ", "

				//FILE* tfile = tmpfile(); // the file to save definition
				//need to initialized before if statement
				DefInfo info = inputDefinition(tfile, def); // return a pair of ptr and size
				dict->insert(make_pair(word, info)); // add to the dictionary

				// done if just processed last word
				found = (word.compare(lastWord) == 0);
			}

		} while (!found && fgets(line, sizeof(line), file));
    }

    return make_pair(dict, tfile);
}

/**
 * Test building a map from entries in Chambers's Twentieth Century Dictionary
 * (part 4 of 4: S-Z and supplements) hosted by Project Gutenberg at
 * http://www.gutenberg.org/cache/epub/38700/pg38700.txt.
 */
void testChambers_20th_CenturyDictionary(void) {
	// read piped output of curl
	// see http://stackoverflow.com/questions/26648857/can-fopen-be-used-to-open-the-url
    cout << endl << "Opening Chambers's Twentieth Century Dictionary (part 4 of 4: S-Z and supplements)" << endl;
	FILE* file = popen("curl -s 'http://www.gutenberg.org/cache/epub/38700/pg38700.txt'", "r");
	if (file == (FILE*)NULL) {
		cout << "...Error opening dictionary" << endl;
		return;
    }

	const string firstWord = "SAB";    // first definition to load
	const string lastWord = "SYZYGY";  // last definition to load
	cout << "...Loading definitions from " << firstWord << " to " << lastWord << endl;
	//get the dictionary and pointer to tempfile
	pair<Dictionary*, FILE*> pair = readChambers_20th_CenturyDictionary(file, firstWord, lastWord);
	Dictionary* dict = pair.first; // the dictionary key word value definfo
	FILE* tfile = pair.second; // pointer to definfo
	pclose(file);

	int nEntries = dict->size();
	cout << "...Loaded " << nEntries << " definitions" << endl;

	string cmd;
	const string cmdChars = "=?#";  // command characters
	do {
		if (cmd.compare("quit") == 0) {
			cout << "Done." << endl;
			break;
		} else if (cmdChars.find(cmd[0]) != string::npos) { // process command
			if (cmd.back() == '*') { // process wildcard command
				// all matching words
				const size_t wordLen = cmd.size()-2;
				const string word = cmd.substr(1, wordLen); // extract word
				size_t count = 0;
				for (auto itr = dict->begin(); itr != dict->end(); itr++) {
					if (itr->first.compare(0, wordLen, word) == 0) {
						if (cmd[0] == '=') { // the word
							cout << endl << itr->first;
						} else if (cmd[0] == '?') { // the definition
							//cout << endl << itr->second;
							//need to retrive the string by calling ouputDefinition function
							DefInfo info = itr->second;
							cout << endl << outputDefinition(tfile, info);
						}
						count++;
					}
				}
				if (cmd[0] == '#') {
					cout << count << endl;
				} else if (count == 0) {
					cout << "No definitions match '" << word << "*'" << endl;
				}
			} else {  // process non-wildcard command
				// match specific word
				const string word = cmd.substr(1);  // extract word
				auto itr = dict->find(word);
				if (itr == dict->end()) { // report no definition
					cout << "No definition for '" << word << "'" << endl;
				} else if (cmd[0] == '#') {   // print count
					cout << ((itr == dict->end()) ? 0 : 1) << endl;
				} else {  // print key or value
					// key word is itr->first, value definfo pair is itr->second
					DefInfo info = itr->second;
					cout << ((cmd[0] == '?') ? outputDefinition(tfile, info) : itr->first) << endl;
				}
			}
		} else {  // unknown command
			// show command list
			cout << "Commands:" << endl;
			cout << "  Quit:  quit" << endl;
			cout << "  List words: =<word> or =<prefix>*" << endl;
		    cout << "  List definitions: ?<word> or ?<prefix>*" << endl;
		    cout << "  Count words: #<word> or #<prefix>*" << endl;
		}
		cout << endl << "> ";  // prompt
	} while (cin >> cmd);

	fclose(tfile); // close temfile reduce reference link so it will be removed
	//https://www.tutorialspoint.com/c_standard_library/c_function_tmpfile.htm
	return;
}

/**
 * Run testChambers_20th_CenturyDictionary function.
 *
 * @return EXIT_SUCCESS
 */
int main(void) {
	testChambers_20th_CenturyDictionary();

	return EXIT_SUCCESS;
}
