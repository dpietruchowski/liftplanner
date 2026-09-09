find_package(Qt6 REQUIRED COMPONENTS Core Sql)

add_subdirectory(third_party/googletest)
add_subdirectory(domain)
add_subdirectory(infrastructure)
add_subdirectory(application)
add_subdirectory(ui)
add_subdirectory(integration)
