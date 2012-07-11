/* b96902124 莫安德 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#define _XOPEN_SOURCE 500
#define MAX_LENGTH 1000
#define TMP_SIZE 2048
#define OMEGA 0xFFFFFFFF


unsigned int *buffer_data;
void initialize(int *c1, int *c2, int *tmp, int *t1);
int compare(const void *a, const void *b);
void write_on(int FD, unsigned *buffer, int *pointer, int offset, int input, int flag);
void merge_process(int process, int data, int mode, unsigned *tenth_number, int readFD, int writeFD);
void time_egg(double *average_time, double *confidence_interval, int processes, int mode, int timeFD);
int test_fork(int processes, int data, int inputFD, int outputFD);
int test_vfork(int processes, int data, int inputFD, int outputFD);


int main(int argc, char *argv[]){

	int mode = 0; //If fork() -> mode = 1, if vfork -> mode = 0
	int processes, data, counter;
	int file_size;
	int inputFD;
	int outputFD;

	if (argc < 9){
		fprintf(stderr, "Use: -n, Number of child process, -N, number_od_data,  {fork | vfork}, input file, output file\n");
		return -1;
	}

	for(counter = 1; counter < 7; counter++){

		if  ( ((strcmp(argv[counter], "-n")) == 0) || ((strcmp(argv[counter], "-m")) == 0) ){
			counter += 1;
			if ((strcmp(argv[counter], "fork")) == 0)
				mode = 1;
			else if ((strcmp(argv[counter], "vfork")) == 0)
				mode = 0;
			else
				processes = atoi (argv[counter]);
		}

		else if ( (strcmp(argv[counter], "-N")) == 0 ){
			counter += 1;
			data = atoi(argv[counter]);
		}
	}

	if ((inputFD = open(argv[7], O_RDONLY, 0777)) < 0){
		fprintf(stderr, "Error while opening the input file\n");
		return 1;
	}

	if ((outputFD = open(argv[8], O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0){
		fprintf(stderr, "Error while opening the output file\n");
		return 1;
	}	

	lseek(inputFD, 0, SEEK_SET);
	lseek(outputFD, 0, SEEK_SET);

	if(mode){
		test_fork(processes, data, inputFD, outputFD);
	}
	else{
		test_vfork(processes, data, inputFD, outputFD);
	}
}

int compare(const void *a, const void *b){
	return (*(unsigned int*)a - *(unsigned int*)b);
}

void merge_process(int process, int data, int mode, unsigned *tenth_number, int readFD, int writeFD){
	int *p_select, pointer_out;
	int counter = 0;
	int counter_2;
	int p_sort, *p_read;
	unsigned **col_buf, *sort_buf;
	int offset;
	p_sort = 0;
	pointer_out = 0;
	col_buf = (unsigned**)malloc(sizeof(unsigned*) * process);
	sort_buf = (unsigned*)malloc(sizeof(unsigned) * TMP_SIZE);	
	p_read = (int*)malloc(sizeof(int) * process);
	p_select = (int*)malloc(sizeof(int) * process);

	while(counter < process){
		col_buf[counter] = (unsigned*)malloc(sizeof(unsigned) * TMP_SIZE);
		if (data > TMP_SIZE){
			for(counter_2 = 0; counter_2 < sizeof(unsigned) * TMP_SIZE;){
				offset = pread(readFD,((char*)col_buf[counter]) + counter_2, sizeof(unsigned) * TMP_SIZE - counter_2, sizeof(unsigned)*(counter * data) + counter_2);
				if(offset > 0)
					counter_2 += offset;
			}
			p_read[counter] = (unsigned)index;
		}	
		else{
			for(counter_2 = 0; counter_2 < sizeof(unsigned) * data;){
				offset = pread(readFD,((char*)col_buf[counter]) + counter_2, sizeof(unsigned) * data - counter_2, sizeof(unsigned)*(counter * data) + counter_2);
				if(offset > 0)
					counter_2 += offset;
			}
			p_read[counter] = (unsigned)index;			
		}
		p_select[counter] = 0;
		counter++;
	}

	for(counter = 0; counter < process * data; counter++){
		int min_data, min_process;
		unsigned val = OMEGA;
		for(counter_2 = 0; counter_2 < process; counter_2++){
			int valid;
			if(TMP_SIZE >= data)
				valid = data;
			else if(p_read[counter_2] > data)
				valid = TMP_SIZE - p_read[counter_2] + data;
			else
				valid = TMP_SIZE;
			if(col_buf[counter_2][p_select[counter_2]] < val && p_select[counter_2] < valid){
				min_process = counter_2;
				min_data = p_select[counter_2];
				val = col_buf[counter_2][p_select[counter_2]];
			}
		}
		p_select[min_process]++;
		sort_buf[p_sort] = val;
		p_sort++;

		if(p_sort >= TMP_SIZE){
			if (mode == 2){

				if(counter == process * data - 1 && (TMP_SIZE > process * data || (process * data) % TMP_SIZE == 0))
					write_on(writeFD, sort_buf, &pointer_out, TMP_SIZE, 1, 0);
				else
					write_on(writeFD, sort_buf, &pointer_out, TMP_SIZE, 0, 0);			
			}
			else{	
				if(mode == 2) 
					write_on(writeFD, sort_buf, &pointer_out, offset, TMP_SIZE, 1);
				else
					write_on(writeFD, sort_buf, &pointer_out, (process * data * mode), TMP_SIZE, 1);
			}
			p_sort = 0;
		}

		if(p_select[min_process] >= TMP_SIZE && p_read[min_process] < data){
			p_read[min_process] += TMP_SIZE;
			p_select[min_process] = 0;
		}

		if(mode != 2)
			if(counter == 9)
				*tenth_number = val;

	}
	if(mode == 2 && (process * data) % TMP_SIZE != 0)
		write_on(writeFD, sort_buf, &pointer_out, p_sort, 1, 0);
	else{
		if(mode < 2)	
			write_on(writeFD, sort_buf, &pointer_out, (process * data * mode), p_sort, 1);
		else
			write_on(writeFD, sort_buf, &pointer_out, offset, p_sort, 1);
	}
	for(counter = 0; counter < process; counter++)
		free(col_buf[counter]);
	
	free(col_buf);
	free(sort_buf);
	free(p_read);
	free(p_select);
}

void initialize(int *c1, int *c2, int *tmp, int *t1){ 
	*c1 = fileno(tmpfile());
	*c2 = fileno(tmpfile());
	*t1 = fileno(tmpfile());
	*tmp = fileno(tmpfile());
	lseek(*c1, 0, SEEK_SET);
	lseek(*c2, 0, SEEK_SET);
	lseek(*t1, 0, SEEK_SET);
	lseek(*tmp, 0, SEEK_SET);
}

void time_egg(double *average_time, double *confidence_interval, int processes, int mode, int timeFD){

	int cont;
	size_t offset;
	unsigned long *end_of_time;
	unsigned long total = 0;
	end_of_time = (unsigned long*)malloc (sizeof(unsigned long) * processes);

	offset = 0;
	while(offset < sizeof(unsigned long) *processes){
		size_t game_over = pread(timeFD, ((char*)end_of_time) + offset, sizeof(unsigned long) * processes - offset, mode * sizeof(unsigned long) * processes);
		if (game_over > 0)
			offset += game_over;
	}

	for (cont = 0; cont < processes; cont++)
		total += end_of_time[cont];
	*average_time = (double) total / (double) processes;

	total = 0;

	for (cont = 0; cont < processes; cont++)
		total += (end_of_time[cont] - *average_time) * (end_of_time[cont] - *average_time);

	*confidence_interval = 1.96 * sqrt(1.0/ (double) (processes) * total) / sqrt((double)(processes));
	printf("%.4f +- %.4f ",*average_time, *confidence_interval);
	free(end_of_time);

}

void write_on(int FD, unsigned *buffer, int *pointer, int offset, int input, int flag){
	int i, index;
	char *str_buf = (char*)malloc(sizeof(char) * offset * 15);
	char tmp[15];
	int cont = 0;
	if(flag == 0){

		str_buf[0] = '\0';
		for(i = 0; i < offset - 1; i++){
			cont += sprintf(tmp,"%u ", buffer[i]);
			strcat(str_buf, tmp);
		}
		if(input > 0){
			cont += sprintf(tmp, "%u\n", buffer[offset - 1]);
			strcat(str_buf, tmp);
		}
		else{
			cont += sprintf(tmp, "%u ",buffer[offset - 1]);
			strcat(str_buf, tmp);
		}
		for(index = 0; index < cont;){
			int done = pwrite(FD, ((char*)str_buf) + index, sizeof(char) * cont - index, sizeof(char) * (*pointer) + index);
			if(done > 0)
				index += done;
		}

		*pointer += cont;
		free(str_buf);
	}
	if (flag == 1){

		for(index = 0; index < input;){
			int done = pwrite(FD, ((char*)buffer) + index, sizeof(unsigned) * input - index, sizeof(unsigned) * (offset + (*pointer)) + index);
			if(done > 0)
				index += done;
		}
		*pointer += input;	
	}
}

int test_fork(int processes, int data, int inputFD, int outputFD){
	int index, neo_index, counter;
	unsigned tenth_number;
	int half_proc = processes / 2;
	double average_time;
	double confidence_interval;
	unsigned long total_time;
	int child1FD, timeFD;
	int child2FD, tempFD;
	struct timespec Chrono;
	struct timespec tv;
	initialize(&child1FD, &child2FD, &timeFD, &tempFD);
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &Chrono) < 0){
		fprintf(stderr, "Error while setting time\n");
		return 1;
	}

	//Father
	pid_t father;
	if ((father = fork()) < 0 ){
		fprintf(stderr, "Error during father fork\n");
		abort();
	}

	else if (father == 0){
		pid_t child;
		for(counter = 0; counter < half_proc; counter++){
			if ((child = fork()) < 0){
				wait(NULL);
				counter--;
			}

			else if (child == 0){		

				buffer_data = (unsigned *) malloc (sizeof(unsigned) * (data / processes));

				for(index = 0; index < sizeof(unsigned) * (data/processes);){
					neo_index = pread(inputFD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data/processes)) + index);
					if(neo_index > 0)
						index += neo_index;
				}

				qsort(buffer_data, (data / processes), sizeof(unsigned), compare);

				for(index = 0; index < sizeof(unsigned) * (data/processes);){
					neo_index = pwrite(child1FD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data/processes)) + index);
					if(neo_index > 0)
						index += neo_index;
				}
				free(buffer_data);

				if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv) < 0){
					fprintf(stderr, "Error while setting auxiliar time\n");
					return 1;
				}
				total_time = (tv.tv_sec - Chrono.tv_sec) * 1000000000.0 + (tv.tv_nsec - Chrono.tv_nsec);
				pwrite(timeFD, &total_time, sizeof(unsigned long),(counter)*sizeof(unsigned long));
				break;
			}

		}
		if(child != 0){
			int temp = half_proc;
			while(temp-- > 0)
				wait(NULL);
			merge_process(half_proc, (data / processes), 0, &tenth_number, child1FD, tempFD);
			printf("fork: ");
			time_egg(&average_time, &confidence_interval, half_proc, 0, timeFD);
			printf("usecs, %u\n", tenth_number);
			return 0;
		}
		else
			return 0;
	}

	else{
		pid_t mother;

		//Mother
		if ((mother = fork()) < 0 ){
			fprintf(stderr, "Error during mother fork\n");
			abort();
		}

		else if (mother == 0){
			pid_t child;
			for(counter = 0; counter < half_proc; counter++){
				if ((child = fork()) < 0){
					wait(NULL);
					counter--;
				}

				else if (child == 0){					
					buffer_data = (unsigned *) malloc (sizeof(unsigned) * (data / processes));

					for(index = 0; index < sizeof(unsigned) * (data / processes);){
						neo_index = pread(inputFD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data / processes) + (data/2)) + index);
						if(neo_index > 0)
							index += neo_index;
					}

					qsort(buffer_data, (data / processes), sizeof(unsigned), compare);

					for(index = 0; index < sizeof(unsigned) * (data / processes);){
						neo_index = pwrite(child2FD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data / processes)) + index);
						if(neo_index > 0)
							index += neo_index;
					}
					free(buffer_data);						

					if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv) < 0){
						fprintf(stderr, "Error while setting auxiliar time\n");
						return 1;
					}
					total_time = (tv.tv_sec - Chrono.tv_sec) * 1000000000.0 + (tv.tv_nsec - Chrono.tv_nsec);
					pwrite(timeFD, &total_time, sizeof(unsigned long),(half_proc + counter)*sizeof(unsigned long));
					break;
				}					

			}
			if (child != 0){			
				for(counter = 0; counter < half_proc; counter++)
					wait(NULL);
				merge_process(half_proc, data/processes, 1, &tenth_number, child2FD, tempFD);
				printf("fork: ");
				time_egg(&average_time, &confidence_interval, half_proc, 1, timeFD);
				printf("usecs, %u\n", tenth_number);
				return 0;
			}
			else
				return 0;
		}
		else{
			int temp = 2;
			while(temp-- > 0){
				wait(NULL);
			}
			merge_process(processes, data/processes, 2, NULL, tempFD, outputFD);
			return 0;
		}

	}

}

int test_vfork(int processes, int data, int inputFD, int outputFD){
	int index, neo_index, counter;
	unsigned tenth_number;
	int half_proc = processes / 2;
	double average_time;
	double confidence_interval;
	unsigned long total_time;
	int child1FD, timeFD;
	int child2FD, tempFD;
	struct timespec Chrono;
	struct timespec tv;
	initialize(&child1FD, &child2FD, &timeFD, &tempFD);
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &Chrono) < 0){
		fprintf(stderr, "Error while setting time\n");
		return 1;
	}	
	pid_t father = -1;

	while(father < 0)
		father = vfork();
	if(father==0){
		pid_t vchild;
		for(counter = 0; counter < half_proc; counter++){
			vchild = vfork();
			if(vchild < 0){
				wait(NULL);
				counter--;
			}
			else if(vchild == 0){
				buffer_data = (unsigned *) malloc (sizeof(unsigned) * (data / processes));

				for(index = 0; index < sizeof(unsigned) * (data/processes);){
					neo_index = pread(inputFD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data/processes)) + index);
					if(neo_index > 0)
						index += neo_index;
				}

				qsort(buffer_data, (data / processes), sizeof(unsigned), compare);

				for(index = 0; index < sizeof(unsigned) * (data/processes);){
					neo_index = pwrite(child1FD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data/processes)) + index);
					if(neo_index > 0)
						index += neo_index;
				}
				free(buffer_data);
				if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv) < 0){
					fprintf(stderr, "Error while setting auxiliar time\n");
					return 1;
				}
				total_time = (tv.tv_sec - Chrono.tv_sec) * 1000000000.0 + (tv.tv_nsec - Chrono.tv_nsec);
				pwrite(timeFD, &total_time, sizeof(unsigned long),(counter)*sizeof(unsigned long));
				break;
			}
		}

		if(vchild != 0){ 		
			int temp = half_proc;
			while(temp-- > 0)
				wait(NULL);
			merge_process(half_proc, (data / processes), 0, &tenth_number, child1FD, tempFD);
			printf("vfork: ");
			time_egg(&average_time, &confidence_interval, half_proc, 0, timeFD);
			printf("usecs, %u\n",tenth_number);
			exit(0);							

		}
		else
			exit(0);
	}
	else{
		pid_t mother = -1;
		while(mother < 0)
			mother = vfork();
		if(mother == 0){
			pid_t vchild;
			for(counter = 0; counter < half_proc; counter++){
				vchild = vfork();
				if(vchild < 0){
					wait(NULL);
					counter--;
				}
				else if(vchild == 0){
					buffer_data = (unsigned *) malloc (sizeof(unsigned) * (data / processes));

					for(index = 0; index < sizeof(unsigned) * (data / processes);){
						neo_index = pread(inputFD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data / processes) + (data/2)) + index);
						if(neo_index > 0)
							index += neo_index;
					}

					qsort(buffer_data, (data / processes), sizeof(unsigned), compare);

					for(index = 0; index < sizeof(unsigned) * (data / processes);){
						neo_index = pwrite(child2FD, buffer_data + index / sizeof(unsigned), sizeof(unsigned) * (data / processes) - index, sizeof(unsigned) * (counter * (data / processes)) + index);
						if(neo_index > 0)
							index += neo_index;
					}
					free(buffer_data);						

					if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv) < 0){
						fprintf(stderr, "Error while setting auxiliar time\n");
						return 1;
					}
					total_time = (tv.tv_sec - Chrono.tv_sec) * 1000000000.0 + (tv.tv_nsec - Chrono.tv_nsec);
					pwrite(timeFD, &total_time, sizeof(unsigned long),(half_proc + counter)*sizeof(unsigned long));
					break;
				}
			}
			if(vchild != 0){
				for(counter = 0; counter < half_proc; counter++)
					wait(NULL);

				merge_process(half_proc, data/processes, 1, &tenth_number, child2FD, tempFD);
				printf("vfork: ");
				time_egg(&average_time, &confidence_interval, half_proc, 1, timeFD);
				printf("usecs, %u\n",tenth_number);
				exit(0);
			}
			else
				exit(0);
		}

		int temp;
		for(temp = 2; temp > 0; temp--){
			wait(NULL);
		}

		merge_process(processes, data/processes, 2, NULL, tempFD, outputFD);
		exit(0);
	}


}
