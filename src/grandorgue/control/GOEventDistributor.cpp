/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEventDistributor.h"

#include "model/GOCacheObject.h"
#include "model/GOEventHandlerList.h"

#include "GOControlChangedHandler.h"
#include "GOEventHandler.h"
#include "GOPlaybackStateHandler.h"
#include "GOSaveableObject.h"

void GOEventDistributor::SendMidi(const GOMidiEvent &event) {
  for (auto handler : p_model->GetMidiEventHandlers())
    handler->ProcessMidi(event);
}

void GOEventDistributor::HandleKey(int key) {
  for (auto handler : p_model->GetMidiEventHandlers())
    handler->HandleKey(key);
}

void GOEventDistributor::ReadCombinations(GOConfigReader &cfg) {
  for (auto obj : p_model->GetSaveableObjects())
    obj->LoadCombination(cfg);
}

void GOEventDistributor::Save(GOConfigWriter &cfg) {
  for (auto obj : p_model->GetSaveableObjects())
    obj->Save(cfg);
}

void GOEventDistributor::ResolveReferences() {
  for (auto obj : p_model->GetCacheObjects())
    obj->Initialize();
}

void GOEventDistributor::UpdateHash(GOHash &hash) {
  for (auto obj : p_model->GetCacheObjects())
    obj->UpdateHash(hash);
}

void GOEventDistributor::PreparePlayback() {
  for (auto handler : p_model->GetPlaybackStateHandlers())
    handler->PreparePlayback();
}

void GOEventDistributor::StartPlayback() {
  for (auto handler : p_model->GetPlaybackStateHandlers())
    handler->StartPlayback();
}

void GOEventDistributor::AbortPlayback() {
  for (auto handler : p_model->GetPlaybackStateHandlers())
    handler->AbortPlayback();
}

void GOEventDistributor::PrepareRecording() {
  for (auto handler : p_model->GetPlaybackStateHandlers())
    handler->PrepareRecording();
}

void GOEventDistributor::ControlChanged(void *control) {
  if (control)
    for (auto handler : p_model->GetControlChangedHandlers())
      handler->ControlChanged(control);
}
