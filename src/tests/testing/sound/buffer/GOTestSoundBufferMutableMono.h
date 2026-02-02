/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt
 * to change this license Click
 * nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this
 * template
 */

/*
 * File:   GOTestSoundMutableBufferMono.h
 * Author: oleg
 *
 * Created on 2 февраля 2026 г., 22:48
 */

#ifndef GOTESTSOUNDMUTABLEBUFFERMONO_H
#define GOTESTSOUNDMUTABLEBUFFERMONO_H

#include "GOTest.h"

class GOTestSoundBufferMutableMono : public GOTest {
private:
  static const std::string TEST_NAME;

  void TestConstructorAndBasicProperties();
  void TestGetSubBuffer();
  void TestCopyChannelFrom();
  void TestCopyChannelTo();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDMUTABLEBUFFERMONO_H */
