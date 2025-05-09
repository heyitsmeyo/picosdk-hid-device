# This project gives your pico the ability to act as a keyboard by sending keystrokes to your linux machine (HID)

https://github.com/user-attachments/assets/1d7d2170-b9f1-4c00-943e-6d0a11768d67

## Project Structure

```
your_project/
├── tinyusb/
├── src/
├── build/
├── pico_sdk_import.cmake
└── CMakeLists.txt
```

#  First , clone Tinyusb library  from the github link below : 

    https://github.com/hathach/tinyusb

# You also need Pico-sdk so  clone it from here : 

    https://github.com/raspberrypi/pico-sdk

# To flash the code into the raspberry , we need picotool that can be cloned from here : 

    https://github.com/raspberrypi/picotool


Copy pico_sdk_import_cmake from pico-sdk folder into your project folder.

Add Your CMakeLists.txt and your main C code. 

Add a folder named "build" within your project folder. 

then : 

    cd build 

then export pico-sdk path : 

    export PICO_SDK_PATH=/

then , run cmake : 

    cmake ..

then : 

    make 

To flash the code into the pico , we use the following command ( Make sure the pico is in bootsel mode ) : 

    sudo picotool load "uf2 file here " 



# if you like the project , you can donate us for more : 

    https://ko-fi.com/heyitsmeyo



### Coming Soon
- Add support for Windows commands.












