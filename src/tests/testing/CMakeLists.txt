set(go_tests
    # Add here your tests files
    model/GOTestOrganModel.cpp
    model/GOTestSwitch.cpp
    model/GOTestWindchest.cpp
)
add_library(GOTests STATIC ${go_tests})

target_link_libraries(GOTests GOTestLib)
