# This file is part of Telegram Desktop,
# the official desktop application for the Telegram messaging service.
#
# For license and copyright information please follow this link:
# https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL

add_executable(test_window_material)
init_target(test_window_material "(tests)")

target_include_directories(test_window_material PRIVATE ${src_loc})

nice_target_sources(test_window_material ${src_loc}
PRIVATE
    window/window_material_policy.cpp
    window/window_material_policy.h
    tests/test_window_material.cpp
)

target_link_libraries(test_window_material
PRIVATE
    desktop-app::external_qt
)

set_target_properties(test_window_material PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
