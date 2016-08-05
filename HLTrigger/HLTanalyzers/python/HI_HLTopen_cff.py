import FWCore.ParameterSet.Config as cms
 
# import the whole HLT menu
from HLTrigger.HLTanalyzers.HLT_HIon_cff import *


#### For the future of Muon HLT in the case of including L3 sequence
DoHLTHIMuon = cms.Path(HLTBeginSequence +
                       HLTDoHILocalPixelSequence + HLTHIRecopixelvetexingSequence + HLTDoHILocalStripSequence +
                       HLTL2muonrecoSequence +    # L2 Muons
                       HLTHIL3muonrecoSequence +  # L3 Muons
                       HLTEndSequence)

