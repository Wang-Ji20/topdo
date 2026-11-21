# Today's events: pure day logic, and reading them from the KDE calendar (Akonadi).
# See docs/design/project-today.md.
add_library(topdoday STATIC
    day.cpp
    day.h
)
target_include_directories(topdoday PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topdoday PUBLIC Qt6::Core)

add_library(topdocalendar STATIC
    akonadicalendar.cpp
    akonadicalendar.h
)
target_include_directories(topdocalendar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topdocalendar PUBLIC topdoday Qt6::Core PRIVATE KPim6::AkonadiCalendar KF6::CalendarCore)
