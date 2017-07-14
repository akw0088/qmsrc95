#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	char id[4]; //PACK
	int dir_offset;
	int dir_length;
} pak_header_t;

typedef struct
{
	unsigned char type[4]; //IWAD or PWAD
	int num_lump;
	int dir_offset;
} wad_header_t;

typedef struct
{
	char name[56];
	int offset;
	int length;
} pak_entry_t;

typedef struct
{
	int offset;
	int length;
	char name[8];
} wad_lump_t;


// Audio header
typedef struct
{
	unsigned short int format; // must be 3
	unsigned short sample_rate; // usually 11025, can be 22050
	unsigned int num_sample;
	unsigned char pad[16];
	unsigned char data;
} dmx_header_t;

char *get_file(char *filename, int *size)
{
	FILE	*file;
	char	*buffer;
	int	file_size, bytes_read;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = malloc(file_size + 1);
	bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size)
	{
		free((void *)buffer);
		fclose(file);
		return 0;
	}
	fclose(file);
	buffer[file_size] = '\0';

	if (size != NULL)
	{
		*size = file_size;
	}

	return buffer;
}

char *get_wadfile(char *wadfile, char *lump, int *lump_size)
{
	char *data = NULL;
	char *lump_data = NULL;
	int size = 0;
	wad_header_t *header = NULL;

	data = get_file(wadfile, &size);
	if (data == NULL)
	{
		printf("Unable to open %s\n", wadfile);
		return NULL;
	}

	header = (wad_header_t *)data;

	if (memcmp((char *)header->type, "IWAD", 4) != 0)
	{
		//printf("IWAD %d lumps\n", header->num_lump);
	}
	else if (memcmp((char *)header->type, "PWAD", 4) != 0)
	{
		//printf("PWAD %d lumps\n", header->num_lump);
	}
	else
	{
		printf("Invalid WAD file\n");
		return NULL;
	}

	wad_lump_t *lump_table = (wad_lump_t *)&data[header->dir_offset];

	for (int i = 0; i < header->num_lump; i++)
	{
		char name[9];

		memcpy(name, lump_table[i].name, 8);
		name[8] = '\0';
		//printf("Lump %d: name %s size %d\n", i, name, lump_table[i].size);

		if (strstr(lump, name) != 0)
		{
			lump_data = &data[lump_table[i].offset];
			*lump_size = lump_table[i].length;
			break;
		}
	}

	return lump_data;
}

char *get_pakfile(char *pakfile, char *file)
{
	int i;

	FILE *pak = (FILE *)fopen(pakfile, "rb");
	if (pak == NULL)
	{
		printf("error opening %s\n", pakfile);
		return NULL;
	}

	pak_header_t header;
	fread(&header, sizeof(pak_header_t), 1, pak);

	if (strncmp(header.id, "PACK", 4) != 0)
	{
		printf("invalid header id\n");
		fclose(pak);
		return NULL;
	}

	if (header.dir_length % sizeof(pak_entry_t) != 0)
	{
		printf("invalid dir length\n");
		fclose(pak);
		return NULL;
	}

	pak_entry_t *entries = (pak_entry_t *) malloc(header.dir_length);
	if (entries == NULL)
	{
		perror("malloc failed");
		fclose(pak);
		return NULL;
	}

	fseek(pak, header.dir_offset, SEEK_SET);
	fread(entries, header.dir_length, 1, pak);

	pak_entry_t *entry = entries;
	int num_entries = header.dir_length / sizeof(pak_entry_t);
	for (i = 0; i < num_entries; ++i, ++entry)
	{
		//		printf("%d: %s (%d, %d)\n", i, entry->name, entry->offset, entry->length);

		if (strcmp(entry->name, file) == 0)
		{
			char *data = (char *) malloc(entry->length);
			if (data == NULL)
			{
				perror("malloc failed");
				return NULL;
			}

			fseek(pak, entry->offset, SEEK_SET);
			fread(data, entry->length, 1, pak);
			free((void *)entries);
			fclose(pak);
			return data;
		}
	}
	free((void *)entries);
	fclose(pak);
	return NULL;
}

