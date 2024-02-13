# Setting dependencies for the Waveshare display
# Originally tried to make it a component but the Arduino interdependency fucked me
# This works though even though it's not very programmatic, whatever it's not the part that actually matters

set(srcs
  "main.cpp"
  "Paper.cpp"
  "../include/esp32-waveshare-epd/DEV_Config.cpp"
  "../include/esp32-waveshare-epd/font8.cpp"
  "../include/esp32-waveshare-epd/font12.cpp"
  "../include/esp32-waveshare-epd/font16.cpp"
  "../include/esp32-waveshare-epd/font20.cpp"
  "../include/esp32-waveshare-epd/font24.cpp"
  "../include/esp32-waveshare-epd/GUI_Paint.cpp"
  "../include/esp32-waveshare-epd/utility/EPD_7in5.cpp"
  "../include/esp32-waveshare-epd/utility/EPD_7in5_V2.cpp"
  "../include/esp32-waveshare-epd/utility/EPD_7in5_HD.cpp"
)

set(includedirs
  "../include/esp32-waveshare-epd"
  "../include/esp32-waveshare-epd/utility")

idf_component_register(INCLUDE_DIRS ${includedirs} SRCS ${srcs})
