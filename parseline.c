#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "parseline.h"
#define LNMAX 512
#define PROMPT 6
#define NULLERR 21
#define LENERR 39
#define ARGS_MAX 10
#define PIPELINE_MAX 10

int main(int argc, char *argv[]) {
	int num, arg_count, cur_stage = 0, i = 0, in_redir = 0, out_redir = 0;
	int j;
	struct stage *stages = (struct stage*)calloc(PIPELINE_MAX,
		sizeof(struct stage));
	char *argv_list[ARGS_MAX];
	char cur_arg[LNMAX];
	char in[LNMAX];
	char out[LNMAX];
	char cmdlne[LNMAX + 1];
	char *marker;
	write(STDOUT_FILENO, "line: ", PROMPT);
	if (1 >= (num = read(STDIN_FILENO, cmdlne, LNMAX + 1))) {
		write(STDERR_FILENO, "invalid null command\n", NULLERR);
		free(stages);
		return 1;	
	}
	else if (num > LNMAX) {
		write(STDERR_FILENO,
			"command line length exceeds max length\n", LENERR);
		free(stages);
		return 2;
	}
	cmdlne[strlen(cmdlne) - 1] = '\0';
	strtok(cmdlne, " ");
	while (strtok(NULL, " ")) {
		/* twiddle */
	}
	marker = cmdlne;
	strcpy(in, "original stdin");
	strcpy(out, "original stdout");
	while (i < num) {
		if (cmdlne[i] == '|' || num - i == 1) {
			if (cmdlne[i] == '|' && !out_redir) {
				sprintf(out, "pipe to stage %d", cur_stage + 1);
			}
			else if (cmdlne[i] == '|' && out_redir) {
				fprintf(stderr, "%s: ambiguous output\n",
					argv_list[0]);
				for (j = 0; j < arg_count; j++) {
					free(argv_list[j]);
				}
				free(stages);
				return 5;
			}
			if (arg_count < 1) {
				write(STDERR_FILENO, "invalid null command\n",
					NULLERR);
				for (j = 0; j < arg_count; j++) {
					free(argv_list[j]);
				}
				free_all(stages, cur_stage);
				return 1;	
			}
			for (j = 0; j < num - 1; j++) {
				if (cmdlne[j] == '\0') {
					cmdlne[j] = ' ';
				}
			}
			cmdlne[i++] = '\0';
			stages[cur_stage].stage = cur_stage;
			stages[cur_stage].pipeline = (char*)malloc(sizeof(char)
				* strlen(marker));
			stages[cur_stage].in = (char*)malloc(sizeof(char) *
				strlen(in));
			stages[cur_stage].out = (char*)malloc(sizeof(char) *
				strlen(out));
			strcpy(stages[cur_stage].pipeline, marker);
			stages[cur_stage].argc = arg_count;
			for (j = 0; j < stages[cur_stage].argc; j++) {
				stages[cur_stage].argv_list[j] = (char*)malloc(
					sizeof(char) * strlen(argv_list[j]));
				strcpy(stages[cur_stage].argv_list[j],
					argv_list[j]);
				free(argv_list[j]);
			}
			strcpy(stages[cur_stage].in, in);
			strcpy(stages[cur_stage].out, out);
			marker = marker + strlen(marker) + 1;
			sprintf(in, "pipe from stage %d", cur_stage);
			strcpy(out, "original stdout");
			in_redir = 1;
			out_redir = 0;
			arg_count = 0;
			strtok(cmdlne, " ");
			while (strtok(NULL, " ")) {
				/* twiddle */
			}
			cur_stage++;
		}
		else if (cmdlne[i] == '<') {
			if (in_redir++) {
				fprintf(stderr, "%s: bad input redirection",
					argv_list[0]);
				for (j = 0; j < arg_count; j++) {
					free(argv_list[j]);
				}
				free_all(stages, cur_stage);
				return 3;
			}
			else {
				while (cmdlne[++i] == '\0' ||
					cmdlne[i] == ' ') {
					/* twiddle */
				}
				strcpy(in, cmdlne + i);
				for (j = 0; in[j] != '\0' &&
					in[j] != '<' && in[j] != '>'
					&& in[j] != '|'; j++) {
					/* twiddle */
				}
				if (in[j] == '<' || in[j] == '>'
					|| in[j] == '|') {
					fprintf(stderr,
						"%s: bad input redirection\n",
						argv_list[0]);
					for (j = 0;j < arg_count;j++) {
						free(argv_list[j]);
					}
					free_all(stages, cur_stage);
					return 4;
				}
				in[j] = '\0';
				i += strlen(in);
			}
		}
		else if (cmdlne[i] == '>') {
			if (out_redir++) {
				fprintf(stderr, "%s: bad output redirection",
					argv_list[0]);
				for (j = 0; j < arg_count; j++) {
					free(argv_list[j]);
				}
				free_all(stages, cur_stage);
				return 4;
			}
			else {
				while (cmdlne[++i] == '\0' ||
					cmdlne[i] == ' ') {
					/* twiddle */
				}
				strcpy(out, cmdlne + i);
				for (j = 0; out[j] != '\0' &&
					out[j] != '<' && out[j] != '>'
					&& out[j] != '|'; j++) {
					/* twiddle */
				}
				if (out[j] == '<' || out[j] == '>' ||
					out[j] == '|') {
					fprintf(stderr,
						"%s: output redirection\n",
						argv_list[0]);
					for (j = 0;j < arg_count;j++) {
						free(argv_list[j]);
					}
					free_all(stages, cur_stage);
					return 4;
				}
				out[j] = '\0';
				i += strlen(out);
			}
		}
		else if (cmdlne[i] == '\0' || cmdlne[i] == ' ') {
			i++;
		}
		else {
			if (arg_count == ARGS_MAX) {
				fprintf(stderr, "maximum pipeline argument"
					        "length exceeded\n");
				for (j = 0; j < arg_count; j++) {
					free(argv_list[j]);
				}
				return 7;
			}
			while (cmdlne[i] == '\0') {
				i++;
			}
			strcpy(cur_arg, cmdlne + i);
			for (j = 0; cur_arg[j] != '\0' && cur_arg[j] != '<' &&
				cur_arg[j] != '>' && cur_arg[j] != '|' &&
				cur_arg[j] != ' '; j++) {
				/* twiddle */
			}
			cur_arg[j] = '\0';
			argv_list[arg_count] = (char*)malloc(sizeof(char)
				* (strlen(cur_arg) + 1));
			strcpy(argv_list[arg_count], cur_arg);
			i += strlen(cur_arg);
			arg_count++;
		}
	}
	print_stage(stages, cur_stage);
	free(stages);
	return 0;
}

void free_all(struct stage *stages, int num_stages) {
	int i, j;
	for (i = 0; i < num_stages; i++ ) {
		free(stages[i].pipeline);
		free(stages[i].in);
		free(stages[i].out);
		for (j = 0; j < stages[i].argc; j++) {
			free(stages[i].argv_list[j]);
		}
	}
	free(stages);
}

void print_stage(struct stage *stages, int num_stages) {
	int i, j;
	for (i = 0; i < num_stages; i++) {
		printf("\n--------\n"
		       "Stage %d: \"%s\"\n"
       	               "--------\n", stages[i].stage, stages[i].pipeline);
		printf("     input: %s\n", stages[i].in);
		printf("    output: %s\n", stages[i].out);
		printf("      argc: %d\n", stages[i].argc);
		printf("      argv: ");
		free(stages[i].pipeline);
		free(stages[i].in);
		free(stages[i].out);
		for (j = 0; j < stages[i].argc; j++) {
			printf("\"%s\"", stages[i].argv_list[j]);
			free(stages[i].argv_list[j]);
			if (j != stages[i].argc - 1) {
				printf(",");
			}
			else {
				printf("\n");
			}
		}
	}	
}
