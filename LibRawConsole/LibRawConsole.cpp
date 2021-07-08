// LibRawTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <libraw.h>

int main()
{
    // Let us create an image processor
    LibRaw iProcessor;
    iProcessor.imgdata.params.no_auto_bright = 1;
    // Open the file and read the metadata
    iProcessor.open_file(L"C:\\tmp\\00_0011.CR2");

    // The metadata are accessible through data fields of the class
    printf("Image size: %d x %d\n", iProcessor.imgdata.sizes.width, iProcessor.imgdata.sizes.height);

    // Let us unpack the image
    iProcessor.unpack();

    iProcessor.dcraw_process();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
