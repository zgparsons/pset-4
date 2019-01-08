// Resizes a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: resize factor infile outfile\n");
        return 1;
    }

    // remember factor & filenames
    int factor = atoi(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    if (factor < 0 || factor > 100)
    {
        printf("factor must be an integer between 1 & 100");
        return 2;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf, bfN;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    bfN = bf;

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi, biN;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    biN = bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // set new width and height dimensions
    biN.biWidth = bi.biWidth * factor;
    biN.biHeight = bi.biHeight * factor;

    // determine padding for scanlines in both files
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int outpadding = (4 - (biN.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // new image sizes
    biN.biSizeImage = ((sizeof(RGBTRIPLE) * biN.biWidth) + outpadding) * abs(biN.biHeight);
    bfN.bfSize = biN.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bfN, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&biN, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // declare an array for storing the entire row to be written
        RGBTRIPLE row[biN.biWidth];

        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to array by factor
            for (int k = 0; k < factor; k++)
            {
                row[k + (factor * j)] = triple;
            }
        }

        // write array to rows and add padding
        for (int l = 0; l < factor; l++)
        {
            fwrite(row, sizeof(row), 1, outptr);

            for (int m = 0; m < outpadding; m++)
            {
                fputc(0x00, outptr);
            }
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
