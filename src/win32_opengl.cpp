#include <iostream>
// #define _ALLOW_KEYWORD_MACROS

#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "winmm")
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h> // @note excluded in lean and mean, used for timeBeginPeriod to set scheduler granularity

#pragma comment(lib, "opengl32")
#include <gl/gl.h>
// #pragma comment(lib, "glu32")
// #include <gl/glu.h>


#include "iml_general.h"
#include "iml_types.h"


#define WIDTH  800
#define HEIGHT  600
#define WINDOW_WIDTH  WIDTH
#define WINDOW_HEIGHT  HEIGHT


struct Win32_Offscreen_Buffer {
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct Win32_Window_Dimension {
    int width;
    int height;
};


global b32 global_running;
#if BUILD_INTERNAL
global b32 global_pause;
#endif
global Win32_Offscreen_Buffer global_backbuffer;
global s64 global_performance_count_frequency;

global GLuint global_blit_texture_handle = 0;


global int x_offset = 0;
global int y_offset = 0;


internal void
win32_init_opengl(HWND window) {
    HDC window_dc = GetDC(window);
    defer { ReleaseDC(window, window_dc); };
    
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize      = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits =  8;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
    
    int suggested_pixel_format_index = ChoosePixelFormat(window_dc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR suggested_pixel_format;
    DescribePixelFormat(window_dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format);
    SetPixelFormat(window_dc, suggested_pixel_format_index, &suggested_pixel_format);
    
    HGLRC opengl_rc = wglCreateContext(window_dc);
    if (!wglMakeCurrent(window_dc, opengl_rc)) {
        assert(false);
        return;
    }
    
    glGenTextures(1, &global_blit_texture_handle);
}

internal void
win32_resize_dib_section(Win32_Offscreen_Buffer *buffer, int width, int height) {
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
    
    buffer->width  = width;
    buffer->height = height;
    
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;
    
    int bytes_per_pixel = 4;
    int bitmap_memory_size = (buffer->width * buffer->height) * bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * bytes_per_pixel;
    buffer->bytes_per_pixel = bytes_per_pixel;
}

internal Win32_Window_Dimension
win32_get_window_dimension(HWND window) {
    Win32_Window_Dimension result;
    
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    
    return result;
}

internal void
render_weird_gradient(Win32_Offscreen_Buffer *buffer, int blue_offset, int green_offset) {
    // @TODO Let's see what the optimizer does
    
    u8 *row = (u8 *)buffer->memory;
    for (int y = 0; y < buffer->height; ++y) {
        u32 *pixel = (u32 *)row;
        for (int x = 0; x < buffer->width; ++x) {
            /*                  0  1  2  3
                                pixel in memory: 00 00 00 00
                                LITTLE ENDIAN ARCHITECTURE
                                pixel in memory:      BB GG RR xx
                                pixel in Register: 0x xx RR GG BB
            */
            u8 blue = (u8)(x + blue_offset);
            u8 green = (u8)(y + green_offset);
            
            *pixel++ = ((green << 16) | blue);
        }
        
        row += buffer->pitch;
    }
}

internal void
win32_display_buffer_in_window(Win32_Offscreen_Buffer *buffer, HDC device_context, int window_width, int window_height) {
#if 0
    // @todo better scaling, centering, black bars, ...
    
#if 0
    StretchDIBits(device_context,
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
    
#else
    
    int buffer_width = WIDTH;
    int buffer_height = HEIGHT;
    
    f32 height_scale = (f32)window_height / (f32)buffer_height;
    int new_width = (int)((f32)buffer_width * height_scale);
    int offset_x = (window_width - new_width) / 2;
    if (new_width <= buffer_width)  {
        offset_x = 0;
        new_width = window_width;
    }
    else {
        PatBlt(device_context, 0, 0, offset_x-2, window_height, BLACKNESS);
        PatBlt(device_context, offset_x-2, 0, offset_x, window_height,  WHITENESS);
        PatBlt(device_context, offset_x+new_width, 0, offset_x+buffer_width+2, window_height, WHITENESS);
        PatBlt(device_context, offset_x+new_width+2, 0, window_width, window_height, BLACKNESS);
    }
    
    StretchDIBits(device_context,
                  offset_x, 0, new_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
#endif
    
#else
    glViewport(0, 0, window_width, window_height);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glBindTexture(GL_TEXTURE_2D, global_blit_texture_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer->width, buffer->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, buffer->memory);
    
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glBegin(GL_TRIANGLES);
    f32 p = 1.f;
    
    // @note Upper triangle
    glTexCoord2f(0.f, 0.f);
    glVertex2f(-p, -p);
    
    glTexCoord2f(1.f, 1.f);
    glVertex2f(p, p);
    
    glTexCoord2f(0.f, 1.f);
    glVertex2f(-p, p);
    
    // @note Lower triangle
    glTexCoord2f(0.f, 0.f);
    glVertex2f(-p, -p);
    
    glTexCoord2f(1.f, 0.f);
    glVertex2f(p, -p);
    
    glTexCoord2f(1.f, 1.f);
    glVertex2f(p, p);
    
    glEnd();
    
#if 0
    glBegin(GL_TRIANGLES);
    f32 p = 0.9f;
    // @note Upper triangle
    glColor3f(1.f, 0.f, 0.f);
    glVertex2f(-p, -p);
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(p, p);
    glColor3f(0.f, 0.f, 1.f);
    glVertex2f(-p, p);
    
    // @note Lower triangle
    glColor3f(1.f, 1.f, 1.f);
    glVertex2f(-p, -p);
    glVertex2f(p, -p);
    glVertex2f(p, p);
    glEnd();
#endif
    
    SwapBuffers(device_context);
#endif
}

LRESULT CALLBACK
win32_main_window_callback(HWND window,
                           UINT message,
                           WPARAM wparam,
                           LPARAM lparam) {
    LRESULT result = 0;
    
    switch (message) {
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        case WM_CLOSE: {
            global_running = false;
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width  = paint.rcPaint.right  - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            Win32_Window_Dimension dimension = win32_get_window_dimension(window);
            win32_display_buffer_in_window(&global_backbuffer, device_context, dimension.width, dimension.height);
            EndPaint(window, &paint);
        } break;
        
        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }
    
    return result;
}

inline LARGE_INTEGER
win32_get_wall_clock() {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter;
}

inline f32
win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    f32 result = ((f32)(end.QuadPart - start.QuadPart) / (f32)global_performance_count_frequency);
    return result;
}


int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR     cmd_line,
        int       show_cmd) {
    LARGE_INTEGER performance_count_frequency_result;
    QueryPerformanceFrequency(&performance_count_frequency_result);
    global_performance_count_frequency = performance_count_frequency_result.QuadPart;
    
    UINT desired_scheduler_ms = 1;
    b32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
    
    win32_resize_dib_section(&global_backbuffer, WIDTH, HEIGHT);
    // win32_resize_dib_section(&global_backbuffer, 1920, 1080);
    
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = win32_main_window_callback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "scrollyWindowClass";
    
    if (!RegisterClassA(&window_class))  {
        // @todo
    }
    HWND window = CreateWindowExA(0, window_class.lpszClassName,
                                  "scrolly",
                                  WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  0, 0, instance, 0);
    if (!window)  {
        // @todo
    }
    
    win32_init_opengl(window);
    
    int monitor_refresh_hz = 60;
    HDC refresh_dc = GetDC(window);
    int win32_refresh_rate = GetDeviceCaps(refresh_dc, VREFRESH);
    ReleaseDC(window, refresh_dc);
    if (win32_refresh_rate > 1)  {
        monitor_refresh_hz = win32_refresh_rate;
    }
    f32 game_update_hz = (monitor_refresh_hz / 2.0f);
    f32 target_seconds_per_frame =  1.0f / (f32)game_update_hz;
    f32 dt = target_seconds_per_frame;
    
    
    global_running = true;
    
    LARGE_INTEGER last_counter = win32_get_wall_clock();
    QueryPerformanceCounter(&last_counter);
    u64 last_cycle_count = __rdtsc();
    while (global_running) {
        Win32_Window_Dimension dimension = win32_get_window_dimension(window);
        
        MSG message;
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            switch (message.message) {
                case WM_QUIT: {
                    global_running = false;
                } break;
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    u32 vk_code = (u32)message.wParam;
                    
                    // @note Since we are comparing was_down to is_down,
                    // we MUST use == and != to convert these bit tests to actual 0 or 1 values.
                    b32 was_down = ((message.lParam & (1 << 30)) != 0);
                    b32 is_down = ((message.lParam & (1 << 31)) == 0);
                    if (is_down) {
                        if (vk_code == 'W') {
                            y_offset += 5;
                        }
                        else if (vk_code == 'A') {
                            x_offset -= 5;
                        }
                        else if (vk_code == 'S') {
                            y_offset -= 5;
                        }
                        else if (vk_code == 'D') {
                            x_offset += 5;
                        }
                    }
                    if (was_down != is_down) {
                        if (vk_code == 'W') {
                        }
                        else if (vk_code == 'A') {
                        }
                        else if (vk_code == 'S') {
                        }
                        else if (vk_code == 'D') {
                        }
                        else if (vk_code == 'Q') {
                        }
                        else if (vk_code == 'E') {
                        }
                        else if (vk_code == VK_UP) {
                        }
                        else if (vk_code == VK_LEFT) {
                        }
                        else if ((vk_code == VK_DOWN) || (vk_code == 'K')) {
                        }
                        else if ((vk_code == VK_RIGHT) || (vk_code == 'J')) {
                        }
                        else if (vk_code == VK_RETURN) {
                        }
                        else if (vk_code == VK_BACK) {
                        }
                        if (vk_code == VK_ESCAPE) {
                            global_running = false;
                        }
                        
#if BUILD_INTERNAL
                        else if (vk_code == 'P') {
                            if (is_down) {
                                // global_pause = !global_pause;
                            }
                        }
                        
                        if (is_down) {
                            b32 alt_key_was_down = (message.lParam & (1 << 29));
                            if ((vk_code == VK_F4) && alt_key_was_down) {
                                global_running = false;
                            }
                            if ((vk_code == VK_RETURN) && alt_key_was_down)  {
                                if (message.hwnd)  {
                                    // toggle_fullscreen(message.hwnd);
                                }
                            }
                        }
#endif
                    }
                } break;
                
                default: {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                } break;
            }
        }
        
        //
        // @note Update and render
        //
        render_weird_gradient(&global_backbuffer, x_offset, y_offset);
        
        //
        // @note frame rate
        //
        LARGE_INTEGER work_counter = win32_get_wall_clock();
        f32 seconds_elapsed_for_work = win32_get_seconds_elapsed(last_counter, work_counter);
        
        f32 seconds_elapsed_for_frame = seconds_elapsed_for_work;
        if (seconds_elapsed_for_frame < target_seconds_per_frame) {
            DWORD sleep_ms;
            if (sleep_is_granular) {
                sleep_ms = (DWORD)(1000 * (DWORD)(target_seconds_per_frame - seconds_elapsed_for_frame));
                if (sleep_ms > 0) {
                    Sleep(sleep_ms);
                }
            }
            
            f32 test_seconds_elapsed_for_frame = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());
            if (test_seconds_elapsed_for_frame > target_seconds_per_frame) {
                // @todo LOG MISSED SLEEP HERE
            }
            
            while (seconds_elapsed_for_frame < target_seconds_per_frame) {
                seconds_elapsed_for_frame = win32_get_seconds_elapsed(last_counter, win32_get_wall_clock());
            }
        }
        else {
            // @todo MISSED FRAME RATE!
            // @todo logging
        }
        
        LARGE_INTEGER end_counter = win32_get_wall_clock();
        f64 ms_per_frame = 1000.0f * win32_get_seconds_elapsed(last_counter, end_counter);
        last_counter = end_counter;
        
        //
        // @note display buffer
        //
        
        HDC device_context = GetDC(window);
        win32_display_buffer_in_window(&global_backbuffer, device_context,
                                       dimension.width, dimension.height);
        ReleaseDC(window, device_context);
        
        
        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed = end_cycle_count - last_cycle_count;
        last_cycle_count = end_cycle_count;
        
        f64 fps = 0; // @note not a relevant measurement (f64)global_performance_count_frequency / (f64)counter_elapsed;
        f64 mcpf = (f64)cycles_elapsed / (1000.0f * 1000.0f);
        
        char fps_buffer[256];
        _snprintf_s(fps_buffer, sizeof(fps_buffer), "%.02fms/work, %.02fms/f, %.02ffps, %.02fmc/f\n", seconds_elapsed_for_work*1000, ms_per_frame, fps, mcpf);
        OutputDebugStringA(fps_buffer);
    }
    
    
    return 0;
}




