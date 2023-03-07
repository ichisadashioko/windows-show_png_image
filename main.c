#include <Windows.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <png.h>

#define VK_Q_KEY 0x51
#define QUIT_APPLICATION_KEY_CODE VK_Q_KEY

static TCHAR szWindowClass[] = _T("windows_class_name");
static TCHAR szTitle[] = _T("windows_title");

const unsigned char IS_IMAGE_TYPE_RGB = 0;
const unsigned char IS_IMAGE_TYPE_RGBA = 1;

struct IS_8BIT_IMAGE_STRUCT
{
    unsigned int width;
    unsigned int height;
    unsigned char type;
    unsigned char *data;
};

typedef struct IS_8BIT_IMAGE_STRUCT IS_8BIT_IMAGE;

IS_8BIT_IMAGE GLOBAL_IS_IMAGE = {
    .width = 0,  //
    .height = 0, //
    .type = 0,   //
    .data = NULL //
};
// BITMAP
void mHandleResizeMessage( //
    _In_ HWND hwnd,        //
    _In_ UINT uMsg,        //
    _In_ WPARAM wParam,    //
    _In_ LPARAM lParam     //
)
{
    InvalidateRect(hwnd, NULL, FALSE);
}

void mHandlePaintMessage( //
    _In_ HWND hwnd,       //
    _In_ UINT uMsg,       //
    _In_ WPARAM wParam,   //
    _In_ LPARAM lParam    //
)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);

    if (rc.bottom == 0)
    {
        return;
    }

    // TODO learn about all these kinds of HDCs
    hdc = BeginPaint(hwnd, &ps);

    // HDC screen_HDC = GetDC(NULL);
    // HDC window_HDC = GetDC(hwnd);
    // HDC memory_HDC = CreateCompatibleDC(window_HDC);

    // DeleteObject(memory_HDC);
    // ReleaseDC(NULL, screen_HDC);
    // ReleaseDC(hwnd, window_HDC);

    // draw black background
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, bgBrush);

    // validate GLOBAL_IS_IMAGE
    if (GLOBAL_IS_IMAGE.data == 0)
    {
        printf("GLOBAL_IS_IMAGE.data is NULL\n");
    }
    else
    {
        HBITMAP bitmap_handle = CreateCompatibleBitmap(hdc, GLOBAL_IS_IMAGE.width, GLOBAL_IS_IMAGE.height);
        HDC bitmap_hdc = CreateCompatibleDC(hdc);
        SelectObject(bitmap_hdc, bitmap_handle);

        // modify the pixel values
        for (unsigned int y = 0; y < GLOBAL_IS_IMAGE.height; y++)
        {
            for (unsigned int x = 0; x < GLOBAL_IS_IMAGE.width; x++)
            {
                unsigned int index = (y * GLOBAL_IS_IMAGE.width + x) * 3;
                unsigned char r = GLOBAL_IS_IMAGE.data[index + 0];
                unsigned char g = GLOBAL_IS_IMAGE.data[index + 1];
                unsigned char b = GLOBAL_IS_IMAGE.data[index + 2];

                SetPixel(bitmap_hdc, x, y, RGB(r, g, b));
            }
        }

        // draw the bitmap
        BitBlt(hdc, 0, 0, GLOBAL_IS_IMAGE.width, GLOBAL_IS_IMAGE.height, bitmap_hdc, 0, 0, SRCCOPY);
        // clean up
        DeleteObject(bitmap_handle);
        DeleteDC(bitmap_hdc);
    }

    DeleteObject(bgBrush);

    EndPaint(hwnd, &ps);
}

void mHandleKeyUpMessage( //
    _In_ HWND hwnd,       //
    _In_ UINT uMsg,       //
    _In_ WPARAM wParam,   //
    _In_ LPARAM lParam    //
)
{
    int virtual_key_code = (int)wParam;
    if (virtual_key_code == QUIT_APPLICATION_KEY_CODE)
    {
        PostQuitMessage(0);
    }
}

LRESULT CALLBACK WndProc( //
    _In_ HWND hwnd,       //
    _In_ UINT uMsg,       //
    _In_ WPARAM wParam,   //
    _In_ LPARAM lParam    //
)
{
    if (uMsg == WM_PAINT)
    {
        mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_SIZE)
    {
        mHandleResizeMessage(hwnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_KEYUP)
    {
        mHandleKeyUpMessage(hwnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int CALLBACK wWinMain(                //
    _In_ HINSTANCE hInstance,         //
    _In_opt_ HINSTANCE hPrevInstance, //
    _In_ LPWSTR lpCmdLine,            //
    _In_ int nCmdShow                 //
)
{
    // default window width and height values
    int nWidth = 720;
    int nHeight = 480;

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(0);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        printf("RegisterClassEx failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    HWND hwnd = CreateWindow(                //
        szWindowClass,                       //
        szTitle,                             //
        WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, //
        CW_USEDEFAULT,                       //
        CW_USEDEFAULT,                       //
        nWidth,                              //
        nHeight,                             //
        NULL,                                //
        NULL,                                //
        hInstance,                           //
        NULL                                 //
    );

    if (!hwnd)
    {
        printf("CreateWindow failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    // make the window transparent
    // SetWindowLong(                                       //
    //     hwnd,                                            //
    //     GWL_EXSTYLE,                                     //
    //     GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED //
    // );
    // SetLayeredWindowAttributes(  //
    //     hwnd,                    //
    //     RGB(0, 0, 0),            //
    //     255,                     //
    //     LWA_ALPHA | LWA_COLORKEY //
    // );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// TODO write a function to decode PNG bytes
unsigned char PNG_MAGIC_BYTES[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

int decode_png_bytes(   //
    unsigned char *bs,  //
    unsigned int length //
)
{
    unsigned int current_index = 0;
    // magic bytes
    if (length < 8)
    {
        printf("decode_png_bytes failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    for (int i = 0; i < 8; i++)
    {
        if (bs[current_index + i] != PNG_MAGIC_BYTES[i])
        {
            printf("Invalid PNG file\ndecode_png_bytes failed at %s:%d\n", __FILE__, __LINE__);
            return -1;
        }
    }

    current_index += 8;

    return 0;
}

int load_png_file_and_convert_to_IS_IMAGE( //
    char *filepath,                        //
    IS_8BIT_IMAGE *is_image_pointer        //
)
{
    int retval = 0;
    FILE *fp = fopen(filepath, "rb");
    if (!fp)
    {
        printf("fopen failed at %s:%d\nUnable to open file %s\n", __FILE__, __LINE__, filepath);
        return -1;
    }

    // use libpng to decode the PNG file
    png_structp png_ptr = png_create_read_struct( //
        PNG_LIBPNG_VER_STRING,                    //
        NULL,                                     //
        NULL,                                     //
        NULL                                      //
    );

    if (!png_ptr)
    {
        printf("png_create_read_struct failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        printf("png_create_info_struct failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("Error during init_io at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("color_type: %d\n", color_type);
    printf("bit_depth: %d\n", bit_depth);

    if (bit_depth != 8)
    {
        printf("unsupported bit depth %d\n", bit_depth);
        retval = -1;
    }
    else if (color_type != PNG_COLOR_TYPE_RGB)
    {
        printf("unsupported color type %d\n", color_type);
        retval = -1;
    }
    else
    {
        int number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        if (setjmp(png_jmpbuf(png_ptr)))
        {
            printf("Error during read_image at %s:%d\n", __FILE__, __LINE__);
            return -1;
        }

        png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
        if (!row_pointers)
        {
            printf("malloc failed at %s:%d\n", __FILE__, __LINE__);
            retval = -1;
        }
        else
        {
            for (int y = 0; y < height; y++)
            {
                row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));
            }

            png_read_image(png_ptr, row_pointers);

            // IS_8BIT_IMAGE is_image0 = {0};

            is_image_pointer->width = (unsigned int)width;
            is_image_pointer->height = (unsigned int)height;
            is_image_pointer->type = IS_IMAGE_TYPE_RGB;
            unsigned int data_length = (unsigned int)(width * height * 3);
            is_image_pointer->data = (unsigned char *)malloc(data_length);
            if (!is_image_pointer->data)
            {
                printf("malloc failed at %s:%d\n", __FILE__, __LINE__);
                retval = -1;
            }
            else
            {
                // convert the PNG image data to a bitmap
                // HBITMAP hbitmap0 = CreateBitmap( //
                //     width,                       // [in] int nWidth
                //     height,                      // [in] int nHeight
                //     1,                           // [in] UINT cPlanes
                //     24,                           // [in] UINT cBitsPerPel
                // );
                // TODO
                for (int i = 0; i < height; i++)
                {
                    png_byte *row = row_pointers[i];
                    for (int j = 0; j < width; j++)
                    {
                        png_byte *pixel_value_pointer = &(row[j * 3]);
                        // printf("(%d, %d, %d) ", pixel_value_pointer[0], pixel_value_pointer[1], pixel_value_pointer[2]);
                        // TODO set pixel color
                        int is_image_data_index = (i * width * 3) + (j * 3);
                        is_image_pointer->data[is_image_data_index + 0] = pixel_value_pointer[0];
                        is_image_pointer->data[is_image_data_index + 1] = pixel_value_pointer[1];
                        is_image_pointer->data[is_image_data_index + 2] = pixel_value_pointer[2];
                    }

                    // printf("\n");
                }
            }

            // free memory
            for (int i = 0; i < height; i++)
            {
                free(row_pointers[i]);
            }
            free(row_pointers);
        }
    }

    fclose(fp);

    return retval;
}

int main()
{
    // char *image_filepath = "test_2x2_image.png";
    char *image_filepath = "input2.png";
    IS_8BIT_IMAGE is_image0 = {0};
    int retval = load_png_file_and_convert_to_IS_IMAGE( //
        image_filepath,                                 //
        &is_image0                                      //
    );
    if (retval != 0)
    {
        printf("load_png_file_and_convert_to_IS_IMAGE failed at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    if (is_image0.data)
    {
        printf("is_image0.data: %p\n", is_image0.data);
        printf("is_image0.width: %d\n", is_image0.width);
        printf("is_image0.height: %d\n", is_image0.height);
        printf("is_image0.type: %d\n", is_image0.type);
    }
    else
    {
        printf("is_image0.data is NULL\n");
        return -1;
    }

    GLOBAL_IS_IMAGE.width = is_image0.width;
    GLOBAL_IS_IMAGE.height = is_image0.height;
    GLOBAL_IS_IMAGE.type = is_image0.type;
    GLOBAL_IS_IMAGE.data = is_image0.data;

    retval = wWinMain(         //
        GetModuleHandle(NULL), //
        NULL,                  //
        GetCommandLine(),      //
        SW_SHOWNORMAL          //
    );

    free(is_image0.data);

    return retval;
};
