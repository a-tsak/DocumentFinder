# Document Finder - C Program

This program was the last university project for the subject "Data Structures" that I did with a classmate from university. The user can load text files which are considered documents, do simple queries such as "query", "stats", and composite queries such as "queryOR", "queryAND", "statsOR", "statsAND", and the program will find which documents contained that one specific word or multiple specific words, or check the statistics for how much a certain word or multiple certain words appear in a document.

##### Preprocessing
The program preprocesses the text by splitting the text into words, turns them into lowercase, and when it's a punctuation mark it removes it unless it's a "'" or "-", in which the consecutive strings will connect through the " " space sign, so as to appear as one word when the user wants to find it.