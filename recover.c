// Recovers files from an SD card image file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t BYTE;

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover filename\n");
        return 1;
    }

    // get filename
    char *infile = argv[1];

    // open infile
    FILE *file = fopen(infile, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // begin counter for filenames
    int counter = 0;

    // array space for filenames
    char filename[8];

    // read infile's bytes
    BYTE buffer[512];
    while (fread(buffer, sizeof(buffer), 1, file) == 1)
    {
        // check first 4 bytes
        while (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0)
        {
            printf("Found JPEG.\n");
            // open file
            sprintf(filename, "%03i.jpg", counter);

            FILE *img = fopen(filename, "w");
            if (img == NULL)
            {
                fclose(img);
                fprintf(stderr, "Could not create image.\n");
                return 3;
            }

            // write to file
            fwrite(buffer, sizeof(buffer), 1, img);

            // continue to write to file if new jpeg not found
            while(fread(buffer, sizeof(buffer), 1, file) == 1)
            {
                if (!(buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0))
                {
                    fwrite(buffer, sizeof(buffer), 1, img);
                }
                else
                {
                    fclose(img);
                    counter++;
                    break;
                }
            }
        }
    }
}