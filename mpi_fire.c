/*
Subject: TSN3151
Lecturer Name: Dr. Ian Chai
Tutorial Section: TT2L
Group Name: Fire

Group Members:
- Tan Yong Qi 1191101570
- Ng Bei Sheng 1191101917
- Tang Ning Kang 1191101645
- Ng Wei Jian 1171201847
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <math.h>

#define MAX_LINE_LENGTH 256
#define MAX_TEXT_LENGTH 256
#define wordQuantity 30000

////////////////////////////////////////////////////////////////////
//>Struct Written By Tan Yong Qi
//Define struct Word for storing the text and its frequency
typedef struct {
    char text[MAX_LINE_LENGTH]; //String for storing the exact text
    int frequency; //For storing frequency
} Word; 
////////////////////////////////////////////////////////////////////
void sortByFrequency(Word* wordArray, int arrayLen);

void sortByAlphabet(Word* wordArray, int arrayLen);

void countText(char* text_line, Word* wordArray, int *currentWordArraySize, int minLen, int maxLen);
////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
	
	//Start>> Written By Tan Yong Qi
	
	int i, final_file_size, fileQuantity, minLen, maxLen;

	FILE *infile;
	FILE* outputFile ;
	
	char* file_content; //For storing raw file_content from each file 
	char* final_file_content; //For storing merged file_contents from all files
	
	char* local_file_content; //For recieving scattered file_content
	
	char text_line[MAX_LINE_LENGTH], fileName[MAX_LINE_LENGTH], sortingChoice[1];
	
	int num_procs, my_rank;
	
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	if (!my_rank){
		
		outputFile = fopen("output_mpi.txt", "w"); //Set writing file
		
		//User input number of files
		printf("Enter the number of text files: ");
		fflush(stdout);
		scanf("%d", &fileQuantity);
		fprintf(outputFile, "Enter the number of text files : %d\n", fileQuantity);

		//Allocate memory for a string that will be used to store
		//entire file content from all files(mergred file content)
		final_file_content = malloc(20000000 * sizeof(char));
		
		long int file_size; //Count size of file
		
		//Loop with total number of files,
		//Read the file, then insert its content into final_file_content
		for (i = 1; i <= fileQuantity; i ++){
			//User input file to read
			printf("Enter the path of text file %d (Include '.txt' ) : ", i);
			fflush(stdout);
			scanf("%s",fileName);
			fprintf(outputFile, "Enter the path of text file %d (Include '.txt' ) : %d\n", i, fileQuantity);
			
			infile = fopen(fileName,"r");
			
			//Move the file pointer from 0 (beginning of the file)
			//to the end of the file
			fseek(infile, 0, SEEK_END);
			
			//Get the current position of the file pointer(obtained from fseek), 
			//which represents the file size
			file_size = ftell(infile);
			
			//Reset the file pointer to the beginning of the file
			//So process of read/write will always start from beginning of the file
			rewind(infile);

			//Allocate memory for a string that will be used to stored 
			//whole file content from current read file
			file_content = malloc(file_size * sizeof(char));
			
			//Read the file content, insert into file_content
			fread(file_content, sizeof(char), file_size, infile);

			
			file_content[file_size] = '\0';  //Ending/terminating the text with a NULL at the end
			
			//Concatenate the file content with the existing final content
			//Basically what doing here is to merge all the file content together
			strcat(final_file_content, file_content); 

			fclose(infile);
		}
		
		free(file_content);

		final_file_size = strlen(final_file_content); //Get string size of final_file_content
		
		//User input minimum length of words
		printf("Enter the minimum length of words to consider : ");
		fflush(stdout);
		scanf("%d", &minLen);
		fprintf(outputFile, "Enter the minimum length of words to consider :  %d\n", minLen);
		
		//User input maximum length of words
		printf("Enter the maximum length of words to consider : ");
		fflush(stdout);
		scanf("%d", &maxLen);
		fprintf(outputFile, "Enter the maximum length of words to consider : %d\n", maxLen);
		
		//User input the choice for words order
		printf("Enter 'a' for alphabetical order or 'n' for number of words order: ");
		fflush(stdout);
		scanf("%s", sortingChoice);
		fprintf(outputFile, "Enter 'a' for alphabetical order or 'n' for number of words order: %s\n", sortingChoice);	

		fclose(outputFile);
	}
	
	//<<End Written By Tan Yong Qi
	
	//Start>> Written By Tang Ning Kang
	
	//Broadcast of variables
	MPI_Bcast(&final_file_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&minLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	//Allocate memory for an array that stores 
	//Word object in each processors
	Word* wordArray = (Word*) malloc(wordQuantity * sizeof(Word));
	
	Word* globalWordArray = NULL; //For gathered wordArray
	Word* finalizedWordArray = NULL; //For cleaned version of gathered wordArray
	
	if(my_rank == 0){ //Processor 0
		//Allocate memory for globalWordArray
		//It is used for gathering wordArray from all processors
		globalWordArray = (Word*) malloc(wordQuantity * num_procs * sizeof(Word));
	
		//Allocate memory for finalizedWordArray
		//It is used for storing cleaned & sorted verison of globalWordArray
		finalizedWordArray = (Word*) malloc(wordQuantity * num_procs  * sizeof(Word));
	}

	//Amount of content that should be recieved by each processors
	int content_per_procs = final_file_size / num_procs;
	
	//Allocate memory for a string that will be used to store
	//scattered file content from final_file_content
	local_file_content = malloc(10000000 * sizeof(char));
	
	//Scatter the final_file_content to all processors
	//Each processors recieve the scattered part as local_file_content, with the amount of content_per_procs
	MPI_Scatter(final_file_content, content_per_procs, MPI_CHAR, local_file_content, content_per_procs, MPI_CHAR, 0, MPI_COMM_WORLD);

	//<<End Written By Tang Ning Kang
	
	//Start>> Written By Ng Wei Jian
	
	int currentWordArraySize = 0; //Initialize counter for wordArray size with 0
	
	//Count the numbers of unique word inside the distributed file content in each processors
	countText(local_file_content, wordArray, &currentWordArraySize, minLen, maxLen);
	
    //Gather the wordArray from all processes to the globalWordArray on process 0
    MPI_Gather(wordArray, wordQuantity * sizeof(Word), MPI_BYTE, globalWordArray, wordQuantity * sizeof(Word), MPI_BYTE, 0, MPI_COMM_WORLD);
	
	//<<End Written By Ng Wei Jian
	
	//Start>> Written By Ng Bei Sheng
	
	if(!my_rank){ //Processor 0
		outputFile = fopen("output_mpi.txt", "a");
		
		int finalizedArrayLen = 0;  //Initialize counter for finalizedArrayLen size with 0 
		
		//Count array length for globalWordArray, to avoid out of bound
		//Since the size of globalWordArray is also wordQuantity * num_procs
		int maxLen = wordQuantity * num_procs; 
		
		for (int i = 0; i < maxLen; i++) {
			
			Word currentWord;
			int exist = 0; //If 0, not exist in wordArray; If 1, exists in wordArray
			
			//If currentWord is not invalid Word record
			if (globalWordArray[i].frequency != 0 || globalWordArray[i].text != NULL){
				
				strcpy(currentWord.text, globalWordArray[i].text); //Copy Text of current index Word to currentWord
				currentWord.frequency = globalWordArray[i].frequency; //Copy Frequency of current index Word to currentWord
				
				//If finalizedWordArray is empty in the first place
				//Directly inserts currentWord, this only happens for the first Word
				if (finalizedArrayLen == 0){
					finalizedWordArray[finalizedArrayLen++] = currentWord;
					
				} else { //If finalizedWordArray isn't empty
					
					//Check through finalizedWordArray
					//If the currentWord exist in finalizedWordArray, exist = 1 (as record of being found)
					//Then, add its frequency to the corresponding index Word (where the Word is same based on its Text)
					for (int j = 0; j < maxLen; j++){ 
						if (strcmp(currentWord.text, finalizedWordArray[j].text) == 0){
							exist = 1; //Found currentWord in finalizedWordArray, exist = 1
							
							//Add frequency to the same Word
							finalizedWordArray[j].frequency += currentWord.frequency;
						}
					}
					
					//If its not outside the size of finalizedWordArray, 
					//and currentWord was not found in finalizedWordArray
					//Add the currentWord into finalizedWordArray, at the latest index
					if (finalizedArrayLen < maxLen && exist == 0) {
						finalizedWordArray[finalizedArrayLen] = currentWord; //Insert currentWord into finalizedWordArray
						finalizedArrayLen++; //Update index (size of finalizedWordArray)
						exist = 1; //After added into finalizedWordArray, exist = 1 because it is exist in finalizedWordArray now
					}
				}
			}
		}
		
		if (sortingChoice[0] == 'a'){ //Sort the finalizedWordArray Alphabetically
			sortByAlphabet(finalizedWordArray, finalizedArrayLen);
			printf("\nWord Count Report (Aphabetical Order):\n");
			fprintf(outputFile, "Word Count Report (Aphabetical Order):\n");
			
		} else if (sortingChoice[0] == 'n'){ //Sort the finalizedWordArray based on its Frequency (Descendingly)
			sortByFrequency(finalizedWordArray, finalizedArrayLen);
			printf("\nWord Count Report (Number of Words Order):\n");
			fprintf(outputFile, "Word Count Report (Number of Words Order [Descendingly] ):\n");
		}
		
		for (int i = 0; i < finalizedArrayLen; i++) { //Output all recorded Word one by one
			printf("Word %d\t:\t %s \t Frequency: %d\n", i+1, finalizedWordArray[i].text, finalizedWordArray[i].frequency);
				
			// Write the Word and its frequency to the output file
            fprintf(outputFile, "%s: %d\n", finalizedWordArray[i].text, finalizedWordArray[i].frequency);
		}	
		
		fclose(outputFile);
	}

	//<<End Written By Ng Bei Sheng

	//Start>> Written By Tan Yong Qi
	free(wordArray);
	free(local_file_content);
	
	if(!my_rank){
		free(final_file_content);
		free(globalWordArray);
		free(finalizedWordArray);
	}
	
	MPI_Finalize();
	//<<End Written By Tan Yong Qi
	return 0;
}
////////////////////////////////////////////////////////////////////
//>>Function Written By Ng Wei Jian
//Count the numbers of unique word inside the distributed file content
void countText(char* text_line, Word* wordArray, int *currentWordArraySize, int minLen, int maxLen) {

    //Spliting the file content into a string array by several symbols/blank spacing
	char* testing_word = strtok(text_line, ".,'’ \t\n-");
	
	while (testing_word != NULL) { //If there is not the end of line/there are still words remaining
		
		char cleaned_testing_word[MAX_TEXT_LENGTH] = ""; //Predefined a new string
		int wordLen = strlen(testing_word); //Identify length for current obtained text
		
		//Remove non-alphabetic symbols/chars from the word, then lowercase the alphabets
		//Then add into cleaned word string
		int cleanedLen = 0;
		for (int i = 0; i < wordLen; i++) {
			if (isalpha(testing_word[i])) { //Check if it is alphabet
				cleaned_testing_word[cleanedLen++] = tolower(testing_word[i]); //Turn it into lowercase, then add into cleaned string
			}
		}
		
		cleaned_testing_word[cleanedLen] = '\0'; //Ending/terminating the text with a NULL at the end
		
		//Getting size of wordArray
		//that stores each unique Word and its frequency
		int currentSize = *currentWordArraySize; 
		
		//Make sure the current word is within the min length and max length
		if ((strlen(cleaned_testing_word) >= minLen) && (strlen(cleaned_testing_word) <= maxLen)){
			// Check if the text exists within the wordArray
			int wordIndex = -1;
			
			//If found current word inside wordArray, return the its index
			for (int i = 0; i < wordQuantity; i++) {
				if (strcmp(wordArray[i].text, cleaned_testing_word) == 0) { 
					wordIndex = i; //Get its index in the wordArray
					break;
				}
			}
			if (wordIndex != -1) { //Word exist inside wordArray
				//Based on the index, add up its frequency
				wordArray[wordIndex].frequency++;
			
			} else {
				//If the current word doesn't exist in the wordArray, 
				//create a new Word object and add it to wordArray
				if (currentSize < wordQuantity) {
					Word newWord; //Define new Word
					strcpy(newWord.text, cleaned_testing_word); //Copy Text of current Word
					newWord.frequency = 1; //Set its initial Frequency to 1
					
					wordArray[currentSize] = newWord; //Insert the Word into the wordArray
					currentSize++; //Increment of wordArray size count, since new Word being added
				}
			}
		}
        //Once everything done with current word
		//Get the next Word from the splited word array
        testing_word = strtok(NULL, ".,'’ \t\n-");
		
		*currentWordArraySize = currentSize; //Return wordArray size
    }
	
}
////////////////////////////////////////////////////////////////////
//>>Function Written By Ng Bei Sheng
//Sort Word based on its Frequency  (Descendingly) (Bubble Sort)
void sortByFrequency(Word* wordArray, int arrayLen){
	int i,j;
	Word tempWord;
	
	for (i=0; i < arrayLen-1; i++){
		for (j = i+1; j < arrayLen; j++){
			//If current index Word's Frequency is bigger than the next index Word's Frequency
			if(wordArray[i].frequency < wordArray[j].frequency){
				
				strcpy(tempWord.text, wordArray[i].text); //Copy Text of current index Word to tempWord
				tempWord.frequency = wordArray[i].frequency; //Copy Frequency of current index Word to tempWord
				
				//Swap Word of current index and next index
				wordArray[i] = wordArray[j];
				wordArray[j] = tempWord;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////
//>>Function Written By Ng Bei Sheng
//Sort Word Alphabetically (Bubble Sort)
void sortByAlphabet(Word* wordArray, int arrayLen){
	int i,j;
	Word tempWord;
	
	for (i=0; i < arrayLen-1; i++){
		for (j = i+1; j < arrayLen; j++){
			//If current index Word's alphabets is bigger than the next index Word's alphabets
			if(strcmp(wordArray[j].text, wordArray[i].text) < 0){
				
				strcpy(tempWord.text, wordArray[i].text); //Copy Text of current index Word to tempWord
				tempWord.frequency = wordArray[i].frequency; //Copy Frequency of current index Word to tempWord
				
				//Swap Word of current index and next index
				wordArray[i] = wordArray[j];
				wordArray[j] = tempWord;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////
