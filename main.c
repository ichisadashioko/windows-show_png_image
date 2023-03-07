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

    hdc = BeginPaint(hwnd, &ps);

    // draw black background
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, bgBrush);
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

int load_png_file_and_convert_to_bitmap(
    char *filepath)
{
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

    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("Error during read_image at %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++)
    {
        row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, row_pointers);
    fclose(fp);

    // convert the PNG image data to a bitmap
    // TODO
    for (int i = 0; i < height; i++)
    {
        png_byte *row = row_pointers[i];
        for (int j = 0; j < width; j++)
        {
            png_byte *pixel_value_pointer = &(row[j * 3]);
            // printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", j, i, pixel_value_pointer[0], pixel_value_pointer[1], pixel_value_pointer[2], pixel_value_pointer[3]);

            printf("(%d, %d, %d) ", pixel_value_pointer[0], pixel_value_pointer[1], pixel_value_pointer[2]);
            // TODO set pixel color
        }

        printf("\n");
    }

    // free memory
    for (int i = 0; i < height; i++)
    {
        free(row_pointers[i]);
    }
    free(row_pointers);

    return 0;
}

int main()
{
    char *image_filepath = "test_2x2_image.png";
    load_png_file_and_convert_to_bitmap(image_filepath);

    return wWinMain(           //
        GetModuleHandle(NULL), //
        NULL,                  //
        GetCommandLine(),      //
        SW_SHOWNORMAL          //
    );
};
