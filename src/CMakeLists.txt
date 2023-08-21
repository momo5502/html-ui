add_library(html-ui
    html_ui.cpp
    window.cpp
    html_value.cpp
    html_frame.cpp
    html_window.cpp
    ie_setup.cpp
)

target_include_directories(html-ui PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")