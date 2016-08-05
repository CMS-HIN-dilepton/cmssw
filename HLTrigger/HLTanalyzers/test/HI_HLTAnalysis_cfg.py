import FWCore.ParameterSet.Config as cms

##################################################################

# useful options
isData=1 # =1 running on real data, =0 running on MC


OUTPUT_HIST='openhlt.root'
NEVTS=-1
MENU="HIon" # LUMI8e29 or LUMI1e31 for pre-38X MC, or GRun for data
isRelval=1 # =1 for running on MC RelVals, =0 for standard production MC, no effect for data 

WhichHLTProcess="HLT"
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

GLOBAL_TAG='75X_dataRun2_PromptHI_v3' # collisions2010 tag for CMSSW_3_8_X
    
    
##################################################################

process = cms.Process("ANALYSIS")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
    'file:/home/llr/cms/stahl/RERECO/HIOniaTnP_RAW.root'
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( 100 ),
    skipBadFiles = cms.bool(True)
    )

if(isRaw):
    from RecoLuminosity.LumiProducer.lumiProducer_cff import *
    process.load('RecoLuminosity.LumiProducer.lumiProducer_cff')

process.load("HLTrigger.HLTanalyzers.HLT_HIon_cff")

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = GLOBAL_TAG

process.load('Configuration/StandardSequences/SimL1Emulator_cff')

# OpenHLT specificss
# Define the HLT reco paths
process.load("HLTrigger.HLTanalyzers.HI_HLTopen_cff")

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
process.hltanalysis.l1GtReadoutRecord = cms.InputTag( 'hltGtDigis','',process.name_() ) # get gtDigis extract from the RAW
process.hltanalysis.hltresults = cms.InputTag( 'TriggerResults','',WhichHLTProcess)
process.hltanalysis.HLTProcessName = cms.string(WhichHLTProcess)
process.hltanalysis.muon = cms.InputTag("muons")
process.hltanalysis.l1extramu = cms.string("hltL1extraParticles")
process.hltanalysis.MuCandTag2 = cms.InputTag("hltL2MuonCandidates")
process.hltanalysis.MuCandTag3 = cms.InputTag("hltHIL3MuonCandidates")
process.hltanalysis.OfflinePrimaryVertices0 = cms.InputTag("hiSelectedVertex") # For pp use  cms.InputTag('offlinePrimaryVertices')
process.hltanalysis.PrimaryVertices = cms.InputTag("hltHISelectedVertex") # For pp use  cms.InputTag('hltPixelVertices')

# TFile service output
process.TFileService = cms.Service('TFileService',
    fileName = cms.string("hltana.root")
    )

# Schedule the whole thing
if (MENU == "HIon"):
    print "menu HIon"
    process.schedule = cms.Schedule(
        process.DoHLTHIMuon, 
	process.analyzeThis)


# to run the emulator on the output of the unpacker (which we run as part of HLTBeginSequence, independant of the emulator per se)
process.load('L1Trigger.GlobalCaloTrigger.gctDigis_cfi')
process.gctDigis.writeInternalData = cms.bool(True)
process.gctDigis.inputLabel = cms.InputTag("hltGctDigis")
                                                                                                            
        
#########################################################################################
from FWCore.ParameterSet import Mixins
for module in process.__dict__.itervalues():
    if isinstance(module, Mixins._Parameterizable):
        for parameter in module.__dict__.itervalues():
            if isinstance(parameter, cms.InputTag):
                if parameter.moduleLabel == 'rawDataCollector':
                    parameter.moduleLabel = 'source'



# Automatic addition of the customisation function from Configuration.DataProcessing.RecoTLR
from Configuration.DataProcessing.RecoTLR import customiseRun2CommonHI 
#call to customisation function customiseRun2CommonHI imported from Configuration.DataProcessing.RecoTLR
process = customiseRun2CommonHI(process)
