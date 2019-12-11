from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "DoubleMu_Run2017G_AOD_Run_306546_306826_OniaTree_TripleMuBc_flippedJpsi_13112019" #change lumi range
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "HiAnalysis/HiOnia/test/hioniaanalyzer_ppPrompt_trimuons_flipJpsiDir_94X_DATA_cfg.py"
config.JobType.maxMemoryMB = 2500         # request high memory machines.
config.JobType.maxJobRuntimeMin = 600

config.section_("Data")
config.Data.inputDataset = '/DoubleMuon/Run2017G-17Nov2017-v1/AOD'#'/DoubleMuon/gfalmagn-crab_DoubleMu_Run2017G_AOD_Run_306546_306826_trimuonSkim_05082019-85855fac0c6c80518ea9ff4269debc56/USER'#'/DoubleMuon/Run2017G-17Nov2017-v1/AOD'
config.Data.inputDBS = 'global'#'phys03'
config.Data.unitsPerJob = 40#3
#config.Data.totalUnits = -1
config.Data.splitting = 'LumiBased'#'FileBased'
config.Data.outLFNDirBase = '/store/user/gfalmagn/PromptAOD/%s' % (config.General.requestName)
#config.Data.outLFNDirBase = '/store/group/phys_heavyions/dileptons/Data2018/PbPb502TeV/TTrees/PromptAOD'
config.Data.publication = False
config.Data.runRange = '306546-306826'
config.Data.lumiMask = 'https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions17/5TeV/ReReco/Cert_306546-306826_5TeV_EOY2017ReReco_Collisions17_JSON_MuonPhys.txt'
#config.Data.ignoreLocality = True

config.section_('Site')
config.Site.storageSite = 'T2_FR_GRIF_LLR'
#config.Site.whitelist = ['T2_FR_*','T2_CH_*','T2_BE_*']

