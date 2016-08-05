from CRABClient.UserUtilities import config, getUsernameFromSiteDB
from WMCore.DataStructs.LumiList import LumiList

config = config()

config.section_('General')
config.General.requestName = 'HLTanalyzer_HITrackerVirginRaw_VIRGINRAW_ORIGINAL_160612'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'HI_HLTAnalysis_VirginRawORIGINAL_cfg.py'
config.JobType.outputFiles = ['openhlt.root']
config.JobType.maxMemoryMB = 2500

config.section_('Data')
config.Data.inputDataset ='/HITrackerVirginRaw/HIRun2015-v1/RAW'
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 10
config.Data.splitting = 'LumiBased'
config.Data.lumiMask = '/afs/cern.ch/user/a/anstahll/work/OfficialMC/TRY2/UNPACKERRAW10/CMSSW_7_5_8_patch1/src/FRANCE_lumySummary.json'

config.Data.publication = False
config.Data.outputDatasetTag = 'HLTanalyzer_HITrackerVirginRaw_VIRGINRAW_ORIGINAL_160612'
config.Data.outLFNDirBase = '/store/user/%s/HLTStudy/%s' % (getUsernameFromSiteDB(), config.Data.outputDatasetTag)

config.section_('Site')
config.Site.storageSite = 'T2_FR_GRIF_LLR'
