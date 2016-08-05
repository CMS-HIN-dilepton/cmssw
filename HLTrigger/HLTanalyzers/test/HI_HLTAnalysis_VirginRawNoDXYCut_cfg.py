import FWCore.ParameterSet.Config as cms

##################################################################

# useful options
isData=1 # =1 running on real data, =0 running on MC


OUTPUT_HIST='openhlt.root'
NEVTS=-1
MENU="HIon" # LUMI8e29 or LUMI1e31 for pre-38X MC, or GRun for data
isRelval=1 # =1 for running on MC RelVals, =0 for standard production MC, no effect for data 

WhichHLTProcess="reHLT"
isRaw=1

#####  Global Tag ###############################################
    
# Which AlCa condition for what. Available from pre11
# * DESIGN_31X_V1 - no smearing, alignment and calibration constants = 1.  No bad channels.
# * MC_31X_V1 (was IDEAL_31X) - conditions intended for 31X physics MC production: no smearing,
#   alignment and calibration constants = 1.  Bad channels are masked.
# * STARTUP_31X_V1 (was STARTUP_31X) - conditions needed for HLT 8E29 menu studies: As MC_31X_V1 (including bad channels),
#   but with alignment and calibration constants smeared according to knowledge from CRAFT.
# * CRAFT08_31X_V1 (was CRAFT_31X) - conditions for CRAFT08 reprocessing.
# * CRAFT_31X_V1P, CRAFT_31X_V1H - initial conditions for 2009 cosmic data taking - as CRAFT08_31X_V1 but with different
#   tag names to allow append IOV, and DT cabling map corresponding to 2009 configuration (10 FEDs).
# Meanwhile...:

GLOBAL_TAG='75X_dataRun2_HLTHI_v4' # collisions2010 tag for CMSSW_3_8_X
    
    
##################################################################

process = cms.Process("ANALYSIS")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(False)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring( 'root://cms-xrd-global.cern.ch//store/user/anstahll/HLTStudy/reHLT_HITrackerVirginRaw_RAW_VIRGINRAW_GLBUNPACKER_160604/HITrackerVirginRaw/reHLT_HITrackerVirginRaw_RAW_VIRGINRAW_GLBUNPACKER_160604/160605_164527/0000/step2_HiHLT_new_101.root'
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( -1 ),
    skipBadFiles = cms.bool(True)
    )

if(isRaw):
    from RecoLuminosity.LumiProducer.lumiProducer_cff import *
    process.load('RecoLuminosity.LumiProducer.lumiProducer_cff')

process.load("HLTrigger.HLTanalyzers.HLT_HIon_ORIGINAL_NODXYCUTS_cff")

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = GLOBAL_TAG

process.load('Configuration/StandardSequences/SimL1Emulator_cff')

# OpenHLT specificss
# Define the HLT reco paths
process.load("HLTrigger.HLTanalyzers.HI_HLTopen_ORIGINAL_NODXYCUTS_cff")

# Remove the PrescaleService which, in 31X, it is expected once HLT_XXX_cff is imported
# del process.PrescaleService ## ccla no longer needed in for releases in 33x+?

process.DQM = cms.Service( "DQM",)
process.DQMStore = cms.Service( "DQMStore",)

# AlCa OpenHLT specific settings

# Define the analyzer modules
process.load("HLTrigger.HLTanalyzers.HI_HLTAnalyser_cff")
process.analyzeThis = cms.Path( process.HLTBeginSequence 
    * process.hltanalysis
    )

##LUMI CODE
if (isRaw):
    process.analyzeThis = cms.Path(process.lumiProducer + process.HLTBeginSequence + process.hltanalysis )   
else:
    process.analyzeThis = cms.Path( process.HLTBeginSequence + process.hltanalysis )


process.hltanalysis.RunParameters.HistogramFile = cms.untracked.string(OUTPUT_HIST)
process.hltanalysis.xSection=1.0
process.hltanalysis.filterEff=1.0
process.hltanalysis.l1GtReadoutRecord = cms.InputTag( 'hltGtDigis','',WhichHLTProcess ) 
process.hltanalysis.l1GtObjectMapRecord = cms.InputTag( 'hltL1GtObjectMap','',WhichHLTProcess )
process.hltanalysis.hltresults = cms.InputTag( 'TriggerResults','',WhichHLTProcess)
process.hltanalysis.HLTProcessName = cms.string(WhichHLTProcess)
process.hltTrigReport.HLTriggerResults = cms.InputTag( 'TriggerResults', '', WhichHLTProcess )


process.hltanalysis.muonFilters = cms.VInputTag( 
    cms.InputTag("hltL1sL1SingleMu3MinBiasHF1AND",""),     # 0
    cms.InputTag("hltL1sL1SingleMu3HFTower0",""),          # 1
    cms.InputTag("hltHIL1SingleMu3MinBiasFiltered",""),    # 2
    cms.InputTag("hltHIL2Mu3N10HitQ2HF0L2Filtered",""),    # 3
    cms.InputTag("hltHIL2Mu3N10HitQ2HFL2Filtered",""),     # 4
    cms.InputTag("hltHISingleMu3NHit152HFL3Filtered",""),  # 5
    cms.InputTag("hltHISingleMu3NHit152HF0L3Filtered",""), # 6
    cms.InputTag("hltL1sL1SingleMu5MinBiasHF1AND",""),     # 7
    cms.InputTag("hltL1sL1SingleMu5HFTower0",""),          # 8
    cms.InputTag("hltHIL1SingleMu5MinBiasFiltered",""),    # 9
    cms.InputTag("hltHIL1SingleMu5HFTower0Filtered",""),   # 10
    cms.InputTag("hltHIL2Mu5N10HitQ2HF0L2Filtered",""),    # 11
    cms.InputTag("hltHIL2Mu5N10HitQ2HFL2Filtered",""),     # 12
    cms.InputTag("hltHISingleMu5NHit152HFL3Filtered",""),  # 13
    cms.InputTag("hltHISingleMu5NHit152HF0L3Filtered",""), # 14
    cms.InputTag("hltL1sL1SingleMu7HFTower0",""),          # 15
    cms.InputTag("hltHIL1SingleMu7MinBiasFiltered",""),    # 16
    cms.InputTag("hltHIL1SingleMu7HFTower0Filtered",""),   # 17
    cms.InputTag("hltHIL2Mu7N10HitQ2HF0L2Filtered",""),    # 18
    cms.InputTag("hltHIL2Mu7N10HitQ2HFL2Filtered",""),     # 19
    cms.InputTag("hltHISingleMu7NHit152HFL3Filtered",""),  # 20
    cms.InputTag("hltHISingleMu7NHit152HF0L3Filtered",""), # 21
    cms.InputTag("hltL1sL1SingleMu12BptxAND",""),          # 22
    cms.InputTag("hltL1sL1SingleMu12MinBiasHF1AND",""),    # 23
    cms.InputTag("hltL1sL1SingleMu12HFTower0",""),         # 24
    cms.InputTag("hltHIL1SingleMu12Filtered",""),          # 25
    cms.InputTag("hltHIL1SingleMu12MinBiasFiltered",""),   # 26
    cms.InputTag("hltHIL1SingleMu12HFTower0Filtered",""),  # 27
    cms.InputTag("hltHIL2Mu15L2Filtered",""),              # 28
    cms.InputTag("hltHIL2Mu152HFL2Filtered",""),           # 29
    cms.InputTag("hltHIL2Mu152HF0L2Filtered",""),          # 30
    cms.InputTag("hltHIL3Mu15L2Filtered",""),              # 31
    cms.InputTag("hltHIL3Mu152HFL2Filtered",""),           # 32
    cms.InputTag("hltHIL3Mu152HF0L2Filtered",""),          # 33
    cms.InputTag("hltHIL3SingleMu15L3Filtered",""),        # 34
    cms.InputTag("hltHISingleMu152HFL3Filtered",""),       # 35
    cms.InputTag("hltHISingleMu152HF0L3Filtered",""),      # 36
    cms.InputTag("hltL1sL1SingleMu16BptxAND",""),          # 37
    cms.InputTag("hltL1sL1SingleMu16MinBiasHF1AND",""),    # 38
    cms.InputTag("hltL1sL1SingleMu16HFTower0",""),         # 39
    cms.InputTag("hltHIL1SingleMu16Filtered",""),          # 40
    cms.InputTag("hltHIL1SingleMu16MinBiasFiltered",""),   # 41
    cms.InputTag("hltHIL1SingleMu16HFTower0Filtered",""),  # 42
    cms.InputTag("hltHIL2Mu20L2Filtered",""),              # 43
    cms.InputTag("hltHIL2Mu202HFL2Filtered",""),           # 44
    cms.InputTag("hltHIL2Mu202HF0L2Filtered",""),          # 45
    cms.InputTag("hltHIL3Mu20L2Filtered",""),              # 46
    cms.InputTag("hltHIL3Mu202HFL2Filtered",""),           # 47
    cms.InputTag("hltHIL3Mu202HF0L2Filtered",""),          # 48
    cms.InputTag("hltHIL3SingleMu20L3Filtered",""),        # 49
    cms.InputTag("hltHISingleMu202HFL3Filtered",""),       # 50
    cms.InputTag("hltHISingleMu202HF0L3Filtered","")       # 51
    );

process.hltanalysis.muon = cms.InputTag("muons")
process.hltanalysis.l1extramu = cms.string("hltL1extraParticles")
process.hltanalysis.MuCandTag2 = cms.InputTag("hltL2MuonCandidates")
process.hltanalysis.MuCandTag3 = cms.InputTag("hltHIL3MuonCandidates")
process.hltanalysis.L3TkTracksFromL2OIStateTag = cms.InputTag("hltHIL3TkTracksFromL2OIState")
process.hltanalysis.L3TkTracksFromL2OIHitTag = cms.InputTag("hltHIL3TkTracksFromL2OIHit")
process.hltanalysis.OfflinePrimaryVertices0 = cms.InputTag("hiSelectedVertex") # For pp use  cms.InputTag('offlinePrimaryVertices')
process.hltanalysis.PrimaryVertices = cms.InputTag("hltHISelectedVertex") # For pp use  cms.InputTag('hltPixelVertices')


# TFile service output
process.TFileService = cms.Service('TFileService',
    fileName = cms.string("hltana.root")
    )


# Schedule the whole thing
if (MENU == "HIon"):
    print "menu HIon Virgin Raw Original Unpacker with no DXY cuts"
    process.schedule = cms.Schedule(
        process.DoHLTHIMuon,
        process.analyzeThis
        )


# to run the emulator on the output of the unpacker (which we run as part of HLTBeginSequence, independant of the emulator per se)
process.load('L1Trigger.GlobalCaloTrigger.gctDigis_cfi')
process.gctDigis.writeInternalData = cms.bool(True)
process.gctDigis.inputLabel = cms.InputTag("hltGctDigis",'',WhichHLTProcess)


# Automatic addition of the customisation function from Configuration.DataProcessing.RecoTLR
from Configuration.DataProcessing.RecoTLR import customiseRun2CommonHI 
#call to customisation function customiseRun2CommonHI imported from Configuration.DataProcessing.RecoTLR
process = customiseRun2CommonHI(process)
    
