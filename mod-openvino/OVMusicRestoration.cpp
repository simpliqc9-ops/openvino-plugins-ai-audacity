// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: GPL-3.0-only

#include "OVMusicRestoration.h"
#include "WaveTrack.h"
#include "EffectOutputTracks.h"
#include "effects/EffectEditor.h"
#include <math.h>
#include <iostream>
#include <wx/log.h>

#include "ViewInfo.h"
#include "TimeWarper.h"
#include "LoadEffects.h"

#include <wx/intl.h>
#include <wx/valgen.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>

#include "ShuttleGui.h"
#include <wx/choice.h>
#include "FileNames.h"
#include "CodeConversions.h"
#include <future>

#include "widgets/valnum.h"

#include "OVStringUtils.h"
#include <openvino/openvino.hpp>
#include "OVModelManager.h"
#include "OVModelManagerUI.h"

#include "demix/demix.h"
#include "demix/htdemucs.h"
#include "demix/mel_band_roformer.h"

const ComponentInterfaceSymbol EffectOVMusicRestoration::Symbol{ XO("Music Restoration") };

namespace { BuiltinEffectsModule::Registration< EffectOVMusicRestoration > reg; }


EffectOVMusicRestoration::EffectOVMusicRestoration()
{

}

EffectOVMusicRestoration::~EffectOVMusicRestoration()
{

}

// ComponentInterface implementation
ComponentInterfaceSymbol EffectOVMusicRestoration::GetSymbol() const
{
   return Symbol;
}

TranslatableString EffectOVMusicRestoration::GetDescription() const
{
   return XO("Removes reverberation from spoken audio or vocals");
}

const std::string EffectOVMusicRestoration::ModelManagerName() const
{
   return OVModelManager::MusicRestorationName();
}

std::unordered_map<std::string, EffectOVDemixerEffect::SeparationModeEntry> EffectOVMusicRestoration::GetModelMap()
{
   std::unordered_map<std::string, EffectOVDemixerEffect::SeparationModeEntry> model_to_separation_map;

   {
      SeparationModeEntry entry;
      entry.stems = { "Restored" };
      entry.target_stem_for_instrumental = 0; //restored stem
      entry.bZeroPad = true;
      model_to_separation_map.emplace("Apollo MP3 Restore (@JusperLee)", entry);
   }

   {
      SeparationModeEntry entry;
      entry.stems = { "Restored" };
      entry.target_stem_for_instrumental = 0; //restored stem
      entry.bZeroPad = true;
      model_to_separation_map.emplace("Apollo Universal Restore (@Lew)", entry);
   }

   // Disable 'Separation Mode' selection drop-down box. It doesn't really make sense for Music Restoration.
   m_bDisplaySepMode = false;

   return model_to_separation_map;
}
