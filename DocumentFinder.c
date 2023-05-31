#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TABLE_SIZE 5000001		// Size for the Hash Table buckets

typedef struct node node;

struct node
{
   char *data;      	// ID of each document
   struct node *next;
};

struct node *temp;

typedef struct HashTable HashTable;

struct HashTable{
    int size;				// Size that the Hash Table can hold (will be TABLE_SIZE)
    int count;				// How many buckets are occupied
    int *DF;				// Document Frequency, a dynamic array of integers, its size is the number of words
    char **word;			// Word buckets, a dynamic array of strings, its size is the number of words
    struct node **head;		// Pointer to linked list that contains all the IDs that a word has
};

HashTable* create_table(int size);						// Creates table by allocating memory for each field
static node** create_head(HashTable* table);			// Creates head by allocating memory for the head of every list
void insert(HashTable* table, char* str, char *ID2);	// Inserts to dictionary
unsigned long hash_function(char *str);
void delete_table(HashTable *table);					// Frees allocated memory for table
void free_node(struct node* node);						// Frees all nodes for all lists

char *preprocess(HashTable *table, char *token);		// Preprocessing user input
int traverse_list(HashTable* table, unsigned long i, char *str, char *ID2);		// Traverses list to check if a word is unique in a document

void query(HashTable *table, char *h_word);									// Query task (query [word])
void query_bool(HashTable *table, char *h_word, char *h_word2, int choice);	// Query boolean task (queryAND [word1] [word2], queryOR [word1] [word2])
struct node* query_intersection(HashTable *table, int index1, int index2);		// Finds all IDs needed for the queryAND task
struct node* query_union(HashTable *table, int index1, int index2);			// Finds all IDs needed for the queryOR task

void stats(HashTable *table, char *h_word);									// Stats task (stats [word])
void stats_bool(HashTable *table, char *h_word, char *h_word2, int choice);	// Stats boolean task (statsAND [word1] [word2], statsOR [word1] [word2])
int stats_intersection(HashTable *table, int index1, int index2);		// Finds all IDs needed for the statsAND task
int stats_union(HashTable *table, int index1, int index2);				// Finds all IDs needed for the statsOR task

int exists(HashTable *table, char *data, int index);

int main()
{
    
    int choice,key,i,j;		// Choice = User input (query, stats, etc)
    
    HashTable* ht;
    ht = create_table(TABLE_SIZE);		// Creates table
    
	char *str = malloc (255*sizeof(char));
    char *task, *file_name, *ID;
    
    FILE *fp;
    char line[600], *token;
    
	//User's input
    
    int pl;
    char input[300], *token2;
    
	while (fgets(input,sizeof(input),stdin)){
		
        for (i=0; i<strlen(input); i++){
	    	if (input[i]=='\r'){
	    		input[i] = '\0';
			}
		}
		
        if (input[strlen(input)-1]=='\n'){		// Removing newline character
            input[strlen(input)-1]='\0';
        }
        
        for (i=0; i<strlen(input); i++){		// Make each letter lowercase
            input[i]=tolower(input[i]);
        }
        
        token = strtok(input, " ");				// Get task (for example "query")
        
		task = malloc(sizeof(char)*100+1);    		// Allocating enough memory for task string
        
		if (token!=NULL){
			memcpy(task, token, strlen(token)+1);
		}
		
        if (strcmp(token,"query")==0){			// User chose query
            
			pl=0;
            
			while(token!=NULL && pl<1){         // Check the first word, if there any other words, ignore them (pl<1)
                
				pl++;
                token = strtok(NULL, " ");		// Get the only word (for example "the")
                token = preprocess(ht, token);	// Preprocess this word
                
                query(ht, token);				// Find which documents have this word
                
            }
            
        }
        
        else if (strcmp(token,"queryand")==0){	// User chose queryAND
            
			token = strtok(NULL, " ");      	// Get first word
            token = preprocess(ht, token);
            
            token2 = strtok(NULL, " ");			// Get second word
            token2 = preprocess(ht, token2);
            
            query_bool(ht, token, token2, 2);	// Find which documents have both words
            
        }
        
        else if (strcmp(token,"queryor")==0){	// User chose queryOR
            
			token = strtok(NULL, " ");;      	// Get first word
            token = preprocess(ht, token);
            
            token2 = strtok(NULL, " ");			// Get second word
            token2 = preprocess(ht, token2);
            
            query_bool(ht, token, token2, 1);	// Find which documents have either of these words
            
        }
        
        else if (strcmp(token,"stats")==0){			// User chose stats
            token = strtok(NULL, " ");				// Get word
            token = preprocess(ht,token);
			stats(ht, token);
        }
        
        else if (strcmp(token,"statsand")==0){			// User chose statsAND
        	
            token = strtok(NULL, " ");			// Get first word
            token = preprocess(ht, token);
            token2 = strtok(NULL, " ");			// Get second word
            token2 = preprocess(ht, token2);
            stats_bool(ht, token, token2, 2);	// Find how many documents have both words
		
		}
        
        else if (strcmp(token,"statsor")==0){			// User chose statsOR
            
			token = strtok(NULL, " ");			// Get first word
            token = preprocess(ht, token);
            token2 = strtok(NULL, " ");			// Get second word
            token2 = preprocess(ht, token2);
            stats_bool(ht, token, token2, 1);	// Find how many documents have either of these words
        }
        
        else if (strcmp(token,"load")==0){			// User chose load
            
            token = strtok(NULL, " ");			// filename.txt
            file_name = (char*) malloc(sizeof(char)*100+1);	// 100 = size for file name and its extension
            strcpy(file_name,token);
            
            fp = fopen(file_name,"r");			// Open the file
    
		    if (fp==NULL){						// In case there isn't enough memory or the file cannot open
		        printf("There isn't enough memory. The program is going to exit.\n");
		        exit(1);
		    }
		    
		    int count=0;
		    
		    while (fgets(line, sizeof(line), fp)){		// Reading file line by line
		        
		        /* Preprocess */
		        
		        for (i=0; i<strlen(line); i++){
		        	if (line[i]=='\r'){
		        		line[i] = '\0';
					}
				}
		        if (line[strlen(line)-1]=='\n'){			// Removing newline character
		            line[strlen(line)-1]='\0';
		        }
		        for (i=0; i<strlen(line); i++){				// Making all letters lowercase
		            line[i]=tolower(line[i]);
		        }
		        
		        token = strtok(line, " ");					// Get ID
		        
				for (i=0; i<strlen(token); i++){
		        	if (token[i]==':'){
		        		token[i] = token[i+1];				// Remove ":" character
					}
				}
				
		        ID = malloc(sizeof(char)*20);
		        if (token!=NULL){
					memcpy(ID, token, strlen(token)+1);
				}
		        
				while(token!=NULL){						// Get every word from the line (document)
					token = strtok(NULL, " ");
					if (token!=NULL){
						preprocess(ht, token);			// Preprocess each word
						if (token!=NULL && strlen(token)>0){
						
							insert(ht, token, ID);		// Insert word to dictionary
						}
					}
				}
		        /* Preprocessing finished */
		        
		        count++;
		        
		    }
		    printf("1 document loaded and indexed!\n");
		    
		    free(file_name);		// Frees memory allocated for file's name
			fclose(fp);				// Closes file
            
            
        }
        
        else if (strcmp(token,"exit")==0){
        	
        	delete_table(ht);		// Frees all table's fields
        	free(str);
        	free(ID);				// Frees memory allocated for ID
        	
            exit(1);
        }
        
        printf("\n");
    }

    
}

char *preprocess(HashTable *table, char *token){
    
	// Preprocess for user input words
	
	int i,j;
    
	for (i = 0, j; token[i] != '\0'; ++i) {			// For every letter of the word
        
		while ((ispunct(token[i])) && !(token[i] == '\0') && !(token[i]=='-') && !(token[i]=='\'')) {   // If it's a punctuation mark, skip it, unless it's "'" or "-"
            
			for (j=i; token[j]!='\0'; ++j) {
                token[j] = token[j + 1];			// Skip character
            }
            
            token[j] = '\0';
        }
        
        if (isdigit(token[i])){			// If a number exists delete the whole string (for example "2021", "21st")
            token[0] = '\0';
            break;
        }
        
        if ( (token[i]=='\'') || (token[i]=='-') ){		// Put " " in every "'" or "-" character
            token[i] = ' ';
        }
    }
    
    return token;
}

void query(HashTable *table, char *h_word){
    
	long unsigned i = hash_function(h_word);		// Calculate hash key
    
    int h=1;
    int index = i;
    
    if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){
            
			// Collision
			
			i = (i + (h * h)) % TABLE_SIZE;	// Quadratic probing, handling collision
			h++;
            
			if (i==(TABLE_SIZE-1)){			// If index is the last bucket of the array, go to the first bucket
                i=0;
            }
        }
    }
    
    if(table->head[i] != NULL)
    {
        printf("\nDocument IDs that answer the query: ");
        for(temp=table->head[i];temp!=NULL;temp=temp->next){	// Traverse list
			printf("%s->",temp->data);			// Print all document IDs
		}
    }
    else{
        printf("\nNo documents found!");
    }
        
}

void query_bool(HashTable *table, char *h_word, char *h_word2, int choice){
    
	long unsigned i, index1 = hash_function(h_word);
	int h;
	
	i = index1;
	h=1;
	
    if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){

			i = (i + (h * h)) % TABLE_SIZE;	// Quadratic probing, handling collision
			h++;
            if (i==(TABLE_SIZE-1)){			// If index is the last bucket of the array, go to the first bucket
                i=0;
            }
        }
    }
	
	long unsigned index2 = hash_function(h_word2);
    
    i = index2;
	h=1;
	
	if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){

			i = (i + (h * h)) % TABLE_SIZE;	// Quadratic probing, handling collision
			h++;
            if (i==(TABLE_SIZE-1)){			// If index is the last bucket of the array, go to the first bucket
                i=0;
            }
        }
    }
	
    if (choice==1){ 		//queryOR
        
		struct node* unionn;
		unionn = query_union(table,index1,index2);
        if (unionn== NULL){
        	printf("No documents found!\n");
        }
		else
		{
			printf("Documents are: ");
        	while (unionn != NULL) {
				printf("%s->", unionn->data);			// Print all document IDs that share both words
				unionn = unionn->next;
			}
		}

    }
	if (choice==2){ 		//queryAND
		
		struct node* intersection;
		intersection = query_intersection(table,index1,index2);
        if (intersection== NULL){
        	printf("No documents found!\n");
        }
		else
		{
			printf("Documents are: ");
        	while (intersection != NULL) {
				printf("%s->", intersection->data);			// Print all document IDs that contain either of these words
				intersection = intersection->next;
			}
		}
	}
    
}



struct node* query_union(HashTable *table, int index1, int index2)
{
	struct node* list;
	
	// Insert all elements of list1 to the result list
	
	while (table->head[index1] != NULL) {
		
		struct node* new_node = (struct node*)malloc(sizeof(struct node));
		
		new_node->data = table->head[index1]->data;
		new_node->next = list;
		list = new_node;
		table->head[index1] = table->head[index1]->next;
		
	}
	
	// Insert in third list
	
	while (table->head[index2] != NULL) {
	
		if (!exists(table, table->head[index2]->data,index1)){
		
			struct node* new_node = (struct node*)malloc(sizeof(struct node));
			
			new_node->data = table->head[index2]->data;
			new_node->next = list;
			list = new_node;
			
		}
		
		table->head[index2] = table->head[index2]->next;
	}	
	
	return list;

}

struct node* query_intersection(HashTable *table, int index1, int index2)
{
	struct node* list;
	
	while (table->head[index1] != NULL) {		// Traverse first word's ID list
		
		table->head[index2] = table->head[index2];
		
		while(table->head[index2]!=NULL){				// Compare each ID of both lists
			
			if (strcmp(table->head[index2]->data, table->head[index1]->data)==0){		// If they have the same ID, then add them to a third list (the result list which will be returned)
				
				struct node* new_node = (struct node*)malloc(sizeof(struct node));
				
				new_node->data = table->head[index1]->data;
				new_node->next = list;
				list = new_node;
				
			}
			
			table->head[index2] = table->head[index2]->next;		// Go to second list's next node
			
		}
		
		table->head[index1] = table->head[index1]->next;		// Go to first list's next node
	}
	
	return list;
}

int exists(HashTable *table, char *data, int index)
{
	while (table->head[index] != NULL) {
		if (strcmp(table->head[index]->data,data)==0){
			return 1;
		}
		table->head[index] = table->head[index]->next;
	}
	return 0;
}

void stats(HashTable *table, char *h_word){
	
    long unsigned index, i = hash_function(h_word);
    
	int h=1;
    index = i;
    
	if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){
            // Collision
			i = (i + (h * h)) % TABLE_SIZE;	// Quadratic probing, handling collision
			h++;
            if (i==(TABLE_SIZE-1)){			// If index is the last bucket of the array, go to the first bucket
                i=0;
            }
        }
    }
    if(table->head[i] != NULL){
        printf("\n%d documents\n", table->DF[i]);
    }
    else{
        printf("\nNo documents found!\n");
    }
}

int stats_intersection(HashTable *table, int index1, int index2)
{
	struct node* list;
	
	int count=0;
	
	while (table->head[index1] != NULL) {		// Traverse first list
		
		if (exists(table, table->head[index1]->data, index2) ){	// If it exists in second list, count it
			count++;
		}
		
		table->head[index1] = table->head[index1]->next;
	}
	
	return count;
}

int stats_union(HashTable *table, int index1, int index2){
	
	struct node *list;
	int count = 0;
	
	while (table->head[index1]!=NULL){
		table->head[index1]=table->head[index1]->next;			// Traverse first list, and count how many IDs this list has
		count++;
	}
	
	while (table->head[index2]!=NULL){			// Traverse second list
		if (!exists(table, table->head[index2]->data, index1)){	// If the ID does not already exist in the second list (so it is unique), count how many IDs this second list has
			count++;
		}
		table->head[index2]=table->head[index2]->next;
	}
	return count;

}

void stats_bool(HashTable *table, char *h_word, char *h_word2, int choice){
    
	long unsigned i, index1 = hash_function(h_word);		// Calculate hash key
    int h;
	
	i = index1;
	h=1;
	
    if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){
            // Collision
			i = (i + (h * h)) % TABLE_SIZE;		// Quadratic probing
			h++;
            if (i==(TABLE_SIZE-1)){				// If index is in the last bucket of the array, go to the first bucket of the array
                i=0;
            }
        }
    }
    
    long unsigned index2 = hash_function(h_word2);
	
	i = index2;
	h=1;
	
    if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],h_word)==0)) ){
            // Collision
			i = (i + (h * h)) % TABLE_SIZE;		// Quadratic probing
			h++;
            if (i==(TABLE_SIZE-1)){				// If index is in the last bucket of the array, go to the first bucket of the array
                i=0;
            }
        }
    }
    
    int count_docs;
    
    if (choice==1){				//statsOR
	
		count_docs = stats_union(table, index1, index2);
		
        if (count_docs==0){
			printf("\nNo documents found!\n");
		}
		else
		{
			printf("\n%d documents found!\n", count_docs);
		}
	}
	
    else if (choice==2){		//statsAND
    	
		count_docs = stats_intersection(table, index1, index2);
		
        if (count_docs==0){
			printf("\nNo documents found!\n");
		}
		else
		{
			printf("\n%d documents found!\n", count_docs);
		}
	}
}

HashTable* create_table(int size) {
    
    int i;
    
    HashTable* table = (HashTable*) malloc (sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->DF = (int*) malloc (sizeof(int)*TABLE_SIZE);
    table->word = malloc(TABLE_SIZE * sizeof(char *));
    
    for (i=0; i<TABLE_SIZE; ++i) {
        table->word[i] = (char *)malloc(200+1);		// Maximum word length: 200 characters
    }
    
    for (i=0; i<table->size; i++){
        table->DF[i] = 0;
    }
    
    table->head = create_head(table);
 
    return table;
}

static struct node** create_head(HashTable* table) {
    int i;
    struct node** head = (struct node**) calloc (table->size, sizeof(struct node*));
    for (i=0; i<table->size; i++){
        head[i] = NULL;
    }
    return head;
}

void delete_table(HashTable* table){
    int i;
    for (i=0; i<table->size; i++) {
        struct node *temp = table->head[i];
        if (temp != NULL)
            free_node(temp);
    }
    
    free(table);
}

void free_node(struct node* node){
    free(node->data);
    free(node);
}

int traverse_list(HashTable* table, unsigned long i, char *str, char *ID2)
{
    if (table->head[i]!=NULL){
        for(temp=table->head[i];temp!=NULL;temp=temp->next){
            if (strcmp(table->word[i],str)==0){
                if (strcmp(ID2,temp->data)==0){
                    return 0;
                }
            }
        }
    }
    return 1;
}

void insert(HashTable* table, char *str, char *ID2)
{
    int key, j;
    
    unsigned long i;
    
    i = hash_function(str);			// Calculate hash key
    
    int flag;
    
    flag = traverse_list(table, i, str, ID2);		// Check if the word has been repeated (if the document ID is already on this word's list)
    
    if (flag==0){					// If the word isn't unique, do not add it
        return;
    }
    
    int h=1;
    unsigned long index = i;
    
    // Collision handling
    if (table->word[i]!=NULL){
        
        while( (strlen(table->word[i])!=0) && ( !(strcmp(table->word[i],str)==0)) ){
            
			// Collision
			
			i = (i + (h * h)) % TABLE_SIZE;			// Quadratic Probing
			
			h++;
			
			flag = traverse_list(table, i, str, ID2);	// The index changed because of the collision, check list with this index
		    
		    if (flag==0){
		        return;
		    }
		    
            if (i==(TABLE_SIZE-1)){					// If index is in the last bucket of the array, go to the first bucket of the array
                i=0;
            }
            
        }
    }
    
    char *ID = malloc(255*sizeof(char));
    strncpy(ID, ID2, strlen(ID2) + 1);
    
    struct node * newnode=(struct node *)malloc(sizeof(struct node));
    newnode->data=ID;
    newnode->next = NULL;
    
    strcpy(table->word[i], str);			// The word's bucket will now contain the word read by file
    
    table->DF[i]= table->DF[i] + 1;			// The word's document frequency will get bigger by 1, one more document contains this unique word
    
    table->count++;							// Hash table will have one more word/element
    
	// Insert this node (data, and pointer to next node) to list
	if(table->head[i] == NULL){		// If the list is empty, add node as the head of the list
        table->head[i] = newnode;
    }
    else							// If the list is not empty, add node as the last element
    {
        temp=table->head[i];
        while(temp->next != NULL)
        {
        	temp=temp->next;			// Traversing the list
        }
        temp->next=newnode;
    }
}

unsigned long hash_function(char* str) {
    
	unsigned long i, h=0;
    
	for(i = 0; i < strlen(str); ++i){
        h = 3*h + 7*str[i];
    }
	return h % TABLE_SIZE;
}

