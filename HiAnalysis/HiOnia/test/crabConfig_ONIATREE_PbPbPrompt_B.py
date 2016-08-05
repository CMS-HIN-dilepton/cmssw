from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'HIOniaL1DoubleMu0B_HIRun2015-PromptReco-v1_GT_75X_dataRun2v12_ONIATREE_20160420_2015CUT'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'hioniaanalyzer_PbPbPrompt_B_cfg.py'
config.JobType.maxMemoryMB = 2500
config.JobType.outputFiles = ['OniaTree.root']

config.section_('Data')
config.Data.inputDataset = '/HIOniaL1DoubleMu0B/anstahll-HIOniaL1DoubleMu0B_HIRun2015-PromptReco-v1_Run_262548_263757_GT_75X_dataRun2_v12_ONIASKIM_v1-9bce861cf85679a873b858c6fe030a31/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 20
config.Data.splitting = 'LumiBased'
config.Data.runRange = '262548-263757'
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/HI/Cert_262548-263757_PromptReco_HICollisions15_JSON_MuonPhys_v2.txt'
config.Data.outLFNDirBase = '/store/user/%s/HIPromptReco/TTrees/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = False

config.section_('Site')
config.Site.whitelist = ["T2_FR_GRIF_LLR"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

# If your site is blacklisted by crab, use:
#config.Data.ignoreLocality = True
#config.Site.whitelist = ["T1_FR*","T2_FR*","T3_FR*"]
