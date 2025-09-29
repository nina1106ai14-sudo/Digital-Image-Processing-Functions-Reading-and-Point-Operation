# Image Processing with SDL2

## Overview
This project contains several C programs for **grayscale image processing** using SDL2.  
The programs support **BMP (8-bit grayscale)** and **RAW** images of size 512×512.  
You can display images, perform image transformations, and experiment with resizing techniques.

Supported image formats:  
- 8-bit grayscale BMP (`.bmp`)  
- RAW (`.raw`)  

Supported image size: **512×512**

---

## Programs Included

### 1. `a` -  Image reading
- **Description**: Reads BMP and RAW images, displays them in an SDL2 window, and prints the center 10×10 pixel values to the terminal.  
  This program addresses the **image reading** requirement.
- **Input**:
  - 3 RAW grayscale images (512×512, row-major)
  - 3 BMP grayscale images (8-bit, 512×512)
- **Features**:
  - Read BMP grayscale images (`readBMPGray`)
  - Read RAW images (`readRAW`)
  - Display images in SDL2 window (`showImageSDL`)
  - Print center pixel values (`printCenter`)
- **Purpose**:
  - Verify correct image reading
  - Provide pixel-level inspection
- **Usage**: Close each window to proceed to the next transformation.

### 2. `b` - Image enhancement toolkit
- **Description**: Performs **logarithmic**, **gamma**, and **negative** transformations on images and displays each result in SDL2 windows.  
- **Input**: 6 test images (RAW or BMP, 512×512)
- **Features**:
  - Logarithmic transform (`logTransform`): Enhance details in dark regions
  - Gamma transform (`gammaTransform`): Adjust brightness with different gamma values
  - Negative transform (`negativeTransform`): Invert pixel intensities
  - Displays each transformed image in a separate SDL2 window
- **Purpose**:
  - Compare the effects of different point operations 
- **Usage**: Close each window to proceed to the next transformation.

### 3. `c` -  Image downsampling and upsampling
- **Description**: Demonstrates **nearest neighbor** and **bilinear interpolation** resizing methods for images. 
- **Input**: 6 images (RAW or BMP, 512×512)
- **Features**:
  - Nearest neighbor resizing (`resizeNearest`)
  - Bilinear resizing (`resizeBilinear`)
  - Displays resized images in SDL2 window
- **Resize Cases**:
  1. Original (512×512) → 128×128
  2. Original (512×512) → 32×32
  3. Original (32×32) → 512×512
  4. Original (512×512) → 1024×512
  5. Original (128×128) → 256×512
- **Purpose**:
  - Compare **nearest neighbor vs. bilinear** results
  - Observe artifacts and quality differences
- **Usage**: Close each window to proceed to the next transformation.

---

## images
Place the following images in the `images/` folder:

- `baboon.bmp`  
- `boat.bmp`  
- `F16.bmp`  
- `goldhill.raw`  
- `lena.raw`  
- `peppers.raw`  

---

## Requirements
- **SDL2 library** (required to compile and run the programs)  
  Official download: [https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-devel-2.32.10-mingw.zip]
- C compiler (e.g., `g++` or `gcc`)

---

## Compilation

**Windows (g++):**
```bash
g++ a.c -Iinc -Llib -lSDL2main -lSDL2 -o a.exe
g++ b.c -Iinc -Llib -lSDL2main -lSDL2 -o b.exe
g++ c.c -Iinc -Llib -lSDL2main -lSDL2 -o c.exe
```

## Run
```bash
.\a.exe
.\b.exe
.\c.exe
```

## Project Structure
```bash
C:.
|   a.c
|   a.exe
|   b.c
|   b.exe
|   c.c
|   c.exe
|   README.md
|   SDL2.dll
|
+---images
|       baboon.bmp
|       boat.bmp
|       F16.bmp
|       goldhill.raw
|       lena.raw
|       peppers.raw
|
+---inc
|       begin_code.h
|       close_code.h
|       SDL.h
|       SDL_assert.h
|       SDL_atomic.h
|       SDL_audio.h
|       SDL_bits.h
|       SDL_blendmode.h
|       SDL_clipboard.h
|       SDL_config.h
|       SDL_config_android.h
|       SDL_config_emscripten.h
|       SDL_config_iphoneos.h
|       SDL_config_macosx.h
|       SDL_config_minimal.h
|       SDL_config_ngage.h
|       SDL_config_os2.h
|       SDL_config_pandora.h
|       SDL_config_windows.h
|       SDL_config_wingdk.h
|       SDL_config_winrt.h
|       SDL_config_xbox.h
|       SDL_cpuinfo.h
|       SDL_egl.h
|       SDL_endian.h
|       SDL_error.h
|       SDL_events.h
|       SDL_filesystem.h
|       SDL_gamecontroller.h
|       SDL_gesture.h
|       SDL_guid.h
|       SDL_haptic.h
|       SDL_hidapi.h
|       SDL_hints.h
|       SDL_joystick.h
|       SDL_keyboard.h
|       SDL_keycode.h
|       SDL_loadso.h
|       SDL_locale.h
|       SDL_log.h
|       SDL_main.h
|       SDL_messagebox.h
|       SDL_metal.h
|       SDL_misc.h
|       SDL_mouse.h
|       SDL_mutex.h
|       SDL_name.h
|       SDL_opengl.h
|       SDL_opengles.h
|       SDL_opengles2.h
|       SDL_opengles2_gl2.h
|       SDL_opengles2_gl2ext.h
|       SDL_opengles2_gl2platform.h
|       SDL_opengles2_khrplatform.h
|       SDL_opengl_glext.h
|       SDL_pixels.h
|       SDL_platform.h
|       SDL_power.h
|       SDL_quit.h
|       SDL_rect.h
|       SDL_render.h
|       SDL_revision.h
|       SDL_rwops.h
|       SDL_scancode.h
|       SDL_sensor.h
|       SDL_shape.h
|       SDL_stdinc.h
|       SDL_surface.h
|       SDL_system.h
|       SDL_syswm.h
|       SDL_test.h
|       SDL_test_assert.h
|       SDL_test_common.h
|       SDL_test_compare.h
|       SDL_test_crc32.h
|       SDL_test_font.h
|       SDL_test_fuzzer.h
|       SDL_test_harness.h
|       SDL_test_images.h
|       SDL_test_log.h
|       SDL_test_md5.h
|       SDL_test_memory.h
|       SDL_test_random.h
|       SDL_thread.h
|       SDL_timer.h
|       SDL_touch.h
|       SDL_types.h
|       SDL_version.h
|       SDL_video.h
|       SDL_vulkan.h
|
\---lib
    |   libSDL2.a
    |   libSDL2.dll.a
    |   libSDL2.la
    |   libSDL2main.a
    |   libSDL2main.la
    |   libSDL2_test.a
    |   libSDL2_test.la
    |
    +---cmake
    |   \---SDL2
    |           sdl2-config-version.cmake
    |           sdl2-config.cmake
    |
    \---pkgconfig
            sdl2.pc
