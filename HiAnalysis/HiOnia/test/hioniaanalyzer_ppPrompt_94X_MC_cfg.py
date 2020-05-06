import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing


#----------------------------------------------------------------------------

# Setup Settings for ONIA TREE: PbPb 2018

HLTProcess     = "HLT" # Name of HLT process 
isMC           = True # if input is MONTECARLO: True or if it's DATA: False
muonSelection  = "GlbOrTrk" # Single muon selection: Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker), TwoGlbAmongThree (which requires two isGlobal for a trimuon, and one isGlobal for a dimuon) are available
applyEventSel  = True # Only apply Event Selection if the required collections are present
OnlySoftMuons  = True # Keep only isSoftMuon's (with highPurity because this is pp config) from the beginning of HiSkim. In any case, if applyCuts=True, isSoftMuon is required at HiAnalysis level for muons of selected dimuons.
applyCuts      = False # At HiAnalysis level, apply kinematic acceptance cuts + identification cuts (isSoftMuon or isTightMuon, depending on TightGlobalMuon flag) for muons from selected di(tri)muons + hard-coded cuts on the di(tri)muon that you would want to add (but recommended to add everything in LateDimuonSelection, applied at the end of HiSkim)
SofterSgMuAcceptance = True # Whether to accept muons with a softer acceptance cuts than the usual (pt>3.5GeV at central eta, pt>1.8 at high |eta|). Applies when applyCuts=True
doTrimuons     = False # Make collections of trimuon candidates in addition to dimuons, and keep only events with >0 trimuons
doDimuonTrk    = False # Make collections of Jpsi+track candidates in addition to dimuons
atLeastOneCand = False # Keep only events that have one selected dimuon (or at least one trimuon if doTrimuons = true). BEWARE this can cause trouble in .root output if no event is selected by onia2MuMuPatGlbGlbFilter!
OneMatchedHLTMu = -1 # Keep only di(tri)muons of which the one(two) muon(s) are matched to the HLT Filter of this number. You can get the desired number in the output of oniaTree. Set to -1 for no matching.
muonLessPV     = True  # Recalculate the PV without the two muons from the selected dimuon
#############################################################################
keepExtraColl  = False # General Tracks + Stand Alone Muons + Converted Photon collections
useSVfinder    = False # External SV finder to check if the muons are from a resolved SV
#saveHLTBit     = False # for trigger analysis
#saveHLTobj     = False # For trigger analysis
#----------------------------------------------------------------------------

# Print Onia Tree settings:
print( " " )
print( "[INFO] Settings used for ONIA TREE: " )
print( "[INFO] isMC                 = " + ("True" if isMC else "False") )
print( "[INFO] applyEventSel        = " + ("True" if applyEventSel else "False") )
print( "[INFO] keepExtraColl        = " + ("True" if keepExtraColl else "False") )
print( "[INFO] SofterSgMuAcceptance = " + ("True" if SofterSgMuAcceptance else "False") )
print( "[INFO] muonSelection        = " + muonSelection )
print( "[INFO] onlySoftMuons        = " + ("True" if OnlySoftMuons else "False") )
print( "[INFO] doTrimuons           = " + ("True" if doTrimuons else "False") )
print( " " )

# set up process
process = cms.Process("HIOnia")

# setup 'analysis'  options
options = VarParsing.VarParsing ('analysis')

# Input and Output File Names
options.outputFile = "Oniatree_MC.root"
options.secondaryOutputFile = "Jpsi_DataSet.root"
options.inputFiles =[#'file:/home/llr/cms/falmagne/production/pp2017/NonPromptJpsi/genMC/CMSSW_9_4_11/src/HIN-RunIIpp5Spring18DR-00076_MinBias_onlyJpsi.root'
    #'/store/himc/RunIIpp5Spring18DR/BJPsiMM_TuneCUETP8M1_5p02TeV_pythia8/AODSIM/94X_mc2017_realistic_forppRef5TeV_v1_ext1-v1/30000/FE62BDED-09A4-E911-97B3-1418774124DE.root',
    #'/store/user/gfalmagn/Bc_analysis/MC/BcToJpsiMuNu_BCVEGPY_PYTHIA8_pp5TeV_22012019_3_reco_NoCuts/BcToJpsiMuNu/BcToJpsiMuNu_BCVEGPY_PYTHIA8_pp5TeV_22012019_3_reco_NoCuts/190125_103233/0000/Bc_reconstructed_15.root'
    '/store/himc/RunIIpp5Spring18DR/JPsiMM_TuneCUETP8M1_5p02TeV_pythia8/AODSIM/94X_mc2017_realistic_forppRef5TeV-v2/60000/FED7EDEF-CE46-E911-B2DB-782BCB1CFD1A.root'
    ]
options.maxEvents = -1 # -1 means all events

# Get and parse the command line arguments
options.parseArguments()

#User can change the default trigger list here
triggerList    = {
    # Double Muon Trigger List
    'DoubleMuonTrigger' : cms.vstring(
        "HLT_HIL1DoubleMuOpen_v1",
        "HLT_HIL1DoubleMuOpen_OS_v1",
        "HLT_HIL1DoubleMuOpen_SS_v1",
        "HLT_HIL1DoubleMu0_v1",
        "HLT_HIL1DoubleMu0_HighQ_v1",
        "HLT_HIL1DoubleMu10_v1",
        "HLT_HIL2DoubleMu0_v1",
        "HLT_HIL2DoubleMu10_v1",
        "HLT_HIL3DoubleMu0_v1",
        "HLT_HIL3DoubleMu10_v1",
        ),
    # Double Muon Filter List
    'DoubleMuonFilter'  : cms.vstring(
        "hltL1fL1sDoubleMuOpenL1Filtered0",
        "hltL1fL1sDoubleMuOpenOSL1Filtered0",
        "hltL1fL1sDoubleMuOpenSSL1Filtered0",
        "hltL1fL1sDoubleMu0L1Filtered0",
        "hltL1fL1sDoubleMu0L1HighQFiltered0",
        "hltL1fL1sDoubleMu10L1Filtered0",
        "hltL2fL1sDoubleMu0L1f0L2Filtered0",
        "hltL2fL1sDoubleMu10L1f0L2Filtered10",
        "hltL3fL1sDoubleMu0L1f0L2f0L3Filtered0",
        "hltL3fL1sDoubleMu10L1f0L2f0L3Filtered10",
        ),
    # Single Muon Trigger List
    'SingleMuonTrigger' : cms.vstring(
        "HLT_HIL1Mu12_v1",
        "HLT_HIL1Mu16_v1",
        "HLT_HIL2Mu7_v1",
        "HLT_HIL2Mu12_v1",
        "HLT_HIL2Mu15_v1",
        "HLT_HIL2Mu20_v1",
        "HLT_HIL3Mu3_v1",
        "HLT_HIL3Mu5_v1",
        "HLT_HIL3Mu7_v1",
        "HLT_HIL3Mu12_v1",
        "HLT_HIL3Mu15_v1",
        "HLT_HIL3Mu20_v1",
        "HLT_HIL2Mu3_NHitQ10_v1",
        "HLT_HIL3Mu3_NHitQ10_v1",
        "HLT_HIL2Mu5_NHitQ10_v1",
        "HLT_HIL3Mu5_NHitQ10_v1",
        ),
    # Single Muon Filter List
    'SingleMuonFilter'  : cms.vstring(
        "hltL1fL1sSingleMu12L1Filtered0",
        "hltL1fL1sSingleMu16L1Filtered0",
        "hltL2fL1sSingleMu3OR5L1f0L2Filtered7",
        "hltL2fL1sSingleMu7L1f0L2Filtered12",
        "hltL2fL1sSingleMu7L1f0L2Filtered15",
        "hltL2fL1sSingleMu7L1f0L2Filtered20",
        "hltL3fL1sSingleMu3L1f0L2f0L3Filtered3",
        "hltL3fL1sSingleMu3OR5L1f0L2f0L3Filtered5",
        "hltL3fL1sSingleMu3OR5L1f0L2f0L3Filtered7",
        "hltL3fL1sSingleMu7L1f0L2f0L3Filtered12",
        "hltL3fL1sSingleMu7L1f0L2f0L3Filtered15",
        "hltL3fL1sSingleMu7L1f0L2f0L3Filtered20",
        "hltL2fL1sSingleMu3L1f0L2NHitQ10L2Filtered3",
        "hltL3fL1sSingleMu3L1f0L2f0L3NHitQ10L3Filtered3",
        "hltL2fL1sSingleMu3OR5L1f0L2NHitQ10L2Filtered5",
        "hltL3fL1sSingleMu3OR5L1f0L2f0L3NHitQ10L3Filtered5",
        )
}

if isMC:
    globalTag = '92X_upgrade2017_realistic_v11'
else:
    globalTag = '94X_dataRun2_ReReco_EOY17_v6'

# load the Geometry and Magnetic Field
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load('Configuration.StandardSequences.Reconstruction_cff')
    
# Global Tag:
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')
    
#----------------------------------------------------------------------------

# For OniaTree Analyzer
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=triggerList, HLTProName=HLTProcess, #useL1Stage2=True, 
                 muonSelection=muonSelection, isMC=isMC, outputFileName=options.outputFile, muonlessPV=muonLessPV, doTrimu=doTrimuons, doDimuTrk=doDimuonTrk)
process.oniaTreeAna = cms.Path(process.oniaTreeAna)

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("2.4 < mass && mass < 3.7 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection  = cms.string("(isGlobalMuon || isTrackerMuon) && abs(innerTrack.dxy)<4 && abs(innerTrack.dz)<25 && abs(eta) < 2.4 && ((abs(eta) < 1. && pt >= 3.3) || (1. <= abs(eta) && abs(eta) < 2. && p >= 2.9) || (2. <= abs(eta) && pt >= 0.8))")#tracker muon acceptance
#process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("") ## No need to repeat lowerPuritySelection in there, already included
if applyCuts:
    process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.01") 
process.onia2MuMuPatGlbGlb.onlySoftMuons         = cms.bool(OnlySoftMuons)

process.hionia.minimumFlag      = cms.bool(keepExtraColl)           #for Reco_trk_*
process.hionia.useGeTracks      = cms.untracked.bool(keepExtraColl) #for Reco_trk_*
process.hionia.fillRecoTracks   = cms.bool(keepExtraColl)           #for Reco_trk_*
process.hionia.srcTracks        = cms.InputTag("generalTracks")
process.hionia.primaryVertexTag = cms.InputTag("offlinePrimaryVertices")
process.hionia.genParticles     = cms.InputTag("genParticles")
process.hionia.SofterSgMuAcceptance = cms.bool(SofterSgMuAcceptance)
process.hionia.applyCuts        = cms.bool(applyCuts)
process.hionia.AtLeastOneCand   = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu  = cms.int32(OneMatchedHLTMu)
process.hionia.useSVfinder      = cms.bool(useSVfinder)
process.hionia.genealogyInfo    = cms.bool(True)
process.hionia.isHI             = cms.untracked.bool(False)

process.NoScraping = cms.EDFilter("FilterOutScraping",
                          applyfilter = cms.untracked.bool(True),
                          debugOn = cms.untracked.bool(False),
                          numtrack = cms.untracked.uint32(10),
                          thresh = cms.untracked.double(0.25)
                          )
process.primaryVertexFilter = cms.EDFilter("VertexSelector",
                                   src = cms.InputTag("offlinePrimaryVertices"),
                                   cut = cms.string("!isFake && abs(z) <= 25 && position.Rho <= 2 && tracksSize >= 2"), 
                                   filter = cms.bool(True),   # otherwise it won't filter the events
                                   )

if applyEventSel:
    process.oniaTreeAna.replace(process.hionia, process.primaryVertexFilter * process.NoScraping * process.hionia )

if atLeastOneCand:
  if doTrimuons:
      process.oniaTreeAna.replace(process.onia2MuMuPatGlbGlb, process.onia2MuMuPatGlbGlb * process.onia2MuMuPatGlbGlbFilterTrimu)
      process.oniaTreeAna.replace(process.patMuonSequence, process.filter3mu * process.pseudoDimuonFilterSequence * process.patMuonSequence)
  elif doDimuonTrk:
      process.oniaTreeAna.replace(process.onia2MuMuPatGlbGlb, process.onia2MuMuPatGlbGlb * process.onia2MuMuPatGlbGlbFilterDimutrk)
      process.oniaTreeAna.replace(process.patMuonSequence, process.pseudoDimuonFilterSequence * process.patMuonSequence)
  else:
      process.oniaTreeAna.replace(process.onia2MuMuPatGlbGlb, process.onia2MuMuPatGlbGlb * process.onia2MuMuPatGlbGlbFilter)
      #BEWARE, pseudoDimuonFilterSequence asks for opposite-sign dimuon in given mass range. But saves a lot of time by filtering before running PAT muons
      process.oniaTreeAna.replace(process.patMuonSequence, process.pseudoDimuonFilterSequence * process.patMuonSequence)

#----------------------------------------------------------------------------
'''
# For HLTBitAnalyzer
process.load("HLTrigger.HLTanalyzers.HLTBitAnalyser_cfi")
process.hltbitanalysis.HLTProcessName              = HLTProcess
process.hltbitanalysis.hltresults                  = cms.InputTag("TriggerResults","",HLTProcess)
process.hltbitanalysis.l1tAlgBlkInputTag           = cms.InputTag("hltGtStage2Digis","",HLTProcess)
process.hltbitanalysis.l1tExtBlkInputTag           = cms.InputTag("hltGtStage2Digis","",HLTProcess)
process.hltbitanalysis.gObjectMapRecord            = cms.InputTag("hltGtStage2ObjectMap","",HLTProcess)
process.hltbitanalysis.gmtStage2Digis              = cms.string("hltGtStage2Digis")
process.hltbitanalysis.caloStage2Digis             = cms.string("hltGtStage2Digis")
process.hltbitanalysis.UseL1Stage2                 = cms.untracked.bool(True)
process.hltbitanalysis.getPrescales                = cms.untracked.bool(False)
process.hltbitanalysis.getL1InfoFromEventSetup     = cms.untracked.bool(False)
process.hltbitanalysis.UseTFileService             = cms.untracked.bool(True)
process.hltbitanalysis.RunParameters.HistogramFile = cms.untracked.string(options.outputFile)
process.hltbitanalysis.RunParameters.isData        = cms.untracked.bool(not isMC)
process.hltbitanalysis.RunParameters.Monte         = cms.bool(isMC)
process.hltbitanalysis.RunParameters.GenTracks     = cms.bool(False)
if (HLTProcess == "HLT") :
	process.hltbitanalysis.l1tAlgBlkInputTag = cms.InputTag("gtStage2Digis","","RECO")
	process.hltbitanalysis.l1tExtBlkInputTag = cms.InputTag("gtStage2Digis","","RECO")
	process.hltbitanalysis.gmtStage2Digis    = cms.string("gtStage2Digis")
	process.hltbitanalysis.caloStage2Digis   = cms.string("gtStage2Digis")
        if saveHLTBit:
          process.hltBitAna = cms.EndPath(process.hltbitanalysis)
#
##----------------------------------------------------------------------------
#
# For HLTObject Analyzer
process.load("HeavyIonsAnalysis.EventAnalysis.hltobject_cfi")
process.hltobject.processName = cms.string(HLTProcess)
process.hltobject.treeName = cms.string(options.outputFile)
process.hltobject.loadTriggersFromHLT = cms.untracked.bool(False)
process.hltobject.triggerNames = triggerList['DoubleMuonTrigger'] + triggerList['SingleMuonTrigger']
process.hltobject.triggerResults = cms.InputTag("TriggerResults","",HLTProcess)
process.hltobject.triggerEvent   = cms.InputTag("hltTriggerSummaryAOD","",HLTProcess)
if saveHLTobj:
  process.hltObjectAna = cms.EndPath(process.hltobject)
'''

#----------------------------------------------------------------------------
#Options:
process.source = cms.Source("PoolSource",
#process.source = cms.Source("NewEventStreamFileReader", # for streamer data
		fileNames = cms.untracked.vstring( options.inputFiles ),
		)
process.TFileService = cms.Service("TFileService", 
		fileName = cms.string( options.outputFile )
		)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.options   = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

#if saveHLTobj or saveHLTBit:
#    process.schedule  = cms.Schedule( process.oniaTreeAna , process.hltBitAna , process.hltObjectAna )
#else:
process.schedule  = cms.Schedule( process.oniaTreeAna )
