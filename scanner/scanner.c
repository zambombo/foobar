#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 4096

void token_handler(FILE *fp, char input);
void comment_handler(FILE *file_pointer, int flag);
void digit_handler(FILE *file_pointer, char input);
int symbol_handler(FILE *fp, char input_char);
void string_handler(FILE *fp);
FILE *output_fp;

int main(int argc, char *argv[]){

	char input_char;

	FILE *fp;

	if((fp = fopen(argv[1], "r")) == NULL){
		printf("\nError!\n\n%s file doesn't exist!\n\n", argv[1]);
		return -1;
	}
	if((output_fp = fopen(argv[2], "w")) == NULL){
		printf("\nError!\n\n%s.out file can't be created doesn't exist!\n\n", argv[2]);
		return -1;
	}

	while(!feof(fp)){
		input_char = fgetc(fp);
		if (isalnum(input_char)){
			if (isdigit(input_char)){
				digit_handler(fp, input_char);
			}
			if (isalpha(input_char)){
				token_handler(fp, input_char);
			}
		}
		if (ispunct(input_char)){
			symbol_handler(fp, input_char);
		}
	}
	fclose(fp);
	fclose(output_fp);
	return 0;
}

void token_handler(FILE *fp, char input){
	char string[MAXLINE];
	string[0] = '\0';
	char temp[5];
	while ( (isalnum(input)) || (input == '_') ){
		temp[0] = input;
		temp[1] = '\0';
		strcat(string, temp);
		input = fgetc(fp);
	}
	if ( (strcmp(string, "int")) == 0){
		fprintf(output_fp, "262\n");
	}
	else if ( (strcmp(string, "if")) == 0){
		fprintf(output_fp, "263\n");
	}
	else if ( (strcmp(string, "else")) == 0){
		fprintf(output_fp, "264\n");
	}
	else if ( (strcmp(string, "while")) == 0){
		fprintf(output_fp, "265\n");
	}
	else if ( (strcmp(string, "break")) == 0){
		fprintf(output_fp, "266\n");
	}
	else if ( (strcmp(string, "continue")) == 0){
		fprintf(output_fp, "267\n");
	}
	else if ( (strcmp(string, "scan")) == 0){
		fprintf(output_fp, "268\n");
	}
	else if ( (strcmp(string, "print")) == 0){
		fprintf(output_fp, "269\n");
	}
	else{
		fprintf(output_fp, "270\t%s\n", string);
	}
	string[0] = '\0';
	if (ispunct(input)){
		symbol_handler(fp, input);
	}

}

void comment_handler(FILE *file_pointer, int flag){
	char temp;
	temp = fgetc(file_pointer);
	//Comment line //
	if (flag == 1){
		while(temp != '\n'){
			temp = fgetc(file_pointer);
		}
	}
	//Comment line /*
	else{
		while(1){
			if(temp == '*'){
				temp = fgetc(file_pointer);
				if (temp == '/')
					break;
			}
			else
				temp = fgetc(file_pointer);
		}
	}
}

void digit_handler(FILE *file_pointer, char input){
	int output;
	if(input == '0'){
		fprintf(output_fp, "271\t0x00000000\n");
	}
	else{
		output = input - '0';
		input = fgetc(file_pointer);
		while(isdigit(input)){
			output = ( (output * 10) + (input - '0') );
			input = fgetc(file_pointer);
		}
		fprintf(output_fp, "271\t%#010x\n", output);
		if (ispunct(input)){
			symbol_handler(file_pointer, input);
		}
	}
}

int symbol_handler(FILE *fp, char input_char){
	while(ispunct(input_char)){
		switch(input_char){
			case '+':
				fprintf(output_fp, "43\n");
				break;

			case '-':
				fprintf(output_fp, "45\n");
				break;

			case '*':
				fprintf(output_fp, "42\n");
				break;

			case '/':
				input_char = fgetc(fp);
				if (input_char == '/')
					comment_handler(fp, 1);
				else if (input_char == '*')
					comment_handler(fp, 2);
				else{
					fprintf(output_fp, "47\n");
					if (isalnum(input_char)){
						if (isdigit(input_char)){
							digit_handler(fp, input_char);
						}
						if (isalpha(input_char)){
							token_handler(fp, input_char);
						}
					}
					continue;
				}
				break;

			case '&':
				input_char = fgetc(fp);
				if (input_char == '&'){
					fprintf(output_fp, "256\n");
				}						
				else{
					fprintf(output_fp, "\nError!\n Unless you're trying C code on my compiler. The scanner is yet not able to accept this kind of input\n\n");
					return -1;
				}
				break;

			case '|':
				input_char = fgetc(fp);
				if (input_char == '|'){
					fprintf(output_fp, "257\n");
				}
				else{
					printf("\nError!\n Unless you're trying C code on my compiler. The scanner is yet not able to accept this kind of input\n\n");
					return -1;
				}
				break;

			case '!':
				input_char = fgetc(fp);
				if (input_char == '=')
					fprintf(output_fp, "261\n");
				else{
					fprintf(output_fp, "33\n");
					if (isalnum(input_char)){
						if (isdigit(input_char)){
							digit_handler(fp, input_char);
						}
						if (isalpha(input_char)){
							token_handler(fp, input_char);
						}
					}
					continue;
				}
				break;

			case '<':
				input_char = fgetc(fp);
				if (input_char == '=')
					fprintf(output_fp, "258\n");
				else{
					fprintf(output_fp, "60\n");
					if (isalnum(input_char)){
						if (isdigit(input_char)){
							digit_handler(fp, input_char);
						}
						if (isalpha(input_char)){
							token_handler(fp, input_char);
						}
					}
					continue;						
				}
				break;

			case '>':
				input_char = fgetc(fp);
				if (input_char == '=')
					fprintf(output_fp, "259\n");
				else{				
					fprintf(output_fp, "62\n");
					if (isalnum(input_char)){
						if (isdigit(input_char)){
							digit_handler(fp, input_char);
						}
						if (isalpha(input_char)){
							token_handler(fp, input_char);
						}
					}
					continue;
				}
				break;

			case '=':
				input_char = fgetc(fp);
				if (input_char == '=')
					fprintf(output_fp, "260\n");
				else{
					fprintf(output_fp, "61\n");
					if (isalnum(input_char)){
						if (isdigit(input_char)){
							digit_handler(fp, input_char);
						}
						if (isalpha(input_char)){
							token_handler(fp, input_char);
						}
					}
					continue;
				}						
				break;

			case '(':
				fprintf(output_fp, "40\n");
				break;

			case ')':
				fprintf(output_fp, "41\n");
				break;

			case '[':
				fprintf(output_fp, "91\n");
				break;

			case ']':
				fprintf(output_fp, "93\n");
				break;

			case '{':
				fprintf(output_fp, "123\n");
				break;

			case '}':
				fprintf(output_fp, "125\n");
				break;

			case ',':
				fprintf(output_fp, "44\n");
				break;

			case ';':
				fprintf(output_fp, "59\n");
				break;

			case '"':
				string_handler(fp);
				break;

		}
		input_char = fgetc(fp);
		if (isdigit(input_char)){
			digit_handler(fp, input_char);
		}
		if (isalpha(input_char)){
			token_handler(fp, input_char);
		}
	}
	return 0;

}

void string_handler(FILE *fp){
	char input_char;
	input_char = fgetc(fp);
		fprintf(output_fp, "272\t");
	while (input_char != '"'){
		if (input_char == '\\'){
			input_char = fgetc(fp);
		}
		fprintf(output_fp, "%c", input_char);
		input_char = fgetc(fp);
	}
	fprintf(output_fp, "\n");

}
