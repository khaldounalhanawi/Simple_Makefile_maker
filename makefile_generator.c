#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_FILES 100
#define MAX_FILENAME 256
#define MAX_NAME_LENGTH 256

typedef struct {
	char **files;
	int count;
} FileList;

int ends_with(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);
	if (suffix_len > str_len)
		return 0;
	return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int compare_strings(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

FileList *scan_directory(const char *extension)
{
	DIR *dir;
	struct dirent *entry;
	FileList *list;

	list = (FileList *)malloc(sizeof(FileList));
	if (!list)
		return NULL;
	
	list->files = (char **)malloc(sizeof(char *) * MAX_FILES);
	if (!list->files)
	{
		free(list);
		return NULL;
	}
	list->count = 0;

	dir = opendir(".");
	if (!dir)
	{
		perror("opendir");
		free(list->files);
		free(list);
		return NULL;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (ends_with(entry->d_name, extension))
		{
			if (list->count >= MAX_FILES)
			{
				fprintf(stderr, "Warning: Too many files, skipping some.\n");
				break;
			}
			list->files[list->count] = (char *)malloc(strlen(entry->d_name) + 1);
			if (!list->files[list->count])
			{
				perror("malloc");
				closedir(dir);
				return list;
			}
			strcpy(list->files[list->count], entry->d_name);
			list->count++;
		}
	}

	closedir(dir);

	if (list->count > 0)
		qsort(list->files, list->count, sizeof(char *), compare_strings);

	return list;
}

void free_file_list(FileList *list)
{
	int i;

	if (!list)
		return;
	
	for (i = 0; i < list->count; i++)
	{
		free(list->files[i]);
	}
	free(list->files);
	free(list);
}

void write_file_list(FILE *fp, const char *variable_name, FileList *files)
{
	int i;

	if (!files || files->count == 0)
	{
		fprintf(fp, "%s =\n", variable_name);
		return;
	}

	fprintf(fp, "%s =", variable_name);
	for (i = 0; i < files->count; i++)
	{
		if (i == 0)
		{
			fprintf(fp, " %s", files->files[i]);
		}
		else
		{
			fprintf(fp, " \\\n\t\t%s", files->files[i]);
		}
	}
	fprintf(fp, "\n");
}

int main(void)
{
	FileList *headers;
	FileList *sources;
	char project_name[MAX_NAME_LENGTH];
	FILE *makefile;

	printf("Makefile Generator\n");
	printf("==================\n\n");

	printf("Enter project name: ");
	if (fgets(project_name, sizeof(project_name), stdin) == NULL)
	{
		fprintf(stderr, "Error reading project name.\n");
		return 1;
	}

	// Remove trailing newline
	project_name[strcspn(project_name, "\n")] = '\0';

	// Validate project name
	if (strlen(project_name) == 0)
	{
		fprintf(stderr, "Error: Project name cannot be empty.\n");
		return 1;
	}

	printf("\nScanning directory for files...\n");

	headers = scan_directory(".hpp");
	sources = scan_directory(".cpp");

	if (!headers || !sources)
	{
		fprintf(stderr, "Error: Memory allocation failed.\n");
		if (headers)
			free_file_list(headers);
		if (sources)
			free_file_list(sources);
		return 1;
	}

	printf("Found %d header file(s) and %d source file(s).\n", headers->count, sources->count);

	makefile = fopen("Makefile", "w");
	if (!makefile)
	{
		perror("fopen");
		free_file_list(headers);
		free_file_list(sources);
		return 1;
	}

	fprintf(makefile, "NAME = %s\n", project_name);
	fprintf(makefile, "CC = c++\n");
	write_file_list(makefile, "HEADER", headers);
	fprintf(makefile, "FLAGS = -Wall -Wextra -Werror -std=c++98\n");
	write_file_list(makefile, "SRCS", sources);
	fprintf(makefile, "\n");
	fprintf(makefile, "OBJS = $(SRCS:.cpp=.o)\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "all: $(NAME)\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "$(NAME): $(OBJS)\n");
	fprintf(makefile, "\t$(CC) $(FLAGS) $(OBJS) -o $(NAME)\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "%%.o: %%.cpp $(HEADER)\n");
	fprintf(makefile, "\t$(CC) $(FLAGS) -c $< -o $@\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "clean:\n");
	fprintf(makefile, "\trm -f $(OBJS)\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "fclean: clean\n");
	fprintf(makefile, "\trm -f $(NAME)\n");
	fprintf(makefile, "\n");
	fprintf(makefile, "re: fclean all\n");
	fprintf(makefile, "\n");
	fprintf(makefile, ".PHONY: all clean fclean re\n");

	fclose(makefile);

	printf("Makefile generated successfully!\n");

	free_file_list(headers);
	free_file_list(sources);

	return 0;
}
