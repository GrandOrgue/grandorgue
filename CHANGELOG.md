# 3.14.2 (2024-04-29)
- Fixed divisional couplers with setter divisionals https://github.com/GrandOrgue/grandorgue/issues/1787
- Fixed behavior of "Detect complex MIDI setup" with Note events in different cases https://github.com/GrandOrgue/grandorgue/issues/1762
- Fixed the order of sending midi events from an On indicator. Now they are sent after sending all events from other controls https://github.com/GrandOrgue/grandorgue/issues/1762
# 3.14.1 (2024-04-17)
- Fixed changing sound of a playing pipe without Pipe999IsTremulant when a wave tremulant state is changed https://github.com/GrandOrgue/grandorgue/issues/1855
- Fixed crash on opening a very old config https://github.com/GrandOrgue/grandorgue/discussions/1869
- Fixed ignoring initial midi setup when loading an organ with a preset without midi events configured https://github.com/GrandOrgue/grandorgue/issues/1785
- Fixed saving Max release tail to the organ preset https://github.com/GrandOrgue/grandorgue/issues/1804
- Fixed required package names in the BUILD.md file https://github.com/GrandOrgue/grandorgue/issues/1799
- Added support of macOS on Apple silicon. GrandOrgue for macOS on Apple silicon requires macOS 14 or higher. GrandOrgue for macOS on Intel requires macOS 12.1 or higher. https://github.com/GrandOrgue/grandorgue/discussions/1153
- Added ad-hoc code signing of GrandOrgue for macOS https://github.com/GrandOrgue/grandorgue/issues/1835
- Fixed an error that occurred when trying to install the GrandOrgue RPM package https://github.com/GrandOrgue/grandorgue/issues/1859
# 3.14.0 (2024-03-29)
- Fixed crash on loading an organ without a pedal but wit a unison-off coupler https://github.com/GrandOrgue/grandorgue/issues/1846
- Changed displaying of the right part of paths https://github.com/GrandOrgue/grandorgue/issues/1663
- Added automatic update checking at startup (configurable) and the ability to check for updates from Help menu  
- Increased maximum value of allowed size to 32000 https://github.com/GrandOrgue/grandorgue/issues/1840
- Added capability of specifying AmplitudeLevel, Gain, PitchTuning, PitchCorrection and TrackerDelay, Percussive, HasIndependentRelease at the WindchestGroup level of ODF
- Added capability of specifying Percussive and HasIndependentRelease at the Organ level of ODF
- Added capability of playing release of percussive pipes with Pipe999HasIndependentRelease=Y https://github.com/GrandOrgue/grandorgue/issues/1385
- Fixed different encoding of combination .yaml files on Windows, Linux and MacOS https://github.com/GrandOrgue/grandorgue/issues/1818
- Added support of "Couple Through" mode of Virtual Couplers https://github.com/GrandOrgue/grandorgue/issues/1657
- Added capability of loading only GUI panels without audio samples by specifying the "-g" switch from the command line https://github.com/GrandOrgue/grandorgue/issues/1602
- Removed support of MacOS 11 https://github.com/GrandOrgue/grandorgue/issues/1791
- Fixed crash on loading an organ with a crescendo in Add mode https://github.com/GrandOrgue/grandorgue/issues/1772
- Fixed crash with rtaudio/asio https://github.com/GrandOrgue/grandorgue/issues/1772
- Added support of specifying different crossfade length for additional attack and release files https://github.com/GrandOrgue/grandorgue/issues/1760
- Fixed wrong filename of the midi recorder file with Hungarian language settings https://github.com/GrandOrgue/grandorgue/issues/1644
- Fixed unability to select the Hungarian language in the Organ Settings dialog
- Eliminated resetting audio group with the Default button of Organ Settings dialog https://github.com/GrandOrgue/grandorgue/issues/731
# 3.13.3 (2024-01-07)
- Fixed not loading a pipe if some loop was not suitable for crossfade https://github.com/GrandOrgue/grandorgue/issues/1724
- Fixed a wrong .wav filename in the log message window https://github.com/GrandOrgue/grandorgue/issues/1724
- Increased the maximum number of Tremulants from 10 to 999
- Fixed setting a reverb file name by default to the current directory https://github.com/GrandOrgue/grandorgue/issues/1741
- Fixed crash on enabling convolution reverb https://github.com/GrandOrgue/grandorgue/issues/1741
- Fixed hang on Panic button press on MacOs https://github.com/GrandOrgue/grandorgue/issues/1726
- Fixed crash on switching divisionals when a bidirectional devisional coupler was engaged https://github.com/GrandOrgue/grandorgue/issues/1725
# 3.13.2 (2023-11-19)
- Fixed loading an organ when some configuration entry is out of range https://github.com/GrandOrgue/grandorgue/issues/1696
- Fixed crash when trying to load a sample set with a truncated wave file https://github.com/GrandOrgue/grandorgue/discussions/370
- Fixed crash on closing an organ https://github.com/GrandOrgue/grandorgue/issues/1678
# 3.13.1 (2023-11-05)
- Fixed the maximal value of the "SYSEX Hauptwerk 32 Byte LCD" midi send events https://github.com/GrandOrgue/grandorgue/issues/1686
- Fixed discard in the Organ Settings dialog when multiple objects are selected https://github.com/GrandOrgue/grandorgue/issues/1674
- Moved all Organ Settings dialog buttons to the single button row https://github.com/GrandOrgue/grandorgue/issues/1674
- Decreased number of decimal digits in the Organ Settings dialog https://github.com/GrandOrgue/grandorgue/issues/1627
- Added a confirmation box when restoring Organ Settings to default https://github.com/GrandOrgue/grandorgue/issues/1674
- Replaced the Cancel and OK buttons in the Organ Settings dialog with the single Close button https://github.com/GrandOrgue/grandorgue/issues/1674
- Fixed change tracking and enableing/disabling the Discard and Apply buttons in the Organ Settings dialog https://github.com/GrandOrgue/grandorgue/issues/1674
- Renamed the Reset button to Discard in the Organ Settings dialog https://github.com/GrandOrgue/grandorgue/issues/1674
- Adopted the build instruction and the build scripts to new ubuntu versions https://github.com/GrandOrgue/grandorgue/issues/1673
- Improved concurrency handling
- Added deregistering organs in the temporary directory that do not more exist https://github.com/GrandOrgue/grandorgue/issues/1660
- Fixed error messages after multiple runs of GrandOrgue ftom Appimage with a demo organ https://github.com/GrandOrgue/grandorgue/issues/1660
# 3.13.0 (2023-10-11)
- Implemented option to send MIDI Note Off as 0x8X or 0x9X with velocity 0 https://github.com/GrandOrgue/grandorgue/issues/1640
- Added capability of control buttons with Control Change MIDI events with different keys but same values using "Bx Ctrl Change Fixed On Value Toggle" and "Bx Ctrl Change Fixed Off Value Toggle" https://github.com/GrandOrgue/grandorgue/issues/1392
- Added saving dialog positions and sizes https://github.com/GrandOrgue/grandorgue/issues/1035
- Increased maximum value of Pipe999LoopCrossfadeLength and Pipe999ReleaseCrossfadeLength. Now they are 3000 https://github.com/GrandOrgue/grandorgue/issues/1612
# 3.12.3 (2023-08-14)
- Fixed saving manual switches as global in yaml https://github.com/GrandOrgue/grandorgue/issues/1599
- Fixed convolution enabling warrning in the Settings dialog https://github.com/GrandOrgue/grandorgue/issues/1617
- Fixed "Release Length is not valid" error when applying changes for several Organ Settings objects at once https://github.com/GrandOrgue/grandorgue/issues/1601
- Fixed saving all combinations as full
# 3.12.2 (2023-07-23)
- Fixed size of the Organ Settings Dialog https://github.com/GrandOrgue/grandorgue/issues/1415
- Fixed an incorrect dialog window icon
- Fixed absence of the Help button on the Organ Setting dialog https://github.com/GrandOrgue/grandorgue/issues/1416
- Fixed displaying buttons if the manual is not visible https://github.com/GrandOrgue/grandorgue/issues/1566
- Changed the default value of the CombinationsStoreNonDisplayedDrawstops ODF settings to false
- Fixed capability of running on MacOs 11.3
- Fixed "The device does not support requested channel count" error when using an USB audio card on MacOS https://github.com/GrandOrgue/grandorgue/issues/1550
# 3.12.1 (2023-06-06)
- Fixed not storing switch state in combinations in organs with panels of the new style https://github.com/GrandOrgue/grandorgue/issues/1498
- Fixed displaying light of various combination buttons https://github.com/GrandOrgue/grandorgue/issues/1536 
- Fixed saving empty and scoped combinations to yaml https://github.com/GrandOrgue/grandorgue/issues/1531
- Fixed bug of GC not working on manual with only a single stop https://github.com/GrandOrgue/grandorgue/issues/1556
- Fixed installation on linux with another yaml-cpp version than 6.2 https://github.com/GrandOrgue/grandorgue/issues/1548
# 3.12.0 (2023-05-25)
- Reverted back default display of GeneralPrev and GeneralNext https://github.com/GrandOrgue/grandorgue/issues/1538
- Added new builtin bitmap set of manual keys and one new drawstop bitmap https://github.com/GrandOrgue/grandorgue/discussions/1490
- Removed support of MacOs 10.15. Now MacOs 11 or above is required https://github.com/GrandOrgue/grandorgue/discussions/1149
- Fixed warning "temperament would retune pipe by more than 600 cents" for retuned pipes https://github.com/GrandOrgue/ODFEdit/discussions/11#discussioncomment-5877020
- Increased the maximum value of Enclosures from 50 to 999 https://github.com/GrandOrgue/grandorgue/issues/1484
# 3.11.2 (2023-05-08)
- Fixed crash on loading organ with general of divisional buttons https://github.com/GrandOrgue/grandorgue/issues/1512
# 3.11.1 (2023-05-03)
- Fixed processing yaml combinations on organs with invisible couplers https://github.com/GrandOrgue/grandorgue/issues/1489
- Fixed uncompatibility with recent linux distros. Use grandorgue-wx32 on new linux distros (where wxWidgets 3.0 is not available) https://github.com/GrandOrgue/grandorgue/issues/1480
- Added support of negative y-axis adjustment of manual keys https://github.com/GrandOrgue/grandorgue/issues/1485
- Fixed opening an organ with more than 50 windchest groups https://github.com/GrandOrgue/grandorgue/issues/1484
# 3.11.0 (2023-04-17)
- Added capability of navigating over, loading and saving the YAML combinations files on the Combination Setter panel https://github.com/GrandOrgue/grandorgue/issues/1196
- Added capability of exporting and importing combinations in yaml format files https://github.com/GrandOrgue/grandorgue/issues/1195
- Removed support of old linux distros (with libc6 versions before 2.31) https://github.com/GrandOrgue/grandorgue/discussions/1334
- Fixed the translation of the desktop icon on Linux https://github.com/GrandOrgue/grandorgue/issues/1429
- Added support of labels without a background image by specifying DispImageNum=0 https://github.com/GrandOrgue/grandorgue/issues/1386
- Added capability of overriding wav MIDIPitchFraction with the Pipe999MIDIPitchFraction key https://github.com/GrandOrgue/grandorgue/issues/1378
# 3.10.1 (2023-02-24)
- Fixed crash on loading an incorrect organ
# 3.10.0 (2023-02-17)
- Added storing the GrandOrgueVersion key in the Organ Settings file https://github.com/GrandOrgue/grandorgue/issues/1375
- Added capability of redefining ReleaseLength and IgnorePitch on each level of Organ Settings
- Changed defaults for the General Setter elements to Piston https://github.com/GrandOrgue/grandorgue/issues/1360
- Added new fields ManualTuning and AutoTuningCorrection of Organ Settings https://github.com/GrandOrgue/grandorgue/issues/1333
- Fixed duplicating packaged organs in the organ list https://github.com/GrandOrgue/grandorgue/issues/1367
- Fixed displaying a popup window when a cache had been created with another GO version https://github.com/GrandOrgue/grandorgue/issues/1363
- Fixed not saving midi settings of divisional buttons https://github.com/GrandOrgue/grandorgue/issues/1350
- Added PitchCorrection for organs and windchest. Pipe999PitchCorrection became additive to PitchCorrection of the rank, of the windchest and of the organ https://github.com/GrandOrgue/grandorgue/issues/1351
- Added full support of '/' as the file sepearator in ODF unless 'Check ODF for HW1-compatibility' is set https://github.com/GrandOrgue/grandorgue/issues/827
- Added support of ODF comments from ';' at any position to the end of line https://github.com/GrandOrgue/grandorgue/issues/828
# 3.9.5 (2023-01-23)
- Fixed saving position when some panel is outside the screen area https://github.com/GrandOrgue/grandorgue/issues/1271
- Fixed playing release samples for very short notes https://github.com/GrandOrgue/grandorgue/issues/1222
- Fixed help: the wrong panel positioning on wayland was documented https://github.com/GrandOrgue/grandorgue/issues/1271
- Fixed typos and spelling errors in the Help
- Fixed continuing loading an organ after an exception in one loading thread
- Fixed size of text fields in the Organ Settings dialog on OsX https://github.com/GrandOrgue/grandorgue/issues/1315
- Fixed missing the object filename in an error message if some exception occured when loading this object
# 3.9.4 (2022-12-12)
- Fixed starting 3.9.3 on Windows https://github.com/GrandOrgue/grandorgue/issues/1311
# 3.9.3 (2022-12-09)
- Fixed an old version number in the gnome application metadata https://github.com/GrandOrgue/grandorgue/issues/1304
- Fixed switching off generals when using crescendo in not override mode https://github.com/GrandOrgue/grandorgue/issues/1299
- Fixed error messages of GrandOrguePerf https://github.com/GrandOrgue/grandorgue/issues/1280
- Fixed exit from GrandOrgue with an unhandled exception occurred on loading an organ
- Fixed displaying output volume indicators on OSx https://github.com/GrandOrgue/grandorgue/issues/1255
- Fixed sending midi-off events from generals and another pushbuttons https://github.com/GrandOrgue/grandorgue/issues/1291
- Fixed crashing on exit in some cases
# 3.9.2 (2022-11-29)
- Fixed sending incorrect midi values from an enclosure https://github.com/GrandOrgue/grandorgue/issues/1267
- Fixed not sending midi events from pushbuttons, including GC https://github.com/GrandOrgue/grandorgue/issues/1220
- Fixed sending a midi event from an enclosure when it's value is not changed https://github.com/GrandOrgue/grandorgue/issues/1206
- Fixed processing enclosures with high value is less than low value https://github.com/GrandOrgue/grandorgue/issues/1266
# 3.9.1 (2022-11-14)
- Fixed crash on loading an organ without a pedal https://github.com/GrandOrgue/grandorgue/issues/1249
- Fixed SYSEX Hauptwerk max. length in MIDI event editor https://github.com/GrandOrgue/grandorgue/issues/1207
- Fixed enclosure value calculation https://github.com/GrandOrgue/grandorgue/issues/1203
- Fixed controlling stops with a touchscreen on Raspberry Pi https://github.com/GrandOrgue/grandorgue/issues/1208
# 3.9.0 (2022-11-03)
- Fixed playing multitrack midi files with changes of tempo https://github.com/GrandOrgue/grandorgue/discussions/1225
- Fixed displaying audio ports on OSx https://github.com/GrandOrgue/grandorgue/issues/1216
- Added divisional combination banks https://github.com/GrandOrgue/grandorgue/issues/708
- Renamed audio ports: Pa to PortAudio and Rt to RtAudio https://github.com/GrandOrgue/grandorgue/issues/1216
- Fixed size of the Organ Selection Dialog https://github.com/GrandOrgue/grandorgue/issues/1215
- Fixed generals buttons behaviour with the crescendo in add mode https://github.com/GrandOrgue/grandorgue/issues/1209
- Fixed an empty stop set to a general combination https://github.com/GrandOrgue/grandorgue/issues/1212
# 3.8.0 (2022-09-15)
- Fixed setting an empty stop set to a divisional combination https://github.com/GrandOrgue/grandorgue/issues/1068
- Added capability of switching between the Override and the Add crescendo mode https://github.com/GrandOrgue/grandorgue/issues/1170
- Fixed rebuilding the cache every time upon the appimage starts https://github.com/GrandOrgue/grandorgue/issues/1174
- Fixed building appimage in an ubuntu container
# 3.7.1 (2022-09-02)
- Fixed translation of temperaments' names https://github.com/GrandOrgue/grandorgue/issues/1104
- Fixed saving ReleaseTail https://github.com/GrandOrgue/grandorgue/issues/1161
- Moved build for OsX to OsX 11 https://github.com/GrandOrgue/grandorgue/discussions/1149
# 3.7.0 (2022-08-11)
- Fixed packaging for OSx https://github.com/GrandOrgue/grandorgue/issues/1135
- Deleting an organ in the Organ tab of the Settings dialog causes also deleting it's cache and all it's presets https://github.com/GrandOrgue/grandorgue/issues/1049
- Reorganised the Organs tab of the Settings dialog https://github.com/GrandOrgue/grandorgue/issues/1119
- Added capability of restoring organ settings values for all subobjects https://github.com/GrandOrgue/grandorgue/issues/1119
# 3.6.7 (2022-05-15)
- Added publishing debug symbols on windows builds https://github.com/GrandOrgue/grandorgue/issues/853
# 3.6.6 (2022-04-29)
- Fixed help issues https://github.com/GrandOrgue/grandorgue/issues/1066
- Fixed saving organ settings on closing an organ or on finishing GrandOrgue https://github.com/GrandOrgue/grandorgue/issues/1069
- Fixed setting up output midi events for all devices https://github.com/GrandOrgue/grandorgue/issues/1097
# 3.6.5 (2022-04-13)
- Fixed not saving the main window position and size in organ settings https://github.com/GrandOrgue/grandorgue/issues/1093
- Fixed sound distortion while a reveberation is active https://github.com/GrandOrgue/grandorgue/issues/983
- Fixed hang on reopening sound device (exit from settings, Panic) while a reveberation is active https://github.com/GrandOrgue/grandorgue/issues/983
- Added support for building against system-wide ZitaConvolver library https://github.com/GrandOrgue/grandorgue/issues/1095
# 3.6.4 (2022-03-18)
- Updated the copyright headers in the source code
- Switched ZitaConvolver to an external source https://github.com/GrandOrgue/grandorgue/issues/983
- Added distribution of GrandOrguePerfTest in the GrandOrgue package https://github.com/GrandOrgue/grandorgue/issues/868
- Added the image file name to messages on loading this image https://github.com/GrandOrgue/grandorgue/issues/1063
- Fixed allowing to select disabled devices in the Midi Event Dialog https://github.com/GrandOrgue/grandorgue/issues/1044
- Fixed limits for PGM numbers of the Cx Program Change Range midi event https://github.com/GrandOrgue/grandorgue/issues/1058
- Clarified the guide how to obtain submodule sources manually in the BUILD.md https://github.com/GrandOrgue/grandorgue/issues/1062
# 3.6.3 (2022-03-02)
- Added the wave file name to an error message related to a certain sample file https://github.com/GrandOrgue/grandorgue/discussions/1055
- Fixed console output of GrandOrgueTool under windows https://github.com/GrandOrgue/grandorgue/issues/949
- Added the Help button on the MidiEventDialog https://github.com/GrandOrgue/grandorgue/issues/1001
- Fixed covering the help window opened from a modal dialog under linux https://github.com/GrandOrgue/grandorgue/issues/1004
- Fixed size of the Setting dialog if a lot of audio channels exist https://github.com/GrandOrgue/grandorgue/issues/1034
- Fixed navigation from the Setting dialog pages to help with non-english language https://github.com/GrandOrgue/grandorgue/issues/1003
# 3.6.2 (2022-02-09)
- Fixed "Copy current receive event" in the Midi Event Send dialog https://github.com/GrandOrgue/grandorgue/issues/985
- Fixed automatic opening the Reverb settings page when there were no active midi devices https://github.com/GrandOrgue/grandorgue/issues/1002
- Fixed matching midi events with midi devices https://github.com/GrandOrgue/grandorgue/issues/1000
# 3.6.1 (2022-01-29)
- Updated French, German, Polish, Spanish and Swedish translations of the Settings Dialog https://github.com/GrandOrgue/grandorgue/discussions/934
- Switched to RtMidi 5.0.0
- Switched to RtAudio 5.2.0
- Fixed processing of debounce time for Bx Controller events https://github.com/GrandOrgue/grandorgue/issues/967
- Fixed size of the Load organ dialog https://github.com/GrandOrgue/grandorgue/issues/963
# 3.6.0 (2022-01-19)
- Fixed crash under OSx when closing a MIDI event dialog opened from a Settings window https://github.com/GrandOrgue/grandorgue/issues/966
- Reorganised tabs and their layouts of the Settings dialog https://github.com/GrandOrgue/grandorgue/issues/948
- Changed default organ cache and organ settings directories to ones inside the GrandOrgue directory https://github.com/GrandOrgue/grandorgue/discussions/934
- Added automatic formatting code with pre-commit and clang-format https://github.com/GrandOrgue/grandorgue/issues/918
# 3.5.0 (2022-01-05)
- Added default regex patterns for matching MIDI devices with alsa, jack and winmm https://github.com/GrandOrgue/grandorgue/issues/885
- Added capability of matching MIDI devices with regex expressions https://github.com/GrandOrgue/grandorgue/issues/885
- Added a setting option whether to check for existence of active midi devices on startup https://github.com/GrandOrgue/grandorgue/issues/796
- Added capability of switching auto enabling of new midi devices on/off https://github.com/GrandOrgue/grandorgue/issues/703
- Added capability of enabling and disabling particular MIDI APIs https://github.com/GrandOrgue/grandorgue/issues/703
- Added support for building against system-wide RtAudio and RtMidi libraries
# 3.4.4 (2021-11-27)
- Fixed crash after loading an organ several times https://github.com/GrandOrgue/grandorgue/issues/707
# 3.4.3 (2021-11-22)
- Fixed hang after changing the audio device https://github.com/GrandOrgue/grandorgue/issues/701
- Fixed a possible crash when playing with shared pipes https://github.com/GrandOrgue/grandorgue/issues/847
# 3.4.2 (2021-11-14)
- Added ASIO logo to About splash screen https://github.com/GrandOrgue/grandorgue/issues/823
- Upgraded PortAudio Library to the latest stable release, v19.7.0. The upgraded library corrects PortAudio errors in GrandOrgue on macOS 11.
- Restructured the source code directory tree https://github.com/GrandOrgue/grandorgue/issues/834
- Renamed the source code files to have the single GO prefix https://github.com/GrandOrgue/grandorgue/issues/816
- Added implementation of the threading primitives with standard C++11 ones https://github.com/GrandOrgue/grandorgue/issues/817
- Changed build default directory to 'build' https://github.com/GrandOrgue/grandorgue/issues/843
# 3.4.1 (2021-11-02)
- Changed log error to warning when importing combinations https://github.com/GrandOrgue/grandorgue/issues/809
- Eased restrictions on importing .cmb settings https://github.com/GrandOrgue/grandorgue/issues/807
- Fixed restoring the size and the position of panels when they did not fit their displays https://github.com/GrandOrgue/grandorgue/issues/787
- Removed translation of GrandOrgue midi ports https://github.com/GrandOrgue/grandorgue/issues/791
- Switched the build for windows to use wxWidgets 3.1.5 https://github.com/GrandOrgue/grandorgue/issues/792
- Added bundling feature of settings file and ODF with same name in same directory
- Fixed the addition of .cmb file extension when exporting settings if necessary https://github.com/GrandOrgue/grandorgue/issues/802
# 3.4.0 (2021-10-24)
- Eliminated using opensuse repositories when building for windows on github https://github.com/GrandOrgue/grandorgue/issues/28
- Fixed hang on Panic button after the sound had been suddenly stopped https://github.com/GrandOrgue/grandorgue/issues/762
- Make the Paths settings in the Options tab expandable
- Added build for linux appimage https://github.com/GrandOrgue/grandorgue/issues/698
- Fixed bug preventing building with -DGO_USE_JACK=OFF
- Moved the netbeans project files to the ide-projects/NetBeans12 subdirectory https://github.com/GrandOrgue/grandorgue/discussions/771
# 3.3.0 (2021-10-08)
- Added automated release tagging on GitHub https://github.com/GrandOrgue/grandorgue/issues/711
- Added polish language support https://github.com/GrandOrgue/grandorgue/discussions/743
- Added upgrade and uninstall guide to INSTALL.md https://github.com/GrandOrgue/grandorgue/issues/720
- Fixed scrolling of Organsettings dialog https://github.com/GrandOrgue/grandorgue/issues/735
- Got rid of separate linux packages grandorgue-resources and grandorgue-demo https://github.com/GrandOrgue/grandorgue/issues/741
# 3.2.0 (2021-09-27)
- Renewed a default GO version during cmake build https://github.com/GrandOrgue/grandorgue/issues/14
- Fixed initial sizing of Panel menu when organ is loaded https://github.com/GrandOrgue/grandorgue/issues/712
- Fixed hyperlinks to the GO distribution in INSTALL.md
- Made paths fieldset and dropdowns of default settings dialog adjust size https://github.com/GrandOrgue/grandorgue/issues/713
- Increasing the limit of number of ranks from 400 to 999 https://github.com/GrandOrgue/grandorgue/discussions/680
# 0.3.1.2342 (2021-09-16)
- Fixed build for windows on github https://github.com/GrandOrgue/grandorgue/issues/29
- Fixed crash on OSx when opening some pannels https://github.com/oleg68/GrandOrgue/issues/94
- Updated help with changes in menus and Settings... dialog https://github.com/GrandOrgue/grandorgue/issues/17
- The project has been moved to the new official GrandOrgue repository https://github.com/GrandOrgue/grandorgue
# 0.3.1.2341-15.os (2021-09-04)
- Fixed incorrect ``Apply changes first`` in the Organ Settings dialog on OSX https://github.com/oleg68/GrandOrgue/issues/87
- Fixed size of the Organ Selection dialog https://github.com/oleg68/GrandOrgue/issues/85
- Added asking for restart after changing language (does not work on Windows yet)
- Added displaying a reason of entering to the settings on GO startup https://github.com/oleg68/GrandOrgue/issues/66
# 0.3.1.2341-14.os (2021-08-31)
- Fixed sizing of the setting dialog https://github.com/oleg68/GrandOrgue/issues/77
- `Sample Rate`` and ``Sample Per Buffer`` settings have been moved to the ``Audio Output`` tab
# 0.3.1.2341-13.os (2021-08-24)
- Enabled resizing of the main window https://github.com/oleg68/GrandOrgue/issues/71
- Fixed sizing issue with the main window https://github.com/oleg68/GrandOrgue/issues/71
- Fixed crash on OSx when GrandOrgue was exiting https://github.com/oleg68/GrandOrgue/issues/68
- Removed obsolete and unused files from the source code
# 0.3.1.2341-12.os (2021-08-17)
- Fixed ``error creating OS-X MIDI client object`` error messages on OSx
# 0.3.1.2341-11.os (2021-08-16)
- Switched to releases of RtAudio 5.1.0 and RtMidi 4.0.0
- Fixed name of arm64 architecture for the debian package https://github.com/oleg68/GrandOrgue/issues/63
# 0.3.1.2341-10.os (2021-08-13)
- Displaying the full build version in the title and in the About window https://github.com/oleg68/GrandOrgue/issues/61
- Converted ``Index has bad magic - bypassing index`` errors to warnings https://github.com/oleg68/GrandOrgue/issues/58
# 0.3.1.2341-9.os (2021-08-07)
- Fixed ``A device wants different samples per buffer: 15`` on OSx  with RtAudio https://github.com/oleg68/GrandOrgue/issues/54
- Displaying actual error text instead of ``Internal Portaudio Error`` https://github.com/oleg68/GrandOrgue/issues/52
- Added capability of disabling some sound ports and API's https://github.com/oleg68/GrandOrgue/issues/48
- Eliminated extra opening sound devices https://github.com/oleg68/GrandOrgue/issues/48
- Changed the sound device name format to ``Subsys: Api: Device`` https://github.com/oleg68/GrandOrgue/issues/48
# 0.3.1.2341-8.os (2021-07-26)
- Moved Language selection on top of the first setting page
- Fixed hang after opening the About window on OsX https://github.com/oleg68/GrandOrgue/issues/25
# 0.3.1.2341-7.os (2021-07-24)
- ``Audio/Midi &Settings`` menu item has been renamed to ``Settings...`` and has been moved to the File menu
- Added capability of setting the UI language https://github.com/oleg68/GrandOrgue/issues/45
- Fixed formatting of .md files
# 0.3.1.2341-6.os (2021-07-07)
- Fixed dialog colours in the Dark mode on OSx.
- Changed the application icons. Now they are generated from a single svg file during the build https://github.com/oleg68/GrandOrgue/issues/20
# 0.3.1.2341-5.os (2021-06-18)
- Added support of jack native output in Github windows build
- Fixed compiling for windows with GO_USES_JACK=ON https://github.com/oleg68/GrandOrgue/issues/7
- README.md has been splitted to three separates files: README.md, INSTALL.md and BUILD.md
# 0.3.1.2341-4.os (2021-06-12)
- Hiding dependency on libgcc-s1 for all cpu architectures
- Could not install on ubuntu 18.04 and debian 9 caused by unsatisfied dependency on libgcc-s1
# 0.3.1.2341-3.os (2021-06-05)
- Switched RTAudio from the local copy to the git submodule https://github.com/thestk/rtaudio
- Added ASIO support to the build for windows on github. https://github.com/oleg68/GrandOrgue/issues/8
- Beautified README and renamed to README.md
# 0.3.1.2341-2.os (2021-05-29)
- Added build for linux armhf (arm 32) and aarch64 (arm64), including raspberry Pi
# 0.3.1.2341-1.os (2021-05-22)
- Increased the limit of PitchTuning and PitchCorrection up from 1200 to 1800 cents https://github.com/oleg68/GrandOrgue/issues/33
- Increase builtin button font size (0.3.1.2341)
- Ensure fixed font sizes (0.3.1.2340)
- Revert layout destroying resizing (0.3.1.2339)
# 0.3.1.2338-8.os (2021-05-18)
- Closing unnecessary jack clients https://github.com/oleg68/GrandOrgue/issues/30
# 0.3.1.2338-6.os (2021-05-10)
- Added CHANGELOG.md https://github.com/oleg68/GrandOrgue/issues/23
# 0.3.1.2338-5.os (2021-05-08)
- Fixed issues various pipewire-related issues https://github.com/oleg68/GrandOrgue/issues/28
- Fixed compiling on Fedora34 (switched from `jack-devel` to `pipewire-jack-devel`)
# 0.3.1.2338-4.os (2021-05-06)
- Fixed crash on osx 11.x https://github.com/oleg68/GrandOrgue/issues/22
- Disabled striping some debuginfo from osx executables
- Simplified build on Github
# 0.3.1.2338-3.os (2021-04-26)
- Added compatibility with FlatPack https://github.com/oleg68/GrandOrgue/pull/18
# 0.3.1.2338-2.os (2021-04-25)
- Updated README for installation doc
- Added generating `rpm`, `src.rpm` and `deb` packages for linux on GitHub https://github.com/oleg68/GrandOrgue/issues/6
# 0.3.1.2338-1.os (2021-04-17)
- Capability of passing `-DVERSION` and `-DBERSION_BUILD` to cmake #4
- Simplified build from source with prepared shell scripts
- Automatic build linux `tar.gz`, osx `dmg` and windows `exe` packages on GitHub
- Allowed importing combination after moving a stop to another manual or deleting a stop in ODF https://sourceforge.net/p/ourorgan/feature-requests/90/
- Capability of scaling out GrandOrgue pannels over their initial size https://sourceforge.net/p/ourorgan/feature-requests/89/
- Added capability of specifying debounce time for BxController midi events https://sourceforge.net/p/ourorgan/bugs/72/
- Added support of Jack Native sound output https://sourceforge.net/p/ourorgan/feature-requests/52/

    Use ``Jack Output`` device in ``Audio/Midi Settings``-> ``Audio Output``

- Added project for editing and building with Netbeans IDE
# 0.3.1.2338 (2021-01-05)
- Remove old limits
# 0.3.1.2337 (2020-07-18)
- Added settings for font sizes in buttons and labels of built-in panels
# 0.3.1.2336 (2020-07-04)
- Auto down-sizing of large panels and improved panel scaling
# 0.3.1.2335 (2020-07-04)
- Corrected number of allowed BitmapCount for enclosures from 127 to 128
# 0.3.1.2334 (2020-06-22)
- Support sequencer label on new style pannels
# 0.3.1.2333 (2019-11-10)
- Move memory into thread pool
# 0.3.1.2330 (2019-07-24)
- Store negative window coordinates
# 0.3.1.2329 (2019-07-22)
- Build arch using wxgtk3
# 0.3.1.2328 (2019-07-22)
- Suggest wxWidgets translation
# 0.3.1.2327 (2019-07-22)
- Compress settings
# 0.3.1.2326 (2019-07-22)
- Update HU translation
# 0.3.1.2325 (2019-07-22)
- Fix enclosure bug ()
# 0.3.1.2324 (2019-06-05)
- Add Hungarian translation
# 0.3.1.2323 (2019-05-04)
- Ahlborn-Galanti-Sysex
# 0.3.1.2322 (2019-04-22)
- HW message format supports 2^14 LCD displays
# 0.3.1.2321 (2019-03-31)
- Use newer wxWidgets for OS X
# 0.3.1.2320 (2019-03-31)
- Update Windows build environment
# 0.3.1.2319 (2019-03-29)
- Remove unused templates
# 0.3.1.2318 (2019-03-29)
- bad MIDI_SYSEX_HW_STRING message header (JBE)
# 0.3.1.2317 (2019-03-29)
- wrong label value over midi (JBE)
# 0.3.1.2316 (2019-02-06)
- Fix cmake call in rpm build
# 0.3.1.2315 (2019-01-13)
- Allow building with gtk3
# 0.3.1.2314 (2019-01-13)
- Add spacing to the toolbar
# 0.3.1.2313 (2019-01-01)
- Preliminary rodgers support
- based on Keith Packard patch
# 0.3.1.2312 (2019-01-01)
- produce reproducible organ package
# 0.3.1.2311 (2019-01-01)
- Update copyright
# 0.3.1.2310 (2019-01-01)
- Add organ package creation tool
# 0.3.1.2309 (2019-01-01)
- Fix wavpack bugs
# 0.3.1.2308 (2019-01-01)
- Fix ZIP implementation bugs
# 0.3.1.2307 (2018-12-31)
- orgue Archive creator
# 0.3.1.2306 (2018-12-31)
- Add support for writing wav-Files
# 0.3.1.2305 (2018-12-31)
- Add wavpack compression wrapper
# 0.3.1.2304 (2018-12-31)
- Drop old wavpack source copy
# 0.3.1.2303 (2018-12-29)
- Split brew wxmac install
# 0.3.1.2302 (2018-12-29)
- Fix incorrect object number limits
# 0.3.1.2301 (2018-11-24)
- Improve toolbar sizing issues
# 0.3.1.2300 (2018-10-26)
- Store appdata according to new standard
# 0.3.1.2299 (2018-10-26)
- Don't encode icon typ in desktop file
# 0.3.1.2298 (2018-10-26)
- Align reload behaviour with reload via MID
# 0.3.1.2297 (2018-10-16)
- Allow loading first favorite at startup
# 0.3.1.2296 (2018-10-08)
- Introduce GOrgueChoice
# 0.3.1.2295 (2018-05-18)
- Make dialogs resizeable
- Slightly reworked version of Keith Packard patch
# 0.3.1.2294 (2018-03-29)
- Fix MIDI event length
# 0.3.1.2293 (2018-03-14)
- Fix perftest
# 0.3.1.2292 (2018-02-17)
- Fix warnings
# 0.3.1.2291 (2018-02-16)
- Fix debian build dependencies
# 0.3.1.2290 (2018-02-05)
- Fix build
# 0.3.1.2289 (2018-02-04)
- Add missing files
# 0.3.1.2288 (2018-02-04)
- Allow more bit per sample settings
# 0.3.1.2287 (2018-02-04)
- Fix transpose init
# 0.3.1.2286 (2018-02-04)
- Don't report load errors on not loaded pipes
# 0.3.1.2285 (2018-02-04)
- Allow to display multiple values on a LCD display
# 0.3.1.2284 (2018-02-02)
- Allow reading compressed data
# 0.3.1.2283 (2018-02-02)
- Allow sending button/enclosure state via HW SYSEX
# 0.3.1.2282 (2018-02-02)
- Remove not necessary include
# 0.3.1.2281 (2018-02-02)
- Support reading more bit depth
# 0.3.1.2280 (2018-01-31)
- Use GOrgueBuffer for WAV-reading
# 0.3.1.2279 (2018-01-31)
- Sample data access functions
# 0.3.1.2278 (2018-01-31)
- Use constants for RIFF chunk types
# 0.3.1.2277 (2018-01-31)
- ArchiveWriter class
# 0.3.1.2276 (2018-01-31)
- Move GOrgueArchiveManager
# 0.3.1.2275 (2018-01-31)
- Allow GOrgueSetting without setting store
# 0.3.1.2274 (2018-01-31)
- Allow sending object name via HW SYSEX
# 0.3.1.2273 (2018-01-31)
- Use GOrgueBuffer
# 0.3.1.2272 (2018-01-31)
- Use GOrgueBuffer for BitmapCache
# 0.3.1.2271 (2018-01-31)
- Use GOrgueBuffer for ConfigFileReader
# 0.3.1.2270 (2018-01-31)
- Use GOrgueBuffer for MidiFileReader
# 0.3.1.2269 (2018-01-31)
- Use GOrgueBuffer for config writing
# 0.3.1.2268 (2018-01-31)
- Show state of all output channels
# 0.3.1.2267 (2018-01-30)
- Move organ/archive list
# 0.3.1.2266 (2018-01-30)
- Move archive reader
# 0.3.1.2265 (2018-01-30)
- Add compression functions
# 0.3.1.2264 (2018-01-30)
- Make GOrgueInt read const
# 0.3.1.2263 (2018-01-30)
- Implement GOrgueFile::Read
# 0.3.1.2262 (2018-01-30)
- Move MIDIList
# 0.3.1.2261 (2018-01-30)
- Move GOrgueView
# 0.3.1.2260 (2018-01-30)
- Improve reverb error message
# 0.3.1.2259 (2018-01-30)
- Improve reverb dialog feedback
# 0.3.1.2258 (2018-01-30)
- Use non-overlapping values for internal image names
# 0.3.1.2257 (2018-01-29)
- Use const pointers for sample data
# 0.3.1.2256 (2018-01-29)
- Add GOrgueBuffer
# 0.3.1.2255 (2018-01-29)
- Check CRC of organ packages
# 0.3.1.2254 (2018-01-29)
- Move files
# 0.3.1.2253 (2018-01-29)
- Move GrandOrgueDef.h
# 0.3.1.2252 (2018-01-28)
- Update copyright 2018
# 0.3.1.2251 (2017-12-24)
- Remove not necessary file
# 0.3.1.2250 (2017-12-24)
- Fix wx3.1 build error
# 0.3.1.2249 (2017-12-23)
- Remove fftw sources
# 0.3.1.2248 (2017-12-08)
- Don't strip debug info
# 0.3.1.2247 (2017-11-13)
- Convert main panel to normal panel
# 0.3.1.2246 (2017-11-13)
- DisplayKey999Note
# 0.3.1.2245 (2017-11-13)
- Support XDG compilant paths on wx3.1
# 0.3.1.2244 (2017-11-03)
- Fix release selection
# 0.3.1.2243 (2017-10-29)
- Simplify bitmap selection code
# 0.3.1.2242 (2017-07-10)
- Don't draw outside of paint events
# 0.3.1.2241 (2017-06-14)
- Fix crash, if no MIDI file is loaded ()
# 0.3.1.2240 (2017-06-02)
- Use GOrgueAlloc
# 0.3.1.2239 (2017-06-02)
- Use GOrgueAlloc
# 0.3.1.2238 (2017-05-29)
- Use GOrgueAlloc
# 0.3.1.2237 (2017-05-28)
- Use GOrgueAlloc
# 0.3.1.2236 (2017-05-28)
- Use GOrgueAlloc for MidiReader
# 0.3.1.2235 (2017-05-15)
- Fix effect stop initialisation
# 0.3.1.2234 (2017-04-16)
- Fix OS X paths
# 0.3.1.2233 (2017-04-16)
- OS X build fix
# 0.3.1.2232 (2017-04-16)
- Support different attacks for fast pipe retriggering
# 0.3.1.2231 (2017-04-16)
- Increase banked general count
# 0.3.1.2230 (2017-04-01)
- Fix CMake install
# 0.3.1.2229 (2017-03-22)
- Move AppData
# 0.3.1.2228 (2017-03-22)
- Convert man-page to Docbook
# 0.3.1.2227 (2017-03-22)
- Add desktop translation support
# 0.3.1.2226 (2017-03-22)
- Move resource files
# 0.3.1.2225 (2017-03-17)
- Keep track of time between key presses
# 0.3.1.2224 (2017-03-12)
- Update Copyright
# 0.3.1.2223 (2017-02-23)
- Fix Johannus SYSEX detection
# 0.3.1.2222 (2017-02-07)
- Rework MIDI player
# 0.3.1.2221 (2017-02-07)
- Fix documentation
# 0.3.1.2220 (2017-02-07)
- Add accessible/debug interface to GUI elemenets
# 0.3.1.2219 (2017-02-07)
- Add element status interface
# 0.3.1.2218 (2017-01-14)
- Implement Johannus 11 byte SYSEX
# 0.3.1.2217 (2016-12-09)
- Use correct wavpack include directory
# 0.3.1.2216 (2016-12-09)
- Keep resolved values
# 0.3.1.2215 (2016-12-09)
- Workaround for cmake GetPrerequisties bug
# 0.3.1.2214 (2016-12-04)
- Remove wx2.8 code
# 0.3.1.2213 (2016-12-04)
- Prepare for a separate cache directory
# 0.3.1.2212 (2016-12-04)
- Fix reverb threading issues
# 0.3.1.2211 (2016-12-04)
- Use C++11 threads
# 0.3.1.2210 (2016-12-03)
- Remove unsued include
# 0.3.1.2209 (2016-12-03)
- Fix file location
# 0.3.1.2208 (2016-12-03)
- Move more files
# 0.3.1.2207 (2016-12-03)
- Prepare core library
# 0.3.1.2206 (2016-12-01)
- Add MIDI file content storage object
# 0.3.1.2205 (2016-12-01)
- separate SYSEX from manual MIDI assignment
# 0.3.1.2204 (2016-11-25)
- Remove unreferenced cache files
# 0.3.1.2203 (2016-11-25)
- Add debian source format version
# 0.3.1.2202 (2016-11-25)
- Move organ hash generation
# 0.3.1.2201 (2016-11-19)
- Fix warnings
# 0.3.1.2200 (2016-11-15)
- Improve debian metadata
# 0.3.1.2199 (2016-11-15)
- Refactor
# 0.3.1.2198 (2016-11-13)
- Fix typo
# 0.3.1.2197 (2016-11-12)
- Fix warnings
# 0.3.1.2196 (2016-11-12)
- Remove dllcopy stamp file
# 0.3.1.2195 (2016-11-12)
- Add missing fileAdd missing filess
# 0.3.1.2194 (2016-11-12)
- Refactor library building code
# 0.3.1.2193 (2016-11-12)
- Use default rpath for Apple packages
# 0.3.1.2192 (2016-11-12)
- Fix syntax
# 0.3.1.2191 (2016-11-12)
- Fix OS X sed syntax
# 0.3.1.2190 (2016-11-12)
- Create matching version on bintray
# 0.3.1.2189 (2016-11-12)
- Update Standards-Version
# 0.3.1.2188 (2016-11-10)
- Unlink cmake before installing
# 0.3.1.2187 (2016-11-08)
- Split image library
# 0.3.1.2186 (2016-11-08)
- Allow copying receive events to send events
# 0.3.1.2185 (2016-11-06)
- Move bin-Directory path specification
# 0.3.1.2184 (2016-10-30)
- Preinstall pkg-config before jack
# 0.3.1.2183 (2016-10-30)
- Adjust default build options
# 0.3.1.2182 (2016-10-30)
- Move optimization options
# 0.3.1.2181 (2016-10-30)
- Move GO image library directory
# 0.3.1.2180 (2016-10-30)
- Fix dll dependency copy
# 0.3.1.2179 (2016-10-30)
- Don't use different build directories
# 0.3.1.2178 (2016-10-30)
- Use simple, standard wxWidgets detection
# 0.3.1.2177 (2016-10-29)
- Introduce include directory variables for all libraries
# 0.3.1.2176 (2016-10-29)
- Disable the splash screen
- It just delays the GO startup
# 0.3.1.2175 (2016-10-29)
- Default to linear interpolation
- Linear allows to handle more polyphony
# 0.3.1.2174 (2016-10-26)
- Change device name hashing
- * Remove the bus number in all USB device IDs
- * Renumber all USB bus entries for controllers starting with 0
- /sys/devices/pci0000:00/0000:00:14.0/usb3/3-1/3-1.2/3-1.2:1.0/sound/card4
- will be hashed as
- /sys/devices/pci0000:00/0000:00:14.0/usb1/-1/-1.2/-1.2:1.0/sound/
- [Assuming, that there is an /sys/devices/pci0000:00/0000:00:14.0/usb2 entry]
# 0.3.1.2173 (2016-10-18)
- Conflict with old grandorgue packaging
# 0.3.1.2172 (2016-10-14)
- Don't hardcode SpinCtrl sizes
# 0.3.1.2171 (2016-10-01)
- Fix OBS dependencies
# 0.3.1.2170 (2016-10-01)
- Fix reinstall issues
# 0.3.1.2169 (2016-10-01)
- Fix stable device IDs
# 0.3.1.2168 (2016-09-22)
- Deprecate internal wavpack
# 0.3.1.2167 (2016-09-19)
- Start reorganizing README
# 0.3.1.2166 (2016-09-19)
- Deprecate our own FFTW copy
# 0.3.1.2165 (2016-09-19)
- Fix mingw dependencies
# 0.3.1.2164 (2016-09-13)
- Fix default build options
# 0.3.1.2163 (2016-09-13)
- Fix build errors
# 0.3.1.2162 (2016-09-12)
- Split resources into a separate package
# 0.3.1.2161 (2016-09-12)
- Create portable windows package
# 0.3.1.2160 (2016-09-12)
- Fix metronome minus key wrapping
# 0.3.1.2159 (2016-09-09)
- Fix release processor dependencies
# 0.3.1.2158 (2016-09-09)
- Display more sample statistic
# 0.3.1.2157 (2016-09-09)
- Remove workaround for GCC 4.6
# 0.3.1.2156 (2016-09-09)
- Enable fast-math
# 0.3.1.2155 (2016-09-09)
- Remove old GOInt24 type
# 0.3.1.2154 (2016-09-09)
- Use GOInt for wave access
# 0.3.1.2153 (2016-09-09)
- Reduce number of screen updates
# 0.3.1.2152 (2016-09-09)
- Use better label for the concurrency options
# 0.3.1.2151 (2016-09-06)
- Include cleanup
# 0.3.1.2150 (2016-09-06)
- Display memory usage in the organ dialog
# 0.3.1.2149 (2016-09-02)
- Use GOInt for ZIP reading
# 0.3.1.2148 (2016-09-01)
- Use GOInt for MIDI files
# 0.3.1.2147 (2016-08-31)
- Add new integer types
# 0.3.1.2146 (2016-08-31)
- Don't pass HW LCD strings via MIDI map
# 0.3.1.2145 (2016-08-30)
- Raise incorrect release concurrency level
# 0.3.1.2144 (2016-08-11)
- Document OS X building
# 0.3.1.2143 (2016-08-04)
- Fix OS X bundle
# 0.3.1.2142 (2016-08-04)
- Add msw wxWidgets fix
# 0.3.1.2141 (2016-07-21)
- Update french translation
# 0.3.1.2140 (2016-07-19)
- Help Update: Document On/Off Toggle Received MIDI events
# 0.3.1.2139 (2016-07-19)
- Help Update: minor typo and range value fixes
# 0.3.1.2138 (2016-07-15)
- Cleanup Thread
# 0.3.1.2137 (2016-07-07)
- Help Update: Add Cx Program Change Range to Enclosure MIDI Editor
# 0.3.1.2136 (2016-06-24)
- Help Update: Paneled recorder, document file switching while recording
# 0.3.1.2135 (2016-06-23)
- Help Update: Fix Organ Packages
# 0.3.1.2134 (2016-06-23)
- Help Update: Enclosure keyboard shortcut
# 0.3.1.2133 (2016-06-23)
- Use GOMessageBox
# 0.3.1.2132 (2016-06-23)
- Fix variable init
# 0.3.1.2131 (2016-06-22)
- Unify filenames
# 0.3.1.2130 (2016-06-21)
- Help Update: document windchest-level settings in Organ Properties
# 0.3.1.2129 (2016-06-21)
- Help Update: Paneled Audio and Midi recorders
# 0.3.1.2128 (2016-06-21)
- Help update: Organ packages
# 0.3.1.2127 (2016-06-19)
- Audio and Midi recording filenames: fix forbidden ':' in Windows
# 0.3.1.2126 (2016-06-17)
- Show recording time
# 0.3.1.2125 (2016-06-17)
- Switch metronome to GOrgueTimer
# 0.3.1.2124 (2016-06-17)
- Add timer infrastructure
# 0.3.1.2123 (2016-06-13)
- Panel based audio recorder
# 0.3.1.2122 (2016-06-08)
- Implement per windchest user tuning option
# 0.3.1.2121 (2016-05-11)
- Fix arch build
# 0.3.1.2120 (2016-05-11)
- Fix windows build
# 0.3.1.2119 (2016-05-10)
- Fix debian packaging
# 0.3.1.2118 (2016-05-08)
- Add OS X build files
# 0.3.1.2117 (2016-05-08)
- Fix cpack configuration
# 0.3.1.2116 (2016-05-08)
- Fix OS X copy depedencies @ install time
# 0.3.1.2115 (2016-05-08)
- Fix perftest
# 0.3.1.2114 (2016-05-07)
- Fix OS X dependeny handling
# 0.3.1.2113 (2016-05-05)
- Support shortcuts for enclosures
# 0.3.1.2112 (2016-05-05)
- Add on/off toogle MIDI match types
# 0.3.1.2111 (2016-05-05)
- Introduce key receiver type
# 0.3.1.2110 (2016-05-05)
- Improve load management
# 0.3.1.2109 (2016-04-23)
- Drop releases, if the system is overloaded
# 0.3.1.2108 (2016-04-23)
- Protect cross-fade samples
# 0.3.1.2107 (2016-04-23)
- Cap release alignment calculations
# 0.3.1.2106 (2016-04-23)
- Add missing files
# 0.3.1.2105 (2016-04-23)
- Introduce stop signal
# 0.3.1.2104 (2016-04-23)
- Indicate last device
# 0.3.1.2103 (2016-04-21)
- Ignore after touch in no-velocity mode
# 0.3.1.2102 (2016-04-15)
- Raise minimum concurrency level to 1
# 0.3.1.2101 (2016-04-15)
- Move release handling
# 0.3.1.2100 (2016-04-12)
- Use repeat flag in the scheduler
# 0.3.1.2099 (2016-04-12)
- Add repeat field
# 0.3.1.2098 (2016-04-09)
- Refactor sampler list handling
# 0.3.1.2097 (2016-04-07)
- Fix release crossfade length
# 0.3.1.2096 (2016-04-05)
- Document crossfade length unit
# 0.3.1.2095 (2016-04-03)
- Make release cross fade length configureable
# 0.3.1.2094 (2016-03-18)
- Add support for stable USB device ID
- Requires Linux kernel >= 4.6-rc1 and alsa-lib >= 1.1.1
# 0.3.1.2093 (2016-02-27)
- Fix GTK shotcut focus problem
# 0.3.1.2092 (2016-02-04)
- Fix manual bounding box
# 0.3.1.2091 (2016-02-03)
- Fix PKGBUILD
# 0.3.1.2090 (2016-01-31)
- Limit initial window size to the screen size
# 0.3.1.2089 (2016-01-30)
- Refactor mutex
# 0.3.1.2088 (2016-01-30)
- Refactor atomic support
# 0.3.1.2087 (2016-01-30)
- Require c++11
# 0.3.1.2086 (2016-01-29)
- Update copyright
# 0.3.1.2085 (2016-01-29)
- Remove old compiler workaround
# 0.3.1.2084 (2016-01-19)
- ASIO Workarounds
# 0.3.1.2083 (2016-01-17)
- Fix warnings
# 0.3.1.2082 (2016-01-17)
- Switch to panel based MIDI recorder
# 0.3.1.2081 (2016-01-16)
- Small corrections in the GO file format help
# 0.3.1.2080 (2016-01-16)
- Fix InfoFilename description
# 0.3.1.2079 (2016-01-09)
- Fix organ package related InfoFilename warnings
# 0.3.1.2078 (2016-01-08)
- Add support for "Info-ZIP New Unix Extra Field" (0x7875)
# 0.3.1.2077 (2016-01-08)
- Don't record stop changes for other samplesets
# 0.3.1.2076 (2016-01-08)
- Add rename event
# 0.3.1.2075 (2016-01-08)
- Unify button create
# 0.3.1.2074 (2016-01-08)
- Unify button feedback
# 0.3.1.2073 (2016-01-07)
- Unify GetButton
# 0.3.1.2072 (2016-01-07)
- Add clear command
# 0.3.1.2071 (2016-01-03)
- separate event mapping
# 0.3.1.2070 (2016-01-03)
- Unify event writing
# 0.3.1.2069 (2016-01-03)
- Add sampleset SYSEX
# 0.3.1.2068 (2016-01-03)
- Fix GO sysex handling
# 0.3.1.2067 (2016-01-02)
- Fix MIDI monitor
# 0.3.1.2066 (2015-12-25)
- Fix typo
# 0.3.1.2065 (2015-12-22)
- Help update: Master controls panel readiness indicators and sound output
- state display
# 0.3.1.2064 (2015-12-20)
- Update README
# 0.3.1.2063 (2015-12-20)
- GrandOrgue isn't "Education"
# 0.3.1.2062 (2015-12-20)
- Add support for localized images
# 0.3.1.2061 (2015-12-20)
- Don't delete non-existing file
# 0.3.1.2060 (2015-12-20)
- Update AUTHORS
# 0.3.1.2059 (2015-12-20)
- Add Spanish translation
# 0.3.1.2058 (2015-12-15)
- Fix perftest
# 0.3.1.2057 (2015-12-15)
- Add a simple midi monitor to GO
# 0.3.1.2056 (2015-12-09)
- Update French translation
# 0.3.1.2055 (2015-12-07)
- Update of Swedish translation
# 0.3.1.2054 (2015-12-06)
- Fix install dir
# 0.3.1.2053 (2015-12-06)
- Fix build error
# 0.3.1.2052 (2015-12-06)
- Include archive ID in demo set name
# 0.3.1.2051 (2015-12-06)
- Remove old demo set from RPM/DEB
# 0.3.1.2050 (2015-12-06)
- Remove unpacked demo set
# 0.3.1.2049 (2015-12-06)
- Remove LastFile setting
# 0.3.1.2048 (2015-12-06)
- Scan for organ packages
# 0.3.1.2047 (2015-12-05)
- Improve modified organ package detection
# 0.3.1.2046 (2015-12-05)
- Document organ package format
# 0.3.1.2045 (2015-12-05)
- Better handle change orgue files
# 0.3.1.2044 (2015-12-05)
- Add demo organ package
# 0.3.1.2043 (2015-12-04)
- Refactor organ package install
# 0.3.1.2042 (2015-12-04)
- Show organ package list
# 0.3.1.2041 (2015-12-04)
- Document 32 bit limits
# 0.3.1.2040 (2015-12-04)
- Associate GrandOrgue with .orgue files
# 0.3.1.2039 (2015-12-04)
- Check for different organ metadata
# 0.3.1.2038 (2015-12-04)
- Check for duplicate filenames
# 0.3.1.2037 (2015-12-04)
- Fix compile error
# 0.3.1.2036 (2015-12-04)
- Show archive in organ select dialog
# 0.3.1.2035 (2015-12-03)
- Improve scaling quality
# 0.3.1.2034 (2015-12-03)
- Prescale bitmaps
# 0.3.1.2033 (2015-12-02)
- Fix memory free
# 0.3.1.2032 (2015-12-02)
- Always use alpha instead of a mask
# 0.3.1.2031 (2015-12-02)
- Remove TileWood
# 0.3.1.2030 (2015-12-02)
- Use GetWood
# 0.3.1.2029 (2015-12-02)
- Use GetWood
# 0.3.1.2028 (2015-12-02)
- Add GetWood function
# 0.3.1.2027 (2015-12-02)
- Add Pre-Draw Hook
# 0.3.1.2026 (2015-11-25)
- Show organ package in the organ list
# 0.3.1.2025 (2015-11-24)
- Allow configuration of the organ package directory
# 0.3.1.2024 (2015-11-24)
- Archive management interface
# 0.3.1.2023 (2015-11-24)
- Prepare organ package install command
# 0.3.1.2022 (2015-11-24)
- Add organ packages directory
# 0.3.1.2021 (2015-11-24)
- Move sound reset
# 0.3.1.2020 (2015-11-23)
- Include packages directory in deb/rpm
# 0.3.1.2019 (2015-11-23)
- Ship packages directory
# 0.3.1.2018 (2015-11-23)
- Add support for loading organ from a archive
# 0.3.1.2017 (2015-11-22)
- Cleanup demo organ file path
# 0.3.1.2016 (2015-11-22)
- Add GOrgueArchiveManager
# 0.3.1.2015 (2015-11-22)
- Use recent used organ list for loading the last used organ
# 0.3.1.2014 (2015-11-22)
- Support reading files from archives
# 0.3.1.2013 (2015-11-22)
- Add GOrgueArchiveEntryFile
# 0.3.1.2012 (2015-11-22)
- Store archive list in GrandOrgueFile
# 0.3.1.2011 (2015-11-21)
- Add GOrgueArchive
# 0.3.1.2010 (2015-11-21)
- Add zip parser
# 0.3.1.2009 (2015-11-21)
- Add zip file format definitions
# 0.3.1.2008 (2015-11-21)
- Add class for caching the archiv index
# 0.3.1.2007 (2015-11-20)
- Update of Swedish translation
# 0.3.1.2006 (2015-11-19)
- Allow association GOrgueOrgan with GOrgueArchiveFile
# 0.3.1.2005 (2015-11-18)
- Fix label
# 0.3.1.2004 (2015-11-17)
- Add GOrgueArchiveFile
# 0.3.1.2003 (2015-11-14)
- Load organs via GOrgueOrgan
# 0.3.1.2002 (2015-11-14)
- Add support for getting the currenty displayed organs
# 0.3.1.2001 (2015-11-14)
- Don't offer missing organs to the user
# 0.3.1.2000 (2015-11-14)
- We are case sensitive by default
# 0.3.1.1999 (2015-11-14)
- open ODF via GOrgueFilename
# 0.3.1.1998 (2015-11-14)
- Add method for returing the organ path displayed to the user
# 0.3.1.1997 (2015-11-14)
- Pass organ via GOrgueOrgan to LRU list
# 0.3.1.1996 (2015-11-14)
- Refactor setting/cache filename generation
# 0.3.1.1995 (2015-11-14)
- Return GOrgueHashType
# 0.3.1.1994 (2015-11-04)
- Detect modified samples
# 0.3.1.1993 (2015-11-02)
- Use better rescaling method
# 0.3.1.1992 (2015-11-02)
- UNICODE=1 is already set by default
# 0.3.1.1991 (2015-10-31)
- Adapt windows builds for a recent cmake
# 0.3.1.1990 (2015-10-31)
- Fix build
# 0.3.1.1989 (2015-10-30)
- Move OS X bundle support to drag&drop
# 0.3.1.1988 (2015-10-30)
- Add support for revered value ranges in MIDI messages
# 0.3.1.1987 (2015-10-30)
- Help Update: Audio backends
# 0.3.1.1986 (2015-10-30)
- Help Update: MIDI usage notes
# 0.3.1.1985 (2015-10-28)
- Run fixup as external process
# 0.3.1.1984 (2015-10-28)
- Don't hardcode the project name
# 0.3.1.1983 (2015-10-28)
- Don't hardcode the project name
# 0.3.1.1982 (2015-10-28)
- Remove not necessary copy
# 0.3.1.1981 (2015-10-28)
- Unify version handling
# 0.3.1.1980 (2015-10-28)
- Fix addoption
# 0.3.1.1979 (2015-10-28)
- Fix escaping
# 0.3.1.1978 (2015-10-26)
- Simplify windows resource file handling
# 0.3.1.1977 (2015-10-24)
- Fix patch
# 0.3.1.1976 (2015-10-24)
- Handle definditions correctly
# 0.3.1.1975 (2015-10-24)
- Get rid of windres workaround and fix compiler option definition
# 0.3.1.1974 (2015-10-24)
- Rework dll depenency installation
# 0.3.1.1973 (2015-10-24)
- Use fixed test runtime
# 0.3.1.1972 (2015-10-24)
- cmake 2.8 is still popular (and 3.0 is not really yet necessary)
# 0.3.1.1971 (2015-10-21)
- Help Update: Panels / Combination setter
# 0.3.1.1970 (2015-10-15)
- Current cmake does not require a patches GetPrerequisites any more
# 0.3.1.1969 (2015-10-11)
- Move translation bundeling macros
# 0.3.1.1968 (2015-10-11)
- Unify manifests
# 0.3.1.1967 (2015-10-08)
- Fix windows link error
# 0.3.1.1966 (2015-10-02)
- Document UTF-8 support
# 0.3.1.1965 (2015-10-01)
- Report reversible piston connect to a read-only drawstop
# 0.3.1.1964 (2015-09-29)
- Help Update: add missing png file
# 0.3.1.1963 (2015-09-24)
- Help Update: Panels/Crescendo pedal
# 0.3.1.1962 (2015-09-23)
- Use libjack.dylib for OS X
- Homebrew ships libjack
- JackOSX ships according to its docs also libjack.
- Jack2 sources indicat, that it uses libjack too.
# 0.3.1.1961 (2015-09-23)
- Move linker option macro
# 0.3.1.1960 (2015-09-23)
- Move cmake external libary handling
# 0.3.1.1959 (2015-09-23)
- Separate cmake image conversion macros
# 0.3.1.1958 (2015-09-23)
- Refactor cmake po macros
# 0.3.1.1957 (2015-09-19)
- Refactor cmake help macros
# 0.3.1.1956 (2015-09-18)
- Enable Unicode by default
# 0.3.1.1955 (2015-09-18)
- Check for a missing release
# 0.3.1.1954 (2015-09-18)
- Move help images into a subdirectory
# 0.3.1.1953 (2015-09-17)
- Use "make runperftest" instead of the reserved "make test"
# 0.3.1.1952 (2015-09-17)
- Add copyright header
# 0.3.1.1951 (2015-09-17)
- Unify cmake file copy macros
# 0.3.1.1950 (2015-09-17)
- Migrate cmake macros to functions
# 0.3.1.1949 (2015-09-17)
- Remove old directory
# 0.3.1.1948 (2015-09-16)
- Unify mo file installation
# 0.3.1.1947 (2015-09-16)
- Unify installing files into the resource dirs
# 0.3.1.1946 (2015-09-01)
- Fix default latency initialisation
# 0.3.1.1945 (2015-08-27)
- Report missing rank setting: FirstMidiNoteNumber
# 0.3.1.1944 (2015-08-24)
- Fix OS X type overloading issues
# 0.3.1.1943 (2015-08-24)
- Fix empty list cmake error
# 0.3.1.1942 (2015-08-12)
- Fix 32 bit builds
# 0.3.1.1941 (2015-08-06)
- Use wxGTK3 for openSuSE
# 0.3.1.1940 (2015-08-06)
- Avoid divde by zero
# 0.3.1.1939 (2015-08-06)
- Use GOrgueFile for resource access
# 0.3.1.1938 (2015-08-06)
- Add support for GOrgueFile to the config reader
# 0.3.1.1937 (2015-08-06)
- Support changing buttons in multiple GO windows with one mouse press
# 0.3.1.1936 (2015-08-06)
- Refactor mouse event handling
# 0.3.1.1935 (2015-08-06)
- Allow storing of a event ID
# 0.3.1.1934 (2015-07-29)
- Help Update: Panels / Divisionals
- Conflicts:
- 	help/grandorgue.xml
# 0.3.1.1933 (2015-07-29)
- Help Update: Panels / Generals
# 0.3.1.1932 (2015-07-29)
- Help Update: Panels / Regular, Scope & Scoped, Full
# 0.3.1.1931 (2015-07-27)
- Add file wrapper classes
# 0.3.1.1930 (2015-07-27)
- Refactor hash handling
# 0.3.1.1929 (2015-06-24)
- Rework actual latency display
# 0.3.1.1928 (2015-06-24)
- Allow desired latency of 0
# 0.3.1.1927 (2015-06-24)
- Honor desired latency for RtAudio too
# 0.3.1.1926 (2015-06-24)
- Migrate desired latency setting
# 0.3.1.1925 (2015-06-24)
- Proofreading by Mick Berg
# 0.3.1.1924 (2015-06-24)
- Fix assert
# 0.3.1.1923 (2015-06-23)
- Ensure, that an audio group exists
# 0.3.1.1922 (2015-06-23)
- Fix splash screen
# 0.3.1.1921 (2015-06-16)
- Switch rpm packages to wx3.0
# 0.3.1.1920 (2015-06-09)
- Proofreading by Mick Berg
# 0.3.1.1919 (2015-05-28)
- Downmix increases CPU usage
# 0.3.1.1918 (2015-05-28)
- Proofreading by Mick Berg
# 0.3.1.1917 (2015-05-25)
- Fix build
# 0.3.1.1916 (2015-05-25)
- Use wx3.0 for the debian packages
# 0.3.1.1915 (2015-05-25)
- Use wx3.0 for Arch Linux packages
# 0.3.1.1914 (2015-05-22)
- Update Windows build instructions
# 0.3.1.1913 (2015-05-22)
- Fix return type
# 0.3.1.1912 (2015-05-21)
- Fix package name
# 0.3.1.1911 (2015-05-21)
- Build windows rpm with wx3.0
# 0.3.1.1910 (2015-05-21)
- Fix wx2.8 build
# 0.3.1.1909 (2015-05-21)
- Help Update: Default and initial settings in Audio/Midi settings menu
# 0.3.1.1908 (2015-05-17)
- Fix encoding issues for GO settings
# 0.3.1.1907 (2015-05-17)
- Build fix for wx 3.0.2
# 0.3.1.1906 (2015-05-17)
- Fix coupler manuals
# 0.3.1.1905 (2015-05-11)
- Help Update: Panels / Coupler Manuals and Volume
# 0.3.1.1904 (2015-05-06)
- Help Update: Add stereo mix checkbox to options tab
# 0.3.1.1903 (2015-04-29)
- Help Update: Organ settings
- Add Collapse Tree
# 0.3.1.1902 (2015-04-20)
- Initial MIDI for the metronome
# 0.3.1.1901 (2015-04-20)
- Align volume handling
# 0.3.1.1900 (2015-04-19)
- Default values for the metronome
# 0.3.1.1899 (2015-04-19)
- Use wx functions for the extension handling
# 0.3.1.1898 (2015-04-19)
- Config options for various directories
# 0.3.1.1897 (2015-04-19)
- Remove reading outdated global options
# 0.3.1.1896 (2015-04-09)
- Fix pattern
# 0.3.1.1895 (2015-04-09)
- Remove GetConfig function
# 0.3.1.1894 (2015-04-09)
- Migrate string settings
# 0.3.1.1893 (2015-04-09)
- Add file/directory setting
# 0.3.1.1892 (2015-04-09)
- Add string setting
# 0.3.1.1891 (2015-04-09)
- Introduce validate hook
# 0.3.1.1890 (2015-04-09)
- Ship translations in the debian packages
# 0.3.1.1889 (2015-03-28)
- Fix perftest
# 0.3.1.1888 (2015-03-23)
- Migrate setting handling
# 0.3.1.1887 (2015-03-23)
- Helper classes for handling settings
# 0.3.1.1886 (2015-03-20)
- Help Update: Fix Metronome
# 0.3.1.1885 (2015-03-16)
- Fix audio recorder
# 0.3.1.1884 (2015-03-14)
- Help Update: Panels / Metronome
# 0.3.1.1883 (2015-03-14)
- Ship all binaries
# 0.3.1.1882 (2015-03-10)
- Fix scheduler
# 0.3.1.1881 (2015-03-07)
- Remove unused methods
# 0.3.1.1880 (2015-03-07)
- Remove unused code
# 0.3.1.1879 (2015-03-07)
- Refactor setter GUI
# 0.3.1.1878 (2015-03-07)
- Don't route events during cleanup
# 0.3.1.1877 (2015-03-04)
- Temporary workaround for condition issue
# 0.3.1.1876 (2015-03-03)
- Fix race condition
# 0.3.1.1875 (2015-03-01)
- Split metronom GUI
# 0.3.1.1874 (2015-03-01)
- Rename panel creator
# 0.3.1.1873 (2015-03-01)
- Add element creator
# 0.3.1.1872 (2015-03-01)
- Fix deadlock
# 0.3.1.1871 (2015-02-26)
- Remove old files
# 0.3.1.1870 (2015-02-26)
- Fix button type
# 0.3.1.1869 (2015-02-22)
- Change metronome beats
# 0.3.1.1868 (2015-02-22)
- Add a simple metronome
# 0.3.1.1867 (2015-02-22)
- Add stub sound files
# 0.3.1.1866 (2015-02-21)
- Ship sounds directory
# 0.3.1.1865 (2015-02-21)
- Add init to GOrgueSoundingPipe
# 0.3.1.1864 (2015-02-21)
- Create setter panels via panel creator
# 0.3.1.1863 (2015-02-21)
- Add resource directory support to GOCreateFilename
# 0.3.1.1862 (2015-02-21)
- store resource directory in GOrgueSettings
# 0.3.1.1861 (2015-02-21)
- Refactor coupler panel
# 0.3.1.1860 (2015-02-21)
- Refactor floating panel
# 0.3.1.1859 (2015-02-21)
- Refactor control changed handling
# 0.3.1.1858 (2015-02-21)
- Refactor playback state handling
# 0.3.1.1857 (2015-02-21)
- Fix layout calculation
# 0.3.1.1856 (2015-02-21)
- Don't export receiver objects
# 0.3.1.1855 (2015-02-21)
- Add panel creator hook
# 0.3.1.1854 (2015-02-21)
- Add into to rank
# 0.3.1.1853 (2015-02-21)
- Add init to windchest
# 0.3.1.1852 (2015-02-21)
- Add into to PipeConfig
# 0.3.1.1851 (2015-02-21)
- Fix enclosure label
# 0.3.1.1850 (2015-02-21)
- Improve error messages
# 0.3.1.1849 (2015-02-08)
- Fix shutdown bug
# 0.3.1.1848 (2015-02-07)
- Rework multi soundcard support
# 0.3.1.1847 (2015-02-07)
- Add Exec hook
# 0.3.1.1846 (2015-02-07)
- Group work items
# 0.3.1.1845 (2015-02-07)
- Refactor meter update
# 0.3.1.1844 (2015-02-07)
- Get rid of SAMPLE_HANDLE
# 0.3.1.1843 (2015-02-06)
- Refactor scheduling
# 0.3.1.1842 (2015-02-06)
- Build everything from one source package
# 0.3.1.1841 (2015-02-06)
- Prepare for passing build settings via rpm parameters
# 0.3.1.1840 (2015-01-31)
- Remove GO_SOUND_BUFFER_SIZE
# 0.3.1.1839 (2015-01-31)
- Add support for multi channel recoding
# 0.3.1.1838 (2015-01-30)
- Add clear/reset to the interface
# 0.3.1.1837 (2015-01-30)
- Calculate audio recorder stream via output work item
# 0.3.1.1836 (2015-01-30)
- Rework meter info
# 0.3.1.1835 (2015-01-30)
- add noretune option for effect stops
# 0.3.1.1834 (2015-01-30)
- Fix docbook bugs
# 0.3.1.1833 (2015-01-30)
- Fix volume setup
# 0.3.1.1832 (2015-01-29)
- Offload sound output generation
# 0.3.1.1831 (2015-01-29)
- Refactor sound buffer storage
# 0.3.1.1830 (2015-01-29)
- Store samples per buffer in the sound engine
# 0.3.1.1829 (2015-01-29)
- Fix SMP support
# 0.3.1.1828 (2015-01-27)
- Help Update: Panels / Coupler
# 0.3.1.1827 (2015-01-26)
- Fix latency calucation
# 0.3.1.1826 (2015-01-26)
- Don't report C of a 64' with overwritten pitch info
# 0.3.1.1825 (2015-01-10)
- mingw already ships dsound.h
# 0.3.1.1824 (2015-01-10)
- Adjust header to current mingw64
# 0.3.1.1823 (2015-01-10)
- Move event handling code
# 0.3.1.1822 (2015-01-10)
- Delay sound init
# 0.3.1.1821 (2015-01-10)
- Fix compression
# 0.3.1.1820 (2015-01-10)
- Update copyright
# 0.3.1.1819 (2014-12-20)
- Fix perftest
# 0.3.1.1818 (2014-12-12)
- Update demo organ
# 0.3.1.1817 (2014-12-12)
- Use N as default for Displayed
# 0.3.1.1816 (2014-12-12)
- Don't ask for saving, if reload is triggered via MIDI
# 0.3.1.1815 (2014-12-12)
- Report missing long release
# 0.3.1.1814 (2014-12-08)
- Fix no-release case
# 0.3.1.1813 (2014-12-07)
- Don't assume Displayed=Y for new panel format
# 0.3.1.1812 (2014-12-06)
- Allow configuartion of normal divisional/general content
# 0.3.1.1811 (2014-12-06)
- Refactor combination definition
# 0.3.1.1810 (2014-12-06)
- Use better term for our "floating" manuals
# 0.3.1.1809 (2014-12-06)
- Updated Swedish translation.
# 0.3.1.1808 (2014-12-05)
- Add scrollbar to option tab, if necessary
# 0.3.1.1807 (2014-12-05)
- Fix crash
# 0.3.1.1806 (2014-12-03)
- Fix sound engine cleanup
# 0.3.1.1805 (2014-12-02)
- Build fix
# 0.3.1.1804 (2014-12-01)
- Help Update: Panels / Master Controls
# 0.3.1.1803 (2014-11-29)
- Refactor sampler handling
# 0.3.1.1802 (2014-11-29)
- Rework windchest volume handling
# 0.3.1.1801 (2014-11-29)
- Restructure tremulant processing
# 0.3.1.1800 (2014-11-29)
- Restructure release creation code
# 0.3.1.1799 (2014-11-29)
- Add cross-frade suggest by Nick
# 0.3.1.1798 (2014-11-29)
- Allow sending sampleset name/ready via MIDI
# 0.3.1.1797 (2014-11-29)
- Allow overriding button image
# 0.3.1.1796 (2014-11-29)
- Allow sending modified state via MIDI
# 0.3.1.1795 (2014-11-29)
- Move GO_SOUND_BUFFER_SIZE
# 0.3.1.1794 (2014-11-29)
- Get rid of not necessary compression state
# 0.3.1.1793 (2014-11-23)
- Fix perftest
# 0.3.1.1792 (2014-11-15)
- Lower default desired latency
# 0.3.1.1791 (2014-11-15)
- Cache device list
# 0.3.1.1790 (2014-11-15)
- More RtAudio patches
# 0.3.1.1789 (2014-11-15)
- Merge new RtAudio version
# 0.3.1.1788 (2014-11-15)
- Merge RtAudio ASIO changes
# 0.3.1.1787 (2014-11-15)
- Merge RtAudio changes
# 0.3.1.1786 (2014-11-15)
- Merge new RtMidi error callback
# 0.3.1.1785 (2014-11-15)
- Merge RtMidi OS X changes
# 0.3.1.1784 (2014-11-15)
- RtMidi update
# 0.3.1.1783 (2014-11-15)
- Increase default polyphony limit with more CPUs
# 0.3.1.1782 (2014-11-11)
- Update demo organ
# 0.3.1.1781 (2014-11-11)
- Panel format 2.0
# 0.3.1.1780 (2014-11-11)
- Portaudio update
# 0.3.1.1779 (2014-11-11)
- Fix polyphase audio issues
# 0.3.1.1778 (2014-11-11)
- Use data before the loop start cross fading
# 0.3.1.1777 (2014-11-10)
- Patch for wxWidgets windows bug
# 0.3.1.1776 (2014-11-04)
- Merge more RtMidi changes
# 0.3.1.1775 (2014-11-04)
- Merge RtAudio::setStreamTime
# 0.3.1.1774 (2014-11-04)
- Merge indent changes
# 0.3.1.1773 (2014-11-04)
- Import RtAudio WASAPI
# 0.3.1.1772 (2014-11-04)
- Remove GOrgueRtHelper include
# 0.3.1.1771 (2014-11-04)
- Avoid data copy
# 0.3.1.1770 (2014-11-04)
- Merge RtMidi Windows KS drop
# 0.3.1.1769 (2014-11-04)
- Merge RtMidi error callback
# 0.3.1.1768 (2014-11-04)
- Merge separate RtAudio/RtMidi error classes
# 0.3.1.1767 (2014-11-04)
- Refactor sound output
# 0.3.1.1766 (2014-11-04)
- Separate internal audio device information
# 0.3.1.1765 (2014-11-04)
- Remove not used return type
# 0.3.1.1764 (2014-11-04)
- Get rid of stream time
# 0.3.1.1763 (2014-11-03)
- Fix wx3.0 build
# 0.3.1.1762 (2014-10-27)
- Save enclosure state
# 0.3.1.1761 (2014-10-26)
- Fix
# 0.3.1.1760 (2014-10-26)
- Refactor MIDI subsystem
# 0.3.1.1759 (2014-10-26)
- Simplify pool free
# 0.3.1.1758 (2014-10-26)
- Increase touch rate
# 0.3.1.1757 (2014-10-26)
- Remember last enclosure value (similar to stops)
# 0.3.1.1756 (2014-10-26)
- Don't expose internal state from GOrgueMidi
# 0.3.1.1755 (2014-10-26)
- ToMidi is const
# 0.3.1.1754 (2014-10-15)
- Help Update: Remove obsolete comments
# 0.3.1.1753 (2014-09-30)
- Help Update: demo organ
# 0.3.1.1752 (2014-09-30)
- Help Update: Midi event editor
- Add Drawstop: Received:
-     Cx Program Change Range
-     RPN Range
-     NRPN Range
-     Sys Ex Viscount
-     Sys Ex Viscount Toggle
- Add Drawstop: Sent:
-     Cx Program Change Range
-     RPN Range
-     NRPN Range
# 0.3.1.1751 (2014-09-24)
- Report broken InfoFilename
# 0.3.1.1750 (2014-09-20)
- Fix loop length check
# 0.3.1.1749 (2014-09-19)
- Add support for loop crossfades
# 0.3.1.1748 (2014-09-19)
- Report invalid loops
# 0.3.1.1747 (2014-09-12)
- Help Update: Midi event editor
- 	Add Manual: Send division output
- 	Add Rank: Send
# 0.3.1.1746 (2014-09-02)
- Add engine for loop crossfade
# 0.3.1.1745 (2014-09-01)
- SetSampleData function
# 0.3.1.1744 (2014-09-01)
- Prepare crossfade length parameter
# 0.3.1.1743 (2014-08-30)
- Fix mingw build definitions
# 0.3.1.1742 (2014-08-30)
- Remove dead code
# 0.3.1.1741 (2014-08-30)
- Add missing pitch info
# 0.3.1.1740 (2014-08-29)
- Report missing temperament annotations
# 0.3.1.1739 (2014-08-29)
- Fix temperament annotations for effect stops
# 0.3.1.1738 (2014-08-29)
- Hint empty object names
# 0.3.1.1737 (2014-08-29)
- collaps organ setting pipe tree
# 0.3.1.1736 (2014-08-25)
- Help Update: Audio & Midi Settings menu and dialog
-     User-defined temperaments
-     Midi recorder
# 0.3.1.1735 (2014-08-22)
- Correct package name
# 0.3.1.1734 (2014-08-22)
- Prepare for panel format 2.0
# 0.3.1.1733 (2014-08-22)
- Unify translation location on Windows and Linux
# 0.3.1.1732 (2014-08-22)
- Try to keep the samples in memory
# 0.3.1.1731 (2014-08-08)
- Fix label positions
# 0.3.1.1730 (2014-08-07)
- Fix sysex related MIDI event loading bug
# 0.3.1.1729 (2014-08-07)
- Update GetPrerequisites
# 0.3.1.1728 (2014-08-05)
- Drop invalid comment
# 0.3.1.1727 (2014-08-05)
- Use golib library
# 0.3.1.1726 (2014-07-30)
- Update OBS service definition
# 0.3.1.1725 (2014-07-29)
- Refactor panel creation
# 0.3.1.1724 (2014-07-29)
- Rework panel background setup
# 0.3.1.1723 (2014-07-26)
- Use LoadControl for panel setup
# 0.3.1.1722 (2014-07-26)
- Rework CreateGUIElement
# 0.3.1.1721 (2014-07-26)
- GOGUIControl::Init is not virtual
# 0.3.1.1720 (2014-07-22)
- Help Update: MIDI event editor - Drawstop/Pushbutton specifics
# 0.3.1.1719 (2014-07-18)
- Make ODF path separator check strict mode only
# 0.3.1.1718 (2014-07-18)
- Allow disabling sample loading to aid ODF development
# 0.3.1.1717 (2014-07-17)
- Separate layout engine
# 0.3.1.1716 (2014-07-17)
- DisplayMetrics don't depend on organ file
# 0.3.1.1715 (2014-07-17)
- Introduce getter methods
# 0.3.1.1714 (2014-07-17)
- Use wxString::Format
# 0.3.1.1713 (2014-07-17)
- Check for a invalid directory separator
# 0.3.1.1712 (2014-07-17)
- Fix manual background
# 0.3.1.1711 (2014-07-15)
- Rework manual detection by the layout engine
# 0.3.1.1710 (2014-07-12)
- Rework enclosure registration
# 0.3.1.1709 (2014-07-12)
- Label: move m_layout access to Layout()
# 0.3.1.1708 (2014-07-12)
- Enclosure: move m_layout access to Layout()
# 0.3.1.1707 (2014-07-12)
- Button: move m_layout access to Layout()
# 0.3.1.1706 (2014-07-12)
- Fix Viscount SYSEX
# 0.3.1.1705 (2014-07-11)
- manual: move m_layout access to Layout()
# 0.3.1.1704 (2014-07-11)
- manual background: move m_layout access to Layout()
# 0.3.1.1703 (2014-07-11)
- Add PO4A as build dependency
# 0.3.1.1702 (2014-07-11)
- Use better sample
# 0.3.1.1701 (2014-07-07)
- Add layout hook
# 0.3.1.1700 (2014-07-07)
- Pass variables by reference
# 0.3.1.1699 (2014-07-07)
- separate access of calculated metrics
# 0.3.1.1698 (2014-07-07)
- Pass button configuration via Init
# 0.3.1.1697 (2014-07-07)
- Pass label configuration via Init
# 0.3.1.1696 (2014-07-07)
- Fix incorrect midi match descriptions
# 0.3.1.1695 (2014-07-07)
- Correct error message
# 0.3.1.1694 (2014-06-12)
- Add Arch Linux build file
# 0.3.1.1693 (2014-06-12)
- Add more docbook search paths
# 0.3.1.1692 (2014-06-12)
- Fix wave based tremulants
# 0.3.1.1691 (2014-05-29)
- Improve MIDI player timing
# 0.3.1.1690 (2014-05-20)
- Help Update: MIDI event editor - Label specifics
# 0.3.1.1689 (2014-05-20)
- Fix merge error
# 0.3.1.1688 (2014-05-20)
- Add support for Viscount SYSEX
# 0.3.1.1687 (2014-05-20)
- MIDI bug fixes
# 0.3.1.1686 (2014-05-20)
- Add transpose support to the MIDI player
# 0.3.1.1685 (2014-05-20)
- Add colour brown
# 0.3.1.1684 (2014-05-04)
- Lockless sampler list handling
# 0.3.1.1683 (2014-05-04)
- Separate tremulant processing
# 0.3.1.1682 (2014-05-03)
- fix OS X installer containing a wrong copy of the demo organ
# 0.3.1.1681 (2014-05-03)
- commit Martin's patch. undo previous fixes.
# 0.3.1.1680 (2014-04-30)
- Help Update: MIDI event editor - Keyboard specifics
# 0.3.1.1679 (2014-04-30)
- Help Update: MIDI event editor - Enclosure specifics
# 0.3.1.1678 (2014-04-30)
- Help Update: MIDI event editor - Common data
# 0.3.1.1677 (2014-04-30)
- fix progress dialog causing seg-faults on OS X when the dialog is destroyed and recreated
# 0.3.1.1676 (2014-04-29)
- fix some issues causing asserts to trip in debug builds
# 0.3.1.1675 (2014-04-29)
- fix guard names
# 0.3.1.1674 (2014-04-26)
- Refactor sampler handling
# 0.3.1.1673 (2014-04-26)
- Update README ()
# 0.3.1.1672 (2014-04-26)
- Tempo events use old speed for their timing ()
# 0.3.1.1671 (2014-04-23)
- Fix MIDI player debouncer bug
# 0.3.1.1670 (2014-04-23)
- Increase scale factor
# 0.3.1.1669 (2014-04-22)
- Fix LCD part 2
# 0.3.1.1668 (2014-04-22)
- Fix 16 bytes HW LCD message
# 0.3.1.1667 (2014-04-17)
- Fix load race condition
# 0.3.1.1666 (2014-04-12)
- Fix first startup error message
# 0.3.1.1665 (2014-04-12)
- Fix shutdown crash
# 0.3.1.1664 (2014-04-08)
- Add support for user temperments
# 0.3.1.1663 (2014-04-06)
- Refactor temperament list
# 0.3.1.1662 (2014-04-05)
- Add missing files
# 0.3.1.1661 (2014-04-05)
- Allow sending rank state
# 0.3.1.1660 (2014-04-05)
- Introduce dummy pipe
# 0.3.1.1659 (2014-04-05)
- Refactor reference pipe
# 0.3.1.1658 (2014-04-05)
- Refactor code of a sounding pipe
# 0.3.1.1657 (2014-04-05)
- Cleanups
# 0.3.1.1656 (2014-04-05)
- Introduce PipeConfigTreeNode
# 0.3.1.1655 (2014-04-05)
- Introduce PipeConfigNode
# 0.3.1.1654 (2014-04-03)
- Last patch was applied incorrectly
# 0.3.1.1653 (2014-04-02)
- Fix incorrect bit field limit
# 0.3.1.1652 (2014-04-01)
- Fix pulseaudio issues
# 0.3.1.1651 (2014-03-30)
- Move interface definition
# 0.3.1.1650 (2014-03-30)
- Get rid of unused fields
# 0.3.1.1649 (2014-03-30)
- Refactor windchest tremulant callback
# 0.3.1.1648 (2014-03-30)
- Update OBS definitions
# 0.3.1.1647 (2014-03-30)
- fix bug in call to memset which was taking the size of an array of pointers
# 0.3.1.1646 (2014-03-29)
- Raise divisional count
# 0.3.1.1645 (2014-03-23)
- Fix various bugs
# 0.3.1.1644 (2014-03-23)
- Allow adjusting y coordinate of manual keys
# 0.3.1.1643 (2014-03-23)
- Fix MIDI detection
# 0.3.1.1642 (2014-03-22)
- Fix organ dialog close procedure
# 0.3.1.1641 (2014-03-22)
- Allow sending the division state
# 0.3.1.1640 (2014-03-22)
- Also allow MIDI configuration via menu
# 0.3.1.1639 (2014-03-22)
- Rework MIDI config dialog handling
# 0.3.1.1638 (2014-03-22)
- Rework save handling
# 0.3.1.1637 (2014-03-22)
- Rework cache object handling
# 0.3.1.1636 (2014-03-22)
- Rework event handling
# 0.3.1.1635 (2014-03-22)
- Rework setter button
# 0.3.1.1634 (2014-03-22)
- Fix manual MIDIKey999 missing error
# 0.3.1.1633 (2014-03-15)
- Add support for strange Ahlborn NRPN messages
# 0.3.1.1632 (2014-03-15)
- Refactor similar event filtering
# 0.3.1.1631 (2014-03-15)
- PGM change for enclosures (Ahlborn)
# 0.3.1.1630 (2014-03-13)
- We don't allow recursive bass/melody couplers
# 0.3.1.1629 (2014-03-13)
- Improve unused entry reporting
# 0.3.1.1628 (2014-03-12)
- OBS files
# 0.3.1.1627 (2014-03-11)
- Reorg OBS build system
# 0.3.1.1626 (2014-03-11)
- MIDI player
# 0.3.1.1625 (2014-03-11)
- Refactor midi message handling
# 0.3.1.1624 (2014-03-06)
- Fix GOrgueMidiReceiver
# 0.3.1.1623 (2014-03-03)
- Update nl translation
# 0.3.1.1622 (2014-03-02)
- MIDI LCD support
# 0.3.1.1621 (2014-03-02)
- Matching type: manual without midi map
# 0.3.1.1620 (2014-03-02)
- Disable unused fields in the MIDI receive dialog
# 0.3.1.1619 (2014-03-02)
- Disable unused fields in MIDI send dialog
# 0.3.1.1618 (2014-03-02)
- Updated/corrected Swedish translation
# 0.3.1.1617 (2014-03-01)
- Dutch translation
# 0.3.1.1616 (2014-02-27)
- Correct MIDI message
# 0.3.1.1615 (2014-02-27)
- Move midi recorder to GOrgueMidi
# 0.3.1.1614 (2014-02-27)
- Put release in the file name
# 0.3.1.1613 (2014-02-26)
- Add missing file
# 0.3.1.1612 (2014-02-26)
- MIDI recorder
# 0.3.1.1611 (2014-02-26)
- Move elementID generation
# 0.3.1.1610 (2014-02-22)
- Allow to route MIDI stream for recording to a external software
# 0.3.1.1609 (2014-02-22)
- Recognize GO recorder events
# 0.3.1.1608 (2014-02-22)
- midi recorder event generation
# 0.3.1.1607 (2014-02-22)
- Define GO SYSEX messages
# 0.3.1.1606 (2014-02-22)
- Configure element IDs
# 0.3.1.1605 (2014-02-22)
- Add missing files
# 0.3.1.1604 (2014-02-22)
- Route MIDI events for recording
# 0.3.1.1603 (2014-02-22)
- Add midi element map
# 0.3.1.1602 (2014-02-22)
- Add support for fixed value ctrl change (Albhorn)
# 0.3.1.1601 (2014-02-22)
- Introduce element ID / PrepareRecoding Hook
# 0.3.1.1600 (2014-02-22)
- Allow sending 9x note for manuals with/without velocity
# 0.3.1.1599 (2014-02-22)
- Improve pitch selection
# 0.3.1.1598 (2014-02-22)
- Improve load error reporting
# 0.3.1.1597 (2014-02-22)
- Don't enable GrandOrgue ports by default
# 0.3.1.1596 (2014-02-21)
- Fix sysex handling
# 0.3.1.1595 (2014-02-14)
- Change label text based on the selected event.
# 0.3.1.1594 (2014-02-13)
- Fix RPN/NRPN
# 0.3.1.1593 (2014-02-09)
- Detect short octave
# 0.3.1.1592 (2014-02-09)
- /usr/share/appdata must be owned by a package
# 0.3.1.1591 (2014-02-09)
- Separate reverb processing for the individual channels
# 0.3.1.1590 (2014-02-09)
- Rework end segement pointer
# 0.3.1.1589 (2014-02-09)
- Cache windchest volume
# 0.3.1.1588 (2014-02-09)
- pThreads are part of the base install
# 0.3.1.1587 (2014-02-09)
- Cleanup
# 0.3.1.1586 (2014-02-09)
- Detect complex midi events
# 0.3.1.1585 (2014-02-08)
- Check for mixing of percussive and non-percussive attacks
# 0.3.1.1584 (2014-02-08)
- Drop not required BLOCKS_PER_FRAME limit
# 0.3.1.1583 (2014-02-08)
- Rework scaled releases
# 0.3.1.1582 (2014-02-07)
- Add initial support for Ctrl change bitfields (Content)
# 0.3.1.1581 (2014-02-06)
- Fix wx-3.0 build
# 0.3.1.1580 (2014-02-06)
- Include appdata in debian package
# 0.3.1.1579 (2014-02-06)
- Samplerate independend cross-fade length
# 0.3.1.1578 (2014-02-02)
- Help Update: Perform strict ODF, message log window
# 0.3.1.1577 (2014-02-02)
- Updated Swedish translation.
# 0.3.1.1576 (2014-02-02)
- Lost bug fix
# 0.3.1.1575 (2014-02-02)
- Don't build all object for perftest and GO twice
# 0.3.1.1574 (2014-02-02)
- Separate internal midi event from wxEvent
# 0.3.1.1573 (2014-02-01)
- Don't store device id as string
# 0.3.1.1572 (2014-02-01)
- Fix initial settings setup
# 0.3.1.1571 (2014-02-01)
- Ship AppData file
# 0.3.1.1570 (2014-02-01)
- Update copyright
# 0.3.1.1569 (2014-01-29)
- Rework file loading
# 0.3.1.1568 (2014-01-29)
- Fix strict mode ODF errors
# 0.3.1.1567 (2014-01-26)
- Use fader for the tremulant
# 0.3.1.1566 (2014-01-26)
- Restructure Fader
# 0.3.1.1565 (2014-01-26)
- Rework offset handling (part 1)
# 0.3.1.1564 (2014-01-24)
- Correct polyphase readahead
# 0.3.1.1563 (2014-01-22)
- Help Update: Reverb tab
# 0.3.1.1562 (2014-01-21)
- Handle invalid sample per buffer values more gracefully
# 0.3.1.1561 (2014-01-20)
- Remove ReadSamplerFrames
# 0.3.1.1560 (2014-01-20)
- 64 bit samples from Kalvtrask
# 0.3.1.1559 (2014-01-18)
- Use variable block size
# 0.3.1.1558 (2014-01-18)
- Prepare for variable block size
# 0.3.1.1557 (2014-01-18)
- Correct end loop handling
# 0.3.1.1556 (2014-01-18)
- Count time in samples
# 0.3.1.1555 (2014-01-18)
- Introduce yet fixed len parameter
# 0.3.1.1554 (2014-01-18)
- Disallow wrong case ODFs in strict mode
# 0.3.1.1553 (2014-01-18)
- Make decoder functions variable
# 0.3.1.1552 (2014-01-18)
- Cleanup
# 0.3.1.1551 (2014-01-18)
- Store end section length
# 0.3.1.1550 (2014-01-17)
- Remove old history code
# 0.3.1.1549 (2014-01-17)
- Calcualte history on demand
# 0.3.1.1548 (2014-01-17)
- Help Update: Tracker Delay, update Memory Limit
# 0.3.1.1547 (2014-01-16)
- Refactor decompression code
# 0.3.1.1546 (2014-01-16)
- Use decompression cache as data storage for the deconding loop
# 0.3.1.1545 (2014-01-16)
- Use decompression cache as storage for start segements
# 0.3.1.1544 (2014-01-16)
- Encapsulate history access
# 0.3.1.1543 (2014-01-16)
- Align decompression cache with other decompression code
# 0.3.1.1542 (2014-01-16)
- Refactor sample data access
# 0.3.1.1541 (2014-01-16)
- Add other relevant structure sizes to the hash
# 0.3.1.1540 (2014-01-15)
- update README file for explanations to build with TDM
# 0.3.1.1539 (2014-01-15)
- update Windows TDM building
# 0.3.1.1538 (2014-01-14)
- Optimize GetSamples
# 0.3.1.1537 (2014-01-13)
- Fix saving MIDI In state
# 0.3.1.1536 (2014-01-12)
- Fix perftest
# 0.3.1.1535 (2014-01-11)
- Align color parsing with the ODF reference
# 0.3.1.1534 (2014-01-11)
- Refactor number parsing
# 0.3.1.1533 (2014-01-11)
- Report trailing whitespace
# 0.3.1.1532 (2014-01-11)
- Add stricter ODF check mode
# 0.3.1.1531 (2014-01-11)
- Don't trim texts
# 0.3.1.1530 (2014-01-11)
- Use string read without trim for internal settings
# 0.3.1.1529 (2014-01-11)
- Rename old string read function
# 0.3.1.1528 (2014-01-11)
- Add constant for limits
# 0.3.1.1527 (2014-01-11)
- Load important settings first
# 0.3.1.1526 (2014-01-11)
- Move trim out of the generic read
# 0.3.1.1525 (2014-01-02)
- avoid unnecessary memory pool warning with Windows 32 bits platforms
# 0.3.1.1524 (2013-12-31)
- Fix panel use after free
# 0.3.1.1523 (2013-12-31)
- Fix build issues
# 0.3.1.1522 (2013-12-30)
- Fix section names
# 0.3.1.1521 (2013-12-30)
- Specify GenericName
# 0.3.1.1520 (2013-12-29)
- Enable memory limit by default
# 0.3.1.1519 (2013-12-29)
- Fix out-of-memory handling
# 0.3.1.1518 (2013-12-28)
- Fix compression
# 0.3.1.1517 (2013-12-26)
- Fix LRU
# 0.3.1.1516 (2013-12-26)
- Fix empty organ path
# 0.3.1.1515 (2013-12-24)
- Rename "old" reverb
# 0.3.1.1514 (2013-12-24)
- Migrate more settings
# 0.3.1.1513 (2013-12-24)
- Move midi settings
# 0.3.1.1512 (2013-12-24)
- Migrate reverb settings
# 0.3.1.1511 (2013-12-24)
- Use common range checks
# 0.3.1.1510 (2013-12-24)
- Migrate more settings
# 0.3.1.1509 (2013-12-24)
- Fix warnings
# 0.3.1.1508 (2013-12-22)
- don't use named initialisers
# 0.3.1.1507 (2013-12-22)
- Remove unused variables
# 0.3.1.1506 (2013-12-22)
- Add missing header file
# 0.3.1.1505 (2013-12-22)
- use c99 syntax for named initialiser list rather than gnu
# 0.3.1.1504 (2013-12-22)
- fix broken partition size
# 0.3.1.1503 (2013-12-22)
- patch GOrgueMemoryPool.cpp with Martin's OS X changes
# 0.3.1.1502 (2013-12-21)
- Document wavpack
# 0.3.1.1501 (2013-12-21)
- Catch more malloc errors
# 0.3.1.1500 (2013-12-21)
- Catch wavpack outofmemory errors
# 0.3.1.1499 (2013-12-21)
- fix a stupid indentation mistake
# 0.3.1.1498 (2013-12-21)
- modify the way GrandOrgue transforms into a foreground process as GetCurrentProcess has been deprecated in OS X 10.9
# 0.3.1.1497 (2013-12-21)
- change ceil into an equivilent expression. The previous was also doing an integer division which probably wasn't the desired operation
# 0.3.1.1496 (2013-12-21)
- change std::copy into a memcpy to get around std::copy not working with the complex data type
# 0.3.1.1495 (2013-12-21)
- Pack demo set
# 0.3.1.1494 (2013-12-21)
- Add initial Wavpack support
# 0.3.1.1493 (2013-12-21)
- Build wavpack library
# 0.3.1.1492 (2013-12-21)
- Import wavpack sources
# 0.3.1.1491 (2013-12-19)
- Cleanup GOrgueWave
# 0.3.1.1490 (2013-12-19)
- Reformat stylesheet
# 0.3.1.1489 (2013-12-19)
- Unshare string
# 0.3.1.1488 (2013-12-19)
- Add FFTW license
# 0.3.1.1487 (2013-12-19)
- Use const for strings
# 0.3.1.1486 (2013-12-19)
- Allow truncation of WAV files
# 0.3.1.1485 (2013-12-19)
- Use -g for C flags too
# 0.3.1.1484 (2013-12-19)
- Build fix
# 0.3.1.1483 (2013-12-18)
- Help update: Load concurrency, rewording to differentiate preset numbers and exported settings files
# 0.3.1.1482 (2013-12-18)
- Help update: Initial MIDI Configuration tab
# 0.3.1.1481 (2013-12-18)
- Help update: Organs tab; Fix indentation
# 0.3.1.1480 (2013-12-18)
- Help update: New items in 'File' menu
# 0.3.1.1479 (2013-12-15)
- Limit concurrency in low-memory situations
# 0.3.1.1478 (2013-12-15)
- Make arguments const
# 0.3.1.1477 (2013-12-15)
- Fix small memory leak
# 0.3.1.1476 (2013-12-14)
- Scale panel content
# 0.3.1.1475 (2013-12-14)
- Prepare for panel scaling
# 0.3.1.1474 (2013-12-14)
- Store builtin graphics as wxImage
# 0.3.1.1473 (2013-12-14)
- Fix windows wanrings
# 0.3.1.1472 (2013-12-14)
- Fix update cache crash
# 0.3.1.1471 (2013-12-09)
- Allow organ selection via double-click
# 0.3.1.1470 (2013-12-08)
- Enable load concurrency
# 0.3.1.1469 (2013-12-08)
- Prepare for multi-threaded loading
# 0.3.1.1468 (2013-12-08)
- restructure load process
# 0.3.1.1467 (2013-12-08)
- Move dummy memory allocation to main routine
# 0.3.1.1466 (2013-12-07)
- Fix limit of 10 organs
# 0.3.1.1465 (2013-12-07)
- Fix setting migration
# 0.3.1.1464 (2013-12-06)
- Handle window close
# 0.3.1.1463 (2013-12-05)
- Fix wood images
# 0.3.1.1462 (2013-12-04)
- Fix drawstop display bug
# 0.3.1.1461 (2013-12-04)
- Keep old enclosure style as default
# 0.3.1.1460 (2013-12-03)
- Add more enclosure styles from Lars
# 0.3.1.1459 (2013-12-03)
- Unify image names
# 0.3.1.1458 (2013-12-03)
- Refactor font handling
# 0.3.1.1457 (2013-12-03)
- Cleanup progress dialog
# 0.3.1.1456 (2013-12-03)
- Fix manual regression
# 0.3.1.1455 (2013-12-01)
- Add locking to memory pool
# 0.3.1.1454 (2013-12-01)
- Import OBS build files
# 0.3.1.1453 (2013-12-01)
- Add missing file
# 0.3.1.1452 (2013-12-01)
- Unify include usage
# 0.3.1.1451 (2013-12-01)
- Use stdint
# 0.3.1.1450 (2013-12-01)
- Rework version number defines
# 0.3.1.1449 (2013-12-01)
- Refactor DC handling
# 0.3.1.1448 (2013-11-30)
- Use unsigned for width/height
# 0.3.1.1447 (2013-11-30)
- Introduce GOrgueBitmap
# 0.3.1.1446 (2013-11-30)
- Unify wood image handling
# 0.3.1.1445 (2013-11-30)
- Clarify MIDIInputNumber in help
# 0.3.1.1444 (2013-11-30)
- Rework progress dialog handling
# 0.3.1.1443 (2013-11-30)
- Don't put resizeable elements into the pool
# 0.3.1.1442 (2013-11-30)
- A reference has not further attributes
# 0.3.1.1441 (2013-11-26)
- Get rid of wxDocument
# 0.3.1.1440 (2013-11-26)
- Rename GOrgueDialogView to GOrgueView (the name has been blocked some time ago)
# 0.3.1.1439 (2013-11-26)
- Get rid of UserSetting
# 0.3.1.1438 (2013-11-26)
- Fix reload bug
# 0.3.1.1437 (2013-11-25)
- Fix for window
# 0.3.1.1436 (2013-11-24)
- Optimize ODF parser
# 0.3.1.1435 (2013-11-24)
- Get rid of wxDocManager
# 0.3.1.1434 (2013-11-24)
- Cleanup GOrgueDocument access
# 0.3.1.1433 (2013-11-24)
- Add copy to GOLogWindow
# 0.3.1.1432 (2013-11-23)
- MIDI devices tab
# 0.3.1.1431 (2013-11-22)
- Fix bad commit
# 0.3.1.1430 (2013-11-22)
- Add debounce function
# 0.3.1.1429 (2013-11-22)
- Correct ODF reference
# 0.3.1.1428 (2013-11-22)
- Allow to specify trigger value for buttons
# 0.3.1.1427 (2013-11-22)
- Improve MIDI dialog
# 0.3.1.1426 (2013-11-22)
- Rename velocity to value
# 0.3.1.1425 (2013-11-22)
- Remove artificial limits
# 0.3.1.1424 (2013-11-22)
- Use letter for the general bank name
- We already use number for the sequencer and crescendo
# 0.3.1.1423 (2013-11-20)
- Fix LRU segfault
# 0.3.1.1422 (2013-11-18)
- Use organ titles in LRU list
# 0.3.1.1421 (2013-11-18)
- Receive RPN/NRPN
# 0.3.1.1420 (2013-11-18)
- Send RPN/NRPN
# 0.3.1.1419 (2013-11-18)
- Decode/encode RPN/NRPN
# 0.3.1.1418 (2013-11-18)
- Store last use time in GOrgueOrgan
# 0.3.1.1417 (2013-11-18)
- Move window title handling
# 0.3.1.1416 (2013-11-18)
- Migrate more settings
# 0.3.1.1415 (2013-11-18)
- Move audio output settings too
# 0.3.1.1414 (2013-11-18)
- Migrate storage of the audio groups
# 0.3.1.1413 (2013-11-18)
- Move integer write to the non-overloaded functions
- Get rid of some non-necessary HW1 int formats too
# 0.3.1.1412 (2013-11-18)
- Move float write to the non-overloaded functions
# 0.3.1.1411 (2013-11-18)
- Move enum write to non-overloaded functions
# 0.3.1.1410 (2013-11-18)
- Move string write to non-overloaded functions
# 0.3.1.1409 (2013-11-18)
- Move boolean to non-overloaded write functions
# 0.3.1.1408 (2013-11-18)
- Implement non-overload write functions
# 0.3.1.1407 (2013-11-17)
- Fix MIDI event dialog
# 0.3.1.1406 (2013-11-16)
- Use correct data type
# 0.3.1.1405 (2013-11-16)
- Fix MIDI sender
# 0.3.1.1404 (2013-11-16)
- Provide quick access to a few organs
# 0.3.1.1403 (2013-11-16)
- Fix for wx3.0
# 0.3.1.1402 (2013-11-16)
- Refactor load/save
# 0.3.1.1401 (2013-11-15)
- Show ODF location in the organ properties
# 0.3.1.1400 (2013-11-15)
- Add sequencer elements to initial MIDI
# 0.3.1.1399 (2013-11-15)
- Export setter elements
# 0.3.1.1398 (2013-11-15)
- Add enclosures to intial MIDI
# 0.3.1.1397 (2013-11-15)
- Add manuals to the initial MIDI
# 0.3.1.1396 (2013-11-15)
- Backup last good settings
# 0.3.1.1395 (2013-11-15)
- Add core for new initial MIDI configuration
# 0.3.1.1394 (2013-11-13)
- Build fix
# 0.3.1.1393 (2013-11-12)
- Align panel handling with dialogs
# 0.3.1.1392 (2013-11-12)
- Refactor subwindow creation
# 0.3.1.1391 (2013-11-10)
- Remove old initial MIDI configuration tab (part 1)
# 0.3.1.1390 (2013-11-10)
- amend help: midi note number also resets pitch fraction.
# 0.3.1.1389 (2013-11-10)
- Small bug fix
# 0.3.1.1388 (2013-11-10)
- Don't preconfigure by default
# 0.3.1.1387 (2013-11-10)
- Prepare for Johannus SysEx message
# 0.3.1.1386 (2013-11-09)
- Cross fading improvements by JLD
# 0.3.1.1385 (2013-11-09)
- Update French translation
# 0.3.1.1384 (2013-11-07)
- Fix error log message on Windows
# 0.3.1.1383 (2013-11-02)
- Fix mingw build
# 0.3.1.1382 (2013-11-02)
- Allow organ selection from the known organ list
# 0.3.1.1381 (2013-11-02)
- Merge OK handlers
# 0.3.1.1380 (2013-11-01)
- Improve MIDI matching for Organs
# 0.3.1.1379 (2013-11-01)
- Introduce GOrgueOrgan
# 0.3.1.1378 (2013-11-01)
- Various senders/receivers should not depend on GrandOrgueFile
# 0.3.1.1377 (2013-11-01)
- Introduce GrandOrgueConfig settings file
# 0.3.1.1376 (2013-11-01)
- Add missing initialisation (PA)
# 0.3.1.1375 (2013-11-01)
- Improve CMB writting error handling
# 0.3.1.1374 (2013-11-01)
- Add hint about initial MIDI config
- Beginners don't understand the function of the panel
# 0.3.1.1373 (2013-11-01)
- Add ASIO build capatibility to spec file
# 0.3.1.1372 (2013-10-26)
- Fix modal window destory race condition
- This time, we need the result. The chooser is implemented as
- normal wxDialog, so fixing the problem is easier.
# 0.3.1.1371 (2013-10-26)
- Don't create MessageBoxes from non-main window
- MessageBox use plattform functions and is created on the stack.
- While it is shown, it is also part of the return stack.
- Trying to close the parent window (MIDI organ load events) while the message box is shown
- results therefore in a crash.
- Work around by moving the display of the messagebox to a safe location.
- Ugly, but the simplest solution.
# 0.3.1.1370 (2013-10-26)
- All user to interact with organ while using organ/midi dialogs
# 0.3.1.1369 (2013-10-23)
- Rework MIDI handling
# 0.3.1.1368 (2013-10-22)
- Specify parent for MessageBox
# 0.3.1.1367 (2013-10-18)
- Update demo organ (http://www.magle.dk/music-forums/18709-grandorgue-rev-1331-demo-4.html)
# 0.3.1.1366 (2013-09-29)
- Check channel count while changing the audio interface
# 0.3.1.1365 (2013-09-29)
- Fix RtAudio ressource leaks
# 0.3.1.1364 (2013-09-26)
- Fix pitch label setter element
# 0.3.1.1363 (2013-09-25)
- Windows build fixes
# 0.3.1.1362 (2013-09-25)
- Use GNU regex for Windows
# 0.3.1.1361 (2013-09-25)
- Fix jack audio support for Windows
# 0.3.1.1360 (2013-09-25)
- Make CMB write crash save (linux)
# 0.3.1.1359 (2013-09-25)
- Jack Midi Support for Windows
# 0.3.1.1358 (2013-09-25)
- Fix help
# 0.3.1.1357 (2013-09-25)
- Add tracker delay config option
# 0.3.1.1356 (2013-09-25)
- Add support for delaying samples
# 0.3.1.1355 (2013-09-21)
- Help updates(Audio & Organ settings)
# 0.3.1.1354 (2013-09-21)
- Organ settings dialog
# 0.3.1.1353 (2013-09-21)
- Audio and organ settings overview
# 0.3.1.1352 (2013-09-21)
- Audio & Midi options, Audio groups tab
# 0.3.1.1351 (2013-09-21)
- Audio & Midi options, Audio output tab
# 0.3.1.1350 (2013-09-08)
- Enable Jack MIDI support
# 0.3.1.1349 (2013-09-08)
- RtAudio update
# 0.3.1.1348 (2013-09-08)
- Portaudio update
# 0.3.1.1347 (2013-09-01)
- Translation corrections
# 0.3.1.1346 (2013-08-31)
- Fix invalid format strings
# 0.3.1.1345 (2013-08-31)
- Add Italian translation
# 0.3.1.1344 (2013-08-28)
- Redo audio group configuration
# 0.3.1.1343 (2013-08-16)
- Display unlimited memory pool less confusing
# 0.3.1.1342 (2013-08-10)
- Fix SpinCtrl on Windows
# 0.3.1.1341 (2013-07-06)
- Fix switch bug
# 0.3.1.1340 (2013-07-03)
- Avoid copying the whole receiver/sender object
# 0.3.1.1339 (2013-07-03)
- Rework MIDI event dialog apply logic
# 0.3.1.1338 (2013-07-03)
- Fix loop
# 0.3.1.1337 (2013-07-03)
- Fix LRU on wx2.8 and log on wx2.9
# 0.3.1.1336 (2013-07-03)
- Correct LRU usage
# 0.3.1.1335 (2013-07-03)
- Cleanup
# 0.3.1.1334 (2013-07-03)
- Add user contributed temperament
# 0.3.1.1333 (2013-07-03)
- Only load one organ
# 0.3.1.1332 (2013-06-23)
- Second wordaround for # in file names - this time for Windows
# 0.3.1.1331 (2013-06-14)
- explicitly cast the result of c_str() to wxChar so that wxWidgets 2.9 will return the correct buffer type. this finally fixes the cache not working on OS X.
# 0.3.1.1330 (2013-06-14)
- stop the organ dialog from crashing on OS X (this happens because an update event gets triggered before the constructor finishes, calling Modified() while m_Apply and m_Reset are undefined.
# 0.3.1.1329 (2013-06-14)
- fix more incorrect grid sizer definitions to prevent assertions on OS X
# 0.3.1.1328 (2013-06-14)
- prevent trying to create two progress dialogs at the same time (load organ and save cache) - this explodes on OS X debug builds
# 0.3.1.1327 (2013-06-14)
- prevent assertion that too many items have been inserted into grid
# 0.3.1.1326 (2013-06-14)
- fix since-inception issue with polyphase resampler
# 0.3.1.1325 (2013-06-13)
- Ugly workaround for sample set file names (# is broken in cmake)
# 0.3.1.1324 (2013-06-11)
- Update demo organ
# 0.3.1.1323 (2013-06-11)
- Samples for new demo sampleset (from Lars)
# 0.3.1.1322 (2013-06-11)
- Try fixing the rare display problems
# 0.3.1.1321 (2013-06-11)
- Show MIDI key number in the tuning dialog
# 0.3.1.1320 (2013-06-11)
- Fix incorrect load warning
# 0.3.1.1319 (2013-06-11)
- Audio & MIDI settings, Options tab
# 0.3.1.1318 (2013-06-01)
- Add 96kHz sample rate support
# 0.3.1.1317 (2013-06-01)
- Increase enclosure count
# 0.3.1.1316 (2013-06-01)
- Add mingw spec files
# 0.3.1.1315 (2013-05-28)
- Tighten help structure
- Move menu & toolbar chapters as sections into user interface section
# 0.3.1.1314 (2013-05-25)
- Fix incorrect load warning
# 0.3.1.1313 (2013-05-23)
- Fix OBS build errors
# 0.3.1.1312 (2013-05-23)
- Ship demo sampleset as extra package
# 0.3.1.1311 (2013-05-23)
- Allow to control demo sample set inclusion
# 0.3.1.1310 (2013-05-23)
- Check for modified ODFs
# 0.3.1.1309 (2013-05-17)
- Fix windows crash
# 0.3.1.1308 (2013-05-15)
- Additional update for building GO under Windows using TDM
# 0.3.1.1307 (2013-05-15)
- update readme for building under Windows
# 0.3.1.1306 (2013-05-06)
- Load demo organ by default
# 0.3.1.1305 (2013-05-06)
- Ship demo organ
# 0.3.1.1304 (2013-05-06)
- Add a very simple stub for demo organ
# 0.3.1.1303 (2013-05-04)
- Enable switches
# 0.3.1.1302 (2013-04-30)
- Add more logical functions
# 0.3.1.1301 (2013-04-28)
- Show organ name in the title bar
# 0.3.1.1300 (2013-04-28)
- Add support for logical functions
# 0.3.1.1299 (2013-04-28)
- Bug fix
# 0.3.1.1298 (2013-04-28)
- Read input switch number from the ODF
# 0.3.1.1297 (2013-04-28)
- Prepare for creating switches
# 0.3.1.1296 (2013-04-28)
- Allow to place switches on additional panels
# 0.3.1.1295 (2013-04-28)
- Prepare drawstop for logical functions
# 0.3.1.1294 (2013-04-27)
- Don't call back from widget into the view
# 0.3.1.1293 (2013-04-27)
- Separate panel from the panel widget
# 0.3.1.1292 (2013-04-27)
- Access frame via view
# 0.3.1.1291 (2013-04-27)
- Locale independend setting store on windows
# 0.3.1.1290 (2013-04-22)
- Flush CMB
# 0.3.1.1289 (2013-04-19)
- Update on-line help
- User interface, toolbar and menus
# 0.3.1.1288 (2013-04-17)
- readonly support for buttons
# 0.3.1.1287 (2013-04-17)
- Forward signals to the switches
# 0.3.1.1286 (2013-04-17)
- Add new Bitmaps to ODF
# 0.3.1.1285 (2013-04-17)
- Add new bitmaps from Lars
# 0.3.1.1284 (2013-04-14)
- Read-Only flag for buttons
# 0.3.1.1283 (2013-04-14)
- Add switches to the combination definition
# 0.3.1.1282 (2013-04-14)
- Allow reversible pistons to reference switches
# 0.3.1.1281 (2013-04-14)
- Implement switches as drawstops
# 0.3.1.1280 (2013-04-13)
- Store switches in combinations
# 0.3.1.1279 (2013-04-07)
- Associate switches with a manual
# 0.3.1.1278 (2013-04-07)
- Introduce GOrgueSwitch dummy class
# 0.3.1.1277 (2013-04-06)
- Flush config
# 0.3.1.1276 (2013-04-06)
- Variable name cleanup
# 0.3.1.1275 (2013-04-05)
- Correct translations
# 0.3.1.1274 (2013-04-05)
- DE translations part 2
# 0.3.1.1273 (2013-04-05)
- More german translations
# 0.3.1.1272 (2013-04-04)
- Fix GCC 4.8 build error
# 0.3.1.1271 (2013-04-04)
- Fix mingw build with newer Mingw versions
# 0.3.1.1270 (2013-04-03)
- Refresh dialog content
# 0.3.1.1269 (2013-04-02)
- Manage cache + load last file
# 0.3.1.1268 (2013-04-02)
- Use 24bit as default sample depth
# 0.3.1.1267 (2013-04-02)
- Don't load samples with not necessary precision
# 0.3.1.1266 (2013-03-16)
- Introduce GOrgueCombinationElement
# 0.3.1.1265 (2013-03-16)
- Don't handle mouse events twice
# 0.3.1.1264 (2013-03-13)
- Audio group setup help
# 0.3.1.1263 (2013-03-13)
- Cleanup
# 0.3.1.1262 (2013-03-13)
- Fix windchest selection
# 0.3.1.1261 (2013-03-09)
- Explain sample unit
# 0.3.1.1260 (2013-03-09)
- Check for duplicate objects
# 0.3.1.1259 (2013-03-09)
- Variable name cleanup
# 0.3.1.1258 (2013-03-09)
- Avoid OS X crash
# 0.3.1.1257 (2013-03-04)
- Variable name cleanup
# 0.3.1.1256 (2013-03-03)
- Update first help chapter
# 0.3.1.1255 (2013-03-03)
- Allow to display a subset of keys in the GUI manual
# 0.3.1.1254 (2013-03-03)
- Implement MIDI matching table
# 0.3.1.1253 (2013-03-02)
- Tuning +/- 100 cent buttons
# 0.3.1.1252 (2013-02-23)
- Implment transpose via MIDI
# 0.3.1.1251 (2013-02-19)
- Fix pedal layout issue
# 0.3.1.1250 (2013-02-17)
- Fix wx2.9 build
# 0.3.1.1249 (2013-02-17)
- Increase file history size
# 0.3.1.1248 (2013-02-16)
- More de translations
# 0.3.1.1247 (2013-02-16)
- Correct typos
# 0.3.1.1246 (2013-02-16)
- Don't read setter state form the ODF
# 0.3.1.1245 (2013-02-16)
- Internal setter elemenets are not part of the ODF
# 0.3.1.1244 (2013-02-16)
- Catch invalid section reuse
# 0.3.1.1243 (2013-02-16)
- Add a few german translations
# 0.3.1.1242 (2013-02-16)
- Fix coding style
# 0.3.1.1241 (2013-02-15)
- Improve rounding
# 0.3.1.1240 (2013-02-11)
- Locale independent float parser
# 0.3.1.1239 (2013-02-11)
- Prepare for locale independend float reading
# 0.3.1.1238 (2013-02-09)
- Avoid hang, while logging many messages
# 0.3.1.1237 (2013-02-09)
- Fix general next/prev
# 0.3.1.1236 (2013-02-03)
- Don't read setter manual from ODF
# 0.3.1.1235 (2013-02-03)
- Don't read setter lables from ODF
# 0.3.1.1234 (2013-02-03)
- Don't read setter enclosure from ODF
# 0.3.1.1233 (2013-02-03)
- Don't read setter displaymetrics from ODF
# 0.3.1.1232 (2013-02-03)
- Reading from CMB should not result in read from ODF. ODF must be read explicitly.
# 0.3.1.1231 (2013-02-03)
- Add missing call to parent
# 0.3.1.1230 (2013-02-03)
- Don't read setter buttons from ODF
# 0.3.1.1229 (2013-02-03)
- Implement HW1Background::Init
# 0.3.1.1228 (2013-02-03)
- Don't treat exta space at section headers as fatal error
# 0.3.1.1227 (2013-02-03)
- Setter drawstop state should not be read from ODF
# 0.3.1.1226 (2013-01-29)
- Fix load warnings regression
# 0.3.1.1225 (2013-01-27)
- Only allow creating the cache for a fully loaded organ
# 0.3.1.1224 (2013-01-27)
- Read shaddowed ODF settings
# 0.3.1.1223 (2013-01-27)
- Don't read Label from ODF
# 0.3.1.1222 (2013-01-27)
- Don't report unused for combination import
# 0.3.1.1221 (2013-01-27)
- Upstream portaudio update
# 0.3.1.1220 (2013-01-27)
- Fix combination import
# 0.3.1.1219 (2013-01-27)
- Don't read builtin panel buttons from ODF
# 0.3.1.1218 (2013-01-27)
- Update copyright
# 0.3.1.1217 (2013-01-21)
- Allow to build built tool separately
# 0.3.1.1216 (2013-01-19)
- Remove HauptwerkOrganFileFormatVersion from documentation
# 0.3.1.1215 (2013-01-19)
- Fix wx2.9 warnings
# 0.3.1.1214 (2013-01-18)
- minor bug-fix. test to see if macro defined rather than the macro value itself
# 0.3.1.1213 (2013-01-15)
- Fix ODF parser loop
# 0.3.1.1212 (2013-01-13)
- Check for duplicate combinations in the ODF
# 0.3.1.1211 (2013-01-13)
- Check for duplicate general entries
# 0.3.1.1210 (2013-01-13)
- Check for duplicate divisional entries
# 0.3.1.1209 (2013-01-13)
- Check for invalid general combinations
# 0.3.1.1208 (2013-01-13)
- Check for invalid divisional combinations
# 0.3.1.1207 (2013-01-13)
- Don't report unused warnings in the case of fatal error
# 0.3.1.1206 (2013-01-13)
- Avoid incorrect unused warnings for combinations
# 0.3.1.1205 (2013-01-13)
- Improve ODF parser
# 0.3.1.1204 (2013-01-12)
- Fix enclosure image display
# 0.3.1.1203 (2013-01-07)
- Windchest selection selects children
# 0.3.1.1202 (2013-01-06)
- Warn about old styled settings
# 0.3.1.1201 (2013-01-06)
- Log ODF entry usage
# 0.3.1.1200 (2013-01-06)
- Avoid same start for Reverb partions
# 0.3.1.1199 (2013-01-06)
- Simplify partion setup code
# 0.3.1.1198 (2013-01-06)
- Mark old style settings as read
# 0.3.1.1197 (2013-01-06)
- Mark information entries as read
# 0.3.1.1196 (2013-01-06)
- Don't write unused entry
# 0.3.1.1195 (2013-01-06)
- Don't write unused entries
# 0.3.1.1194 (2013-01-06)
- Fix MIDI sender for setter swells
# 0.3.1.1193 (2013-01-06)
- Improve error messages
# 0.3.1.1192 (2013-01-06)
- Allow reverting to default audio config
# 0.3.1.1191 (2012-12-31)
- Let dialog adapt size from content
# 0.3.1.1190 (2012-12-30)
- Stricter Ini reader
# 0.3.1.1189 (2012-12-29)
- Add GOSoundReverbEngine
# 0.3.1.1188 (2012-12-29)
- Add GOrgueConfigFileReader
# 0.3.1.1187 (2012-12-29)
- Clarify comments
# 0.3.1.1186 (2012-12-29)
- Rename IniFileConfig
# 0.3.1.1185 (2012-12-28)
- update French translation to 1184
# 0.3.1.1184 (2012-12-28)
- update French translation
# 0.3.1.1183 (2012-12-27)
- Updated swedish translation
# 0.3.1.1182 (2012-12-27)
- Differentiate nomenclature between release tail length and reverb
# 0.3.1.1181 (2012-12-23)
- Add more MIDI matching types for manuals
# 0.3.1.1180 (2012-12-23)
- Rework transpose
# 0.3.1.1179 (2012-12-21)
- Update Manpage
# 0.3.1.1178 (2012-12-20)
- Fix reverb crashes
# 0.3.1.1177 (2012-12-19)
- Remove global functions
# 0.3.1.1176 (2012-12-19)
- RtMidi merge - part 3
# 0.3.1.1175 (2012-12-19)
- RtMidi multiapi support
# 0.3.1.1174 (2012-12-19)
- RtMidi merge - part 2
# 0.3.1.1173 (2012-12-19)
- RtMidi Merge - part 1
# 0.3.1.1172 (2012-12-19)
- Remove global objects
# 0.3.1.1171 (2012-12-15)
- Fix apply bug
# 0.3.1.1170 (2012-12-13)
- Avoid crash
# 0.3.1.1169 (2012-12-13)
- Portaudio update
# 0.3.1.1168 (2012-12-13)
- RtAudio update
# 0.3.1.1167 (2012-12-11)
- Fix reverb bugs
# 0.3.1.1166 (2012-12-09)
- Adjust IR sample rate
# 0.3.1.1165 (2012-12-09)
- Fix MEL/BAS coupler
# 0.3.1.1164 (2012-12-08)
- Workaround for sound artifacts of short keypresses
# 0.3.1.1163 (2012-12-08)
- Add delay option
# 0.3.1.1162 (2012-12-05)
- Workaround for velocity 1 problem
# 0.3.1.1161 (2012-12-05)
- Fix wave based tremulants
# 0.3.1.1160 (2012-12-03)
- Adapt to current mingw-w64
# 0.3.1.1159 (2012-12-02)
- Add reverb engine
# 0.3.1.1158 (2012-12-02)
- Enable Reverb Settings
# 0.3.1.1157 (2012-12-02)
- Add FFTW to the build
# 0.3.1.1156 (2012-12-02)
- Reverb configuration GUI
# 0.3.1.1155 (2012-12-02)
- Add settings for reverb
# 0.3.1.1154 (2012-12-02)
- Include fftw in the build system
# 0.3.1.1153 (2012-12-01)
- Fix manual width calculation
# 0.3.1.1152 (2012-11-30)
- Import FFTW
# 0.3.1.1151 (2012-11-28)
- Add hook for a reverb engine
# 0.3.1.1150 (2012-11-28)
- Fix effect stop initialization
# 0.3.1.1149 (2012-11-28)
- Fix various GOrgueWave bugs
# 0.3.1.1148 (2012-11-28)
- Fix perftest
# 0.3.1.1147 (2012-11-28)
- Portaudio upstream update
# 0.3.1.1146 (2012-11-24)
- remove gaussian window option for resampler
# 0.3.1.1145 (2012-11-24)
- change max positive factor in an attempt to conserve power across all sub-filters.
# 0.3.1.1144 (2012-11-24)
- fix gain level in filter calculation (the window function still causes some losses). first tap only should almost achieve PR (minus gain losses).
# 0.3.1.1143 (2012-11-21)
- Fix volume scaling
# 0.3.1.1142 (2012-11-14)
- simplify the options setting for the build using the proper cmake constructs. this also adds some documentation for our settings
# 0.3.1.1141 (2012-11-14)
- do not install translations and dependencies in OS X packages as they are alreadly associated with the bundle executable.
# 0.3.1.1140 (2012-11-12)
- Introduce Update Hook
# 0.3.1.1139 (2012-11-12)
- Introduce Combination state
# 0.3.1.1138 (2012-11-12)
- Initialize setter buttons
# 0.3.1.1137 (2012-11-12)
- Create internal drawstop state
# 0.3.1.1136 (2012-11-11)
- Rework combination file saving
# 0.3.1.1135 (2012-11-11)
- Fix single attack loading mode
# 0.3.1.1134 (2012-11-11)
- Support writing bools
# 0.3.1.1133 (2012-11-06)
- Fix attack selection
# 0.3.1.1132 (2012-11-06)
- Fix floating panel layout bug
# 0.3.1.1131 (2012-11-04)
- Enhance generals with bank support
# 0.3.1.1130 (2012-11-04)
- Fix setter buttons on Windows
# 0.3.1.1129 (2012-11-03)
- Implement Aftertouch support
# 0.3.1.1128 (2012-11-03)
- Allow to scale volume by velocity
# 0.3.1.1127 (2012-11-03)
- Adjust names
# 0.3.1.1126 (2012-11-03)
- Document new settings
# 0.3.1.1125 (2012-10-31)
- Fix attack loading
# 0.3.1.1124 (2012-10-30)
- Allow to select different attack samples by velocity
# 0.3.1.1123 (2012-10-30)
- Pass velocity to manual
# 0.3.1.1122 (2012-10-30)
- Fix assert
# 0.3.1.1121 (2012-10-29)
- Prepare for velocity handling in the sound engine
# 0.3.1.1120 (2012-10-29)
- Allow selectiv loading of specific channels
# 0.3.1.1119 (2012-10-29)
- Add velocity support to manual/coupler/stop
# 0.3.1.1118 (2012-10-29)
- Add velocity support to GOrgueRank
# 0.3.1.1117 (2012-10-29)
- Add Velocity support to GOrguePipe
# 0.3.1.1116 (2012-10-29)
- Decouple GOrgueTremulant from the tremulant type
# 0.3.1.1115 (2012-10-29)
- Fade volume changes
# 0.3.1.1114 (2012-10-28)
- Correct spec file
# 0.3.1.1113 (2012-10-28)
- Allow to specify revision via CMake parameter
# 0.3.1.1112 (2012-10-27)
- Adapt for wx2.9
# 0.3.1.1111 (2012-10-27)
- Add proper command line handling and multi-instance support
# 0.3.1.1110 (2012-10-22)
- Fix delete shortcut key.
# 0.3.1.1109 (2012-10-19)
- Allow shortcut configuration
# 0.3.1.1108 (2012-10-19)
- Fix coding style
# 0.3.1.1107 (2012-10-16)
- Mingw64 already ships these headers
# 0.3.1.1106 (2012-10-16)
- Adapt for new mingw64
# 0.3.1.1105 (2012-10-16)
- Fix license string
# 0.3.1.1104 (2012-10-16)
- Improve GOrgueLogWindow
# 0.3.1.1103 (2012-10-16)
- Get rid of g_sound
# 0.3.1.1102 (2012-10-16)
- Unify Doc/View class names
# 0.3.1.1101 (2012-10-16)
- Rework reload
# 0.3.1.1100 (2012-10-16)
- Use ID_FILE_RELOAD
# 0.3.1.1099 (2012-10-16)
- Upstream update
# 0.3.1.1098 (2012-10-16)
- Upstream update
# 0.3.1.1097 (2012-10-16)
- Upstream update
# 0.3.1.1096 (2012-10-16)
- Upstream update
# 0.3.1.1095 (2012-10-15)
- Prefere C++11 mutexes over wxWidgets mutexes
# 0.3.1.1094 (2012-10-15)
- Check for C++11 mutex support
# 0.3.1.1093 (2012-10-15)
- Raise maximum polyphony limit
# 0.3.1.1092 (2012-10-15)
- Rework logging
# 0.3.1.1091 (2012-10-15)
- Convert shortcut to backend object property (ODF function change)
# 0.3.1.1090 (2012-10-15)
- Determine CPU count
# 0.3.1.1089 (2012-10-15)
- Fix warnings
# 0.3.1.1088 (2012-10-01)
- Fix unison off bug
# 0.3.1.1087 (2012-09-13)
- Convert fader to a class
# 0.3.1.1086 (2012-09-13)
- Precalculate factors
# 0.3.1.1085 (2012-09-03)
- Simplify Polyphase code (remove filter_index)
# 0.3.1.1084 (2012-08-31)
- Monitor CPU usage under Linux
# 0.3.1.1083 (2012-08-31)
- Document save
# 0.3.1.1082 (2012-08-31)
- Build fix
# 0.3.1.1081 (2012-08-17)
- Allow saving via MIDI
# 0.3.1.1080 (2012-08-16)
- MIDI output device config
# 0.3.1.1079 (2012-08-16)
- MIDI sender configuration dialog
# 0.3.1.1078 (2012-08-16)
- Midi output support
# 0.3.1.1077 (2012-08-16)
- Midi encoding function
# 0.3.1.1076 (2012-08-16)
- Implement GOrgueMidiSender
# 0.3.1.1075 (2012-08-16)
- Use safer default value
# 0.3.1.1074 (2012-08-16)
- Cleanup
# 0.3.1.1073 (2012-08-16)
- Add missing functions
# 0.3.1.1072 (2012-08-13)
- Refactor MIDIEventDialog
# 0.3.1.1071 (2012-08-13)
- Add GOrgueMidiSender stub
# 0.3.1.1070 (2012-08-13)
- Add load/save to GOrgueLabel
# 0.3.1.1069 (2012-08-13)
- Add missing prepareplayback/abort methods
# 0.3.1.1068 (2012-08-09)
- Store MIDI Output config
# 0.3.1.1067 (2012-08-09)
- Refactor OrganDocument
# 0.3.1.1066 (2012-08-09)
- Don't use g_sound in MIDIEventDialog
# 0.3.1.1065 (2012-08-09)
- Remove g_sound for MIDIListenDialog
# 0.3.1.1064 (2012-08-09)
- Refactor GOrgueMidi
# 0.3.1.1063 (2012-08-09)
- Refactor MIDI config handling
# 0.3.1.1062 (2012-08-09)
- Use unique output names
# 0.3.1.1061 (2012-08-09)
- Fix bug
# 0.3.1.1060 (2012-08-07)
- Allow to override pitch information
# 0.3.1.1059 (2012-08-05)
- Make pitch/temperaments MIDI controlable
# 0.3.1.1058 (2012-07-26)
- Add swell functions to all organs
# 0.3.1.1057 (2012-07-26)
- Add more temperaments (Lars grouping)
# 0.3.1.1056 (2012-07-26)
- Switch General to GOrgueCombination
# 0.3.1.1055 (2012-07-22)
- Use GOrgueCombination for Divisionals
# 0.3.1.1054 (2012-07-22)
- Fill combination definitions
# 0.3.1.1053 (2012-07-20)
- Fix config reader
# 0.3.1.1052 (2012-07-12)
- Add state to GOrgueCombination
# 0.3.1.1051 (2012-07-12)
- Common structure for for storing combinations
# 0.3.1.1050 (2012-07-12)
- Report invalid integer values
# 0.3.1.1049 (2012-07-12)
- Allow to import GO 0.2 settings => Revert does not need to touch ODFs any more.
# 0.3.1.1048 (2012-07-12)
- Document shortcut keys
# 0.3.1.1047 (2012-07-12)
- Clarify ODF
# 0.3.1.1046 (2012-07-06)
- Document building the wxWidgets translations
# 0.3.1.1045 (2012-07-06)
- Separate ODF settings from user settings
# 0.3.1.1044 (2012-07-06)
- Check for invalid boolean values
# 0.3.1.1043 (2012-07-06)
- Catch invalid float values in the ODF
# 0.3.1.1042 (2012-07-06)
- Catch invalid enum in the ODF
# 0.3.1.1041 (2012-07-06)
- Don't allow invalid size values in the ODF
# 0.3.1.1040 (2012-07-06)
- Introduce base class for combinations
# 0.3.1.1039 (2012-07-06)
- Detect invalid pipe references
# 0.3.1.1038 (2012-07-06)
- Fix assert for short loops
# 0.3.1.1037 (2012-07-06)
- Report invalid number from the user
# 0.3.1.1036 (2012-07-06)
- Group names are not case sensitive
# 0.3.1.1035 (2012-07-06)
- Refactor GrandOrgueFile
# 0.3.1.1034 (2012-07-06)
- Lock contention debugger
# 0.3.1.1033 (2012-06-27)
- Allow to specify GC state
# 0.3.1.1032 (2012-06-27)
- Allow to specify a mouse rectangle for manual keys
# 0.3.1.1031 (2012-06-25)
- fix an installer issue placing the english translation in the wrong place
# 0.3.1.1030 (2012-06-24)
- Fix like to organ information
# 0.3.1.1029 (2012-06-24)
- Increase manual/pedal limits
# 0.3.1.1028 (2012-06-22)
- Add floating manuals
# 0.3.1.1027 (2012-06-22)
- Introduce GetODFManualCount
# 0.3.1.1026 (2012-06-12)
- Fix license string
# 0.3.1.1025 (2012-06-12)
- Allow overriding loops via ODF
# 0.3.1.1024 (2012-06-12)
- Close splash when pressing a key
# 0.3.1.1023 (2012-06-08)
- Add dependency on update-desktop-files
# 0.3.1.1022 (2012-06-07)
- Add Fedora support
# 0.3.1.1021 (2012-06-07)
- Add a simple man page
# 0.3.1.1020 (2012-06-07)
- Use copyright element
# 0.3.1.1019 (2012-06-07)
- Fix deskop file warnings
# 0.3.1.1018 (2012-06-07)
- Correct FSF address
# 0.3.1.1017 (2012-06-07)
- Add linux file association
# 0.3.1.1016 (2012-06-05)
- Remove wx 2.9 only function.
# 0.3.1.1015 (2012-06-04)
- Implement grouping in temperament menu
# 0.3.1.1014 (2012-06-04)
- Force at least one channel per audio device
# 0.3.1.1013 (2012-06-03)
- rework the splash dialog - still need to implement key press cancellation
# 0.3.1.1012 (2012-06-03)
- Fix help build system
# 0.3.1.1011 (2012-06-03)
- Register file extension in NSIS installer
# 0.3.1.1010 (2012-06-03)
- Updated swedish translation.
# 0.3.1.1009 (2012-06-02)
- enlarge the width of the settings dialog
# 0.3.1.1008 (2012-06-02)
- allow GrandOrgue to have organ definitions dragged into it on OS X to load
# 0.3.1.1007 (2012-06-02)
- fix broken generation of some plist variables. remove OS X obsoleted plist variables. re-order the copyright so that the most current is shown first
# 0.3.1.1006 (2012-06-02)
- put dynamic dependencies in the right location for OS X bundles
# 0.3.1.1005 (2012-06-02)
- use LC_PATH environment variable to search for translations. default INSTALL_DEPENDS to on for Mac installation. copy wxWidgets translations correctly for Mac build
# 0.3.1.1004 (2012-05-30)
- Added new temperaments.
# 0.3.1.1003 (2012-05-29)
- fix translation install directory issue
# 0.3.1.1002 (2012-05-27)
- always create a directory for English translations otherwise some menus and the help will be translated to a non-English language in an English environment
# 0.3.1.1001 (2012-05-26)
- fix wxFlexGridSizer initialisation in SettingsOption.cpp
# 0.3.1.1000 (2012-05-26)
- correct build structure for os x builds
# 0.3.1.999 (2012-05-26)
- add organ type to application plist to associate .organ files with GrandOrgue
# 0.3.1.998 (2012-05-26)
- Updated swedish translation.
# 0.3.1.997 (2012-05-24)
- insert the 2006 copyright for MDA into the plist
# 0.3.1.996 (2012-05-24)
- remove some unnecessary files from the Apple installer
# 0.3.1.995 (2012-05-24)
- put translations in right places for Mac installs
# 0.3.1.994 (2012-05-24)
- put help files in the right location for a Mac install
# 0.3.1.993 (2012-05-24)
- Improve CPack Mac OS X bundle support (actually works now)
# 0.3.1.992 (2012-05-23)
- Correct unrealistic latencies
# 0.3.1.991 (2012-05-22)
- update French translation
# 0.3.1.990 (2012-05-22)
- reorganize task bar and menus
# 0.3.1.989 (2012-05-21)
- Fix crash
# 0.3.1.988 (2012-05-20)
- Reset meters when changing values
# 0.3.1.987 (2012-05-20)
- Add overflow checking
# 0.3.1.986 (2012-05-20)
- Fix meter reset
# 0.3.1.985 (2012-05-20)
- Fix parameter passing problem
# 0.3.1.984 (2012-05-17)
- Fix mouse axis
# 0.3.1.983 (2012-05-17)
- Fix shift key
# 0.3.1.982 (2012-05-17)
- Refactor config reading
# 0.3.1.981 (2012-05-17)
- Add missing files
# 0.3.1.980 (2012-05-17)
- Rework path handling
# 0.3.1.979 (2012-05-16)
- Introduce GOrgueConfigReader
# 0.3.1.978 (2012-05-16)
- Move config writing into a new class
# 0.3.1.977 (2012-05-16)
- Annotate config setting type
# 0.3.1.976 (2012-05-16)
- Clarify ODF format
# 0.3.1.975 (2012-05-15)
- fix TDM64 build linker flag
# 0.3.1.974 (2012-05-15)
- Adjust scaled release for more realistic play
# 0.3.1.973 (2012-05-15)
- Increment version
# 0.3.1.972 (2012-05-13)
- Implement sample memory limit
# 0.3.1.971 (2012-05-13)
- Report memory pool usage
# 0.3.1.970 (2012-05-03)
- Enhance midi matching
# 0.3.1.969 (2012-05-03)
- Report not supported configuration
# 0.3.1.968 (2012-05-02)
- Add experimental support for adding more than one audio interface
# 0.3.1.967 (2012-05-01)
- Downgrade error message
# 0.3.1.966 (2012-04-21)
- Syntax error HAvE_ATOMIC corrected to HAVE_ATOMIC
# 0.3.1.965 (2012-04-21)
- reduce BLOKS_PER_FRAME from 64 to 16 to accommodate for sound interfaces that need buffer size in multiple of 16 
# 0.3.1.964 (2012-04-21)
- Reserve memory patch from MK
# 0.3.1.963 (2012-04-21)
- Improve threading  patch from MK
# 0.3.1.962 (2012-04-21)
- Lockless sampler pool patch from MK
# 0.3.1.961 (2012-04-18)
- Rework locking for Windows
# 0.3.1.960 (2012-04-18)
- Cleanup
# 0.3.1.959 (2012-04-16)
- Improve out of memory handling
# 0.3.1.958 (2012-04-14)
- Generic falback atomic implementation
# 0.3.1.957 (2012-04-14)
- Helper code for enabling the fallback code
# 0.3.1.956 (2012-04-11)
- Change load abort behaviour
# 0.3.1.955 (2012-04-11)
- Fix memory pool bugs
# 0.3.1.954 (2012-04-10)
- ensure that fonts are always valid even if the face is unavailable - move SetPointSize calls to be after SetFaceName
# 0.3.1.953 (2012-04-10)
- fix some assertions on 64-bit osx builds
# 0.3.1.952 (2012-04-10)
- make GOrguePipeUpdateCallback have a virtual destructor
# 0.3.1.951 (2012-04-03)
- Switch to std::atomic
# 0.3.1.950 (2012-03-31)
- Get rid of default audio setting
# 0.3.1.949 (2012-03-31)
- Remove old audio output settings
# 0.3.1.948 (2012-03-31)
- Implement multi channel support
# 0.3.1.947 (2012-03-31)
- Allow to configure audio groups
# 0.3.1.946 (2012-03-31)
- Add new audio group and output to the settings dialog
# 0.3.1.945 (2012-03-31)
- Use SamplesPerBuffer Setting
# 0.3.1.944 (2012-03-31)
- Use std::atomic (c++0x mode)
# 0.3.1.943 (2012-03-31)
- Restrict to one sound interface at the moment
# 0.3.1.942 (2012-03-31)
- Split toolbar creation from menu setup
# 0.3.1.941 (2012-03-31)
- Rework menu setup
# 0.3.1.940 (2012-03-30)
- Revert "Adjust GO window horizontal size when using DispScreenSizeHoriz=Small setting in order to accommodate GO larger control panel"
# 0.3.1.939 (2012-03-30)
- Decrease text length in the toolbar
# 0.3.1.938 (2012-03-30)
- Use correct define for linux test
# 0.3.1.937 (2012-03-29)
- Add SamplesPerBuffer setting to the settings dialog
# 0.3.1.936 (2012-03-29)
- Prepare multi output locking
# 0.3.1.935 (2012-03-29)
- Sound output functions for multi channel support
# 0.3.1.934 (2012-03-29)
- Rework for multiple audio groups
# 0.3.1.933 (2012-03-29)
- Fix C++0X conversion errors
# 0.3.1.932 (2012-03-28)
- Fix GOMutex::TryLock
# 0.3.1.931 (2012-03-27)
- Adjust GO window horizontal size when using DispScreenSizeHoriz=Small setting in order to accommodate GO larger control panel
# 0.3.1.930 (2012-03-27)
- fix reverb: setting reverb did not propagate to sounf at GO start.
# 0.3.1.929 (2012-03-27)
- Add cleanup to mutex
# 0.3.1.928 (2012-03-26)
- Implement mutex with fastpath
# 0.3.1.927 (2012-03-26)
- Minimize locking time
# 0.3.1.926 (2012-03-26)
- Don't block helper threads
# 0.3.1.925 (2012-03-26)
- Make new sampler list handling O(1)
# 0.3.1.924 (2012-03-26)
- Switch to GOMutex
# 0.3.1.923 (2012-03-26)
- GOCondition wrapper
# 0.3.1.922 (2012-03-26)
- Introduce GOMutex wrapper
# 0.3.1.921 (2012-03-26)
- Check for __sync_fetch_and_add
# 0.3.1.920 (2012-03-25)
- fix performace test tool
# 0.3.1.919 (2012-03-24)
- Store audio group in the sampler
# 0.3.1.918 (2012-03-24)
- Add audio group to the model
# 0.3.1.917 (2012-03-24)
- Improve memory pool
# 0.3.1.916 (2012-03-24)
- Skip not necessary release alignments
# 0.3.1.915 (2012-03-22)
- Remove duplicated package name
# 0.3.1.914 (2012-03-22)
- Add new locking helpers
# 0.3.1.913 (2012-03-22)
- Remove outdated copyright statement (suggest by Graham)
# 0.3.1.912 (2012-03-21)
- Include Windows howto in README
# 0.3.1.911 (2012-03-20)
- fix grid sizer constructor arguments to prevent wxwidgets assertion
# 0.3.1.910 (2012-03-20)
- cleanup indents 
# 0.3.1.909 (2012-03-20)
- how to build under Windows using TDM64 or Cygwin
# 0.3.1.908 (2012-03-20)
- Remove single instance check
# 0.3.1.907 (2012-03-19)
- cleanup reverb - corrected range testing from 0 to 5 and missing reverb config writing
# 0.3.1.906 (2012-03-19)
- Improved Reverb GUI
# 0.3.1.905 (2012-03-18)
- Add wxWidgets patch
# 0.3.1.904 (2012-03-18)
- Cleanup reverb
# 0.3.1.903 (2012-03-18)
- adding possibility to reduce organ samples reverberation (shortening release duration) by direct menu control
# 0.3.1.902 (2012-03-18)
- update of French translation
# 0.3.1.901 (2012-03-18)
- Add missing files
# 0.3.1.900 (2012-03-18)
- Prepare for new audio output configuration
# 0.3.1.899 (2012-03-18)
- Read/write audio output configuration
# 0.3.1.898 (2012-03-18)
- Prepare for audio group list
# 0.3.1.897 (2012-03-18)
- Fix panic in the case of sound errors
# 0.3.1.896 (2012-03-18)
- Add cygwin location
# 0.3.1.895 (2012-03-18)
- Change defaults
# 0.3.1.894 (2012-03-15)
- Fix syntax error
# 0.3.1.893 (2012-03-13)
- Reindent to simplify further patches
# 0.3.1.892 (2012-03-13)
- Keep number of supported output channels
# 0.3.1.891 (2012-03-13)
- Fix signedness
# 0.3.1.890 (2012-03-13)
- Correct sample size check
# 0.3.1.889 (2012-03-13)
- Avoid using m_Settings
# 0.3.1.888 (2012-03-13)
- Move sample rate setting
# 0.3.1.887 (2012-03-13)
- Output format is fixed
# 0.3.1.886 (2012-03-13)
- Split stream initialisation
# 0.3.1.885 (2012-03-13)
- Split open of streams into its own function
# 0.3.1.884 (2012-03-13)
- Move stream start into it's own function
# 0.3.1.883 (2012-03-13)
- Inline GetLatency
# 0.3.1.882 (2012-03-13)
- Rework task interface
# 0.3.1.881 (2012-03-13)
- Store sound handles inside of GO_SOUND_OUTPUT
# 0.3.1.880 (2012-03-13)
- Rework done logic
# 0.3.1.879 (2012-03-13)
- Cleanup RtMidi/RtAudio handling
# 0.3.1.878 (2012-03-13)
- Allow the callback to distinguish between different audio interfaces
# 0.3.1.877 (2012-03-13)
- Fix typo
# 0.3.1.876 (2012-03-13)
- Fix format string
# 0.3.1.875 (2012-03-13)
- Fix splash screen (Windows)
# 0.3.1.874 (2012-03-13)
- Rework help for setting dialog
# 0.3.1.873 (2012-03-13)
- Split MIDI device configuration
# 0.3.1.872 (2012-03-13)
- Move load format settings
# 0.3.1.871 (2012-03-12)
- Allow to pass flags to windres
# 0.3.1.870 (2012-03-11)
- Avoid wx2.9 warnings
# 0.3.1.869 (2012-03-11)
- Fix build error with some wx-Widgets configurations
# 0.3.1.868 (2012-03-10)
- Use splitted panels
# 0.3.1.867 (2012-03-10)
- Split setting dialog panels
# 0.3.1.866 (2012-03-10)
- Make splash screen text variable
- ASIO message is fixed included to avoid accidental bugs because of
- conditional compilation
# 0.3.1.865 (2012-03-09)
- Increase DispExtraButtonRows limit
# 0.3.1.864 (2012-03-08)
- Increase DispExtraDrawstopRow limit
# 0.3.1.863 (2012-03-04)
- Rework MIDIInputNumber limit
# 0.3.1.862 (2012-03-03)
- Fix midi number bug
# 0.3.1.861 (2012-03-01)
- Catch more out of memory conditions
# 0.3.1.860 (2012-02-29)
- Override for incorrect release markers
# 0.3.1.859 (2012-02-29)
- Remove outdated check
# 0.3.1.858 (2012-02-28)
- cleanup comments style
# 0.3.1.857 (2012-02-28)
- Check for invalid rank numbers
# 0.3.1.856 (2012-02-28)
- Enlarge volume input control
# 0.3.1.855 (2012-02-27)
- cleanup and comments on reverb management for short notes
# 0.3.1.854 (2012-02-27)
- update on forcing release_offset to 0 when no release marker or cue marker is at the end of the wave file. Compatibility with release waves with incorrect cue marker.
# 0.3.1.853 (2012-02-27)
- scaled release further adjustment for reverberation 
# 0.3.1.852 (2012-02-27)
- force release_offset to 0 when no release marker or no loop in wave. Compatibility with release wave with incorrect cue marker
# 0.3.1.851 (2012-02-26)
- Add IEEE sample format support
# 0.3.1.850 (2012-02-26)
- Make image scaling dblatex compatible
# 0.3.1.849 (2012-02-26)
- Scale help images
# 0.3.1.848 (2012-02-25)
- Multi rank support
# 0.3.1.847 (2012-02-25)
- Add gain option to ODF (=amplitude in dB)
# 0.3.1.846 (2012-02-25)
- adjustment of scaled release parameters for more realistic play
# 0.3.1.845 (2012-02-25)
- reverberation length evaluation, correct evaluation of release sample rate
# 0.3.1.844 (2012-02-25)
- Add documentation for backgrounds
- Origin of the artwork:
- http://sites.google.com/site/odfgrandorgue/modeles-et-motifs/code-des-motifs-d-habillage-du-buffet
- http://sourceforge.net/mailarchive/message.php?msg_id=28870697
# 0.3.1.843 (2012-02-24)
- Add more ODF documentation
# 0.3.1.842 (2012-02-24)
- Updated swedish translation.
# 0.3.1.841 (2012-02-23)
- better reverb management in staccato 
# 0.3.1.840 (2012-02-23)
- Volume control in dB
# 0.3.1.839 (2012-02-23)
- Perpare stop for multi-rank support
# 0.3.1.838 (2012-02-23)
- Fix perftest
# 0.3.1.837 (2012-02-23)
- Portaudio upstream update
# 0.3.1.836 (2012-02-23)
- Portaudio upstream update
# 0.3.1.835 (2012-02-23)
- Portaudio upstream update
# 0.3.1.834 (2012-02-23)
- Portaudio upstream update
# 0.3.1.833 (2012-02-22)
- fix grid sizer set bug
# 0.3.1.832 (2012-02-21)
- Don't crash, if veto for Windchests is not working
# 0.3.1.831 (2012-02-20)
- Fix multi release bug
# 0.3.1.830 (2012-02-20)
- update French translation
# 0.3.1.829 (2012-02-20)
- Fix coupler section name
# 0.3.1.828 (2012-02-19)
- Implement single attack/release memory saving option
# 0.3.1.827 (2012-02-19)
- Implement attack/release load setting in the GUI
# 0.3.1.826 (2012-02-19)
- Add support for explicit tremulant samples and multi attack/release to ODF
# 0.3.1.825 (2012-02-18)
- Prepare logic for explicit tremulant samples
# 0.3.1.824 (2012-02-18)
- Implement multi attack/release in the sound provider
# 0.3.1.823 (2012-02-18)
- Cross-fade support
# 0.3.1.822 (2012-02-18)
- Refactor GOSoundProviderWave
# 0.3.1.821 (2012-02-18)
- Add support to align to different start segements
# 0.3.1.820 (2012-02-18)
- Refactor GOSoundProviderWave
# 0.3.1.819 (2012-02-18)
- Refactor sound provider
# 0.3.1.818 (2012-02-18)
- Prepare data structures for multiple audio sections
# 0.3.1.817 (2012-02-18)
- Prepare for multiple attacks/releases
# 0.3.1.816 (2012-02-18)
- Rework attack/release interface
# 0.3.1.815 (2012-02-18)
- Don't keep pointer to oneshot samplers
# 0.3.1.814 (2012-02-18)
- Add accessors for const ptr vectors
# 0.3.1.813 (2012-02-18)
- Link pipes to windchest
# 0.3.1.812 (2012-02-18)
- offset may only be changed, if compress succeds
# 0.3.1.811 (2012-02-18)
- Initialise compression state from start segement
# 0.3.1.810 (2012-02-15)
- Fix 12/20 bit compressed
# 0.3.1.809 (2012-02-15)
- Fix performance test
# 0.3.1.808 (2012-02-15)
- Fix crash released to 12/20 bit
# 0.3.1.807 (2012-02-15)
- update French translation
# 0.3.1.806 (2012-02-15)
- changed format to allow "Windchest" translation
# 0.3.1.805 (2012-02-15)
- update of French translation
# 0.3.1.804 (2012-02-14)
- Increment revision
# 0.3.1.803 (2012-02-14)
- Adapt PA versions string for GO
# 0.3.1.802 (2012-02-14)
- Use libxslt as build dependency
# 0.3.1.801 (2012-02-14)
- Globaly enable stack realignment
# 0.3.1.800 (2012-02-13)
- Change max random detuning from 2 cents to 0.5 cent
# 0.3.1.799 (2012-02-12)
- Increment revision
# 0.3.1.798 (2012-02-12)
- Allow moving tremulant processing into helper threads
# 0.3.1.797 (2012-02-12)
- Adapt RPM build dependencies
# 0.3.1.796 (2012-02-12)
- Increase number of possible manuals
# 0.3.1.795 (2012-02-11)
- Don't mix interpolation types
# 0.3.1.794 (2012-02-11)
- Realign stack
# 0.3.1.793 (2012-02-08)
- Enable loop unrolling
# 0.3.1.792 (2012-02-08)
- Revert vector patches
# 0.3.1.791 (2012-02-06)
- optimise stereo resample decoder
# 0.3.1.790 (2012-02-05)
- Fix manual bug
# 0.3.1.789 (2012-02-05)
- Use GCC vector extension
# 0.3.1.788 (2012-02-05)
- Use GCC vector extension
# 0.3.1.787 (2012-02-05)
- Use GCC vector extension
# 0.3.1.786 (2012-02-05)
- Start using GCC vector extensions
# 0.3.1.785 (2012-02-05)
- Make final buffer local
# 0.3.1.784 (2012-02-05)
- Use local temp buffer
# 0.3.1.783 (2012-02-05)
- Move volume calculation to ProcessAudioSampler
# 0.3.1.782 (2012-02-05)
- Don't rely on the sampler pointer to check, if data is present
# 0.3.1.781 (2012-02-05)
- Move buffer filling to ProcessAudioSamplers
# 0.3.1.780 (2012-02-05)
- Pass GOSampleEntry type to ProcessAudioSamplers
# 0.3.1.779 (2012-02-05)
- Style cleanups
# 0.3.1.778 (2012-02-05)
- Allow to add windchests
# 0.3.1.777 (2012-02-05)
- Add simple performance test
# 0.3.1.776 (2012-02-05)
- Add test samples from Lars (burea funeral)
- http://sourceforge.net/mailarchive/message.php?msg_id=28789122
# 0.3.1.775 (2012-02-05)
- Implement interpolation choice in the setting dialog
# 0.3.1.774 (2012-02-05)
- Fix linker option detection
# 0.3.1.773 (2012-02-05)
- Portaudio warning cleanup
# 0.3.1.772 (2012-02-05)
- Portaudio warning cleanup
# 0.3.1.771 (2012-02-05)
- Fix portaudio unused variable warnings
# 0.3.1.770 (2012-02-05)
- Fix portaudio warnings
# 0.3.1.769 (2012-02-05)
- restore existing block decode functions
# 0.3.1.768 (2012-02-05)
- re-enable compression
# 0.3.1.767 (2012-02-05)
- remove more dead code and move implementation details of resampler to separate source file
# 0.3.1.766 (2012-02-05)
- remove dead code and add missing filter index setup in release transition
# 0.3.1.765 (2012-02-05)
- fis stupid mistake and reduce number of filter taps
# 0.3.1.764 (2012-02-05)
- basic changes required to use polyphase resampler - breaks compression
# 0.3.1.763 (2012-01-26)
- Rework memory pool
# 0.3.1.762 (2012-01-26)
- Optmize config file access
# 0.3.1.761 (2012-01-26)
- Upstream update
# 0.3.1.760 (2012-01-26)
- change contributor email address
# 0.3.1.759 (2012-01-26)
- mistake in comparison operator
# 0.3.1.757 (2012-01-25)
- Very simple improvement for scalable release waiting for more comprehensive solution
# 0.3.1.755 (2012-01-24)
- Merge help-howto into README
# 0.3.1.754 (2012-01-24)
- Add missing file
# 0.3.1.753 (2012-01-24)
- Implement noraml dialog behaviour
# 0.3.1.752 (2012-01-24)
- Allow to configure per pipe load settings
# 0.3.1.751 (2012-01-24)
- Handle out of memory condition
# 0.3.1.750 (2012-01-24)
- Convert Tremulant initialisation to GOrgueCacheObject
# 0.3.1.749 (2012-01-24)
- Abstract loading process from the cache
# 0.3.1.748 (2012-01-24)
- per-pipe windchest setting
# 0.3.1.747 (2012-01-24)
- Use per-pipe settings for loading
# 0.3.1.746 (2012-01-24)
- Calculate effective per-pipe load settings
# 0.3.1.745 (2012-01-24)
- Store per Pipe sample format settings
# 0.3.1.744 (2012-01-24)
- Don't play uninitialized samples
# 0.3.1.743 (2012-01-24)
- Empty file list before writing
# 0.3.1.742 (2012-01-24)
- Cleanup of French translation
# 0.3.1.741 (2012-01-23)
- French translation update
# 0.3.1.740 (2012-01-23)
- Build help and translations on windows
- Updated build stack
- Added a cookbook
# 0.3.1.739 (2012-01-23)
- Updated swedish translation.
# 0.3.1.738 (2012-01-23)
- Add Bach (Bradley Lehman) temperament
# 0.3.1.737 (2012-01-23)
- Add check box to ignore organ sample pitch info (in organ setting)
# 0.3.1.736 (2012-01-23)
- Fix layout problem (GTK)
# 0.3.1.735 (2012-01-22)
- Rework key mapping table (based on wx 2.6)
- The old versions depends on the order in wx-Headers, which already has been changed for some keys
# 0.3.1.734 (2012-01-20)
- Update copyright
# 0.3.1.733 (2012-01-20)
- Keep refencence information out of the cache
- It provides more accurate predictions in the progress dialog too
# 0.3.1.732 (2012-01-20)
- Move reference intialisation to one function
# 0.3.1.731 (2012-01-20)
- Move pipe creation to GOrgueRank
# 0.3.1.730 (2012-01-20)
- Store pipes inside of GOrgueRank
# 0.3.1.729 (2012-01-20)
- Store name in rank
# 0.3.1.728 (2012-01-20)
- Allow to specify names for the windchest via ODF
# 0.3.1.727 (2012-01-20)
- Change tuning dialog to use windchest/rank structure
# 0.3.1.726 (2012-01-20)
- Link ranks to their windchests
# 0.3.1.725 (2012-01-20)
- Parent of pipes it the rank object
# 0.3.1.724 (2012-01-20)
- Read/write sample data via rank
# 0.3.1.723 (2012-01-20)
- Route tuning via Rank object
# 0.3.1.722 (2012-01-20)
- Remove dead code
# 0.3.1.721 (2012-01-20)
- Add missing files
# 0.3.1.720 (2012-01-20)
- Introduce rank object
# 0.3.1.719 (2012-01-20)
- Include file cleanup
# 0.3.1.718 (2012-01-20)
- GO has less strict block size requirements
# 0.3.1.717 (2012-01-17)
- Memory pool for windows
# 0.3.1.716 (2012-01-17)
- Memory mapped cache for window
# 0.3.1.715 (2012-01-17)
- Report memory pool problems
# 0.3.1.714 (2012-01-17)
- Display pool size limit
# 0.3.1.713 (2012-01-17)
- Fix threaded related release click bug
# 0.3.1.712 (2012-01-17)
- Correct height of manual images
# 0.3.1.711 (2012-01-17)
- Correct height of pedal images
# 0.3.1.710 (2012-01-15)
- Allow to use an external portaudio library
# 0.3.1.709 (2012-01-15)
- Link win32 version as large address aware
# 0.3.1.708 (2012-01-11)
- Fix manual position
# 0.3.1.707 (2012-01-11)
- Upstream portaudio update
# 0.3.1.706 (2012-01-11)
- Fix amplitude regression (r690)
# 0.3.1.705 (2012-01-11)
- Check correctly for invalid loop information
# 0.3.1.704 (2012-01-10)
- Check for allocation errors
# 0.3.1.703 (2012-01-10)
- Remove old manual code
# 0.3.1.702 (2012-01-09)
- Updated swedish translation.
# 0.3.1.701 (2012-01-08)
- wxTR_EXTENDED is not portable
# 0.3.1.700 (2012-01-08)
- Implement loop load option
# 0.3.1.699 (2012-01-08)
- Ignore return values
# 0.3.1.698 (2012-01-08)
- fix another compression bug
# 0.3.1.697 (2012-01-08)
- fix some issues in new engine code (including jeux great cornet top-a issue)
# 0.3.1.696 (2012-01-06)
- Fix MIDI event dialog bug
# 0.3.1.695 (2012-01-06)
- Store ODF path in the setting file for informational purposes
# 0.3.1.694 (2012-01-06)
- Multiple loops should overlap
# 0.3.1.693 (2012-01-06)
- Mark portaudio devices
- Portaudio backend will probably be experimental in the next release, so that the user can recognize them
# 0.3.1.692 (2012-01-06)
- Enlarge setting dialog to be able to display larger device names
# 0.3.1.691 (2012-01-06)
- Enable new manual code
# 0.3.1.690 (2012-01-06)
- merge in audio section updates and support for multiple loops
# 0.3.1.689 (2012-01-03)
- reset should not open the audio unless it was previously opened (MK)
# 0.3.1.688 (2012-01-03)
- remove fomit-frame-pointer from debug builds to prevent backtrace issues
# 0.3.1.687 (2012-01-03)
- fix up issues with release scaling
# 0.3.1.686 (2011-12-31)
- close any opened sound device when OpenSound() is called
# 0.3.1.685 (2011-12-31)
- fix mac portaudio build
# 0.3.1.684 (2011-12-30)
- Add prototype for the new manual code
# 0.3.1.683 (2011-12-30)
- Add manual artwork to the bitmap cache
# 0.3.1.682 (2011-12-30)
- Include manual images in the GO build
# 0.3.1.681 (2011-12-30)
- Add first version of artwort for the new manual code
# 0.3.1.680 (2011-12-30)
- Verify mask size
# 0.3.1.679 (2011-12-30)
- Duplicate old manual code as fallback
# 0.3.1.678 (2011-12-30)
- Updated swedish translation.
# 0.3.1.677 (2011-12-28)
- Custom enclosures
# 0.3.1.676 (2011-12-28)
- Allow variable sizes for the layout model
# 0.3.1.675 (2011-12-28)
- ODF is documented in the help
# 0.3.1.674 (2011-12-28)
- Remove dead code
# 0.3.1.673 (2011-12-27)
- Replace fixed widths with constants in the layout model
# 0.3.1.672 (2011-12-27)
- Switch enclosure drawing to bitmaps
# 0.3.1.671 (2011-12-27)
- Images for enclosures
# 0.3.1.670 (2011-12-24)
- Recommend mingw-w64 instead of mingw
# 0.3.1.669 (2011-12-24)
- Separate cache directory from settings directory to allow an easier backup for users
# 0.3.1.668 (2011-12-24)
- Add missing path translation
# 0.3.1.667 (2011-12-24)
- Fix group name
# 0.3.1.666 (2011-12-24)
- Don't reserve space for hidden enclosures
# 0.3.1.665 (2011-12-24)
- Allow to hide enclosures on the main panel
# 0.3.1.664 (2011-12-24)
- More ODF documentation
# 0.3.1.663 (2011-12-24)
- More ODF documentation
# 0.3.1.662 (2011-12-24)
- More ODF documentation
# 0.3.1.661 (2011-12-24)
- Document more objects
# 0.3.1.660 (2011-12-22)
- More ODF documentation
# 0.3.1.659 (2011-12-21)
- Add temerament data from Lars
# 0.3.1.658 (2011-12-20)
- Updated swedish translation.
# 0.3.1.657 (2011-12-19)
- Randomized detuning
# 0.3.1.656 (2011-12-19)
- Store wave midikey/pitch in sound provider
# 0.3.1.655 (2011-12-19)
- Add new file
# 0.3.1.654 (2011-12-19)
- Move audio section caching from GOSoundProvider to the GOAudioSection class (Nick)
# 0.3.1.653 (2011-12-19)
- Name the members of GOAudioSection in line with naming conventions (Nick)
# 0.3.1.652 (2011-12-19)
- ove some existing AUDIO_SECTION functions into the GOAudioSection class (Nick)
# 0.3.1.651 (2011-12-19)
- Make AUDIO_SECTION a class (Nick)
# 0.3.1.650 (2011-12-19)
- Fix another midi number bug
# 0.3.1.649 (2011-12-19)
- Add pitch correction factor
# 0.3.1.648 (2011-12-19)
- Add equal temperament
# 0.3.1.647 (2011-12-19)
- Add GUI selection for temperaments
# 0.3.1.646 (2011-12-19)
- Read Midi note and pitch fraction
# 0.3.1.645 (2011-12-19)
- Add harmonic number parameter in ODF
# 0.3.1.644 (2011-12-19)
- Add temperament change to the model
# 0.3.1.643 (2011-12-19)
- Add temperament class
# 0.3.1.642 (2011-12-18)
- Fix performance bug for compressed samples
# 0.3.1.641 (2011-12-18)
- Fix midi number generation
# 0.3.1.640 (2011-12-18)
- Use a dynamic buffer size for portaudio
# 0.3.1.639 (2011-12-18)
- Report sound configuration problems to the user
# 0.3.1.638 (2011-12-18)
- Prepare for additional sample rates
# 0.3.1.637 (2011-12-18)
- Avoid storing ints as pointer
# 0.3.1.636 (2011-12-18)
- Remove unused variable
# 0.3.1.635 (2011-12-17)
- Store Midi note number in GOrguePipe
# 0.3.1.634 (2011-12-17)
- Handle negative window positions
- Problem hit by a windows user
# 0.3.1.633 (2011-12-17)
- Disable unsupported maximize button
# 0.3.1.632 (2011-12-17)
- Add portaudio support to GO
# 0.3.1.631 (2011-12-17)
- Compile portaudio library
# 0.3.1.630 (2011-12-17)
- Move RtAudio sound setup into an own block
# 0.3.1.629 (2011-12-17)
- Show sound open errors as messagebox
# 0.3.1.628 (2011-12-17)
- Add more portaudio strings to pa_errors.cpp
# 0.3.1.627 (2011-12-17)
- Fix default audio selection
# 0.3.1.626 (2011-12-17)
- Cleanup CR/LF usage
# 0.3.1.625 (2011-12-13)
- CMake build file for portaudio
# 0.3.1.624 (2011-12-13)
- Add source file with portaudio error strings in a translatable format
# 0.3.1.623 (2011-12-13)
- Includes to fix build problems
# 0.3.1.622 (2011-12-13)
- Add modified mingw includes
# 0.3.1.621 (2011-12-13)
- Add includes from wine distributions
# 0.3.1.620 (2011-12-13)
- Add portaudio source
# 0.3.1.619 (2011-12-13)
- Update debian copyright for portaudio
# 0.3.1.618 (2011-12-13)
- Default location for soundcard.h is sys under linux
# 0.3.1.617 (2011-12-11)
- Revert RtAudio DirectSound patch (MK)
# 0.3.1.616 (2011-12-10)
- Updated swedish translation.
# 0.3.1.615 (2011-12-10)
- Force use of wxID_OPEN as ProcessCommand() in wxWidgets 2.9.2+ checks that the parent toolbar has an element with this ID and will not open files if it is not present.
# 0.3.1.614 (2011-12-10)
- Fix number of grid rows in settings dialog.
# 0.3.1.613 (2011-12-10)
- Rework multi entry selection (MK)
# 0.3.1.612 (2011-12-10)
- Rework layout (MK)
# 0.3.1.611 (2011-12-10)
- Unify pipe config (MK)
# 0.3.1.610 (2011-12-10)
- Rework per-pipe amplitude setting (MK)
# 0.3.1.609 (2011-12-10)
- Use signed ints for width constants to avoid conversation problems (MK)
# 0.3.1.608 (2011-12-10)
- Fix channel 16 (MK)
# 0.3.1.607 (2011-12-10)
- Add tuning settings to the GUI (MK)
# 0.3.1.606 (2011-12-10)
- Introduce tuning setting (MK)
# 0.3.1.605 (2011-12-10)
- Very simple retuning of samples (MK)
# 0.3.1.604 (2011-12-10)
- Allow changing the global amplitude too (MK)
# 0.3.1.603 (2011-12-10)
- Organ setting dialog (MK)
# 0.3.1.602 (2011-12-10)
- Allow runtime update of amplitude (MK)
# 0.3.1.601 (2011-12-10)
- Allow to write floats (MK)
# 0.3.1.600 (2011-12-10)
- Avoid calling SaveHelper with size_t (MK)
# 0.3.1.599 (2011-12-10)
- Add JP Marsan to AUTHORS
# 0.3.1.598 (2011-12-10)
- 64-bit windows support (MK)
# 0.3.1.597 (2011-12-10)
- Fix pointer type (MK)
# 0.3.1.596 (2011-12-10)
- Allow per-Pipe amplitude override (MK)
# 0.3.1.595 (2011-12-10)
- Don't cache Gain and prepare GOrguePipe for per-pipe settings
- The gain is fast to recalculate and this allows changing the amplitude without cache rebuild. (MK)
# 0.3.1.594 (2011-12-10)
- Fix RtAudio direct sound (MK)
# 0.3.1.593 (2011-12-10)
- Replace constants with variables (MK)
# 0.3.1.592 (2011-12-10)
- Fix reload bug (MK)
# 0.3.1.591 (2011-12-10)
- Fix author list according to Graham (MK)
# 0.3.1.590 (2011-12-10)
- Remove OS X guards from GrandOrgueFrame.cpp - instead require a 2.9.2+ version of wxWidgets.
# 0.3.1.589 (2011-12-10)
- Revert use of spaces for indents in GrandOrgue.cpp
# 0.3.1.586 (2011-12-09)
- Update source headers (remove mentions of MyOrgan, add copyright for GrandOrgue contributors, correct the FSF address) and update and unify license files. (MK)
# 0.3.1.585 (2011-12-09)
- Don't mention myorgan in the function description (MK)
# 0.3.1.584 (2011-12-09)
- List contributors (MK)
# 0.3.1.583 (2011-12-09)
- Implement presents (MK)
# 0.3.1.582 (2011-12-09)
- Implement setting store (MK)
# 0.3.1.581 (2011-12-09)
- Add option for setting setting/cache path (MK)
# 0.3.1.580 (2011-12-09)
- Move from XPM to PNG icons (MK)
# 0.3.1.579 (2011-12-09)
- Allow export of combinations (MK)
# 0.3.1.578 (2011-12-09)
- Prevent crash, if generals contain invalid data (MK)
# 0.3.1.577 (2011-12-09)
- Improve Mac support + fix uninitialized variable in GOrgueSound.
# 0.3.1.576 (2011-12-08)
- Updated and cleaned version of French translation
# 0.3.1.575 (2011-12-08)
- add missing ")" removed by mistake 
# 0.3.1.574 (2011-12-08)
- Add-support-to-align-stack-at-entry-points (MK)
# 0.3.1.573 (2011-12-08)
- French translation file for GO
# 0.3.1.572 (2011-12-08)
- fix missing asiohelpers/include directory in CmakeLists.txt for rtaudio
# 0.3.1.571 (2011-12-07)
- Use variables for filenames (MK)
# 0.3.1.570 (2011-12-06)
- Martin's DirectSound fix
# 0.3.1.569 (2011-12-05)
- Preliminary support for a Mac build
# 0.3.1.568 (2011-11-25)
- Don-t-clobber-original-ODF-with-user-settings.patch (MK)
# 0.3.1.567 (2011-11-25)
- Add-rpm-build-files.patch (MK)
# 0.3.1.566 (2011-11-25)
- Fix-for-openSuSE.patch (MK)
# 0.3.1.565 (2011-11-25)
- Fix-debian-ubuntu-build.patch (MK)
# 0.3.1.564 (2011-11-25)
- Disable-verification-of-compression-code.patch (MK)
# 0.3.1.563 (2011-11-25)
- Fix-mode.patch manually applied (MK)
# 0.3.1.562 (2011-11-25)
- Implement-12-20-bit-support.patch (MK)
# 0.3.1.561 (2011-11-25)
- Display-memory-usage-information.patch (MK)
# 0.3.1.560 (2011-11-25)
- X-broken-by-wxWS_EX_VALIDATE_RECURSIVELY.patch (MK)
# 0.3.1.559 (2011-11-23)
- Reimplement-compression.patch (MK)
# 0.3.1.558 (2011-11-23)
- Free-cache-file-if-hash-is-not-OK.patch (MK)
# 0.3.1.557 (2011-11-23)
- Improve-Int24.patch (MK)
# 0.3.1.556 (2011-11-23)
- Allow-different-bit-rates.patch (MK)
# 0.3.1.555 (2011-11-23)
- Add-playback-support-for-multiple-sample-formats.patch (MK)
# 0.3.1.554 (2011-11-23)
- Abstract-access-to-sample-data.patch (MK)
# 0.3.1.553 (2011-11-23)
- Move-mono-downmixing-to-GOrgueWave.patch (MK)
# 0.3.1.552 (2011-11-23)
- Generalize-sample-reading-code.patch (MK)
# 0.3.1.551 (2011-11-23)
- Allow-to-open-ODFs-even-without-MIDI-devices-to-aid-.patch (MK)
# 0.3.1.550 (2011-11-23)
- Get-rid-of-zlib-cache-warning.patch (MK)
# 0.3.1.549 (2011-11-23)
- Move-non-device-related-settings-to-new-tab.patch (MK)
# 0.3.1.548 (2011-11-23)
- Fix-sampled-rate.patch (MK)
# 0.3.1.547 (2011-11-23)
- Extend-ODF-documentation.patch (MK)
# 0.3.1.546 (2011-11-23)
- Remove-unused-image-code.patch (MK)
# 0.3.1.545 (2011-11-23)
- Rework-label.patch (MK)
# 0.3.1.544 (2011-11-23)
- Document-images.patch (MK)
# 0.3.1.543 (2011-11-23)
- Implement-GUI-support-for-images.patch (MK)
# 0.3.1.542 (2011-11-23)
- Remove-dead-code.patch (MK)
# 0.3.1.541 (2011-11-23)
- Switch-various-implmentations-to-new-button.patch delete files (MK)
# 0.3.1.540 (2011-11-23)
- Switch-various-implmentations-to-new-button.patch (MK)
# 0.3.1.539 (2011-11-23)
- Cleanup-class-structure-for-generals.patch (MK)
# 0.3.1.538 (2011-11-23)
- Add-new-button-implementation.patch (MK)
# 0.3.1.537 (2011-11-23)
- Implement-framework-for-using-custom-bitmaps.patch (MK)
# 0.3.1.536 (2011-11-23)
- Use-absolute-path-for-loading.patch (MK)
# 0.3.1.535 (2011-11-23)
- Support-additional-values-for-font-size-and-color.patch (MK)
# 0.3.1.534 (2011-11-23)
- Additional-file-format-documentation-covering-new-fe.patch (MK)
# 0.3.1.533 (2011-11-23)
- Fix-manual-position.patch (MK)
# 0.3.1.532 (2011-11-23)
- Remove-FILES-from-SDK-rm-imon manually applied (MK)
# 0.3.1.531 (2011-11-23)
- Update-build-instructions manually applied (MK)
# 0.3.1.530 (2011-11-23)
- The-search-logic-does-not-work-on-empty-directories.patch (MK)
# 0.3.1.529 (2011-11-23)
- Add-linux-menu-icon.patch (MK)
# 0.3.1.528 (2011-11-23)
- Only-add-supported-options.patch (MK)
# 0.3.1.527 (2011-11-23)
- Debian-packaging.patch (MK)
# 0.3.1.526 (2011-11-23)
- Copy-dependencies.patch (MK)
# 0.3.1.525 (2011-11-07)
- Include dlls with CPack (MK)
# 0.3.1.524 (2011-11-07)
- Support for adding wxWidgets core translations to the package (MK)
# 0.3.1.523 (2011-11-07)
- Improve cpack settings (MK)
# 0.3.1.522 (2011-11-07)
- Allow to structure the panel menu (MK)
# 0.3.1.521 (2011-11-07)
- Rewrite panel code to place panels directly in the menu bar (MK)
# 0.3.1.520 (2011-11-07)
- Implement platform independent icon (MK)
# 0.3.1.519 (2011-11-07)
- Don't put the version number in internal strings (MK)
# 0.3.1.518 (2011-11-07)
- Remove windows version specifications (MK)
# 0.3.1.517 (2011-11-07)
- Remove old file (MK)
# 0.3.1.516 (2011-11-05)
- Move MIDI processing to main thread (MK)
# 0.3.1.515 (2011-11-02)
- Fix midi close locking (MK)
# 0.3.1.514 (2011-11-02)
- Adjust thread stop (MK)
# 0.3.1.513 (2011-11-02)
- Update cache create status only every second (MK)
# 0.3.1.512 (2011-11-02)
- Cleanup (MK)
# 0.3.1.511 (2011-11-02)
- Frame is private (MK)
# 0.3.1.510 (2011-11-02)
- Use local reference to Sound Engine in GrandOrgueFrame (MK)
# 0.3.1.509 (2011-11-02)
- Avoid using global sound engine reference (MK)
# 0.3.1.508 (2011-11-02)
- Cleanup (MK)
# 0.3.1.507 (2011-11-02)
- Store MIDI preferences in GOrgueSettings (MK)
# 0.3.1.506 (2011-11-02)
- The sound engine can handle an not loaded organ too, so reuse the common case for it (MK)
# 0.3.1.505 (2011-11-01)
- Transpose is also a runtime setting (MK)
# 0.3.1.504 (2011-11-01)
- Include cleanup (MK)
# 0.3.1.503 (2011-11-01)
- Fix Style (MK)
# 0.3.1.502 (2011-11-01)
- Move path settings (MK)
# 0.3.1.501 (2011-11-01)
- Move generation of list of configured MIDI devices (MK)
# 0.3.1.500 (2011-11-01)
- Move MIDI device enabled settings (MK)
# 0.3.1.499 (2011-11-01)
- Only recognize ON events (MK)
# 0.3.1.498 (2011-11-01)
- Adjust locking (MK)
# 0.3.1.497 (2011-11-01)
- Move OrganList to GOrgueSettings (MK)
# 0.3.1.496 (2011-11-01)
- Send Midi Event only as GOrgueMidiEvent (MK)
# 0.3.1.495 (2011-11-01)
- Settings are not part of Sound engine - they are a global object (MK)
# 0.3.1.494 (2011-11-01)
- Move audio device settings (MK)
# 0.3.1.493 (2011-11-01)
- Cleanup (MK)
# 0.3.1.492 (2011-11-01)
- Move more settings (MK)
# 0.3.1.491 (2011-11-01)
- Move more settings (MK)
# 0.3.1.490 (2011-11-01)
- Move concurrency setting to their own object (MK)
# 0.3.1.489 (2011-11-01)
- Start moving runtime setting to own object (MK)
# 0.3.1.488 (2011-11-01)
- Move Volume to GrandOrgueFrame (MK)
# 0.3.1.487 (2011-11-01)
- Move Setter spinctrl to GrandOrgueFrame (MK)
# 0.3.1.486 (2011-11-01)
- Move Polyphony-setting to GrandOrgueFrame (MK)
# 0.3.1.485 (2011-11-01)
- Move transpose handling to GOrgueFrame (MK)
# 0.3.1.484 (2011-11-01)
- Check for main window before accessing it (MK)
# 0.3.1.483 (2011-11-01)
- Remember panel configuration (MK)
# 0.3.1.482 (2011-11-01)
- Allocate something for empty memory blobs (MK)
# 0.3.1.481 (2011-11-01)
- Move wxGaugeAudio handing to GrandOrgueFrame (MK)
# 0.3.1.480 (2011-11-01)
- Event for changing a value (MK)
# 0.3.1.479 (2011-11-01)
- The size of the gague is fixed by the implementation (MK)
# 0.3.1.478 (2011-11-01)
- Reset Meter display at sound close (MK)
# 0.3.1.477 (2011-11-01)
- Fix typo (MK)
# 0.3.1.476 (2011-11-01)
- Fix load of compressed cache (MK)
# 0.3.1.475 (2011-11-01)
- Fix cache update, while in use (MK)
# 0.3.1.474 (2011-11-01)
- Preload sample data (MK)
# 0.3.1.473 (2011-11-01)
- Implement linux specific allocator (MK)
# 0.3.1.472 (2011-11-01)
- Add framework for pool allocator (MK)
# 0.3.1.471 (2011-11-01)
- Implement support for uncompressed cache (MK)
# 0.3.1.470 (2011-11-01)
- Implement support for uncompressed cache (MK)
# 0.3.1.469 (2011-11-01)
- Add GUI option to determine, if the cache should be compressed (MK)
# 0.3.1.468 (2011-11-01)
- Initialiaze Sound Provider for tremulants with other sample data (MK)
# 0.3.1.467 (2011-11-01)
- Adjust locking (MK)
# 0.3.1.466 (2011-11-01)
- Don't let the splash screen delay initialisation (MK)
# 0.3.1.465 (2011-11-01)
- Fix load stereo option (MK)
# 0.3.1.464 (2011-11-01)
- Implement memory pool (MK)
# 0.3.1.463 (2011-11-01)
- Remove unnecessary include (MK)
# 0.3.1.462 (2011-11-01)
- Correct image list broken by my last patch (MK)
# 0.3.1.461 (2011-11-01)
- Add classes for cache IO (MK)
# 0.3.1.460 (2011-11-01)
- Update progress dialog not for every pipe (Burea load time from cache: ~25 -> ~18s) (MK)
# 0.3.1.459 (2011-11-01)
- Avoid crash, if organ file is not fully initialised (MK)
# 0.3.1.458 (2011-11-01)
- Change imageconverter to implement caching (MK)
# 0.3.1.457 (2011-11-01)
- Implement support for range limit couplers (MK)
# 0.3.1.456 (2011-10-29)
- Fix remembering last position (MK)
# 0.3.1.455 (2011-10-29)
- Allow triggering enclosures by the left button too (MK)
# 0.3.1.454 (2011-10-29)
- Allow triggering multiple controls, while the mouse button is down (MK)
# 0.3.1.453 (2011-10-29)
- Add MouseState to track last change (MK)
# 0.3.1.452 (2011-10-29)
- Use main panel background for panels (MK)
# 0.3.1.451 (2011-10-29)
- Correct Manual range checking (MK)
# 0.3.1.450 (2011-10-29)
- Fix ALSA resource leaks (MK)
# 0.3.1.449 (2011-10-29)
- Don't use polling for ALSA event loop (MK)
# 0.3.1.448 (2011-10-29)
- Fix resource leaks in RtMidi (MK)
# 0.3.1.447 (2011-10-29)
- Update RtAudio (MK)
# 0.3.1.446 (2011-10-24)
- Log RtErrors via wxWidgets (MK)
# 0.3.1.445 (2011-10-24)
- Protect concurrent access to m_Threads (MK)
# 0.3.1.444 (2011-10-24)
- Delete license.rtf file and fix the existing license text file (MK)
# 0.3.1.443 (2011-10-23)
- Update Rt* sources (MK)
# 0.3.1.442 (2011-10-23)
- Mark GO specific changes (MK)
# 0.3.1.441 (2011-10-23)
- Use wxString with wxTRANSLATE (MK)
# 0.3.1.440 (2011-10-23)
- Use const wxChar* for string constant (MK)
# 0.3.1.439 (2011-10-23)
- use wxSscanf (MK)
# 0.3.1.438 (2011-10-23)
- Avoid ternary operator usage with wxString (MK)
# 0.3.1.437 (2011-10-23)
- Don't use wx2.6 compatible constants (MK)
# 0.3.1.436 (2011-10-23)
- Don't pass error message as format string (MK)
# 0.3.1.435 (2011-10-23)
- Use wx internal atoi/atof (MK)
# 0.3.1.434 (2011-10-23)
- Call wxLog without any namespace (MK)
# 0.3.1.433 (2011-10-23)
- Use GetEventHandler function (MK)
# 0.3.1.432 (2011-10-23)
- Use newer name WXK_ key constants (MK)
# 0.3.1.431 (2011-10-23)
- Make cpack use license text file (MK)
# 0.3.1.430 (2011-10-13)
- Break up GOrgueSoundTypes.h into several new headers to make include management a bit cleaner.
# 0.3.1.427 (2011-10-11)
- Coupler Panel (MK)
# 0.3.1.426 (2011-10-11)
- Support invalid coupler numbers from ODFs. Allowing GO to use non-ODF defined couplers and store them in the setter means, that the settings will contain internal numbers for those elements. Allowing GO to read invalid numbers and ignore them later is more user friedly than to abort loading the ODF.
# 0.3.1.425 (2011-10-11)
- Check range of setter values at runtime (MK)
# 0.3.1.424 (2011-10-11)
- Divisional-Panel (MK)
# 0.3.1.423 (2011-10-11)
- Unify GUI label loading (MK)
# 0.3.1.422 (2011-10-11)
- Get rid of chained ifs to determine defaults (MK)
# 0.3.1.421 (2011-10-11)
- Fix display order (MK)
# 0.3.1.420 (2011-10-11)
- Don't create default MIDI event, if setting is missing in ODF (MK)
# 0.3.1.419 (2011-10-11)
- Improve RtMidi error handling (MK)
# 0.3.1.418 (2011-10-11)
- Committed patch from Martin Koegler for import of cmb files
# 0.3.1.413 (2011-10-05)
- Update variable names in fader.
# 0.3.1.412 (2011-10-04)
- Split fade state into separate structure. Add new source to manage application of fade in and outs to clean up the engine a bit. Get rid of some comments which are no longer required.
# 0.3.1.411 (2011-10-04)
- Update GOInt24ToInt inline function.
# 0.3.1.410 (2011-10-04)
- Integrate revisions 404 to 408 of the audio float branch (which did not merge properly) back into the trunk.
# 0.3.1.409 (2011-10-04)
- Branch back from audiofloatbranch. Remove usage of doubles. Make audio engine use floating point arithmetic. Rework fade engine calculations. Remove unnecessary scaling shift counters. Make crossfade length easily settable.
# 0.3.1.398 (2011-10-02)
- Catch RtMidi Init Exception (MK)
# 0.3.1.395 (2011-10-02)
- Minor modification to key rendering to stop segments of adjacent playing keys not being drawn
# 0.3.1.394 (2011-10-02)
- Avoid GUI overload with too many unhandled updates (MK)
# 0.3.1.393 (2011-10-02)
- Support for Midi-Device Hotplug (MK)
# 0.3.1.392 (2011-10-01)
- Remove compile time limits from SamplerPool (MK)
# 0.3.1.391 (2011-10-01)
- Reset-Bugfix (MK)
# 0.3.1.390 (2011-10-01)
- Fix problem for single threaded usage (MK)
# 0.3.1.389 (2011-09-28)
- Change float scaling for sound recorder.
# 0.3.1.388 (2011-09-28)
- Don't print debug message as error (MK)
# 0.3.1.387 (2011-09-28)
- Allow reading multiple formats (MK / NA)
# 0.3.1.386 (2011-09-28)
- Support multiple recorder formats (MK)
# 0.3.1.385 (2011-09-28)
- Rework sampler handling (MK)
# 0.3.1.384 (2011-09-28)
- Bug fix for Windows (MK)
# 0.3.1.383 (2011-09-26)
- Updated Swedish translation
# 0.3.1.382 (2011-09-26)
- Always ensure synchronism between attack/loop fade-out and release fade-in. Make sampler time unsigned.
# 0.3.1.381 (2011-09-26)
- Enable threading (tested only with ALSA/linux) (MK)
# 0.3.1.380 (2011-09-26)
- Add GUI configuration for threading (MK)
# 0.3.1.379 (2011-09-26)
- Add infrastructure for thread handling (MK)
# 0.3.1.378 (2011-09-26)
- Add offload thread implementation (MK)
# 0.3.1.377 (2011-09-26)
- Prepare for multi-threading (MK)
# 0.3.1.376 (2011-09-26)
- Clean variable name (MK)
# 0.3.1.375 (2011-09-26)
- Remove global work buffer (MK)
# 0.3.1.374 (2011-09-26)
- Fix list handling (MK)
# 0.3.1.373 (2011-09-26)
- Support more than one release processor (MK)
# 0.3.1.372 (2011-09-26)
- Count sampler usage (MK)
# 0.3.1.371 (2011-09-26)
- Turn detached releases into an array (MK)
# 0.3.1.370 (2011-09-26)
- Process detached release after normal samples (MK)
# 0.3.1.369 (2011-09-26)
- Fix to bad assert (MK)
# 0.3.1.368 (2011-09-26)
- Make sound recorder platform independent (MK)
# 0.3.1.367 (2011-09-24)
- Support reading wave data into a floating point buffer.
# 0.3.1.366 (2011-09-24)
- Minor style cleanups to GOrgueWave
# 0.3.1.365 (2011-09-24)
- Interpolate (MK)
# 0.3.1.364 (2011-09-24)
- Variable sample rate (MK)
# 0.3.1.363 (2011-09-24)
- Let the C-Compiler do the index number calculations (MK)
# 0.3.1.362 (2011-09-24)
- Allow write-protected generals/divisionals for modeling fixed combinations (MK)
# 0.3.1.361 (2011-09-24)
- Implement general cancel (MK)
# 0.3.1.360 (2011-09-23)
- Remove old unnecessary help files.
# 0.3.1.359 (2011-09-23)
- Remove some magic numbers from sound engine.
# 0.3.1.358 (2011-09-23)
- Minor fix to old release alignment code.
# 0.3.1.357 (2011-09-22)
- Commit Martin's help images.
# 0.3.1.356 (2011-09-22)
- Initial translation sample (MK)
# 0.3.1.355 (2011-09-22)
- Activate new build system for help (MK)
# 0.3.1.354 (2011-09-22)
- Docbook based manual (MK)
# 0.3.1.353 (2011-09-22)
- CMake infrastructure for docbook (MK)
# 0.3.1.352 (2011-09-22)
- Rework help support (MK)
# 0.3.1.351 (2011-09-22)
- Bug fixes (MK)
# 0.3.1.350 (2011-09-22)
- Improve image building and bug fix (MK)
# 0.3.1.349 (2011-09-22)
- Revert the release alignment changes.
# 0.3.1.348 (2011-09-21)
- Minor clarity edit to GOSoundEngine
# 0.3.1.347 (2011-09-21)
- Improve release alignment and fix an engine bug.
# 0.3.1.346 (2011-09-20)
- Fix for MIDI. Patch from Martin
# 0.3.1.345 (2011-09-18)
- Minor style change.
# 0.3.1.344 (2011-09-18)
- Prevent sound access for model, while sound is not running (MK)
# 0.3.1.343 (2011-09-18)
- Make GOrgueSound thread safe (MK)
# 0.3.1.342 (2011-09-18)
- Make sample handling threadsafe (MK)
# 0.3.1.341 (2011-09-18)
- Take MIDI callback out of GOrgueSound and make it thread safe (MK)
# 0.3.1.340 (2011-09-18)
- Thread safe sampler pool (MK)
# 0.3.1.339 (2011-09-18)
- Move GUI access for volume out of GrandOrgueFile (MK)
# 0.3.1.338 (2011-09-18)
- Rework sound recorder (MK)
# 0.3.1.337 (2011-09-18)
- Cleanups (MK)
# 0.3.1.336 (2011-09-18)
- Crescendo fixes (MK)
# 0.3.1.335 (2011-09-18)
- Pure style re-factor to give consistent variable names.
# 0.3.1.334 (2011-09-18)
- Remove any references to tremulants from GOrguePipe. Simplify workings of GOrguePipe. Give GOSoundProvider a virtual destructor.
# 0.3.1.333 (2011-09-18)
- Remove references to GOrguePipe from GOrgueTremulant - instead contain a GOSoundProviderSynthedTrem. Minor cleanups to GOrgueTremulant.
# 0.3.1.332 (2011-09-17)
- Fix to a silly bug I introduced in the previous cleanups.
# 0.3.1.331 (2011-09-17)
- Style changes to variable naming and minor cleanups.
# 0.3.1.330 (2011-09-17)
- Split GOrguePipe into several base classes related to providing sample information into the audio system. The next stage is to get GOrgueTremulant to not require a pipe object and for GOrguePipe to become only for pipes.
# 0.3.1.329 (2011-09-17)
- Removed the wxStopWatch from GOrgueSound. Removed a few unnecessary functions.
# 0.3.1.328 (2011-09-17)
- Removed some useless functions. Created class to manipulate the sampler pool.
# 0.3.1.327 (2011-09-17)
- Remove GOrgueSoundCallbackAudio.cpp and add missing copyright headings to several source files.
# 0.3.1.326 (2011-09-17)
- Move release alignment structure into AUDIO_SECTION to make it more generic. Move start/stop of pipe playback into GOSoundEngine.
# 0.3.1.325 (2011-09-17)
- Move the actual audio sampling engine into a new class - separating it from GOrgueSound.
# 0.3.1.324 (2011-09-15)
- Simple prototype for multi panel support (MK)
# 0.3.1.323 (2011-09-15)
- Allow ODF to place setter elements on the main panel (MK)
# 0.3.1.322 (2011-09-15)
- Implement crescendo swell (MK)
# 0.3.1.321 (2011-09-15)
- Insert/Delete operation for Setter (MK)
# 0.3.1.320 (2011-09-15)
- Add generals panel (MK)
# 0.3.1.319 (2011-09-15)
- Allow user to store invisible objects too (preparation for custom panels) (MK)
# 0.3.1.318 (2011-09-15)
- Prepare for storing invisible objects (MK)
# 0.3.1.317 (2011-09-15)
- Allow ODF to restyle setter panel (MK)
# 0.3.1.316 (2011-09-15)
- Allow ODF to restyle setter label (MK)
# 0.3.1.315 (2011-09-15)
- Allow ODF to restyle Setter buttons (MK)
# 0.3.1.314 (2011-09-15)
- Allow ODF to overwrite button text in Setter Panel (MK)
# 0.3.1.313 (2011-09-15)
- Add support for saving the last panel position (MK)
# 0.3.1.312 (2011-09-15)
- Scoped Divisionals (MK)
# 0.3.1.311 (2011-09-15)
- Enhance FileFormat Reading to support non-standard organ sizes and support reading floats (MK)
# 0.3.1.310 (2011-09-15)
- Rework IniFileConfig and add better default value support (MK)
# 0.3.1.309 (2011-09-15)
- Split Label in GUI and backend (MK)
# 0.3.1.308 (2011-09-15)
- Add basic CPack support (MK)
# 0.3.1.307 (2011-09-15)
- Use more suitable directories for gettext on Windows (MK)
# 0.3.1.306 (2011-09-15)
- Don't create default events, if there is no index (MK)
# 0.3.1.305 (2011-09-15)
- Use free Wine dsound.h, if non dsound.h is present (MK) - and removed windows sdk dsound from repository (NA)
# 0.3.1.304 (2011-09-13)
- Fix bug in old release alignment code.
# 0.3.1.303 (2011-09-11)
- Add GrandOrgue style copyright headers to GOrgueRtHelpers sources
# 0.3.1.302 (2011-09-11)
- Support for scoped setter (MK)
# 0.3.1.301 (2011-09-11)
- Switch to new Setter (MK)
# 0.3.1.300 (2011-09-11)
- Add core for new setter (MK)
# 0.3.1.299 (2011-09-11)
- Add Button for Setter (MK)
# 0.3.1.298 (2011-09-11)
- Add Setter support in MidiReceiver (MK)
# 0.3.1.297 (2011-09-11)
- Prepare Label for Setter (MK)
# 0.3.1.296 (2011-09-11)
- Display Metrics for Setter (MK)
# 0.3.1.295 (2011-09-11)
- Make GOGUIDisplayMetrics methods virtual and make a definition specific GOGUIHW1DDisplayMetrics (MK)
# 0.3.1.294 (2011-09-11)
- ManualNumber in GOrgueReceiver is a generic object number (MK)
# 0.3.1.293 (2011-09-11)
- Move Event definitions, as they don't belong to Sound (MK)
# 0.3.1.292 (2011-09-11)
- Parent window is the application toplevel window (MK)
# 0.3.1.291 (2011-09-11)
- Include-Cleanup (MK)
# 0.3.1.290 (2011-09-11)
- Make some release alignment code more generic by providing samplers with a history set.
# 0.3.1.289 (2011-09-10)
- Use wxWidget functions to find data directory (MK)
# 0.3.1.288 (2011-09-10)
- Cleanup (MK)
# 0.3.1.287 (2011-09-10)
- Rework and cleanup cache handling (MK)
# 0.3.1.286 (2011-09-10)
- Read image data in binary mode (MK)
# 0.3.1.285 (2011-09-10)
- Cleanup SplashScreen (MK)
# 0.3.1.284 (2011-09-10)
- Unify wave file path handling (MK)
# 0.3.1.283 (2011-09-10)
- Only set a default wxWidgets_ROOT_DIR on windows and do not override command line specified build flags (MK)
# 0.3.1.282 (2011-09-10)
- Allow changing the static build setting via command line flags (-DSTATIC) (MK)
# 0.3.1.281 (2011-09-10)
- Fix windows problem (MK)
# 0.3.1.280 (2011-09-10)
- Fix warning (MK)
# 0.3.1.279 (2011-09-10)
- Move recoding path selection dialog code to GrandOrgueFrame (MK)
# 0.3.1.278 (2011-09-10)
- Our events are local (MK)
# 0.3.1.277 (2011-09-10)
- Use local variables in GrandOrgueFrame (MK)
# 0.3.1.276 (2011-09-10)
- Associcate GrandOrgueFile with its OrganDocument owner (MK)
# 0.3.1.275 (2011-09-10)
- Move GOrgueSound related structures into separate header (MK)
# 0.3.1.274 (2011-09-10)
- Cleanup wxGaugeAudio (MK)
# 0.3.1.273 (2011-09-10)
- Fix warning in GOrgueWave (MK)
# 0.3.1.272 (2011-09-08)
- Compile images into object code on compilation of GrandOrgue to avoid unmaintainable image code (MK)
# 0.3.1.271 (2011-09-08)
- Prepare for multi-panel support (MK)
# 0.3.1.270 (2011-09-08)
- Rework OrganView/Panel (MK)
# 0.3.1.269 (2011-09-08)
- Use GrandOrgueFile from OrganDocument in GrandOrgueFrame (MK)
# 0.3.1.268 (2011-09-08)
- Store organ file in OrganDocument (MK)
# 0.3.1.267 (2011-09-08)
- Rework GOGUIDisplayMetrics (MK)
# 0.3.1.266 (2011-09-07)
- Move Setter handling to Frame and fix resize handling (MK)
# 0.3.1.265 (2011-09-07)
- OrganFile presents means loaded (MK)
# 0.3.1.264 (2011-09-07)
- Remove unused code (MK)
# 0.3.1.263 (2011-09-07)
- Remove global organfile from sound subsystem (MK)
# 0.3.1.262 (2011-09-07)
- Remove display metrics from GrandOrgueFile (MK)
# 0.3.1.261 (2011-09-07)
- Move background drawing to separate GUI object (MK)
# 0.3.1.260 (2011-09-07)
- Include cleanup (MK)
# 0.3.1.259 (2011-09-07)
- Split GUI elements of GOrgueManual into new objects (MK)
# 0.3.1.258 (2011-09-07)
- Rework Label (MK)
# 0.3.1.257 (2011-09-07)
- Split GUI element from GOrgueEnclosure into separate object (MK)
# 0.3.1.256 (2011-09-07)
- Remove unused code (MK)
# 0.3.1.255 (2011-09-07)
- Split GUI elements of GOrguePushbutton into a new object (MK)
# 0.3.1.254 (2011-09-07)
- Split the GUI elements from GOrgueDrawStop to a new object. (MK)
# 0.3.1.253 (2011-09-07)
- Cleanup LCD Header (MK)
# 0.3.1.252 (2011-09-07)
- Add base class for GUI controls and a new panel class to hold them (MK)
# 0.3.1.251 (2011-09-07)
- Add control changed broadcast  (MK)
# 0.3.1.250 (2011-09-07)
- Rework default MIDI event generator (MK)
# 0.3.1.249 (2011-09-07)
- Add header for accessing bitmap data (MK)
# 0.3.1.248 (2011-09-07)
- Bug fix to GOrguePipe which was using free instead of delete for a new'ed object. (MK)
# 0.3.1.247 (2011-09-07)
- Add/use more accessor functions and cleanup piston and drawstop drawing logic (MK)
# 0.3.1.246 (2011-09-07)
- Add accessor functions to various objects for their GOrgueMidiReceiver (MK)
# 0.3.1.245 (2011-09-07)
- Get rid of existing ::wxGetApp() calls and replace with simple calls to the new member m_organfile variables to signal component modifications. (MK)
# 0.3.1.244 (2011-09-07)
- Add some accessor methods to GOrgueControl (MK)
# 0.3.1.243 (2011-09-07)
- Rename GOrgueDisplayMetrics to GOGUIDisplayMetrics (MK)
# 0.3.1.242 (2011-09-07)
- Cleanup save methods (MK)
# 0.3.1.241 (2011-09-07)
- Make GOrgueDrawstop DefaultToEngaged member private (MK)
# 0.3.1.240 (2011-09-07)
- Remove global variable usage (MK)
# 0.3.1.239 (2011-08-27)
- Change MIDI-Reset (MK)
# 0.3.1.238 (2011-08-27)
- Add support for midi bank select (MK)
# 0.3.1.237 (2011-08-27)
- Move GOrgueManual to new midi event infrastructure (MK)
# 0.3.1.236 (2011-08-27)
- Move GOrgueEnclosure to new midi event infrastructure (MK)
# 0.3.1.235 (2011-08-27)
- Move GOrgueDivisional/Pushbuttons to new midi event infrastructure (MK)
# 0.3.1.234 (2011-08-27)
- Switch GOrgueDrawStop to new midi infrastructure (MK)
# 0.3.1.233 (2011-08-27)
- Pass Midi data to new infrastructure (MK)
# 0.3.1.232 (2011-08-27)
- Add MIDI Dialog for new midi receiver (MK)
# 0.3.1.231 (2011-08-27)
- Add MIDI event receiver class (MK)
# 0.3.1.230 (2011-08-27)
- Add MidiEvent class (MK)
# 0.3.1.229 (2011-08-27)
- Add hooks to organ-objects which can accept midi input (MK)
# 0.3.1.228 (2011-08-27)
- Add a save place-holder function to GOrgueEnclosure (MK)
# 0.3.1.227 (2011-08-27)
- Cleanup of midi reset handler code (MK)
# 0.3.1.226 (2011-08-27)
- Provide getter for midi transpose setting (MK)
# 0.3.1.225 (2011-08-27)
- Allow toggling of manual key by clicking on it (MK)
# 0.3.1.224 (2011-08-27)
- Use the new IniFileConfig enum support for the coupler type in GOrgueCoupler (MK)
# 0.3.1.223 (2011-08-27)
- Basic enum support helplers for IniFileConfig (MK)
# 0.3.1.222 (2011-08-27)
- Use common return type in IniFileConfig (MK)
# 0.3.1.221 (2011-08-27)
- Addition of readme file for odf format to reflect changes to coupler settings (MK)
# 0.3.1.220 (2011-08-24)
- Cleaning up variables to be unsigned where appropriate (MK)
# 0.3.1.219 (2011-08-24)
- Move Image Handling to Pushbutton (MK)
# 0.3.1.218 (2011-08-15)
- Remove hardcoded limits and cleanups to sound engine (MK)
# 0.3.1.217 (2011-08-15)
- Split combined windchest array into separate groups representing detached samples, trems and 'normal' pipe samples (MK)
# 0.3.1.216 (2011-08-15)
- Use unsigned to represent Tremulant/Windchest counts in GrandOrgueFile (MK)
# 0.3.1.215 (2011-08-15)
- Use ptr_vector everywhere where beneficial (MK)
# 0.3.1.214 (2011-08-15)
- Update GrandOrgueFile to use stl vectors rather than arrays (MK)
# 0.3.1.213 (2011-08-15)
- Compare with size of the vectors to avoid crash, if Load failed (MK)
# 0.3.1.212 (2011-08-15)
- Cleanup DivisionalCoupler (MK)
# 0.3.1.211 (2011-08-15)
- Use correct index to manual (MK)
# 0.3.1.210 (2011-08-15)
- Cleanup GOrgueWindchest (MK)
# 0.3.1.209 (2011-08-15)
- Cleanups to coupling logic including fix to interpret key difference as midi note difference(MK)
# 0.3.1.208 (2011-08-13)
- Removed unused class
# 0.3.1.207 (2011-08-12)
- Rollback of patch 206.
# 0.3.1.206 (2011-08-12)
- Remove MIDI-Pretend (MK)
# 0.3.1.205 (2011-08-12)
- Removal of obscure comment from release alignment code
# 0.3.1.204 (2011-08-12)
- Remove MIDI-Pretend Logic from Stop changes (MK)
# 0.3.1.203 (2011-08-12)
- Remove MIDIAllNotesOff (MK)
# 0.3.1.202 (2011-08-12)
- Move stop start/stop handling to GOrgueStop (MK)
# 0.3.1.201 (2011-08-12)
- Move Tremulant Start/Stop Code to GOrgueTremulant (MK)
# 0.3.1.200 (2011-08-11)
- Cleanup of GOrgueStop
# 0.3.1.199 (2011-08-11)
- Use accessor function (MK)
# 0.3.1.198 (2011-08-11)
- Include BLOCKS_PER_FRAME in the cache hash, as it affectes the saved data structures (MK)
# 0.3.1.197 (2011-08-11)
- Removed dead thread code (MK)
# 0.3.1.196 (2011-08-11)
- Use unsigned for key numbers and refactor GOrgueManual to use vectors (MK)
# 0.3.1.195 (2011-08-10)
- Very minor code cleanups.
# 0.3.1.194 (2011-08-10)
- fix to clicks at the end of releases.
# 0.3.1.193 (2011-08-09)
- Minor cleanups/fixes
# 0.3.1.192 (2011-08-08)
- Moved the block size checking code into the audio callback.
# 0.3.1.191 (2011-08-07)
- Fix to crackling
# 0.3.1.190 (2011-08-07)
- Removal of double definition of MAX_OUTPUT_CHANNELS
# 0.3.1.189 (2011-08-07)
- Fix to audio clicking issue.
# 0.3.1.188 (2011-08-07)
- More auto-vectorizer helper optimizations to audio engine.
# 0.3.1.187 (2011-08-07)
- Changed release build type optimization code to O3 to enable the auto vectorizer by default.
# 0.3.1.186 (2011-08-07)
- Efficiency updates to audio engine. Block decode size now variable. Increased previous default size from 2 to 64 (around 1.5ms latency min).
# 0.3.1.185 (2011-08-07)
- Modified wx link order again to fix a static link error on Linux - should not affect Windows build
# 0.3.1.184 (2011-08-07)
- Windows specific unicode fix to GrandOrgue.cpp. Fixed broken Windows link order for static builds. Updates to wxWidgets finder script.
# 0.3.1.183 (2011-08-06)
- Make the Windows resource linker command a bit more explicit.
# 0.3.1.182 (2011-08-06)
- Changed type of length to wxFileOffset
# 0.3.1.181 (2011-08-06)
- Removed the custom file loader code and replaced it with code using wxFile to make more OS independent. (MK)
# 0.3.1.180 (2011-08-06)
- Cleanup of the automatic playback of sample when stop is on code. (MK)
# 0.3.1.179 (2011-08-06)
- Removal of unused return value of "Set" functions and removal of recursion of piston "Push" functions. (MK)
# 0.3.1.178 (2011-08-06)
- Martin's tremulant re-activation patches and the nice replacement of scalb with ldexp.
# 0.3.1.177 (2011-08-05)
- Committed patch from Martin Koegler for multiple gettext translations
# 0.3.1.176 (2011-08-04)
- Added swedish translation
# 0.3.1.175 (2011-08-04)
- The next commit of Martin's translation fixes and force of reload of organ on settings change.
# 0.3.1.174 (2011-08-04)
- Martin's translation support additions.
# 0.3.1.173 (2011-08-04)
- Martin's re-implementation of text wrapping code for GUI objects
# 0.3.1.172 (2011-08-04)
# 0.3.1.171 (2011-07-31)
- fixes to GrandOrgueFile to restore the original behavior which provided more useful log messages. added detection that cache file exists.
# 0.3.1.170 (2011-07-30)
- bugfix to organ cache generation which results in a seg fault for samples which do not have a release alignment table.
# 0.3.1.169 (2011-07-30)
- Commit of Martin's caching patches and various minor fixes.
# 0.3.1.168 (2011-07-28)
- Minor cleanup
# 0.3.1.167 (2011-07-28)
- Removed the need for global count variables while loading pipes. Moved the progress dialog updating back into GrandOrgueFile until loading can be abstracted away.
# 0.3.1.166 (2011-07-28)
- Updates from Martin  Koegler. Removal of zlib headers, migration of pipe loading into GOrguePipe, combination improvements.
# 0.3.1.165 (2011-07-28)
- Committed patch from Martin Koegler for setting current Memory level
# 0.3.1.162 (2011-07-26)
- Martin Koegler's patches to simplify and extend the combination action with style modifications by NA.
# 0.3.1.157 (2011-07-01)
- Merge cmake branch back to trunk
# 0.3.1.156 (2011-06-30)
- reverted to revision 131
# 0.3.1.155 (2011-06-30)
- deleted files that were incorrectly transfered to the trunk
# 0.3.1.154 (2011-06-30)
- Merge back from cmake branch into trunk
# 0.3.1.131 (2011-06-20)
- More updates to build system.
# 0.3.1.130 (2011-06-19)
- Removed old Makefile and replaced with Makefile.linux
# 0.3.1.129 (2011-06-19)
- Moved some RtAudio specific helper functions into separate source file as part of cleanup.
# 0.3.1.128 (2011-06-19)
- Disabled the setting of an icon unless this is a Windows build. The icon is not embedded into the executable on Linux builds.
# 0.3.1.127 (2011-06-19)
- fix to previous patch due to incorrect wx documentation.
# 0.3.1.126 (2011-06-19)
- The wxLogxxx functions take const char* args and should not have unicode/translatable arguments.
# 0.3.1.125 (2011-06-19)
- cleaning up of makefile and removal of irrelevant comments
# 0.3.1.124 (2011-06-19)
- updates to source and makefile to enable compilation on Windows using MingW msys
# 0.3.1.123 (2011-06-18)
- update to provide executable file extensions on Windows builds
# 0.3.1.122 (2011-06-18)
- major build system update to Makefile
# 0.3.1.121 (2011-06-18)
- Applied patches supplied by Martin Koegler to have unicode support.
# 0.3.1.120 (2011-06-18)
- latency fixes to audio engine
# 0.3.1.119 (2011-06-17)
- Removed the option to disable detached releases - there is no use case to ever want to disable this feature.
# 0.3.1.118 (2011-06-17)
- Added an "actual" latency label to the settings dialog. Fixed a bug where the "actual" latency would get written into the "estimated" latency field resulting in continually changing latencies.
# 0.3.1.117 (2011-06-17)
- Fix to settings dialog latency.
# 0.3.1.116 (2011-06-17)
- Removed dependency on global "organfile" variable in GOrgueDisplayMetrics. Improved and moved code related to rendering manuals into GOrgueManual. Minor cleanups.
# 0.3.1.115 (2011-06-16)
- Updated the Linux CodeBlocks project with Nicks latest MIDI changes
# 0.3.1.114 (2011-06-16)
- Updated windows CodeBlocks project with Nick MIDI changes
# 0.3.1.113 (2011-06-16)
- Big cleanup of MIDI handling code particularly to do with listening dialog. Likely to have introduced bugs. MIDI callback moved into new object and references from GOrgueSound have been removed. General cleanups.
# 0.3.1.112 (2011-06-15)
- Fix to take FirstAccessibleKeyLogicalKeyNumber into account when a key is pressed. Makes it possible to have sub octave couplers with extended bass range.
# 0.3.1.111 (2011-06-06)
- Windows CodeBlocks project update with updated Minor version number to 0.3.0.3
# 0.3.1.110 (2011-05-21)
- Updated RtMidi to latest version
# 0.3.1.109 (2011-05-20)
- Correction and deleted OrganFile.h and .cpp correctly this time
# 0.3.1.108 (2011-05-20)
- Actually deleted OrganFile.h and .cpp
# 0.3.1.107 (2011-05-20)
- Cleanup of includes and removed OrganFile
# 0.3.1.106 (2011-05-19)
- ASIO Cleanup
# 0.3.1.105 (2011-05-09)
- * Removed the registration dialog - it was depressing finding this in the code.
- * Bugfix to audio engine when in non-active mode.
- * Cleanups in audio engine.
- * Fixed broken coupling issue.
- * Removed main bits of tremulant.
# 0.3.1.104 (2011-05-01)
- Fix for percussive sample playback.
# 0.3.1.103 (2011-05-01)
- Cleanup of label rendering code.
# 0.3.1.102 (2011-04-30)
- Some general cleanup of enclosure object.
# 0.3.1.101 (2011-04-30)
- Cleanup of release alignment code to make it more clear and general code cleanups.
# 0.3.1.100 (2011-04-28)
- Defaulted Makefile to release build.
- Updated GOrgueWave.h/cpp in preparation for support for multiple loops.
# 0.3.1.99 (2011-04-27)
- Separated the release alignment code into new header and implementation file.
- Fixed several compiler warnings.
- Revamped the Makefile to make building using different configurations / debug settings easier.
- More cleaning up of audio engine.
# 0.3.1.98 (2011-04-26)
- Fix (hopefully) to some Code Blocks compiler warnings.
# 0.3.1.97 (2011-04-26)
- Added the DNDEBUG define to the Linux Code Blocks release build options
# 0.3.1.96 (2011-04-26)
- Made sure that -DNDEBUG was set for the Windows Code Blocks release build...
# 0.3.1.95 (2011-04-25)
- Fix to Makefile.linux to bring back correct default compilation flags.
# 0.3.1.94 (2011-04-25)
- More code clean up in audio engine.
- Major bug fix to sampling engine present since at least revision 63 causing crashes when detached releases is enabled.
- Structs named in capitals for clarity against classes.
- Release alignment code reintroduced.
- Memory leak in GrandOrgueFile.cpp fixed.
# 0.3.1.93 (2011-04-05)
- Fixed bug in the drawing of adjecant keys when released or pressed
# 0.3.1.92 (2011-03-24)
- Updated RtMidi to 1.0.12
# 0.3.1.91 (2011-03-24)
- Better change to enable multiple MIDI devices with the same name with working channel offset
# 0.3.1.90 (2011-03-22)
- Change to enable multiple MIDI devices with the same name
# 0.3.1.89 (2011-03-13)
- add wav extension to file names for recording. workaround required for Linux. submitted by Lars
# 0.3.1.88 (2011-03-12)
- Repaired stop recording
# 0.3.1.87 (2011-03-07)
- Fix for the 'Generals and Stops overlapping in the upper central console are' issue
# 0.3.1.86 (2011-03-06)
- NOID: loops now play correct number of samples. There should be no clicks/awkward tonal changes during sample playback not related to latency. Release alignment needs implementing.
# 0.3.1.85 (2011-03-06)
- NOID: Fix to loop point clicking.
# 0.3.1.84 (2011-03-06)
- NOID: Workaround for broken wave files
# 0.3.1.83 (2011-03-06)
- NOID: removed a debug printf for pipe-on/off
# 0.3.1.82 (2011-03-05)
- Linux Code Blocks project update. Tested with Jack
# 0.3.1.81 (2011-03-05)
- NOID: Removed a debug printf.
# 0.3.1.80 (2011-03-05)
- NOID: fixed random notes not playing
# 0.3.1.79 (2011-03-05)
- Added GOrgueWave.cpp and .h to Windows project GrandOrgue.Windows.cbp
# 0.3.1.78 (2011-03-05)
- NOID: fix major memory leak and general cleanup
# 0.3.1.77 (2011-03-05)
- Bug 3188036: major rewrite of sample handling code as part of code cleanup.
# 0.3.1.76 (2011-02-27)
- Project update for Windows Code Blocks build
# 0.3.1.75 (2011-02-23)
- NOID: Fix Makefile to enable Jack support (Lars)
# 0.3.1.74 (2011-02-22)
- Bug 3188036: more cleaning up of code
# 0.3.1.73 (2011-02-21)
- Bug 3188036: cleanup of code
# 0.3.1.72 (2011-02-18)
- Linux Code Blocks project update. Tested only with ALSA
# 0.3.1.71 (2011-02-17)
- Windows CodeBlocks Project updated & include/asio updated to 0.3 branches code.
# 0.3.1.70 (2011-02-16)
- NO-ID: * More code cleanup.
- * OrganFile.h is now redundant. All objects have been moved to separate implementation and header files.
- * GOrgueSampler has been changed into a more efficient struct.
# 0.3.1.69 (2011-02-15)
- NO-ID: * Added license information to top of file
# 0.3.1.68 (2011-02-15)
- NO-ID: * Removed some of the classes from OrganFile.h into separate files and updated Makefile.linux
- * Moved and removed some #pragma pack declarations
- * Code Neatening.
# 0.3.1.67 (2011-02-15)
- NO-ID: Updated all headers to contain ANSI C compatible guards
# 0.3.1.66 (2011-02-15)
- NO-ID: * Update of Linux Makefile to support build of development trunk.
# 0.3.1.63 (2011-02-01)
- Merge from GO 0.3 branch
# 0.3.1.62 (2011-01-30)
- * Updated a lot of wxLogWarning calls with the correct types to prevent compiler warnings occuring...
- * Updated several printf formats that were passing incorrect data types...
- The source should now compile with no warnings related to our code.
# 0.3.1.61 (2011-01-30)
- * Updated the Makefile to reflect the renamed object files.
- * Removed use of the >?=, >?, etc operators to allow GCC 4.4 to compile.
- * Modified use of open to include the third argument. Required by some compilers when using the O_CREAT flag.
- Source now compiles successfully using standard make provided the correct external packages have been installed correctly.
# 0.3.1.25 (2009-12-22)
- Updated codebase to reflect the GrandOrgue project naming
# 0.3.1.24 (2009-12-22)
- File renaming to facilitate update of codebase to the GrandOrgue project name
# 0.3.1.23 (2009-12-22)
- Cleanup part 2 of directx/include 
# 0.3.1.22 (2009-12-22)
- Cleanup of directx\include
# 0.3.1.20 (2009-11-29)
- Update
# 0.3.1.19 (2009-11-29)
- Linux Updates for CodeBlocks build
# 0.3.1.18 (2009-11-29)
- Updated all icon images, added new wood background. Made a few fixes for the Linux version. Ready for a Beta 0.2.0.1 release.
# 0.3.1.17 (2009-08-12)
# 0.3.1.16 (2009-08-11)
- added (C) JLDER, will have to add other contributors names to be complete
# 0.3.1.15 (2009-08-11)
- Add copyright information which was omitted by mistake (2006 KLORIA )
# 0.3.1.14 (2009-06-10)
- compilation options modified for better compatibility
# 0.3.1.13 (2009-06-10)
# 0.3.1.12 (2009-06-10)
- Version with correct splash screen
# 0.3.1.11 (2009-06-10)
- Change files info to new project name: GrandOrgue
# 0.3.1.10 (2009-06-09)
# 0.3.1.9 (2009-06-09)
# 0.3.1.8 (2009-06-09)
# 0.3.1.6 (2009-06-08)
- removed unwanted exe from trunk and left it in release
# 0.3.1.5 (2009-06-08)
- change exe release name
# 0.3.1.3 (2009-06-08)
