# -Chamber-s-20th-Century-Dictionary

This demo reads definitions from the Chambers 20th Century Dictionary from Project Gutenberg, and creates a dictionary in memory using 
a map. An interactive command processor enables words and definitions to be looked up in the dictionary.

The strategy is for readChambers_20th_CenturyDictionary() to create a temporary file using the C tmpfile() function, which returns 
a FILE*. The file is created in the "/tmp" directory with mode "w+", and will automatically be deleted when the FILE* is closed. 
The readChambers_20th_CenturyDictionary() function will create Dictionary entries by creating pair<long,size_t> instances with the 
positions and lengths of the definition strings, and write the definition to the definition FILE*. When the read routine is finished,
it will return a pair<Dictionary*, FILE*> with the Dictionary* and the definition FILE* that testChambers_20th_CenturyDictionary() will 
use to retrieve the definition from the file using the position/offset pair.
