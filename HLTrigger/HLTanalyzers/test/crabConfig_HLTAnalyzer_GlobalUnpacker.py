from CRABClient.UserUtilities import config, getUsernameFromSiteDB
from WMCore.DataStructs.LumiList import LumiList

config = config()

config.section_('General')
config.General.requestName = 'HLTanalyzer_HITrackerVirginRaw_VIRGINRAW_GLBUNPACKER_160612'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'HI_HLTAnalysis_GlobalUnpacker_cfg.py'
config.JobType.outputFiles = ['openhlt.root']
config.JobType.maxMemoryMB = 2500

config.section_('Data')
config.Data.inputDataset ='/HITrackerVirginRaw/anstahll-reHLT_HITrackerVirginRaw_RAW_VIRGINRAW_GLBUNPACKER_160604-ec64cbdf8365c8a84afc5ccf73a626cc/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 10
config.Data.splitting = 'FileBased'

config.Data.publication = False
config.Data.outputDatasetTag = 'HLTanalyzer_HITrackerVirginRaw_VIRGINRAW_GLBUNPACKER_160612'
config.Data.outLFNDirBase = '/store/user/%s/HLTStudy/%s' % (getUsernameFromSiteDB(), config.Data.outputDatasetTag)

config.section_('Site')
config.Site.storageSite = 'T2_FR_GRIF_LLR'
