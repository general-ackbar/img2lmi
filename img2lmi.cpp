#define cimg_display 0
#define cimg_use_jpeg

#include "CImg.h"
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


using namespace std;
using namespace cimg_library;

#define VERSION 1

enum ColorSpace
{
    rgb888 = 24,
	rgba8888 = 32,
    rgb565 = 16,
	rgb332 = 8,
    binary = 1,
    binary_vertical_mode = 0
};


void convertImage(CImg<unsigned char> image, ColorSpace bitsPerColor, string output_path, bool append, int fps, bool invert);
void saveImage(CImg<unsigned char> image);
void alterFps(string file, int fps);
void ditherFS(CImg<unsigned char> &image);
void ditherSierra(CImg<unsigned char> &image);


bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}


int main(int argc, char *argv[]) {

    string input_path; //(argv[1]);    
    string output_path;
    int fps = 0;
    bool quiet = false;
    bool append = false;
    bool invert_1bit = false;
    bool dither_1bit = true;
    enum ColorSpace bitsPerColor = rgb565;

	
	
    char c;
    while ((c = getopt (argc, argv, "i:o:r:b:aqlxn")) != -1)
    {
        switch (c)
        {
            case 'i':
                input_path = string(optarg);
                output_path = input_path.substr(0, input_path.find_last_of(".")) + ".lmi";
                break;
            case 'o':
                output_path = string(optarg);
                break;
            case 'r':            
                fps = atoi(optarg);
                break;
            case 'b':
                bitsPerColor = ColorSpace(atoi(optarg));
                break;
			case 'a':
				append = true;
				break;
			case 'q':
				quiet = true;
				break;
            case 'x':
                invert_1bit = true;
                break;
            case 'n':
                dither_1bit = false;
                break;
            case '?':
                if (optopt == 'i' || optopt == 'o' || optopt == 'r')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }




    if(!quiet)
        fprintf(stdout, "img2lmi converter Version %d.%d, created by Codeninja.\n", VERSION>>8,VERSION&255);

    if(has_suffix(input_path, "lmi") && fps > 0)
    {
        alterFps(input_path, fps);
        printf("Framerate for %s changed to %i\n", input_path.c_str(), fps);
        return 0;
    }

    if(!quiet)
        printf("Output: %s\n", output_path.c_str());
    
    CImg<unsigned char> input;
    

    if(has_suffix(input_path, "jpg") || has_suffix(input_path, "jpeg") || has_suffix(input_path, "png"))
    {
        input = CImg<>(input_path.c_str());
    } 
    
    if(!input)
    {   
        printf("Not a valid image:  %s\n", input_path.c_str());
        return 0;
    } else 
        if(!quiet)
            printf("Loaded %s \n", input_path.c_str());

     
    if(!quiet)
        printf("Colorspace: %i\n", bitsPerColor );


    if(bitsPerColor <= 1)
    {
        // Convert to grayscale
        if (input.spectrum() == 3) 
            input.RGBtoYCbCr().channel(0);
                
        if(dither_1bit)
            ditherSierra(input);        
    }


    convertImage(input, bitsPerColor, output_path, append, fps, invert_1bit);

}

void alterFps(string file, int fps)
{
    FILE * pFile;
    uint8_t c = (uint8_t)fps;

    pFile = fopen(file.c_str(), "r+b");

    if (pFile != NULL) {
        fseek(pFile, 9, SEEK_SET);
        fputc(c, pFile);
        fclose(pFile);
    }
}

void convertImage(CImg<unsigned char> image, ColorSpace bitsPerColor, string output_path, bool append, int fps=0, bool invert_1bit=false)
{
    int hFile;
    
    //If append and the file exists. Open it and append image data only
    if(append && access(output_path.c_str(), F_OK) == 0)
    {
        hFile = open(output_path.c_str(), O_APPEND | O_RDWR, 0666);        
    }
    else    //If append not specified or the file doesn't exist. Create new file with header
    {
        hFile = open(output_path.c_str(), O_CREAT|O_RDWR, 0666);

        if(fps > 0){
            uint8_t header[10] = {'L', 'M', 'I', (uint8_t)(bitsPerColor & 0xFF), 0x0A, (uint8_t)((image.width() >> 8) & 0xFF), (uint8_t)(image.width() & 0xFF), (uint8_t)((image.height() >> 8) & 0xFF), (uint8_t)(image.height() & 0xFF), (uint8_t)fps};    
            write(hFile, &header, sizeof(header));
        } else {
            uint8_t header[9] = {'L', 'M', 'I', (uint8_t)(bitsPerColor & 0xFF), 0x09, (uint8_t)((image.width() >> 8) & 0xFF), (uint8_t)(image.width() & 0xFF), (uint8_t)((image.height() >> 8) & 0xFF), (uint8_t)(image.height() & 0xFF)};        
            write(hFile, &header, sizeof(header));            
        }        
    }
	
    bool isHorizontalByteOrder;
    if(bitsPerColor == binary || bitsPerColor == binary_vertical_mode)
    {
        isHorizontalByteOrder = bitsPerColor;
        bitsPerColor = binary;
    }

    printf("Input image has %i chanels\n", image.spectrum());
	for(int y = 0; y < image.height(); y++)
	{
		for(int x = 0; x < image.width(); x++)
		{

            uint8_t currentRedByte, currentBlueByte, currentGreenByte, currentAlphaByte = 0x00;
            uint16_t c_rgb565 = 0;
                
            //Get current color
            uint8_t color[image.spectrum()];
            for(int c = 0; c < image.spectrum(); c++)
            {
                color[c] = image.atXYZC(x, y, 1, c);
            }
            
            currentRedByte = color[0];
            currentGreenByte = color[1];
            currentBlueByte = color[2];
            if(image.spectrum() < 4)
                currentAlphaByte = 255;
            else
                currentAlphaByte = color[3];

            //grayscale so use the same value for all channels
            if(image.spectrum() == 1)
                currentRedByte = currentGreenByte = currentBlueByte = color[0];

            
            //Append byte to array
            if(bitsPerColor == rgb565)
            {
                //16-bit RGB values
                c_rgb565 = (currentRedByte & 0b11111000) << 8;
                c_rgb565 = c_rgb565 + ((currentGreenByte & 0b11111100) << 3);
                c_rgb565 = c_rgb565 + ((currentBlueByte) >> 3);
                c_rgb565 = (c_rgb565>>8) | ((c_rgb565 & 0xff) << 8); //Reverse order of bytes
                write(hFile, &c_rgb565, 2);
            }
            else if(bitsPerColor == rgba8888)
            {
                write(hFile, &currentRedByte, sizeof(currentRedByte));
                write(hFile, &currentGreenByte, sizeof(currentGreenByte));
                write(hFile, &currentBlueByte, sizeof(currentBlueByte));
                write(hFile, &currentAlphaByte, sizeof(currentAlphaByte));
            }
            else if(bitsPerColor == rgb888)
            {
                write(hFile, &currentRedByte, sizeof(currentRedByte));
                write(hFile, &currentGreenByte, sizeof(currentGreenByte));
                write(hFile, &currentBlueByte, sizeof(currentBlueByte));
            }
            else if(bitsPerColor == rgb332)
            {
                    
                uint8_t red = (currentRedByte * 8) / 255;
                uint8_t green = (currentGreenByte * 8) / 255;
                uint8_t blue = (currentBlueByte * 4) / 255;
                    
                uint8_t color332 = (red << 5) | (green << 2) | blue;
                write(hFile, &color332, sizeof(color332));
            }

            else if(bitsPerColor == binary)
            {
                unsigned char currentByte = 0x00;

                for(int bit = 0; bit < 8; bit++) //Traverse the image 8 y-values for each x value
                {
                    //Get current color in duo-tone
                    for(int c = 0; c < image.spectrum(); c++)
                    {
                        color[c] = image.atXYZC( (isHorizontalByteOrder ? x+bit : x), (isHorizontalByteOrder ? y : y+bit), 1, c);                        
                    }                    
                    float gray;
                    if(image.spectrum() > 1)
                        gray = roundf((0.2126 * color[0]) + (0.7152 * color[1]) + (0.0722 * color[2]));
                    else
                        gray = color[0];

                                                
                    //Build the byte value as binary (top pixel is the least significant bit)
                    if(gray > 127 && !invert_1bit)
                    {
                        //currentByte |=  (1 << (7-bit));
                        currentByte |= (isHorizontalByteOrder ? (1 << (7-bit)) : (1 << bit));
                    }
                    else if(gray < 127 && invert_1bit)
                    {
                        //currentByte |=  (1 << (7-bit));
                        currentByte |= (isHorizontalByteOrder ? (1 << (7-bit)) : (1 << bit));
                    }

                }
                
                if(isHorizontalByteOrder)
                    x+=7;
                else if(!isHorizontalByteOrder && x == image.width()-1)
                    y+=7;
                    
                //Append byte to array
                write(hFile, &currentByte, sizeof(currentByte));
            }
		}
	}
    close(hFile);
}

void saveImage(CImg<unsigned char> image)
{
    
    CImg<unsigned char> bitmap(image.width(), image.height(), 1, image.spectrum(), 0);

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0; x < image.width(); x++)
        {
            uint8_t color[image.spectrum()];

            for(int c = 0; c < image.spectrum(); c++)
            { 
                color[c] = image.atXYZC(x, y, 1, c);
            
            }
            bitmap.draw_point(x,y, color);
        }
    }

    bitmap.save("output.png");
}


void ditherFS(CImg<unsigned char>& img) {
    const int width = img.width();
    const int height = img.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            
            unsigned char actualColor = img(x, y);
            unsigned char matchedColor = (actualColor > 128) ? 255 : 0;
            img(x, y) = matchedColor;

            float error = (actualColor - matchedColor) / 16;

            if (x + 1 < width) img(x + 1, y) = CLAMP(img(x + 1, y) + error * 7, 0, 255);
            if (x - 1 >= 0 && y + 1 < height) img(x - 1, y + 1) = CLAMP(img(x-1,y+1) + error * 3, 0, 255);
            if (y + 1 < height) img(x, y + 1) = CLAMP(img(x, y + 1) + error * 5, 0, 255);
            if (x + 1 < width && y + 1 < height) img(x + 1, y + 1) = CLAMP(img(x + 1, y + 1) + error * 1, 0, 255);
        }
    }
}

void ditherSierra(CImg<unsigned char>& img) {
    int width = img.width();
    int height = img.height();

    // Iterate over each pixel in the image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int actualColor = img(x, y);
            int matchedColor = actualColor < 128 ? 0 : 255;
            img(x, y) = matchedColor;
            float error = (actualColor - matchedColor) / 4;

            if (x + 1 < width)
                img(x + 1, y) = CLAMP(img(x + 1, y) + error * 2, 0, 255);
            if (y + 1 < height) {
                if (x - 1 >= 0)
                    img(x - 1, y + 1) = CLAMP( img(x - 1, y + 1) + error * 1, 0, 255);
                img(x, y + 1) = CLAMP(img(x, y + 1) + error * 1, 0, 255);
            }
        }
    }
}