set(go_tests
    # Add here your tests files
    combinations/GOTestDivisionalSetter.cpp
    loader/GOTestOrganReader.cpp
    midi/GOTestMidiPlayerContent.cpp
    midi/GOTestMidiSendProxy.cpp
    model/GOTestDrawStop.cpp
    model/GOTestOrganModel.cpp
    model/GOTestSwitch.cpp
    model/GOTestWindchest.cpp
    sound/buffer/GOTestPerfSoundBufferMutable.cpp
    sound/buffer/GOTestSoundBuffer.cpp
    sound/buffer/GOTestSoundBufferBase.cpp
    sound/buffer/GOTestSoundBufferManaged.cpp
    sound/buffer/GOTestSoundBufferMutable.cpp
    sound/buffer/GOTestSoundBufferMutableMono.cpp
    sound/playing/GOTestReleaseAlignTable.cpp
    sound/playing/GOTestSoundStream.cpp
    sound/GOTestSoundOrganEngineBase.cpp
    sound/GOTestSoundOrganEngine.cpp
    sound/GOTestSoundCallbackConnector.cpp
    sound/GOTestSoundOrganEngineStress.cpp
    sound/tasks/GOSoundCooperativeTaskTestImpl.cpp
    sound/tasks/GOSoundTaskTestImpl.cpp
    sound/tasks/GOTestPerfSoundTaskBase.cpp
    sound/tasks/GOTestSoundTaskBase.cpp
    GOTestNameMap.cpp
    GOTestOrganController.cpp
)
add_library(GOTests STATIC ${go_tests})

target_link_libraries(GOTests GOTestLib)
target_compile_definitions(GOTests PUBLIC
    GOTEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/combinations/fixtures")
target_compile_definitions(GOTests PRIVATE
    GO_TEST_RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
