# for the list of used tags please see:
# https://twiki.cern.ch/twiki/bin/view/CMS/Onia2MuMuSamples

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing


#----------------------------------------------------------------------------

# Setup Settings for ONIA TREE:

isPbPb         = True      # if PbPb data/MC: True or if pp data/MC: False    
isMC           = True      # if input is MONTECARLO: True or if it's DATA: False
isPromptDATA   = False     # if input is Prompt RECO DATA: True or if it's Express Stream DATA: False
isPromptMC     = True      # if MC is Prompt Quarkonia: True or if it's Non Prompt Quarkonia: False
useExtraColl   = False     # General Tracks + Stand Alone Muons + Converted Photon collections
applyEventSel  = False     # Only apply Event Selection if the required collections are present 
applyMuonCuts  = False     # Apply muon ID quality cuts
muonSelection  = "GlbTrk"  # Single muon selection: Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker) are availale
genPDG         = 443       # Generated Particle PDG ID (only needed for MC), Jpsi: 443 , Psi(2S): 100443, Upsilon(1S): 553 , Upsilon(2S): 100553 , Upsilon(2S): 200553

#----------------------------------------------------------------------------


# Print Onia Skim settings:
if (isPromptDATA and isMC): raise SystemExit("[ERROR] isMC and isPromptDATA can not be true at the same time, please fix your settings!.")
print( " " ) 
print( "[INFO] Settings used for ONIA TREE: " )  
print( "[INFO] isPbPb        = " + ("True" if isPbPb else "False") )  
print( "[INFO] isMC          = " + ("True" if isMC else "False") )  
print( "[INFO] isPromptDATA  = " + ("True" if isPromptDATA else "False") ) 
print( "[INFO] isPromptMC    = " + ("True" if isPromptMC else "False") )  
print( "[INFO] useExtraColl  = " + ("True" if useExtraColl else "False") ) 
print( "[INFO] applyEventSel = " + ("True" if applyEventSel else "False") )  
print( "[INFO] applyMuonCuts = " + ("True" if applyMuonCuts else "False") ) 
print( "[INFO] muonSelection = " + muonSelection )  
print( "[INFO] genPDG        = " + str(genPDG) )  
print( " " ) 

# set up process
process = cms.Process("HIOnia")

# Setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

# Input and Output File Names
options.outputFile = "OniaTree.root"
options.secondaryOutputFile = "Jpsi_DataSet.root"
options.inputFiles = 'file:onia2MuMuPAT_DATA_75X_PbPb_MC.root'

options.maxEvents = -1 # -1 means all events

# Get and parse the command line arguments
options.parseArguments()
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

# Global Tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '75X_mcRun2_HeavyIon_v13', '')
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")


#Centrality Tags for CMSSW 7_5_X:               
# Only use if the centrality info is not present or need to apply new calibration
process.load("RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi")
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

HLTProName = "HLT"

process.hionia = cms.EDAnalyzer('HiOniaAnalyzer',
                                #-- Collections
                                srcMuon             = cms.InputTag("patMuonsWithTrigger"),     # Name of PAT Muon Collection
                                srcMuonNoTrig       = cms.InputTag("patMuonsWithoutTrigger"),  # Name of PAT Muon Without Trigger Collection
                                src                 = cms.InputTag("onia2MuMuPatGlbGlb"),      # Name of Onia Skim Collection
                                genParticles        = cms.InputTag("genParticles"),
                                triggerResultsLabel = cms.InputTag("TriggerResults","",HLTProName), # Label of Trigger Results

                                #-- Reco Details
                                useBeamSpot = cms.bool(False),  
                                useRapidity = cms.bool(True),
                                
                                #--
                                maxAbsZ = cms.double(24.0),
                                
                                pTBinRanges      = cms.vdouble(0.0, 6.0, 8.0, 9.0, 10.0, 12.0, 15.0, 40.0),
                                etaBinRanges     = cms.vdouble(0.0, 2.5),
                                centralityRanges = cms.vdouble(40,80,200),

                                onlyTheBest        = cms.bool(False),		
                                applyCuts          = cms.bool(applyMuonCuts),
                                selTightGlobalMuon = cms.bool(False),
                                storeEfficiency    = cms.bool(False),
                      
                                removeSignalEvents = cms.untracked.bool(False),  # Remove/Keep signal events
                                removeTrueMuons    = cms.untracked.bool(False),  # Remove/Keep gen Muons
                                storeSameSign      = cms.untracked.bool(True),   # Store/Drop same sign dimuons
                                
                                #-- Gen Details
                                oniaPDG = cms.int32(genPDG),
                                muonSel = cms.string(muonSelection),
                                isHI = cms.untracked.bool(isPbPb),
                                isPA = cms.untracked.bool(False),
                                isMC = cms.untracked.bool(isMC),
                                isPromptMC = cms.untracked.bool(isPromptMC),
                                useEvtPlane = cms.untracked.bool(True),
                                useGeTracks = cms.untracked.bool(useExtraColl),
                                runVersionChange = cms.untracked.uint32(182133),

                                #-- Histogram configuration
                                combineCategories = cms.bool(False),
                                fillRooDataSet    = cms.bool(False),
                                fillTree          = cms.bool(True),
                                fillHistos        = cms.bool(False),
                                minimumFlag       = cms.bool(False),
                                fillSingleMuons   = cms.bool(True),
                                fillRecoTracks    = cms.bool(useExtraColl),
                                histFileName      = cms.string(options.outputFile),		
                                dataSetName       = cms.string(options.secondaryOutputFile),

                                )

### Only use if prescale warnings are shown.
#process.hionia.l1GtReadoutRecordInputTag = cms.InputTag("gtDigis","","RECO")
###

if isPbPb:           
  process.hionia.primaryVertexTag = cms.InputTag("hiSelectedVertex")
  process.hionia.EvtPlane         = cms.InputTag("hiEvtPlaneFlat","")
  process.hionia.CentralitySrc    = cms.InputTag("hiCentrality") 
  process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
  process.hionia.srcTracks        = cms.InputTag("hiGeneralTracks")
  process.hionia.muonLessPV       = cms.bool(False)      

  # HLT PbPbP MENU:  /online/collisions/2015/HeavyIons/v1.0/HLT/V6
                                
  process.hionia.dblTriggerPathNames   = cms.vstring("HLT_HIL1DoubleMu0_v1",
                                                     "HLT_HIL1DoubleMu0_2HF_v1",
                                                     "HLT_HIL1DoubleMu0_2HF0_v1",
                                                     "HLT_HIL1DoubleMu10_v1",
                                                     "HLT_HIL2DoubleMu0_NHitQ_v2",
                                                     "HLT_HIL2DoubleMu0_NHitQ_2HF_v1",
                                                     "HLT_HIL2DoubleMu0_NHitQ_2HF0_v1",
                                                     "HLT_HIL1DoubleMu0_2HF_Cent30100_v1",
                                                     "HLT_HIL1DoubleMu0_2HF0_Cent30100_v1",
                                                     "HLT_HIL2DoubleMu0_2HF_Cent30100_NHitQ_v1",
                                                     "HLT_HIL1DoubleMu0_Cent30_v1",
                                                     "HLT_HIL2DoubleMu0_2HF0_Cent30100_NHitQ_v1",
                                                     "HLT_HIL2DoubleMu0_Cent30_NHitQ_v1",
                                                     "HLT_HIL2DoubleMu0_Cent30_OS_NHitQ_v1",
                                                     "HLT_HIL3DoubleMu0_Cent30_v1",
                                                     "HLT_HIL3DoubleMu0_Cent30_OS_m2p5to4p5_v1",
                                                     "HLT_HIL3DoubleMu0_Cent30_OS_m7to14_v1",
                                                     "HLT_HIL3DoubleMu0_OS_m2p5to4p5_v1",
                                                     "HLT_HIL3DoubleMu0_OS_m7to14_v1")

  process.hionia.sglTriggerPathNames   = cms.vstring("HLT_HIL2Mu3_NHitQ10_2HF_v1",
                                                     "HLT_HIL2Mu3_NHitQ10_2HF0_v1",
                                                     "HLT_HIL3Mu3_NHitQ15_2HF_v1",
                                                     "HLT_HIL3Mu3_NHitQ15_2HF0_v1",
                                                     "HLT_HIL2Mu5_NHitQ10_2HF_v1",
                                                     "HLT_HIL2Mu5_NHitQ10_2HF0_v1",
                                                     "HLT_HIL3Mu5_NHitQ15_2HF_v1",
                                                     "HLT_HIL3Mu5_NHitQ15_2HF0_v1",
                                                     "HLT_HIL2Mu7_NHitQ10_2HF_v1",
                                                     "HLT_HIL2Mu7_NHitQ10_2HF0_v1",
                                                     "HLT_HIL3Mu7_NHitQ15_2HF_v1",
                                                     "HLT_HIL3Mu7_NHitQ15_2HF0_v1",
                                                     "HLT_HIL2Mu15_v2",
                                                     "HLT_HIL2Mu15_2HF_v1",
                                                     "HLT_HIL2Mu15_2HF0_v1",
                                                     "HLT_HIL3Mu15_v1",
                                                     "HLT_HIL3Mu15_2HF_v1",
                                                     "HLT_HIL3Mu15_2HF0_v1",
                                                     "HLT_HIL2Mu20_v1",
                                                     "HLT_HIL2Mu20_2HF_v1",	
                                                     "HLT_HIL2Mu20_2HF0_v1",
                                                     "HLT_HIL3Mu20_v1",
                                                     "HLT_HIL3Mu20_2HF_v1",
                                                     "HLT_HIL3Mu20_2HF0_v1")

  process.hionia.dblTriggerFilterNames = cms.vstring("hltHIDoubleMu0L1Filtered",
                                                     "hltHIDoubleMu0MinBiasL1Filtered",
                                                     "hltHIDoubleMu0HFTower0Filtered",
                                                     "hltHIDoubleMu10L1Filtered",
                                                     "hltHIL2DoubleMu0NHitQFiltered",
                                                     "hltHIL2DoubleMu0NHitQ2HFFiltered",
                                                     "hltHIL2DoubleMu0NHitQ2HF0Filtered",
                                                     "hltHIDoubleMu0MinBiasCent30to100L1Filtered",
                                                     "hltHIDoubleMu0HFTower0Cent30to100L1Filtered",
                                                     "hltHIL2DoubleMu02HFcent30100NHitQFiltered",
                                                     "hltHIDoubleMu0MinBiasCent30L1Filtered",
                                                     "hltHIL2DoubleMu02HF0cent30100NHitQFiltered",
                                                     "hltHIL2DoubleMu0cent30NHitQFiltered",
                                                     "hltHIL2DoubleMu0cent30OSNHitQFiltered",
                                                     "hltHIDimuonOpenCentrality30L3Filter",
                                                     "hltHIDimuonOpenCentrality30OSm2p5to4p5L3Filter",
                                                     "hltHIDimuonOpenCentrality30OSm7to14L3Filter",
                                                     "hltHIDimuonOpenOSm2p5to4p5L3Filter",
                                                     "hltHIDimuonOpenOSm7to14L3Filter")
  
  process.hionia.sglTriggerFilterNames = cms.vstring("hltHIL2Mu3N10HitQ2HFL2Filtered",
                                                     "hltHIL2Mu3N10HitQ2HF0L2Filtered",
                                                     "hltHISingleMu3NHit152HFL3Filtered",
                                                     "hltHISingleMu3NHit152HF0L3Filtered",
                                                     "hltHIL2Mu5N10HitQ2HFL2Filtered",
                                                     "hltHIL2Mu5N10HitQ2HF0L2Filtered",
                                                     "hltHISingleMu5NHit152HFL3Filtered",
                                                     "hltHISingleMu5NHit152HF0L3Filtered",
                                                     "hltHIL2Mu7N10HitQ2HFL2Filtered",
                                                     "hltHIL2Mu7N10HitQ2HF0L2Filtered",
                                                     "hltHISingleMu7NHit152HFL3Filtered",
                                                     "hltHISingleMu7NHit152HF0L3Filtered",
                                                     "hltHIL2Mu15L2Filtered",
                                                     "hltHIL2Mu152HFFiltered",
                                                     "hltHIL2Mu15N10HitQ2HF0L2Filtered",
                                                     "hltHISingleMu15L3Filtered",
                                                     "hltHISingleMu152HFL3Filtered",
                                                     "hltHISingleMu152HF0L3Filtered",
                                                     "hltHIL2Mu20L2Filtered",
                                                     "hltHIL2Mu202HFL2Filtered",
                                                     "hltHIL2Mu202HF0L2Filtered",
                                                     "hltHIL3SingleMu20L3Filtered",
                                                     "hltHISingleMu202HFL3Filtered",
                                                     "hltHISingleMu202HF0L3Filtered")
else: 
  process.hionia.primaryVertexTag = cms.InputTag("offlinePrimaryVertices")
  process.hionia.EvtPlane         = cms.InputTag("")
  process.hionia.CentralitySrc    = cms.InputTag("")
  process.hionia.CentralityBinSrc = cms.InputTag("")
  process.hionia.srcTracks        = cms.InputTag("generalTracks")       
  process.hionia.muonLessPV       = cms.bool(True)
  
  # HLT PP MENU: FOR RUN 2 PP MC 2015
  
  process.hionia.dblTriggerPathNames   = cms.vstring("HLT_HIL1DoubleMu0ForPPRef_v1",
                                                     "HLT_HIL1DoubleMu10ForPPRef_v1",
                                                     "HLT_HIL2DoubleMu0_NHitQForPPRef_v1",
                                                     "HLT_HIL3DoubleMu0_OS_m2p5to4p5ForPPRef_v1",
                                                     "HLT_HIL3DoubleMu0_OS_m7to14ForPPRef_v1")

  process.hionia.sglTriggerPathNames   = cms.vstring("HLT_HIL2Mu3_NHitQ10ForPPRef_v1",
                                                     "HLT_HIL3Mu3_NHitQ15ForPPRef_v1",
                                                     "HLT_HIL2Mu5_NHitQ10ForPPRef_v1",
                                                     "HLT_HIL3Mu5_NHitQ15ForPPRef_v1",
                                                     "HLT_HIL2Mu7_NHitQ10ForPPRef_v1",
                                                     "HLT_HIL3Mu7_NHitQ15ForPPRef_v1",
                                                     "HLT_HIL2Mu15ForPPRef_v1",
                                                     "HLT_HIL3Mu15ForPPRef_v1",
                                                     "HLT_HIL2Mu20ForPPRef_v1",
                                                     "HLT_HIL3Mu20ForPPRef_v1")
  
  process.hionia.dblTriggerFilterNames = cms.vstring("hltHIDoubleMu0L1Filtered",
                                                     "hltHIDoubleMu10MinBiasL1Filtered",
                                                     "hltHIL2DoubleMu0NHitQFiltered",
                                                     "hltHIDimuonOpenOSm2p5to4p5L3Filter",
                                                     "hltHIDimuonOpenOSm7to14L3Filter")
                                
  process.hionia.sglTriggerFilterNames = cms.vstring("hltHIL2Mu3N10HitQForPPRefL2Filtered",
                                                     "hltHISingleMu3NHit15L3Filtered",
                                                     "hltHIL2Mu5N10HitQL2Filtered",
                                                     "hltHISingleMu5NHit15L3Filtered",
                                                     "hltHISingleMu5NHit15L3Filtered",
                                                     "hltHISingleMu7NHit15L3Filtered",
                                                     "hltHIL2Mu15L2Filtered",
                                                     "hltHISingleMu15L3Filtered",
                                                     "hltHIL2Mu20L2Filtered",
                                                     "hltHIL3SingleMu20L3Filtered")


process.oniaSequence =  cms.Sequence(process.centralityBin*process.hiEvtPlane*process.hiEvtPlaneFlat*process.hionia)


##### Event Selection
if applyEventSel:
  if isPbPb:
    process.load('HeavyIonsAnalysis.Configuration.collisionEventSelection_cff')
    process.load('HeavyIonsAnalysis.EventAnalysis.HIClusterCompatibilityFilter_cfi')
    process.clusterCompatibilityFilter.clusterPars = cms.vdouble(0.0,0.006)
    process.oniaSequence.replace(process.hionia , process.hfCoincFilter3 * process.primaryVertexFilter * process.clusterCompatibilityFilter * process.hionia )
  else:
    process.PAprimaryVertexFilter = cms.EDFilter("VertexSelector",
                                                 src = cms.InputTag("offlinePrimaryVertices"),
                                                 cut = cms.string("!isFake && abs(z) <= 25 && position.Rho <= 2 && tracksSize >= 2"),
                                                 filter = cms.bool(True),
                                                 )
    process.NoScraping = cms.EDFilter("FilterOutScraping",
                                      applyfilter = cms.untracked.bool(True),
                                      debugOn = cms.untracked.bool(False),
                                      numtrack = cms.untracked.uint32(10),
                                      thresh = cms.untracked.double(0.25),
                                      )
    process.oniaSequence.replace(process.hionia , process.PAprimaryVertexFilter * process.NoScraping * process.hionia )



#Options:
process.source    = cms.Source("PoolSource",
                               fileNames = cms.untracked.vstring( options.inputFiles )
                               )
process.TFileService = cms.Service("TFileService", 
                                   fileName = cms.string( options.outputFile )
                                   )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.options   = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))
process.p         = cms.Path(process.oniaSequence)
