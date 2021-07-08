# LibRaw Wrapper (C++/CLI LibRaw)

### .NET Framework assembly for reading and processing RAW images

LibRawWrapper is a single-file C++/CLI assembly containing the [LibRaw](https://github.com/libraw/libraw) library for reading RAW files from digital photo cameras (CRW/CR2, NEF, RAF, DNG, MOS, KDC, DCR, etc, virtually all RAW formats are supported).

It pays special attention to correct retrieval of data required for subsequent RAW conversion.

The library is intended for embedding in RAW converters, data analyzers, and other programs using RAW files as the initial data.

**Documentation for LibRaw can be found at [libraw.org](https://libraw.org/docs)** and it is included in the wrapper using XML documentation. (The original dcraw manual is available [here](https://www.dechifro.org/dcraw/dcraw.1.html).)

## Usage

👉 _If you are looking for a way to show RAW images to users for informational purposes, just get the LibRaw-based WIC codec in the form of [Raw Image Extension](https://www.microsoft.com/store/apps/9nctdw2w1bh8) by Microsoft. It is the same code outputting 8-bit images with default settings, enabling both RAW file thumbnails and previews in Windows Explorer as well as reading images in .NET using [`BitmapDecoder.Create`](https://docs.microsoft.com/en-us/dotnet/api/system.windows.media.imaging.bitmapdecoder.create)._

This LibRaw Wrapper is aiming at image processing pipeline, giving access to 32-bit floating or 16-bit integer data. By default, it produces `PixelFormats.Rgb128` bitmap with fixed white level and linear gamma, corresponding to `-4` dcraw parameter.

⚠️ Since this is mixed native/managed assembly, you need to ensure your application architecture matches the architecture of the LibRaw Wrapper assembly. Most likely, if you are targeting 64-bit platforms and your project configuration is _Any CPU_, you might need to go to project settings > _Build_ and uncheck _Prefer 32-bit_.

### Higher-level API

The high level API is inspired by the [`BitmapDecoder`](https://docs.microsoft.com/en-us/dotnet/api/system.windows.media.imaging.bitmapdecoder) API (the API currently allows extending only by writing a WIC codec):

This example converts raw files passed via command-line into JPEG files:

```C#
using System.IO;
using System.Windows.Media.Imaging;
using LibRawWrapper;

class Program
{
    static void Main(string[] args!!)
    {
        foreach (string filename in args)
        {
            LibRawBitmapDecoder raw = new LibRawBitmapDecoder(new Uri(Path.GetFullPath(filename)),
                                                              BitmapCreateOptions.PreservePixelFormat,
                                                              BitmapCacheOption.None);
            BitmapFrame rawFrame = raw.Frames[0];

            JpegBitmapEncoder jpeg = new JpegBitmapEncoder();
            jpeg.Frames.Add(rawFrame);
            using (FileStream stream = File.Create(Path.ChangeExtension(filename, ".jpg"))
                jpeg.Save(stream);
        }
    }
}
```

Currently the high-level API does not include metadata or color context in the bitmap frames. The resulting frames are as follows:

| `BitmapCreateOptions`:    | `None`        | `IgnoreColorProfile` | `PreservePixelFormat` |
|---------------------------|---------------|----------------------|-----------------------|
| `BitmapFrame.PixelFormat` | `Rgb128Float` | `Rgb128Float`        | `Rgb48`               |
| gamma                     | linear        | linear               | sRGB                  |
| equivalent dcraw settings | `-4`          | `-D -4`              | `-6 -W -g 2.4 12.92`  |

The `BitmapFrame` can be directly shown in the user interface or encoded into an image file as shown above.

### Lower-level API

The low level API directly corresponds to the native LibRaw API.

```C#
using LibRawWrapper.Native;

class Program
{
    static unsafe void Main()
    {
        // Let us create an image processor
        LibRawProcessor iProcessor = new LibRawProcessor();

        // Subscribe to processing notifications
        iProcessor.ProgressChanged += OnProgressChanged;

        // Open the file and read the metadata
        iProcessor.Open(@"C:\tmp\00_0011.CR2");

        // The metadata are accessible through data fields of the class
        Console.WriteLine($"Image size: {iProcessor.Sizes.Width} × {iProcessor.Sizes.Width}");

        // iProcessor.GetProcessedBitmap() calls unpack() and dcraw_process() if needed and
        // returns BitmapSource of either Gray8, Gray16, Gray32Float, Rgb24, Rgb48 or Rgb128Float
        // pixel format with the appropriate gamma (i.e. linear or sRGB).
        // To enforce your gamma regardless of pixel format contract, call DcrawProcess() manually first.

        // Alternatively, access the data manually:

        // Let us unpack the image
        iProcessor.Unpack();

        // Process the image using dcraw settings
        iProcessor.DcrawProcess();

        // Get processed image dimensions
        iProcessor.GetMemoryImageFormat(out int width, out int height, out int channels, out int bpp);
            
        // Get the processed image data
        byte[] data = new byte[width * height * channels];
        fixed (byte* pData = data)
            iProcessor.CopyMemoryImage(pData, width * channels, 0 /* RGB */);

        // And let us print its dump
        for (int i = 0; i < data.Length; i += channels)
            Console.WriteLine($"i={i}, R={data[i]}, G={data[i + 1]}, B={data[i + 2]}");

        // Finally, let us free the image processor for work with the next image
        iProcessor.Recycle();
    }

    static void OnProgressChanged(object sender, LibRawProgressEventArgs e)
    {
        Console.WriteLine($"{e.Description} ({e.Iteration + 1}/{e.Expected})");
    }
}
```

## API coverage

Most of the LibRaw methods are available. The following APIs are NOT implemented:

#### Metadata
* `libraw_makernotes_t`

There is currently no way to access the makernote metadata. This is on the roadmap, subject to demand.

#### File output:
* `int LibRaw::dcraw_ppm_tiff_writer(const char *outfile)`
* `int LibRaw::dcraw_thumb_writer(const char *thumbfile)`

For saving decoded images into TIFF, use [`TiffBitmapEncoder`](https://docs.microsoft.com/en-us/dotnet/api/system.windows.media.imaging.tiffbitmapencoder).

To access image thumbnail, use `LibRawProcessor.GetThumbnailBitmap` and encode as desired.

#### Rendering:
* `int LibRaw::raw2image`
* `void LibRaw::free_image`
* `libraw_processed_image_t *dcraw_make_mem_image(int *errorcode)`
* `libraw_processed_image_t *dcraw_make_mem_thumb(int *errorcode)`
* `void LibRaw::dcraw_clear_mem(libraw_processed_image_t *)`

Legacy LibRaw methods (`raw2image` and `free_image`) are not available. Access `LibRawProcessor.RawData.Buffer` directly.

For image thumbnail, use `LibRawProcessor.GetThumbnailBitmap` or access the `LibRawProcessor.Thumbnail.ThumbnailBuffer` directly.

`dcraw_make_mem_image`:
1. calls `get_mem_image_format`,
2. allocates memory,
3. calls `copy_mem_image`,
4. frees the allocated memory using `dcraw_clear_mem`.

It is expected that managed applications will want to allocate their own memory. The underlying APIs are still available if needed, see the lower-level API example above.

#### Events:
* User callback for exif/makernotes parser routines
* Out-of-Memory Notifier, `LIBRAW_OPTIONS_NO_MEMERR_CALLBACK`
* File Read Error Notifier, `LIBRAW_OPTIONS_NO_DATAERR_CALLBACK`

These are on the roadmap, subject to demand.

## License

The LibRaw Wrapper statically links the LibRaw library (in the _LibRaw_ submodule). The LibRaw library is Copyright © 2008-2021 LibRaw LLC (info@libraw.org) and includes source code from
```
dcraw.c, Dave Coffin's raw photo decoder
Copyright 1997-2018 by Dave Coffin, dcoffin a cybercom o net
```

LibRaw is distributed for free under two different licenses:

* GNU Lesser General Public License, version 2.1
* COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0

Any changes to the LibRaw library in this repository follow the same licensing model as requested.

The C++/CLI wrapper code itself is released under MIT license.

## Build

The solution builds in Visual Studio 2019 and 2022. The required components are:
* MSVC v142 - VS 2019 C++ x64/x86 build tools (latest)
* C++/CLI support for v142 build tools (x86 & x64)
* Optionally .NET desktop development tools including .NET Framework 4.8 (for the testing projects)
