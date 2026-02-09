set(go_tests
    # Add here your tests files
    model/GOTestDrawStop.cpp
    model/GOTestOrganModel.cpp
    model/GOTestSwitch.cpp
    model/GOTestWindchest.cpp
    sound/buffer/GOTestSoundBuffer.cpp
    sound/buffer/GOTestSoundBufferManaged.cpp
    sound/buffer/GOTestSoundBufferMutable.cpp
    sound/buffer/GOTestSoundBufferMutableMono.cpp
    sound/buffer/GOTestPerfSoundBufferMutable.cpp
    GOTestNameMap.cpp
)
add_library(GOTests STATIC ${go_tests})

target_link_libraries(GOTests GOTestLib)
