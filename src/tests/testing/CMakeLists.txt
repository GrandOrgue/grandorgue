set(go_tests
    # Add here your tests files
    GOTestOrganModel.cpp
    GOTestSwitch.cpp
    GOTestWindchest.cpp
)
add_library(GOTests STATIC ${go_tests})

target_link_libraries(GOTests GOTestLib)
