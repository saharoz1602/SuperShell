#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define N 256

typedef struct l_frequency {
	int times;
	char letter;
	float freq;
} L_f;

typedef struct Lock {
	char name[N];
	int time;
	struct timeval T_lock;
} Lock;

int procs(char *path, char args[][N], int n);
int encryptFile(char* name, int x, char* name_new);
int decryptFile(char* name, int x, char* name_new);
int lockCmdForTime(char* name, int time);
int isLocked(char* name);
void release_lock();
int letterFreq(char* name);
int findmax(int arr[], int n); //to help find the maximum.
int uppercaseByIndex(char* name, char* name_new, int index);
int lowercaseByIndex(char* name, char* name_new, int index);
int randomFile(int x, char* name);
int compressFile(char* path, char* newpath);

Lock lock_commands[N];

int main() {
	int i, flag2 = 1;
	srand(time(NULL));

	for (i = 0; i < N; i++) {
		strcpy(lock_commands[i].name, "");
	}

	do {
		char s[2] = " ", *token, args[6][N], path[N] = "/bin/";
		char buffer[N]={0};
		int count = 0, flag = 0, i, fd_o, fd_s, outflag = 0;

		printf("\nSuperShell>");
		fgets(buffer, N, stdin);

		token = strtok(buffer, s); //to divide the input until the space.
		while ((token != NULL) && (count < 6)) {
			strcpy(args[count], token);
			count++;
			token = strtok(NULL, s);
		}

		args[count - 1][strlen(args[count - 1]) - 1] = '\0'; //to remove the enter.

		strcat(path, args[0]);

		//check the case when the output is include ">".
		for (i = 0; i < count; i++) {
			if (!(strcmp(args[i], ">"))) {
				if ((fd_o = open(args[i + 1], O_WRONLY | O_CREAT, 0777))== -1) {
					perror("open File\n");
					return (-1);
				}
				fd_s = dup(1); //saves the default screen fd
				dup2(fd_o, 1); //replace the default output with the opened file.
				outflag = 1; //indicate that the output has been changed.
				count -= 2; //for ">" and "new stdout"
			}
		}

		if (isLocked(args[0])) {
			printf("%s is locked!\n", args[0]);
			flag = 1; //to insure that we d'ont do a system call.
		}

		else if (strcmp("encryptFile", args[0]) == 0) {
			int x = atoi(args[2]);
			if (x > 100 && x < 1) //check if x is between 1-100
					{
				perror("x out of range!\n");
				return (-1);
			}

			encryptFile(args[1], x, args[3]);
			flag = 1;
		}

		else if (strcmp("decryptFile", args[0]) == 0) {
			int x = atoi(args[2]);
			if (x > 100 && x < 1) //check if x is between 1-100
					{
				perror("x out of range!\n");
				return (-1);
			}
			decryptFile(args[1], x, args[3]);
			flag = 1;
		}

		else if (strcmp("lockCmdForTime", args[0]) == 0) {
			int t = atoi(args[2]);
			lockCmdForTime(args[1], t);
			flag = 1;
		}

		else if (strcmp("letterFreq", args[0]) == 0) {
			letterFreq(args[1]);
			flag = 1;
		}

		else if (strcmp("uppercaseByIndex", args[0]) == 0) {
			int n = atoi(args[3]);
			uppercaseByIndex(args[1], args[2], n);
			flag = 1;
		}

		else if (strcmp("lowercaseByIndex", args[0]) == 0) {
			int n = atoi(args[3]);
			lowercaseByIndex(args[1], args[2], n);
			flag = 1;
		}

		else if (strcmp("randomFile", args[0]) == 0) {
			int n = atoi(args[1]);
			randomFile(n, args[2]);
			flag = 1;
		}

		else if (strcmp("compressFile", args[0]) == 0) {

			compressFile(args[1], args[2]);
			flag = 1;
		}

		else if (strcmp("byebye", args[0]) == 0) {
			flag = 1;
			flag2 = 0;
		}

		if (!flag) {
			if (procs(path, args, count - 1) == -1) {
				printf("too many/less arguments!\n");
			}
		}

		if (outflag == 1) {
			dup2(fd_s, 1); //to make sure that stdout default is the screen.
			close(fd_o);
		}

		release_lock();

	} while (flag2);

	return 0;

}

int procs(char *path, char args[][N], int n) {/*this function will check if the commands are from the shell system. */
	int pid;

	if ((pid = fork()) == -1) {
		perror("fork() failed.");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		switch (n) {
		case 0:/*for zero arguments*/

			execlp(args[0], args[1], NULL);
			printf("Not Supported!\n");
			exit(0);
			break;

		case 1: /*for 1 arguments*/

			execl(path, args[0], args[1], NULL);
			printf("Not Supported!\n");
			exit(0);
			break;

		case 2: /*for 2 arguments*/

			execl(path, args[0], args[1], args[2], NULL);
			printf("Not Supported!\n");
			exit(0);
			break;

		case 3:/*for 3 arguments*/

			execl(path, args[0], args[1], args[2], args[3], NULL);
			printf("Not Supported!\n");
			exit(0);
			break;

		default:
			exit(0);
		}
	}

	wait();
	return 0;
}

int encryptFile(char* name, int x, char* name_new) {

	int fd1, fd2, rbytes, wbytes, i;
	char buff[N];

	if ((fd1 = open(name, O_RDONLY)) == -1) //open source file to read.
			{
		perror("open from");
		return (-1);
	}

	if ((fd2 = open(name_new, O_WRONLY | O_CREAT, 0777)) == -1) { //open destination file for write.
		perror("open to");
		return (-1);
	}

	while ((rbytes = read(fd1, buff, N)) != 0) { //read N bytes to the buffer from the source file.

		if (rbytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < rbytes - 1; i++) { //change the buffer chars as asked.
			int temp = buff[i] + x;
			buff[i] = temp % N;

		}
		if ((wbytes = write(fd2, buff, rbytes)) == -1) { //write r_y bytes from the buffer to the destination file.
			perror("write");
			return (-1);
		}

		if (wbytes != rbytes) { //in case there was an Error writing the same text that we read, we abort.
			perror("wrong write");
			return (-1);
		}

	}

	close(fd1);
	close(fd2);

	return 0;

}

int decryptFile(char* name, int x, char* name_new) {

	int fd1, fd2, rbytes, wbytes, i;
	char buff[N];

	if ((fd1 = open(name, O_RDONLY)) == -1) //open source file to read.
			{
		perror("open from");
		return (-1);
	}

	if ((fd2 = open(name_new, O_WRONLY | O_CREAT, 0777)) == -1) { //open destination file for write.
		perror("open to");
		return (-1);
	}

	while ((rbytes = read(fd1, buff, N)) != 0) { //read N bytes to the buffer from the source file.

		if (rbytes == -1) {
			perror("read");
			return (-1);
		}

		for (i = 0; i < rbytes - 1; i++) { //change the buffer chars as asked.
			int temp = buff[i] - x;
			buff[i] = temp % N;

		}
		if ((wbytes = write(fd2, buff, rbytes)) == -1) { //write r_y bytes from the buffer to the destination file.
			perror("write");
			return (-1);
		}

		if (wbytes != rbytes) { //in case there was an Error writing the same text that we read, we abort.
			perror("wrong write");
			return (-1);
		}

	}

	close(fd1);
	close(fd2);

	return 0;
}

int lockCmdForTime(char* name, int time) {/*this function lock the given command by time (seconds).*/

	int i;

	for (i = 0; i < N; i++) {
		if (!strcmp(name, lock_commands[i].name)) {/*check if the command name is in the array.*/
			if (isLocked(name)) {
				lock_commands[i].time += time;
			} else {
				lock_commands[i].time = time;
				gettimeofday(&(lock_commands[i].T_lock), NULL);
			}
			return 0;
		}
	}

	for (i = 0; i < N; i++) {/*add to the array the command name and time.*/
		if (!strcmp("", lock_commands[i].name)) {
			strcpy(lock_commands[i].name, name);
			lock_commands[i].time = time;
			gettimeofday(&(lock_commands[i].T_lock), NULL);
			return 0;
		}
	}
	return (-1);
}

int isLocked(char* name) {/*help function to insure that the command is locked.*/

	double elapsedTime;
	struct timeval t1;
	int i;

	gettimeofday(&(t1), NULL); //to save the current time.

	for (i = 0; i < N; i++) {
		if (!strcmp(name, lock_commands[i].name)) {
			elapsedTime = (t1.tv_sec - lock_commands[i].T_lock.tv_sec);/*if the command is in the array,it will calculate the time elapsed and the time of lock.*/
			if (elapsedTime >= lock_commands[i].time) {
				return 0;
			} else {
				return 1;
			}
		}

	}

	return 0;

}

void release_lock(char * name) {/*release the commands from the array.*/

	int i;

	for (i = 0; i < N; i++) {
		if (strcmp("", lock_commands[i].name)) {
			if (!isLocked(lock_commands[i].name)) {
				strcpy(lock_commands[i].name, "");
			}
		}
	}
}

int letterFreq(char* name) {

	char buff[N] = { 0 };
	int count[26] = { 0 }, fd1, rbytes, i = 0;
	double lettersintext = 0.0; //to sum the letters in the file.
	L_f l[3];

	if ((fd1 = open(name, O_RDONLY)) == -1) {
		perror("Open file\n");
		return (-1);
	}
	if ((rbytes = read(fd1, buff, N)) == -1) {
		perror("Read file\n");
		return (-1);
	}
	while (rbytes > 0) //count the number of time a letter appears.
	{
		if ((buff[i] >= 'a' && buff[i] <= 'z')) {
			count[buff[i] - 'a']++;
			lettersintext++;
		}
		if ((buff[i] >= 'A' && buff[i] <= 'Z')) {
			count[buff[i] - 'A']++;
			lettersintext++;
		}
		i++;
		rbytes--;
	}

	for (i = 0; i < 3; i++) { //print the 3 most frequency letters.
		l[i].times = count[findmax(count, 26)];
		l[i].letter = (findmax(count, 26) + 'a');
		count[l[i].letter - 'a'] = 0;
		l[i].freq = (((l[i].times) / (lettersintext)) * 100);
		printf("%c - %.1f%\n", l[i].letter, l[i].freq);
	}
	if (l[0].letter == 'e') {
		if (l[1].letter == 'a') {
			if (l[2].letter == 'o' || l[2].letter == 'i') {
				printf("Good Letter Frequency\n");
			}
		}
	}
	close(fd1);
	return 0;
}

int findmax(int arr[], int n) { //help function to find the maximum frequency.
	int i, max_index = arr[0];

	for (i = 1; i < n; i++) {
		if (max_index <= arr[i]) {
			max_index = i;
		}
	}

	return max_index;
}

int uppercaseByIndex(char* name, char* name_new, int index) {

	char buff[N], buff_o[N] = { 0 }, *token, s[2] = " ";
	int fd_i, fd_o, rbytes, wbytes;

	if ((fd_i = open(name, O_RDONLY)) == -1) {
		perror("Open file\n");
		return (-1);
	}

	if ((fd_o = open(name_new, O_WRONLY | O_CREAT, 0777)) == -1) {
		perror("Open file\n");
		return (-1);
	}

	if ((rbytes = read(fd_i, buff, N)) == -1) {
		perror("Read File\n");
		return (-1);
	}

	token = strtok(buff, s); //to copy until the space.
	while (token != NULL) {
		if (strlen(token) > index) {
			if (token[index] >= 'a' && token[index] <= 'z') {
				token[index] -= 32;
			}
		}
		strcat(buff_o, token); //to copy the new str
		strcat(buff_o, s); //add the space
		token = strtok(NULL, s);
	}

	if ((wbytes = write(fd_o, buff_o, rbytes)) == -1) {
		perror("Write File\n");
		return (-1);
	}

	close(fd_i);
	close(fd_o);
	return 0;
}

int lowercaseByIndex(char* name, char* name_new, int index) {
	char buff[N], buff_o[N] = { 0 }, *token, s[2] = " ";
	int fd_i, fd_o, rbytes, wbytes;

	if ((fd_i = open(name, O_RDONLY)) == -1) {
		perror("Open file\n");
		return (-1);
	}

	if ((fd_o = open(name_new, O_WRONLY | O_CREAT, 0777)) == -1) {
		perror("Open file\n");
		return (-1);
	}

	if ((rbytes = read(fd_i, buff, N)) == -1) {
		perror("Read File\n");
		return (-1);
	}

	token = strtok(buff, s); //to copy until the space.
	while (token != NULL) {
		if (strlen(token) > index) {
			if (token[index] >= 'A' && token[index] <= 'Z') {
				token[index] += 32;
			}
		}
		strcat(buff_o, token); //to copy the new str
		strcat(buff_o, s); //add the space
		token = strtok(NULL, s);
	}

	if ((wbytes = write(fd_o, buff_o, rbytes)) == -1) {
		perror("Write File\n");
		return (-1);
	}

	close(fd_i);
	close(fd_o);
	return 0;
}

int randomFile(int x, char* name) {
	char buff_o[N];
	int fd_o, wbytes, i;

	if ((fd_o = open(name, O_WRONLY | O_CREAT, 0777)) == -1) {
		perror("Open File\n");
		return (-1);
	}

	for (i = 0; i < x; i++) {
		buff_o[i] = rand() % 26; //for choosing between a-z
		if (rand() % 2) //for choosing uppercase or lowercase
				{
			buff_o[i] += 'a';
		} else {
			buff_o[i] += 'A';
		}
	}

	if ((wbytes = write(fd_o, buff_o, x)) == -1) {
		perror("Write File\n");
		return (-1);
	}

	close(fd_o);
	return 0;
}

int compressFile(char* path, char* newpath) {

	int fd_i, fd_o, rbytes, wbytes, i = 0, count = 1, temp,total_count=0;
	char buff[N], buff_o[N], *p, *q;

	if ((fd_i = open(path, O_RDONLY)) == -1) //we opened the from file for reading.
			{
		perror("Open File\n");
		return (-1);
	}

	if ((fd_o = open(newpath, O_WRONLY | O_CREAT, 0777)) == -1) {
		perror("Open File\n");
		return (-1);
	}

	if ((rbytes = read(fd_i, buff, N)) == -1) {
		perror("Read File\n");
		return (-1);
	}

	p = buff; //a pointer for the start of the buff
	q = buff;
	q++; //pointer that will run if its the same letter
	temp = rbytes;
	while (temp > 1) {
		if (*p != *q) { //if they are different letters it will put P in the buff out
			buff_o[i] = *p;
			p++;
			q++;
			i++;
			temp--;
		} else { //else,q will run until p and q are different and count the number of duplicate letters and put it instead of the duplicate letters.
			while (*p == *q) {
				q++;
				count++;
				temp--;
			}
			buff_o[i] = *p;
			i++;
			buff_o[i] = (count + '0');
			i++;
			p = q;
			q++;
			count = 1;
		}
		total_count++;//for the write.
	}

	if ((wbytes = write(fd_o, buff_o,total_count+1))== -1) {
		perror("Write File\n");
		return (-1);
	}

	printf("\n");

	close(fd_i);
	close(fd_o);
	return 0;
}

